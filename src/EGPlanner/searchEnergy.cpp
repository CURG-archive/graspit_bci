//######################################################################
//
// GraspIt!
// Copyright (C) 2002-2009  Columbia University in the City of New York.
// All rights reserved.
//
// GraspIt! is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GraspIt! is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GraspIt!.  If not, see <http://www.gnu.org/licenses/>.
//
// Author(s): Matei T. Ciocarlie
//
// $Id: searchEnergy.cpp,v 1.40.2.2 2009/07/24 13:39:03 cmatei Exp $
//
//######################################################################

#include "searchEnergy.h"

#include <time.h>

#include "robot.h"
#include "barrett.h"
#include "body.h"
#include "grasp.h"
#include "contact.h"
#include "world.h"
#include "quality.h"
#include "searchState.h"
#include "graspitGUI.h"
#include "ivmgr.h"
#include "matrix.h"

//#define GRASPITDBG
#include "debug.h"

//#define PROF_ENABLED
#include "profiling.h"
#include <fstream>
#include <stdlib.h>     /* getenv */

PROF_DECLARE(QS);

//todo move this out of here
const double unbalancedForceThreshold = 1.0e10;

SearchEnergy::SearchEnergy()
{
	mHand = NULL;
	mObject = NULL;
	mType = ENERGY_CONTACT; //default
	mContactType = CONTACT_LIVE; //default
	mVolQual = NULL;
	mEpsQual = NULL;
	mDisableRendering = true;
	mOut = NULL;


//    from rgb camera info topic for my kinect

//    header:
//      seq: 336
//      stamp:
//        secs: 1422642684
//        nsecs: 378085858
//      frame_id: /camera_rgb_optical_frame
//    height: 480
//    width: 640
//    distortion_model: plumb_bob
//    D: [0.0, 0.0, 0.0, 0.0, 0.0]
//    K: [525.0, 0.0, 319.5, 0.0, 525.0, 239.5, 0.0, 0.0, 1.0]
//    R: [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0]
//    P: [525.0, 0.0, 319.5, 0.0, 0.0, 525.0, 239.5, 0.0, 0.0, 0.0, 1.0, 0.0]
//    binning_x: 0
//    binning_y: 0
//    roi:
//      x_offset: 0
//      y_offset: 0
//      height: 0
//      width: 0
//      do_rectify: False

    sensor_msgs::CameraInfo cam_info;

    double D[] = {0.0, 0.0, 0.0, 0.0, 0.0};
    std::vector<double> D_vec (D, D + sizeof(D) / sizeof(double) );

    boost::array<double, 9> K =  {{525.0, 0.0, 319.5, 0.0, 525.0, 239.5, 0.0, 0.0, 1.0}};
    boost::array<double, 9> R =  {{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}};
    boost::array<double, 12> P =  {{525.0, 0.0, 319.5, 0.0, 0.0, 525.0, 239.5, 0.0, 0.0, 0.0, 1.0, 0.0}};

    cam_info.height = 480;
    cam_info.width = 640;
    cam_info.distortion_model = "plumb_bob";
    cam_info.D = D_vec;
    cam_info.K = K;
    cam_info.R = R;
    cam_info.P = P;
    cam_info.binning_x = 0;
    cam_info.binning_y = 0;
    cam_info.roi.x_offset = 0;
    cam_info.roi.y_offset = 0;
    cam_info.roi.height = 0;
    cam_info.roi.width = 0;
    cam_info.roi.do_rectify = false;

    model_.fromCameraInfo(cam_info);

    int height = 72;
    int width = 352;
    int depth = 512;

    heatmaps.resize(height);
    for(int i=0; i < height; i++){

        heatmaps[i].resize(width);

        for(int j=0; j < width; j++){
            heatmaps[i][j].resize(depth);
        }
    }

    char* heatmapsPath;
    heatmapsPath = getenv ("HEATMAPS_PATH");
    printf("The current path is: %s\n",heatmapsPath);


    double value;
    std::ifstream iFile;
    for(int i=0; i < height; i++)
    {
        std::ostringstream filename;
        //filename << heatmapsPath <<"/heatmaps/" << i << ".txt";
        filename << "/home/jvarley/grasp_deep_learning/gdl/src/graspit_bci/models/captured_meshes/1423149664/heatmaps/" << i << ".txt";


        std::cout << filename.str() << std::endl;
        iFile.open(filename.str());
        if (iFile.is_open())
        {
            for(int j=0; j < width; j++)
            {
                for(int k=0; k < depth; k++)
                {
                    iFile >> value;
                    heatmaps[i][j][k] =  value;
                }
            }
        }
        else
        {
            std::cout << "Error reaading heatmaps, file is not open" << std::endl;
        }


        iFile.close();
    }

    std::cout<< "Heatmaps read in sucessfully \n";

    int num_grasp_priors = 18;
    int num_dof = 4;
    std::cout<< "starting grasp priors import1" << std::endl;
    std::vector<double*> grasp_priors;
    std::cout<< "starting grasp priors import2" << std::endl;
    char* graspit_path;
    graspit_path = getenv ("GRASPIT");

    std::cout<< "starting grasp priors import3" << std::endl;

    for(int i=0; i < num_grasp_priors; i++)
    {
        std::ostringstream filename;
        filename << graspit_path << "/grasp_priors/" << i << ".txt";
        iFile.open(filename.str());

        std::cout<< "looking at: " << filename.str() << std::endl;

        double *dof_values = new double[ num_dof ];

        for (int j=0; j <num_dof; j++)
        {
            std::cout<< i << " " << j <<  "\n";
            iFile >> value;
            dof_values[j] = value;
        }

        iFile.close();

        grasp_priors.push_back(dof_values);

    }
    std::cout<< "Grasp Priors read in successfully \n";

    grasp_priors_matrix = new flann::Matrix<double>(grasp_priors[0], num_grasp_priors, num_dof);
    grasp_priors_index_ = new flann::Index< flann::L2<double> >(*grasp_priors_matrix, flann::KDTreeIndexParams(1));
    grasp_priors_index_->buildIndex();

    std::cout<< "Flann Index built successfully \n";


}

void
SearchEnergy::createQualityMeasures()
{
	if (mVolQual) delete mVolQual;
	if (mEpsQual) delete mEpsQual;
	mVolQual = new QualVolume( mHand->getGrasp(), QString("SimAnn_qvol"),"L1 Norm");
	mEpsQual = new QualEpsilon( mHand->getGrasp(), QString("SimAnn_qeps"), "L1 Norm");
	DBGP("Qual measures created");
}

void
SearchEnergy::setHandAndObject(Hand *h, Body *o)
{
	if (mHand != h) {
		mHand = h;
		createQualityMeasures();
	}
	mObject = o;
}

SearchEnergy::~SearchEnergy()
{
	delete mVolQual;
	delete mEpsQual;
}

bool
SearchEnergy::legal() const
{	
	//hack for iros09
	//the compliant planners do their own checks
	if (mType == ENERGY_COMPLIANT || mType == ENERGY_DYNAMIC) return true;

	//no check at all
	//return true;
	
	//full collision detection
	//if the hand is passed as an argument, this should only check for collisions that
	//actually involve the hand
	return mHand->getWorld()->noCollision(mHand);
	
	/*
	//check only palm
	if ( mHand->getWorld()->getDist( mHand->getPalm(), mObject) <= 0) return false;
	return true;
	*/
}

void
SearchEnergy::analyzeCurrentPosture(Hand *h, Body *o, bool &isLegal, double &stateEnergy, bool noChange)
{
	setHandAndObject(h,o);
	
	if (noChange) {
		h->saveState();
	}

	if ( !legal() ) {
		isLegal = false;
		stateEnergy = 0;
	} else {
		isLegal = true;
		stateEnergy = energy();
	}

	if (noChange) {
		h->restoreState();
	}
}

void SearchEnergy::analyzeState(bool &isLegal, double &stateEnergy, const GraspPlanningState *state, bool noChange)
{
	Hand *h = state->getHand();
	setHandAndObject( h, state->getObject() );
	h->saveState();
	transf objTran = state->getObject()->getTran();

	bool render = h->getRenderGeometry();
	if( mDisableRendering) {
		h->setRenderGeometry(false);
	}

	if ( !state->execute() || !legal() ) {
		isLegal = false;
		stateEnergy = 0;
	} else {
		isLegal = true;
		stateEnergy = energy();
	}

	if (noChange || !isLegal) {
		h->restoreState();
		state->getObject()->setTran(objTran);
	}

	if (render && mDisableRendering) h->setRenderGeometry(true);	
	return;
}


double SearchEnergy::energy() const
{
	/*
		this is if we don't want to use pre-specified contacts, but rather the closest points between each link
		and the object all iterations. Not necessarily needed for contact energy, but useful for GQ energy
	*/
	if (mContactType == CONTACT_LIVE && mType != ENERGY_AUTOGRASP_QUALITY && mType != ENERGY_STRICT_AUTOGRASP) {
		mHand->getWorld()->findVirtualContacts(mHand, mObject);
		DBGP("Live contacts computation");
	}

	double e;
	switch (mType) {
		case ENERGY_CONTACT:
			mHand->getGrasp()->collectVirtualContacts();
			e = contactEnergy();
			break;
		case ENERGY_POTENTIAL_QUALITY:
			mHand->getGrasp()->collectVirtualContacts();
			e = potentialQualityEnergy();
			break;
		case ENERGY_AUTOGRASP_QUALITY:
			// this one will collect REAL contacs after autograsp is completed
			e = autograspQualityEnergy();
			break;
		case ENERGY_CONTACT_QUALITY:
			mHand->getGrasp()->collectVirtualContacts();
			e = guidedPotentialQualityEnergy();
			break;
		case ENERGY_GUIDED_AUTOGRASP:
			// we let this one collect its contact explicitly since is has to do it twice
			e = guidedAutograspEnergy();
			break;
		case ENERGY_STRICT_AUTOGRASP:
			// this one will collect REAL contacs after autograsp is completed
			e = strictAutograspEnergy();
			break;
		case ENERGY_COMPLIANT:
			e = compliantEnergy();
			break;
		case ENERGY_DYNAMIC:
			e = dynamicAutograspEnergy();
			break;

        case ENERGY_HEATMAP:
            mHand->getGrasp()->collectVirtualContacts();
            e = heatmapProjectionEnergy();
            break;

		default:
			fprintf(stderr,"Wrong type of energy requested!\n");
			e = 0;
	}
	return e;
}

double
SearchEnergy::contactEnergy() const
{

	VirtualContact *contact;
    vec3 objectDistance,n,cn;
    vec3 objectNormal;
	double totalError = 0;

    int PALM_INDEX = 0;
    int FINGER_1_INDEX = 7;
    int FINGER_2_INDEX = 11;
    int FINGER_3_INDEX = 15;


    for (int i=0; i<mHand->getGrasp()->getNumContacts(); i++)
    {
        if (i < 4)
        {

            contact = (VirtualContact*)mHand->getGrasp()->getContact(i);
            contact->getObjectDistanceAndNormal(mObject, &objectDistance, &objectNormal);

            double dist = objectDistance.len();

            cn = contact->getWorldNormal();
            n = normalise(objectNormal);

            double d = 1 - cn % n;

            std::cout << "dist: " << dist << " cn%n: " << cn % n << std::endl;
            std::cout << "object normal: " << objectNormal << std::endl;
            std::cout << "contact normal: " << cn << std::endl;


            //beter grasps have contacts closer to the object
            totalError += dist;

            //better grasps contacts normals are better aligned with the object normal at
            //projected point of contact
            totalError += d * 1000.0 / 2.0;
        }


	}
	
    totalError /= 4.0;//mHand->getGrasp()->getNumContacts();
		
	return totalError;
}

/*!	This formulation combines virtual contact energy with autograsp energy. Virtual contact energy is used to "guide"
	initial	stages of the search and to see if we should even bother computing autograsp quality. Autograsp is a couple
	of orders of magnitude higher and so should work very well with later stages of the sim ann search
*/
double
SearchEnergy::guidedAutograspEnergy() const
{
	//first compute regular contact energy; also count how many links are "close" to the object
	VirtualContact *contact;
	vec3 p,n,cn;
	double virtualError = 0; int closeContacts=0;

	//collect virtual contacts first
	mHand->getGrasp()->collectVirtualContacts();
	for (int i=0; i<mHand->getGrasp()->getNumContacts(); i++) {

		contact = (VirtualContact*)mHand->getGrasp()->getContact(i);
		contact->getObjectDistanceAndNormal(mObject, &p, &n);

		double dist = p.len();
		if ( (-1.0 * p) % n  < 0) dist = -dist;
	
		//BEST WORKING VERSION, strangely enough
		virtualError += fabs(dist);
		cn = -1.0 * contact->getWorldNormal();
		double d = 1 - cn % n;
		virtualError += d * 100.0 / 2.0;

		if ( fabs(dist)<20 && d < 0.3 ) closeContacts++;
	}
	
	virtualError /= mHand->getGrasp()->getNumContacts();

	//if more than 2 links are "close" go ahead and compute the true quality
	double volQuality = 0, epsQuality = 0;
	if (closeContacts >= 2) {
		mHand->autoGrasp(false, 1.0);
		//now collect the true contacts;
		mHand->getGrasp()->collectContacts();
		if (mHand->getGrasp()->getNumContacts() >= 4) {
			mHand->getGrasp()->updateWrenchSpaces();
			volQuality = mVolQual->evaluate();
			epsQuality = mEpsQual->evaluate();
			if (epsQuality < 0) epsQuality = 0; //QM returns -1 for non-FC grasps
		}

		DBGP("Virtual error " << virtualError << " and " << closeContacts << " close contacts.");
		DBGP("Volume quality: " << volQuality << " Epsilon quality: " << epsQuality);		
	}

	//now add the two such that the true quality is a couple of orders of magn. bigger than virtual quality
	double q;
	if ( volQuality == 0) q = virtualError;
	else q = virtualError - volQuality * 1.0e3;
	if (volQuality || epsQuality) {DBGP("Final quality: " << q);}

	//DBGP("Final value: " << q << std::endl);
	return q;
}

/*!	This version uses the contact energy to guide the search when quality energy is poor. If quality energy is ok
	then it uses it, scaled to about -10 so that in the early and middle stages of the search it will still jump
	out of it, but not in later stages
*/
double 
SearchEnergy::guidedPotentialQualityEnergy() const
{
	double cEn = contactEnergy();
	double pEn = potentialQualityEnergy(false);

	if (pEn > 0.0) return cEn;
	return pEn;
}

double
SearchEnergy::potentialQualityEnergy(bool verbose) const
{
	VirtualContact *contact;
	vec3 p,n,cn;

	int count = 0;
	//DBGP("Potential quality energy computation")
	for (int i=0; i<mHand->getGrasp()->getNumContacts(); i++) {

		contact = (VirtualContact*)mHand->getGrasp()->getContact(i);
		contact->computeWrenches(true, false);

		contact->getObjectDistanceAndNormal(mObject, &p, NULL);
		n= contact->getWorldNormal();

		double dist = p.len();
		p = normalise(p); //idiot programmer forgot to normalise
		double cosTheta = n % p;

		double factor = potentialQualityScalingFunction(dist, cosTheta);

		if (verbose) {
			fprintf(stderr,"VC %d on finger %d link %d\n",i,contact->getFingerNum(), contact->getLinkNum());
			fprintf(stderr,"Distance %f cosTheta %f\n",dist,cosTheta);
			fprintf(stderr,"Scaling factor %f\n\n",factor);
		}
		contact->scaleWrenches( factor );
		if (factor > 0.25) {
			count++;
			contact->mark(true);
		} else contact->mark(false);
	}

	double gq = -1 ,gv = -1;
	//to make computations more efficient, we only use a 3D approximation
	//of the 6D wrench space
	std::vector<int> forceDimensions(6,0);
	forceDimensions[0] = forceDimensions[1] = forceDimensions[2] = 1;
    if (count >= 4) {
		mHand->getGrasp()->updateWrenchSpaces(forceDimensions);
		gq = mEpsQual->evaluate();
		gv = mVolQual->evaluate();
	}
	if (verbose) {
		fprintf(stderr,"Quality: %f\n\n",gq);
	}
	if (count) {
		DBGP("Count: " << count << "; Gq: " << gq << "; Gv: " << gv);
	}
	return -gq;
}

/*! This function simply closes the hand and computes the real grasp quality that results
*/
double
SearchEnergy::autograspQualityEnergy() const
{
	DBGP("Autograsp quality computation");
	mHand->autoGrasp(false, 1.0);
	mHand->getGrasp()->collectContacts();
	mHand->getGrasp()->updateWrenchSpaces();
	double volQual = mVolQual->evaluate();
	double epsQual = mEpsQual->evaluate();
	if (epsQual < 0) epsQual = 0; //returns -1 for non-FC grasps
	DBGP("Autograsp quality: " << volQual << " volume and " << epsQual << " epsilon.");
	return - (30 * volQual) - (100 * epsQual);
}

/*!	This version moves the palm in the direction of the object, attempting to establish contact on the palm
	before closing the fingers and establishing contacts on the finger.
*/
double
SearchEnergy::approachAutograspQualityEnergy() const
{
	transf initialTran = mHand->getTran();
	bool contact = mHand->approachToContact(30);
	if (contact) {
		if ( mHand->getPalm()->getNumContacts() == 0) contact = false;
	}
	if (!contact) {
		//if moving the hand in does not result in a palm contact, move out and grasp from the initial position
		//this allows us to obtain fingertip grasps if we want those
		DBGP("Approach found no contacts");
		mHand->setTran( initialTran );
	} else {
		DBGP("Approach results in contact");
	}
	return autograspQualityEnergy();
}

void 
SearchEnergy::autoGraspStep(int numCols, bool &stopRequest) const
{
	//if no new contacts have been established, nothing new to compute
	stopRequest = false;
	if (!numCols) {
		return;
	}
	//if any of the kinematic chains is not balanced, early exit
	mHand->getWorld()->resetDynamicWrenches();
	//compute min forces to balance the system (pass it a "false")
	Matrix tau(mHand->staticJointTorques(false));
	int result = mHand->getGrasp()->computeQuasistaticForces(tau);
	if (result) {
		if (result > 0) {
			PRINT_STAT(mOut, "Unbalanced");
		} else {
			PRINT_STAT(mOut, "ERROR");
		}
		mCompUnbalanced = true;
		stopRequest = true;
		return;
	}
	assert(mObject->isDynamic());
	double* extWrench = static_cast<DynamicBody*>(mObject)->getExtWrenchAcc();
	vec3 force(extWrench[0], extWrench[1], extWrench[2]);
	if (force.len() > mMaxUnbalancedForce.len()) {
		mMaxUnbalancedForce = force;
	}
	//we could do an early exit here as well
}

double
SearchEnergy::compliantEnergy() const
{
	PROF_RESET(QS);
	PROF_TIMER_FUNC(QS);
	//approach the object until contact; go really far if needed
	mHand->findInitialContact(200);
	//check if we've actually touched the object
	if (!mHand->getNumContacts(mObject)) return 1;

	//close the hand, but do additional processing when each new contact happens
	mCompUnbalanced = false; 
	mMaxUnbalancedForce.set(0.0,0.0,0.0);
	
	QObject::connect(mHand, SIGNAL(moveDOFStepTaken(int, bool&)), 
					 this, SLOT(autoGraspStep(int, bool&)));
	mHand->autoGrasp(!mDisableRendering, 1.0, false);
	QObject::disconnect(mHand, SIGNAL(moveDOFStepTaken(int, bool&)), 
				        this, SLOT(autoGraspStep(int, bool&)));

	if (mCompUnbalanced || mMaxUnbalancedForce.len() > unbalancedForceThreshold) {
		//the equivalent of an unstable grasp
	}

	//check if we've actually grasped the object
	if (mHand->getNumContacts(mObject) < 2) return 1;

	PRINT_STAT(mOut, "unbal: " << mMaxUnbalancedForce);

	//compute unbalanced force again. Is it zero?
	//but compute it for all the force that the dofs will apply
	//a big hack for now. It is questionable if the hand should even allow
	//this kind of intrusion into its dofs.
	for (int d=0; d<mHand->getNumDOF(); d++) {
		mHand->getDOF(d)->setForce( mHand->getDOF(d)->getMaxForce() );
	}
	mHand->getWorld()->resetDynamicWrenches();
	//passing true means the set dof force will be used in computations
	Matrix tau(mHand->staticJointTorques(true));
	int result = mHand->getGrasp()->computeQuasistaticForces(tau);
	if (result) {
		if (result > 0) {
			PRINT_STAT(mOut, "Final_unbalanced");
		} else {
			PRINT_STAT(mOut, "Final_ERROR");
		}
		return 1.0;
	} 
	double* extWrench = static_cast<DynamicBody*>(mObject)->getExtWrenchAcc();
	vec3 force(extWrench[0], extWrench[1], extWrench[2]);
	vec3 torque(extWrench[3], extWrench[4], extWrench[5]);

	//perform traditional f-c check
	mHand->getGrasp()->collectContacts();
	mHand->getGrasp()->updateWrenchSpaces();
	double epsQual = mEpsQual->evaluate();
	PRINT_STAT(mOut, "eps: " << epsQual);

	if ( epsQual < 0.05) return 1.0;

	PROF_PRINT(QS);
	PRINT_STAT(mOut, "torque: " << torque << " " << torque.len());
	PRINT_STAT(mOut, "force: " << force << " " << force.len());

	return -200.0 + force.len();// + torque.len();
}
void
SearchEnergy::dynamicsError(const char*) const
{
	DBGA("Dynamics error; early exit");
	mDynamicsError = true;
}

bool
SearchEnergy::contactSlip() const
{
	return mHand->contactSlip();
}

/*! Implements the following heuristic: for each chain, either the last link
	must have a contact, or the last joint must be maxed out. If this is true
	the dynamic autograsp *might* have been completed
*/
bool SearchEnergy::dynamicAutograspComplete() const
{
	return mHand->dynamicAutograspComplete();
}

double
SearchEnergy::dynamicAutograspEnergy() const
{
	//approach the object until contact; go really far if needed
	mHand->findInitialContact(200);
	//check if we've actually touched the object
	if (!mHand->getNumContacts(mObject)) return 1;

	//this is more of a hack to cause the hand to autograsp in dynamics way
	//the world's callback for dynamics should never get called as everything 
	//gets done from inside here
	mHand->getWorld()->resetDynamics();
	//I think this happens in each dynamics step anyway
	mHand->getWorld()->resetDynamicWrenches();
	mHand->getWorld()->turnOnDynamics();
	//this should set the desired values of the dof's
	//also close slowly, so we are closer to pseudo-static conditions
	mHand->autoGrasp(false, 0.5);

	QObject::connect(mHand->getWorld(), SIGNAL(dynamicsError(const char*)),
					 this, SLOT(dynamicsError(const char*)));
	//loop until dynamics is done
	mDynamicsError = false;
	int steps=0; int stepFailsafe = 1500;
	int autoGraspDone = 0; int afterSteps = 100;
	//first close fingers to contact with object
	while (1) {
		if (!(steps%100)) {DBGA("Step " << steps);}
		mHand->getWorld()->stepDynamics();
		if (mDynamicsError) break;
		//see if autograsp is done
		if (!autoGraspDone && dynamicAutograspComplete()) {
			autoGraspDone = steps;
		}
		//do some more steps after sutograsp
		if (autoGraspDone && steps - autoGraspDone > afterSteps) {
			break;
		}
		//and finally check stepfailsafe
		if (++steps > stepFailsafe) break;
	}

	if (mDynamicsError) {
		PRINT_STAT(mOut, "Dynamics error");
		return 2.0;
	} else if (steps > stepFailsafe) {
		PRINT_STAT(mOut, "Time failsafe");
		return 2.0;
	}
	PRINT_STAT(mOut, "Autograsp done");

	//disable contacts on pedestal
	Body *obstacle;
	for (int b=0; b<mHand->getWorld()->getNumBodies(); b++) {
		Body *bod = mHand->getWorld()->getBody(b);
		if (bod->isDynamic()) continue;
		if (bod->getOwner() != bod) continue;
		obstacle = bod;
		break;
	}
	if (!obstacle) {
		PRINT_STAT(mOut, "Obstacle not found!");
		return 2.0;
	}
	mHand->getWorld()->toggleCollisions(false, obstacle);
	//and do some more steps
	steps = 0; afterSteps = 400;
	while (1) {
		if (!(steps%100)) {DBGA("After step " << steps);}
		mHand->getWorld()->stepDynamics();
		/*
		if (mHand->getDOF(0)->getForce() + 1.0e3 > mHand->getDOF(0)->getMaxForce()) {
			DBGA("Max force applied");
			break;
		}*/
		if (mDynamicsError) break;
		if (++steps > afterSteps) break;
	}
	mHand->getWorld()->toggleCollisions(true, obstacle);
	QObject::disconnect(mHand->getWorld(), SIGNAL(dynamicsError(const char*)),
				    	this, SLOT(dynamicsError(const char*)));
	//turn off dynamics; world dynamics on shouldn't have done anything anyway
	mHand->getWorld()->turnOffDynamics();

	if (mDynamicsError) {
		PRINT_STAT(mOut, "Dynamics error");
		return 2.0;
	}

	//if the object has been ejected output error
	if (mHand->getNumContacts(mObject) < 2) {
		PRINT_STAT(mOut, "Ejected");
		return 0.0;
	}

	//perform traditional f-c check
	mHand->getGrasp()->collectContacts();
	mHand->getGrasp()->updateWrenchSpaces();
	double epsQual = mEpsQual->evaluate();
	PRINT_STAT(mOut, "eps: " << epsQual);

	if ( epsQual < 0.05) return -0.5;

	//grasp has finished in contact with the object
	PRINT_STAT(mOut, "Success 1");
	return -1.0;
}

/*! This version behaves like the autograsp energy, except it returns a huge penalty if the grasp has 0 hull
It is meant for searches that NEVER want to go where the grasp has no quality
*/
double
SearchEnergy::strictAutograspEnergy() const
{
//	double gq = autograspQualityEnergy();
	double gq = approachAutograspQualityEnergy();
	if (gq==0) return 1.0e8;
	else return gq;
}


 int SearchEnergy::getGraspType() const
 {
     std::vector<double> query_joint_state;

     double *dof_values = new double[ mHand->getNumDOF() ];

     mHand->getDOFVals(dof_values);

     for (int dof_index=0; dof_index < mHand->getNumDOF(); ++dof_index)
     {
         query_joint_state.push_back(dof_values[dof_index]);
         std::cout << "building query; adding for dof_index: " << dof_index << " value: " <<dof_values[dof_index] << std::endl;
     }

     std::vector<std::vector<int> > query_results_tmp;
     flann::Matrix<double> query_pose_mat(&query_joint_state[0], 1,mHand->getNumDOF());

     std::vector<std::vector<double> >query_distances;
     flann::SearchParams params(32, 0.0, true);
     double max_neighbor_distance = 100.0;
     std::cout << "about to query flann" << std::endl;

     grasp_priors_index_->radiusSearch(query_pose_mat, query_results_tmp,  query_distances, max_neighbor_distance,params);
     std::cout << "about to return result" << std::endl;
     return query_results_tmp[0][0];
 }

 double SearchEnergy::heatmapProjectionEnergy() const
 {
     std::cout << std::endl << std::endl << "Entered heatmapProjectionEnergy" << std::endl;

     //negative is better
     double contact_quality = 20.0 * contactEnergy();

     double heatmap_quality = 0;
     VirtualContact *contact;

     for (int i=0; i<mHand->getGrasp()->getNumContacts(); i++)
     {

//         int PALM_INDEX = 0;
//         int FINGER_1_INDEX = 5;
//         int FINGER_2_INDEX = 6;
//         int FINGER_3_INDEX = 7;

         int PALM_INDEX = 0;
         int FINGER_1_INDEX = 7;
         int FINGER_2_INDEX = 11;
         int FINGER_3_INDEX = 15;


         //NEED TO ENSURE THAT WE HAVE PALM OR F1, F2, F3, VC
         if(i==PALM_INDEX || i == FINGER_1_INDEX || i == FINGER_2_INDEX || i ==FINGER_3_INDEX)
         {
             contact = (VirtualContact*)mHand->getGrasp()->getContact(i);

             //Need to get contact location in relation to camera:
             position worldPoint = contact->getWorldLocation();

             //Need to get uv from image_geometry camera model
             cv::Point3d worldPointCV(-worldPoint.x()/1000,-worldPoint.y()/1000,worldPoint.z()/1000);
             cv::Point2d uv = model_.project3dToPixel(worldPointCV);

             std::cout << "World Point in Meters\n" << worldPointCV << std::endl << uv << std::endl;

             int grasp_type = getGraspType();

             std::cout << "Grasp Type: " << grasp_type << std::endl;
             int heatmap_index = 4*grasp_type ;

             if (i == PALM_INDEX)
             {
                 std::cout << "Computing Palm Energy" << std::endl;
                 heatmap_index += 0;
             }
             else if(i==FINGER_1_INDEX)
             {
                 std::cout << "Computing FINGER_1_INDEX Energy" << std::endl;
                 heatmap_index += 1;
             }
             else if(i==FINGER_2_INDEX)
             {
                 std::cout << "Computing FINGER_2_INDEX Energy" << std::endl;
                 heatmap_index += 2;
             }
             else if(i==FINGER_3_INDEX)
             {
                 std::cout << "Computing FINGER_3_INDEX Energy" << std::endl;
                 heatmap_index += 3;
             }

             std::cout << "Heatmap index is: " << heatmap_index << std::endl;

             int x_index = int(uv.x) - 64 ;
             int y_index = int(uv.y) - 64 ;

             std::cout << "x: " << x_index << " y: " << y_index << std::endl;

             if (heatmaps[heatmap_index].size() > x_index && x_index > 0)
             {
                 if (heatmaps[heatmap_index][x_index].size() > y_index && y_index > 0)
                 {
                     std::cout << "appending to HeatMapQuality" << std::endl;
                     heatmap_quality -= heatmaps[heatmap_index][x_index][y_index];
                     std::cout << "HeatMapQuality: " << heatmaps[heatmap_index][x_index][y_index] << std::endl;

                 }
             }


         }
     }

     double grasp_quality = heatmap_quality + contact_quality;

     std::cout << "heatmap_quality: " << heatmap_quality << std::endl;
     std::cout << "contact_quality: " << contact_quality << std::endl;
     std::cout << "grasp_quality: " << grasp_quality << std::endl;

     return grasp_quality;

 }



/* ---------------------------------- SCALING FUNCTIONS ---------------------------------- */
double
SearchEnergy::distanceFunction(double d) const
{
	double ERRORWIDTH=25;	// the function gets very close to 1 at about ERRORWIDTH * 4 mm from the object
	if (d > 0) {
		return ( 40.0 - 40.0 / pow(4.0,d/ERRORWIDTH) );
	} else {
		return -d;
	}
}

double 
SearchEnergy::potentialQualityScalingFunction(double dist, double cosTheta) const
{
	double sf = 0;
	/*
	(void*)&cosTheta;
	if (dist<0) return 0;
	sf += 100.0 / ( pow(2.0,dist/15.0) );
	//comment this out if you don't care about normals
	//sf += 100 - (1 - cosTheta) * 100.0 / 2.0;
	*/
/*
	if (cosTheta < 0.8) return 0; //about 35 degrees
	sf = 10.0 / ( pow((double)3.0,dist/25.0) ); 
	if (sf < 0.25) sf = 0; //cut down on computation for tiny values. more than 50mm away
	return sf;
*/
	/*
	if (cosTheta < 0.8) return 0; //about 35 degrees
	if (dist < 20) sf = 1;
	else {
		sf = 1.5 - 1.5 / ( pow((double)3.0,(dist-20)/25.0) );
		sf = cos( sf*sf ) + 1;
	}
	sf = sf*10;
	if (sf < 0.25) sf = 0; //cut down on computation for tiny values. more than 50mm away
	return sf;
	*/
    if (cosTheta < 0.7) return 0;
	if (dist > 50) return 0;
	sf = cos ( 3.14 * dist / 50.0) + 1;
	return sf;
}

//---------------------------------- CLOSURE SEARCH ---------------------------------

void 
ClosureSearchEnergy::analyzeState(bool &isLegal, double &stateEnergy, const GraspPlanningState *state, bool noChange)
{
	//first we check if we are too close to any state in the avoid list
	if (mAvoidList) {
		std::list<GraspPlanningState*>::const_iterator it; int i=0;
		for (it = mAvoidList->begin(); it!=mAvoidList->end(); it++){
			if ( (*it)->distance(state) < mThreshold ) {
				isLegal = false;
				stateEnergy = 0.0;
				DBGP("State rejected; close to state " << i);
				return;
			}
			i++;
		}
	}

	//if not, we compute everything like we usually do
	SearchEnergy::analyzeState(isLegal, stateEnergy, state, noChange);
}
