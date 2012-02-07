// CoordinateActuator.cpp
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
* Copyright (c)  2005, Stanford University. All rights reserved. 
* Use of the OpenSim software in source form is permitted provided that the following
* conditions are met:
* 	1. The software is used only for non-commercial research and education. It may not
*     be used in relation to any commercial activity.
* 	2. The software is not distributed or redistributed.  Software distribution is allowed 
*     only through https://simtk.org/home/opensim.
* 	3. Use of the OpenSim software or derivatives must be acknowledged in all publications,
*      presentations, or documents describing work in which OpenSim or derivatives are used.
* 	4. Credits to developers may not be removed from executables
*     created from modifications of the source.
* 	5. Modifications of source code must retain the above copyright notice, this list of
*     conditions and the following disclaimer. 
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
*  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
*  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
*  SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
*  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
*  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR BUSINESS INTERRUPTION) OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
*  WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* 
 * Author: Frank C. Anderson, Ajay Seth
 */


//=============================================================================
// INCLUDES
//=============================================================================
#include "CoordinateActuator.h"
#include <OpenSim/Simulation/Model/Model.h>
#include <OpenSim/Simulation/Model/CoordinateSet.h>
#include <OpenSim/Simulation/Model/ForceSet.h>

using namespace OpenSim;
using namespace std;


//=============================================================================
// STATICS
//=============================================================================


//=============================================================================
// CONSTRUCTOR(S) AND DESTRUCTOR
//=============================================================================
//_____________________________________________________________________________
/**
 * Destructor.
 */
CoordinateActuator::~CoordinateActuator()
{
}
//_____________________________________________________________________________
/**
 * Default constructor.
 */
CoordinateActuator::CoordinateActuator( string aCoordinateName) :
	Actuator(),
	_coord(NULL)
{
	// NULL
	setNull();

	// MEMBER VARIABLES
	setPropertyValue("coordinate", aCoordinateName);

	if (_model) {
		_coord = &_model->updCoordinateSet().get(aCoordinateName);
	} 
}
//_____________________________________________________________________________
/**
 * Copy constructor.
 *
 * @param aForce Force to be copied.
 */
CoordinateActuator::CoordinateActuator(const CoordinateActuator &aGenForce) :
	Actuator(aGenForce),
	_coord(NULL)
{
	setNull();
	copyData(aGenForce);
}
//_____________________________________________________________________________
/**
 * Copy this actuator and return a pointer to the copy.
 * The copy constructor for this class is used.
 *
 * @return Pointer to a copy of this actuator.
 */
Object* CoordinateActuator::
copy() const
{
	CoordinateActuator *force = new CoordinateActuator(*this);
	return force;
}


//=============================================================================
// CONSTRUCTION
//=============================================================================
//_____________________________________________________________________________
/**
 * Set the data members of this actuator to their null values.
 */
void CoordinateActuator::setNull()
{
	setType("CoordinateActuator");
	setupProperties();
}

//_____________________________________________________________________________
/**
 * Connect properties to local pointers.
 */
void CoordinateActuator::setupProperties()
{
	addProperty<string>("coordinate",
		"string",
		"",
		"");
	addProperty<double>("optimal_force",
		"double",
		"",
		1.0);
}

//_____________________________________________________________________________
/**
 * Copy the member data of the specified actuator.
 */
void CoordinateActuator::copyData(const CoordinateActuator &aGenForce)
{
	// MEMBER VARIABLES
	setPropertyValue("coordinate", aGenForce.getPropertyValue<string>("coordinate"));
	setCoordinate(aGenForce.getCoordinate());
	setOptimalForce(aGenForce.getOptimalForce());
}


//=============================================================================
// OPERATORS
//=============================================================================
//-----------------------------------------------------------------------------
// ASSIGNMENT
//-----------------------------------------------------------------------------
//_____________________________________________________________________________
/**
 * Assignment operator.
 *
 * @return  aCoordinateID ID (or number, or index) of the generalized coordinate.
 */
CoordinateActuator& CoordinateActuator::operator=(const CoordinateActuator &aGenForce)
{
	// BASE CLASS
	Actuator::operator =(aGenForce);

	copyData(aGenForce);

	return(*this);
}


//=============================================================================
// GET AND SET
//=============================================================================
//-----------------------------------------------------------------------------
// CoordinateID
//-----------------------------------------------------------------------------
//_____________________________________________________________________________
/**
 * Set the generalized coordinate to which the coordinate actuator is applied.
 *
 * @param aCoordinate Pointer to the generalized coordinate.
 */
void CoordinateActuator::setCoordinate(Coordinate* aCoordinate)
{
	_coord = aCoordinate;
	if(aCoordinate)
		setPropertyValue("coordinate", aCoordinate->getName());
}
//_____________________________________________________________________________
/**
 * Get the generalized coordinate to which the coordinate actuator
 * is applied.
 *
 * @return Pointer to the coordinate
 */
Coordinate* CoordinateActuator::getCoordinate() const
{
	return(_coord);
}

//-----------------------------------------------------------------------------
// OPTIMAL FORCE
//-----------------------------------------------------------------------------
//_____________________________________________________________________________
/**
 * Set the optimal force of the force.
 *
 * @param aOptimalForce Optimal force.
 */
void CoordinateActuator::setOptimalForce(double aOptimalForce)
{
	setPropertyValue("optimal_force", aOptimalForce);
}
//_____________________________________________________________________________
/**
 * Get the optimal force of the force.
 *
 * @return Optimal force.
 */
double CoordinateActuator::getOptimalForce() const
{
	return getPropertyValue<double>("optimal_force");
}
//_____________________________________________________________________________
/**
 * Get the stress of the force.
 *
 * @return Stress.
 */
double CoordinateActuator::getStress( const SimTK::State& s) const
{
	return fabs(getForce(s)/getPropertyValue<double>("optimal_force")); 
}


//=============================================================================
// COMPUTATIONS
//=============================================================================
//_____________________________________________________________________________
/**
 * Compute all quantities necessary for applying the actuator force to the
 * model.
 */
double CoordinateActuator::computeActuation( const SimTK::State& s ) const
{
	if(_model==NULL)
		return 0.0;

	// FORCE
	return( getControl(s) * getPropertyValue<double>("optimal_force") );
}


//=============================================================================
// UTILITY
//=============================================================================
//_____________________________________________________________________________
/**
 */
ForceSet *CoordinateActuator::
CreateForceSetOfCoordinateActuatorsForModel(const SimTK::State& s, Model& aModel,double aOptimalForce,bool aIncludeLockedAndConstrainedCoordinates)
{
	ForceSet& as = aModel.updForceSet();
	as.setSize(0);
	const CoordinateSet& cs = aModel.getCoordinateSet();
	for(int i=0; i<cs.getSize(); i++) {
		if(!aIncludeLockedAndConstrainedCoordinates && (cs[i].isConstrained(s))) continue;
		CoordinateActuator *actuator = new CoordinateActuator();
		actuator->setCoordinate(&cs.get(i));
		actuator->setName(cs.get(i).getName()+"_actuator");
		actuator->setOptimalForce(aOptimalForce);
		as.append(actuator);
	}
	as.setup(aModel);
	aModel.invalidateSystem();
	return &as;
}

//=============================================================================
// APPLICATION
//=============================================================================
//_____________________________________________________________________________
/**
 * Apply the actuator force to BodyA and BodyB.
 */
void CoordinateActuator::computeForce( const SimTK::State& s, 
							   SimTK::Vector_<SimTK::SpatialVec>& bodyForces, 
							   SimTK::Vector& mobilityForces) const
{
	if(_model==NULL) return;

   double force;
    if( isForceOverriden(s) ) {
       force = computeOverrideForce(s);
    } else {
       force = computeActuation(s);
    }
    setForce(s,  force );

	if(isCoordinateValid()){
       applyGeneralizedForce(s, *_coord, getForce(s), mobilityForces);
    } else {
       std::cout << "CoordinateActuator::computeForce  Invalid coordinate " << std::endl;
    }
}

double CoordinateActuator::
getSpeed( const SimTK::State& s) const
{
	assert(_coord);
	return _coord->getSpeedValue(s);
};

//_____________________________________________________________________________
/**
 * Perform some setup functions that happen after the
 * object has been deserialized or copied.
 *
 * @param aModel OpenSim model containing this CoordinateActuator.
 */
void CoordinateActuator::setup(Model& aModel)
{
	string errorMessage;

	const string &coordName = getPropertyValue<string>("coordinate");

	// Base class
	Actuator::setup(aModel);

	// Look up the coordinate
	if (!_model->updCoordinateSet().contains(coordName)) {
		errorMessage = "CoordinateActuator: Invalid coordinate (" + coordName + ") specified in Actuator " + getName();
		throw (Exception(errorMessage.c_str()));
	}
	else
		_coord = &_model->updCoordinateSet().get(coordName);
}

//_____________________________________________________________________________
/**
 *  Create underlying SimTK::Force
 */
 void  CoordinateActuator::createSystem(SimTK::MultibodySystem& system) const {

     Actuator::createSystem( system );
}


//=============================================================================
// CHECK
//=============================================================================
//_____________________________________________________________________________
/**
 * Check that this coordinate actuator actuator is valid.
 *
 * @return True if valid, false if invalid.
 */
bool CoordinateActuator::check() const
{
	// CoordinateID
	if(!isCoordinateValid()) {
		printf("CoordinateActuator.check: ERROR- %s actuates ",
			getName().c_str());
		printf("an invalid generalized coordinate (%s).\n", getPropertyValue<string>("coordinate").c_str());
		return(false);
	}

	return(true);
}
//_____________________________________________________________________________
/**
 * Is the.
 */
bool CoordinateActuator::isCoordinateValid() const
{
	if (_model == NULL || _coord == NULL)
		return false;

	return true;
}

//=============================================================================
// XML
//=============================================================================
//-----------------------------------------------------------------------------
// UPDATE FROM XML NODE
//-----------------------------------------------------------------------------
//_____________________________________________________________________________
/**
 * Update this object based on its XML node.
 *
 * This method simply calls Object::updateFromXMLNode(SimTK::Xml::Element& aNode, int versionNumber) and then calls
 * a few methods in this class to ensure that variable members have been
 * set in a consistent manner.
 */
void CoordinateActuator::updateFromXMLNode(SimTK::Xml::Element& aNode, int versionNumber)
{
	Actuator::updateFromXMLNode(aNode, versionNumber);
	setCoordinate(_coord);
}	

/** 
 * Methods to query a Force for the value actually applied during simulation
 * The names of the quantities (column labels) is returned by this first function
 * getRecordLabels()
 */
OpenSim::Array<std::string> CoordinateActuator::getRecordLabels() const {
	OpenSim::Array<std::string> labels("");
	labels.append(getName());
	return labels;
}
/**
 * Given SimTK::State object extract all the values necessary to report forces, application location
 * frame, etc. used in conjunction with getRecordLabels and should return same size Array
 */
OpenSim::Array<double> CoordinateActuator::getRecordValues(const SimTK::State& state) const {
	OpenSim::Array<double> values(1);
	values.append(getForce(state));
	return values;
};

