/*
-----------------------------------------------------------------------
Copyright 2012 iMinds-Vision Lab, University of Antwerp

Contact: astra@ua.ac.be
Website: http://astra.ua.ac.be


This file is part of the
All Scale Tomographic Reconstruction Antwerp Toolbox ("ASTRA Toolbox").

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
$Id$
*/

#ifndef _INC_ASTRA_CUDABACKPROJECTIONALGORITHM3D
#define _INC_ASTRA_CUDABACKPROJECTIONALGORITHM3D

#include "Globals.h"
#include "Config.h"

#include "Algorithm.h"

#include "Float32ProjectionData3DMemory.h"
#include "Float32VolumeData3DMemory.h"
#include "ReconstructionAlgorithm3D.h"

#ifdef ASTRA_CUDA

namespace astra {

class _AstraExport CCudaBackProjectionAlgorithm3D : public CReconstructionAlgorithm3D {

protected:

	/** Check the values of this object.  If everything is ok, the object can be set to the initialized state.
	 * The following statements are then guaranteed to hold:
	 * - no NULL pointers
	 * - all sub-objects are initialized properly
	 * - the projector is compatible with both data objects
	 */
	virtual bool _check();

public:
	
	// type of the algorithm, needed to register with CAlgorithmFactory
	static std::string type;
	
	/** Default constructor, does not initialize the object.
	 */
	CCudaBackProjectionAlgorithm3D();

	/** Constructor with initialization.
	 *
	 * @param _pProjector		Projector Object.
	 * @param _pProjectionData	ProjectionData3D object containing the projection data.
	 * @param _pReconstruction	VolumeData3D object for storing the reconstructed volume.
	 */
	CCudaBackProjectionAlgorithm3D(CProjector3D* _pProjector, 
	                     CFloat32ProjectionData3DMemory* _pProjectionData, 
	                     CFloat32VolumeData3DMemory* _pReconstruction);
	
	/** Copy constructor.
	 */
	CCudaBackProjectionAlgorithm3D(const CCudaBackProjectionAlgorithm3D&);

	/** Destructor.
	 */
	virtual ~CCudaBackProjectionAlgorithm3D();

	/** Clear this class.
	 */
/*	virtual void clear();*/

	/** Initialize the algorithm with a config object.
	 *
	 * @param _cfg Configuration Object
	 * @return initialization successful?
	 */
	virtual bool initialize(const Config& _cfg);

	/** Initialize class.
	 *
	 * @param _pProjector		Projector Object.
	 * @param _pProjectionData	ProjectionData3D object containing the projection data.
	 * @param _pReconstruction	VolumeData3D object for storing the reconstructed volume.
	 * @return initialization successful?
	 */
	bool initialize(CProjector3D* _pProjector, 
					CFloat32ProjectionData3DMemory* _pSinogram, 
					CFloat32VolumeData3DMemory* _pReconstruction);

	/** Get all information parameters
	 *
	 * @return map with all boost::any object
	 */
	virtual map<string,boost::any> getInformation();

	/** Get a single piece of information represented as a boost::any
	 *
	 * @param _sIdentifier identifier string to specify which piece of information you want
	 * @return boost::any object
	 */
	virtual boost::any getInformation(std::string _sIdentifier);

	/** Perform a number of iterations.
	 *
	 * @param _iNrIterations amount of iterations to perform.
	 */
	virtual void run(int _iNrIterations = 0);

	/** Get a description of the class.
	 *
	 * @return description string
	 */
	virtual std::string description() const;

	/**  
	 * Sets the index of the used GPU index: first GPU has index 0
	 *
	 * @param _iGPUIndex New GPU index.
	 */
	void setGPUIndex(int _iGPUIndex) { m_iGPUIndex = _iGPUIndex; }

protected:

	int m_iGPUIndex;
	int m_iVoxelSuperSampling;

};

// inline functions
inline std::string CCudaBackProjectionAlgorithm3D::description() const { return CCudaBackProjectionAlgorithm3D::type; };

} // end namespace

#endif

#endif