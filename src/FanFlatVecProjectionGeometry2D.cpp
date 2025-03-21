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

#include "astra/FanFlatVecProjectionGeometry2D.h"

#include "astra/XMLConfig.h"
#include "astra/Logging.h"

#include <cstring>
#include <sstream>

using namespace std;

namespace astra
{

//----------------------------------------------------------------------------------------
// Default constructor. Sets all variables to zero. 
CFanFlatVecProjectionGeometry2D::CFanFlatVecProjectionGeometry2D()
{
	_clear();
	m_pProjectionAngles = 0;
}

//----------------------------------------------------------------------------------------
// Constructor.
CFanFlatVecProjectionGeometry2D::CFanFlatVecProjectionGeometry2D(int _iProjectionAngleCount, 
                                                                 int _iDetectorCount, 
                                                                 const SFanProjection* _pProjectionAngles)
{
	this->initialize(_iProjectionAngleCount, 
	                 _iDetectorCount, 
	                 _pProjectionAngles);
}

//----------------------------------------------------------------------------------------
// Copy Constructor
CFanFlatVecProjectionGeometry2D::CFanFlatVecProjectionGeometry2D(const CFanFlatVecProjectionGeometry2D& _projGeom)
{
	_clear();
	this->initialize(_projGeom.m_iProjectionAngleCount,
	                 _projGeom.m_iDetectorCount,
	                 _projGeom.m_pProjectionAngles);
}

//----------------------------------------------------------------------------------------
// Assignment operator.
CFanFlatVecProjectionGeometry2D& CFanFlatVecProjectionGeometry2D::operator=(const CFanFlatVecProjectionGeometry2D& _other)
{
	if (m_bInitialized)
		delete[] m_pProjectionAngles;
	m_bInitialized = _other.m_bInitialized;
	if (m_bInitialized) {
		m_iProjectionAngleCount = _other.m_iProjectionAngleCount;
		m_iDetectorCount = _other.m_iDetectorCount;
		m_pProjectionAngles = new SFanProjection[m_iProjectionAngleCount];
		memcpy(m_pProjectionAngles, _other.m_pProjectionAngles, sizeof(m_pProjectionAngles[0])*m_iProjectionAngleCount);
	}
	return *this;
}
//----------------------------------------------------------------------------------------
// Destructor.
CFanFlatVecProjectionGeometry2D::~CFanFlatVecProjectionGeometry2D()
{
	// TODO
	delete[] m_pProjectionAngles;
}


//----------------------------------------------------------------------------------------
// Initialization.
bool CFanFlatVecProjectionGeometry2D::initialize(int _iProjectionAngleCount, 
											  int _iDetectorCount, 
											  const SFanProjection* _pProjectionAngles)
{
	m_iProjectionAngleCount = _iProjectionAngleCount;
	m_iDetectorCount = _iDetectorCount;
	m_pProjectionAngles = new SFanProjection[m_iProjectionAngleCount];
	for (int i = 0; i < m_iProjectionAngleCount; ++i)
		m_pProjectionAngles[i] = _pProjectionAngles[i];

	// TODO: check?

	// success
	m_bInitialized = _check();
	return m_bInitialized;
}

//----------------------------------------------------------------------------------------
// Initialization with a Config object
bool CFanFlatVecProjectionGeometry2D::initialize(const Config& _cfg)
{
	ConfigReader<CProjectionGeometry2D> CR("FanFlatVecProjectionGeometry2D", this, _cfg);	

	XMLNode node;

	// initialization of parent class
	if (!CProjectionGeometry2D::initialize(_cfg))
		return false;


	// success
	m_bInitialized = _check();
	return m_bInitialized;
}

bool CFanFlatVecProjectionGeometry2D::initializeAngles(const Config& _cfg)
{
	ConfigReader<CProjectionGeometry2D> CR("FanFlatVecProjectionGeometry2D", this, _cfg);

	// Required: Vectors
	vector<double> data;
	if (!CR.getRequiredNumericalArray("Vectors", data))
		return false;
	ASTRA_CONFIG_CHECK(data.size() % 6 == 0, "FanFlatVecProjectionGeometry2D", "Vectors doesn't consist of 6-tuples.");
	m_iProjectionAngleCount = data.size() / 6;
	m_pProjectionAngles = new SFanProjection[m_iProjectionAngleCount];

	for (int i = 0; i < m_iProjectionAngleCount; ++i) {
		SFanProjection& p = m_pProjectionAngles[i];
		p.fSrcX  = data[6*i +  0];
		p.fSrcY  = data[6*i +  1];
		p.fDetUX = data[6*i +  4];
		p.fDetUY = data[6*i +  5];

		// The backend code currently expects the corner of the detector, while
		// the matlab interface supplies the center
		p.fDetSX = data[6*i +  2] - 0.5 * m_iDetectorCount * p.fDetUX;
		p.fDetSY = data[6*i +  3] - 0.5 * m_iDetectorCount * p.fDetUY;
	}

	return true;
}

//----------------------------------------------------------------------------------------
// Clone
CProjectionGeometry2D* CFanFlatVecProjectionGeometry2D::clone() const
{
	return new CFanFlatVecProjectionGeometry2D(*this);
}

//----------------------------------------------------------------------------------------
// is equal
bool CFanFlatVecProjectionGeometry2D::isEqual(const CProjectionGeometry2D &_pGeom2) const
{
	// try to cast argument to CFanFlatVecProjectionGeometry2D
	const CFanFlatVecProjectionGeometry2D* pGeom2 = dynamic_cast<const CFanFlatVecProjectionGeometry2D*>(&_pGeom2);
	if (pGeom2 == NULL) return false;

	// both objects must be initialized
	if (!m_bInitialized || !pGeom2->m_bInitialized) return false;

	// check all values
	if (m_iProjectionAngleCount != pGeom2->m_iProjectionAngleCount) return false;
	if (m_iDetectorCount != pGeom2->m_iDetectorCount) return false;
	
	for (int i = 0; i < m_iProjectionAngleCount; ++i) {
		if (memcmp(&m_pProjectionAngles[i], &pGeom2->m_pProjectionAngles[i], sizeof(m_pProjectionAngles[i])) != 0) return false;
	}

	return true;
}

//----------------------------------------------------------------------------------------
// Is of type
bool CFanFlatVecProjectionGeometry2D::isOfType(const std::string& _sType)
{
	return (_sType == "fanflat_vec");
}

//----------------------------------------------------------------------------------------

bool CFanFlatVecProjectionGeometry2D::_check()
{
	// TODO
	return true;
}


//----------------------------------------------------------------------------------------
// Get the configuration object
Config* CFanFlatVecProjectionGeometry2D::getConfiguration() const 
{
	ConfigWriter CW("ProjectionGeometry2D", "fanflat_vec");

	CW.addInt("DetectorCount", getDetectorCount());

	std::vector<double> vectors;
	vectors.resize(6 * m_iProjectionAngleCount);

	for (int i = 0; i < m_iProjectionAngleCount; ++i) {
		SFanProjection& p = m_pProjectionAngles[i];
		vectors[6*i + 0] = p.fSrcX;
		vectors[6*i + 1] = p.fSrcY;
		vectors[6*i + 2] = p.fDetSX + 0.5 * m_iDetectorCount * p.fDetUX;
		vectors[6*i + 3] = p.fDetSY + 0.5 * m_iDetectorCount * p.fDetUY;
		vectors[6*i + 4] = p.fDetUX;
		vectors[6*i + 5] = p.fDetUY;
	}
	CW.addNumericalMatrix("Vectors", &vectors[0], m_iProjectionAngleCount, 6);

	return CW.getConfig();
}
//----------------------------------------------------------------------------------------


} // namespace astra
