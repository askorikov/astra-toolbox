/*
-----------------------------------------------------------------------
Copyright: 2010-2022, imec Vision Lab, University of Antwerp
           2014-2022, CWI, Amsterdam

Contact: astra@astra-toolbox.com
Website: http://www.astra-toolbox.com/

This file is part of the ASTRA Toolbox.


The ASTRA Toolbox is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The ASTRA Toolbox is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the ASTRA Toolbox. If not, see <http://www.gnu.org/licenses/>.

-----------------------------------------------------------------------
*/

#include "astra/CudaFDKAlgorithm3D.h"

#include "astra/AstraObjectManager.h"

#include "astra/CudaProjector3D.h"
#include "astra/ConeProjectionGeometry3D.h"
#include "astra/ConeVecProjectionGeometry3D.h"
#include "astra/CompositeGeometryManager.h"
#include "astra/VolumeGeometry3D.h"

#include "astra/Float32ProjectionData2D.h"

#include "astra/Logging.h"
#include "astra/Filters.h"

using namespace std;
using namespace astraCUDA3d;

namespace astra {

//----------------------------------------------------------------------------------------
// Constructor
CCudaFDKAlgorithm3D::CCudaFDKAlgorithm3D() 
{
	m_bIsInitialized = false;
	m_iGPUIndex = -1;
	m_iVoxelSuperSampling = 1;
}

//----------------------------------------------------------------------------------------
// Constructor with initialization
CCudaFDKAlgorithm3D::CCudaFDKAlgorithm3D(CProjector3D* _pProjector, 
								   CFloat32ProjectionData3D* _pProjectionData, 
								   CFloat32VolumeData3D* _pReconstruction)
{
	_clear();
	initialize(_pProjector, _pProjectionData, _pReconstruction);
}

//----------------------------------------------------------------------------------------
// Destructor
CCudaFDKAlgorithm3D::~CCudaFDKAlgorithm3D() 
{
	CReconstructionAlgorithm3D::_clear();
}


//---------------------------------------------------------------------------------------
// Check
bool CCudaFDKAlgorithm3D::_check()
{
	// check base class
	ASTRA_CONFIG_CHECK(CReconstructionAlgorithm3D::_check(), "CUDA_FDK", "Error in ReconstructionAlgorithm3D initialization");

	const CProjectionGeometry3D& projgeom = m_pSinogram->getGeometry();
	ASTRA_CONFIG_CHECK(dynamic_cast<const CConeProjectionGeometry3D*>(&projgeom) || dynamic_cast<const CConeVecProjectionGeometry3D*>(&projgeom), "CUDA_FDK", "Error setting FDK geometry");


	const CVolumeGeometry3D& volgeom = m_pReconstruction->getGeometry();
	bool cube = true;
	if (abs(volgeom.getPixelLengthX() / volgeom.getPixelLengthY() - 1.0) > 0.00001)
		cube = false;
	if (abs(volgeom.getPixelLengthX() / volgeom.getPixelLengthZ() - 1.0) > 0.00001)
		cube = false;
	ASTRA_CONFIG_CHECK(cube, "CUDA_FDK", "Voxels must be cubes for FDK");



	return true;
}

//---------------------------------------------------------------------------------------
void CCudaFDKAlgorithm3D::initializeFromProjector()
{
	m_iVoxelSuperSampling = 1;
	m_iGPUIndex = -1;

	CCudaProjector3D* pCudaProjector = dynamic_cast<CCudaProjector3D*>(m_pProjector);
	if (!pCudaProjector) {
		if (m_pProjector) {
			ASTRA_WARN("non-CUDA Projector3D passed to FDK_CUDA");
		}
	} else {
		m_iVoxelSuperSampling = pCudaProjector->getVoxelSuperSampling();
		m_iGPUIndex = pCudaProjector->getGPUIndex();
	}

}

//---------------------------------------------------------------------------------------
// Initialize - Config
bool CCudaFDKAlgorithm3D::initialize(const Config& _cfg)
{
	ConfigReader<CAlgorithm> CR("CudaFDKAlgorithm3D", this, _cfg);

	// if already initialized, clear first
	if (m_bIsInitialized) {
		clear();
	}

	// initialization of parent class
	if (!CReconstructionAlgorithm3D::initialize(_cfg)) {
		return false;
	}

	initializeFromProjector();

	// Deprecated options
	bool ok = true;
	ok &= CR.getOptionInt("VoxelSuperSampling", m_iVoxelSuperSampling, m_iVoxelSuperSampling);
	if (CR.hasOption("GPUIndex"))
		ok &= CR.getOptionInt("GPUIndex", m_iGPUIndex, m_iGPUIndex);
	else
		ok &= CR.getOptionInt("GPUindex", m_iGPUIndex, m_iGPUIndex);
	if (!ok)
		return false;

	if ((CR.has("FilterSinogramId") || CR.hasOption("FilterSinogramId")) &&
	    !(CR.has("FilterType") || CR.hasOption("FilterType")))
	{
		ASTRA_WARN("Setting FilterSinogramId without FilterType is no longer supported. Set FilterType to 'sinogram' for the old behaviour.");
	}
	m_filterConfig = getFilterConfigForAlgorithm(_cfg, this);

	ok &= CR.getOptionBool("ShortScan", m_bShortScan, false);
	if (!ok)
		return false;

	// success
	m_bIsInitialized = _check();
	return m_bIsInitialized;
}

//----------------------------------------------------------------------------------------
// Initialize - C++
bool CCudaFDKAlgorithm3D::initialize(CProjector3D* _pProjector, 
								  CFloat32ProjectionData3D* _pSinogram, 
								  CFloat32VolumeData3D* _pReconstruction)
{
	// if already initialized, clear first
	if (m_bIsInitialized) {
		clear();
	}

	// required classes
	m_pProjector = _pProjector;
	m_pSinogram = _pSinogram;
	m_pReconstruction = _pReconstruction;

	// success
	m_bIsInitialized = _check();
	return m_bIsInitialized;
}

//----------------------------------------------------------------------------------------
// Iterate
bool CCudaFDKAlgorithm3D::run(int _iNrIterations)
{
	// check initialized
	ASTRA_ASSERT(m_bIsInitialized);

	CFloat32ProjectionData3D* pSinoMem = dynamic_cast<CFloat32ProjectionData3D*>(m_pSinogram);
	ASTRA_ASSERT(pSinoMem);
	CFloat32VolumeData3D* pReconMem = dynamic_cast<CFloat32VolumeData3D*>(m_pReconstruction);
	ASTRA_ASSERT(pReconMem);

#if 0
	bool ok = true;
	
	ok = astraCudaFDK(pReconMem->getData(), pSinoMem->getDataConst(),
	                  &volgeom, conegeom,
	                  m_bShortScan, m_iGPUIndex, m_iVoxelSuperSampling, filter);

	ASTRA_ASSERT(ok);
#endif

	CCompositeGeometryManager cgm;

	return cgm.doFDK(m_pProjector, pReconMem, pSinoMem, m_bShortScan, m_filterConfig);
}
//----------------------------------------------------------------------------------------

} // namespace astra
