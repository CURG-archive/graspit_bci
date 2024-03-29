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
// $Id: onLinePlanner.cpp,v 1.30.2.1 2009/07/23 21:18:07 cmatei Exp $
//
//######################################################################

#include "onLinePlanner.h"

#include <Inventor/nodes/SoSeparator.h>
#include <QMutexLocker>
#include "world.h"
#include "robot.h"
#include "simAnn.h"
#include "searchState.h"
#include "searchEnergy.h"
#include "graspTesterThread.h"
#include "onLineGraspInterface.h"
#include "grasp.h"
#include "eigenGrasp.h"
#include "matvec3D.h"
#include "graspitGUI.h"
#include "ivmgr.h"

//#define GRASPITDBG
#include "debug.h"

#define SHOW_RECENT_SOLUTION 1
#define CANDIDATE_BUFFER_SIZE 20
#define SOLUTION_BUFFER_SIZE 10

OnLinePlanner::OnLinePlanner(Hand *h) : SimAnnPlanner(h)
{
	mSolutionClone = NULL;
    mProgressClone = NULL;
    mMarkSolutions = false;
	mCurrentBest = NULL;
	mSimAnn->setParameters(ANNEAL_ONLINE);
    setRenderType(RENDER_ALWAYS);
	mRepeat = true;

	mGraspTester = new GraspTester(h);
  //mGraspTester->getTargetState()->setObject(h->getGrasp()->getObject());
	mGraspTester->startThread();
	mGraspTester->showClone(false);

	//the on-line planner ALWAYS uses a clone for the search but the original hand is saved as the reference hand
	mRefHand = h;
    createSolutionClone();
    createProgressClone();
    //createAndUseClone();//after this point mHand now points to a new clone
	//in case that later we might want to see what the clone is doing
	mHand->setRenderGeometry(true);
	//but for now it is hidden
    //showClone(false);
	//hack - I need a better way to handle collisions when the planner is using a clone
	//we have three hands we need to take care of: the original hand, this clone and the parallel tester's clone
	//some of the collisions are turned off by createAndUseClone(), but not this one
    //this class will actually be used to set the DOF's of the refHand if we are actually performing
	//grasping tasks.
	mInterface = new OnLineGraspInterface(mRefHand);
}

OnLinePlanner::~OnLinePlanner()
{
	mRefHand->setTransparency(0);
	mGraspTester->stopPlanner();
	mGraspTester->wait();
	delete mGraspTester;
	DBGA("Grasp tester deleted");
	if (mSolutionClone) {
		mHand->getWorld()->destroyElement(mSolutionClone);
	}
	delete mInterface;
}

void
OnLinePlanner::resetParameters()
{
	SimAnnPlanner::resetParameters();
	if (mCurrentBest) mCurrentBest->setEnergy(1.0e8);
}

void
OnLinePlanner::createSolutionClone()
{

	if(mSolutionClone) {
		DBGA("Solution clone exists already!");
		return;
	}

	mSolutionClone = new Hand(mHand->getWorld(), "Solution clone");
	mSolutionClone->cloneFrom(mRefHand);//CHANGED! was mHand - for some reason this makes setting transparency not tied to mHand??
    mSolutionClone->setTransparency(0.03);//Make the clone that shows the solutions slightly transparent so we can still see the object below it.
	mSolutionClone->showVirtualContacts(false);
	mSolutionClone->setRenderGeometry(true);
	//solution clone is always added to scene graph
	mHand->getWorld()->addRobot(mSolutionClone, true);
	mHand->getWorld()->toggleCollisions(false, mSolutionClone);
	mSolutionClone->setTran( mRefHand->getTran() );//CHANGED!  was mHand
}

void
OnLinePlanner::action(ActionType a)
{
	mInterface->action(a);
}

void
OnLinePlanner::useRealBarrettHand(bool s)
{
	mInterface->useRealBarrettHand(s);
}

ActionType
OnLinePlanner::getAction()
{
	return mInterface->getAction();
}

void
OnLinePlanner::showSolutionClone(bool s)
{
	if (s) {
		if (!mSolutionClone) createSolutionClone();
		else mHand->getWorld()->addElementToSceneGraph(mSolutionClone);
	} else {
		if (mSolutionClone) mHand->getWorld()->removeElementFromSceneGraph(mSolutionClone);
	}
}

void
OnLinePlanner::showClone(bool s)
{
	SimAnnPlanner::showClone(s);
}

bool
OnLinePlanner::resetPlanner()
{
	DBGA("Online planner reset");
	if (!mGraspTester->resetPlanner()) {
		DBGA("Failed to reset parallel tester!");
		return false;
	}
	while (!mCandidateList.empty()) {
		delete mCandidateList.front(); mCandidateList.pop_front();
	}
	if (!SimAnnPlanner::resetPlanner()) return false;
	if (mCurrentBest) delete mCurrentBest;
	mCurrentBest = new GraspPlanningState(mCurrentState);
	return true;
}

void OnLinePlanner::createProgressClone()
{

    if(mProgressClone) {
        DBGA("Solution clone exists already!");
        return;
    }

    mProgressClone = new Hand(mRefHand->getWorld(), "Progress clone");
    mProgressClone->cloneFrom(mRefHand);//CHANGED! was mHand - for some reason this makes setting transparency not tied to mHand??
    mProgressClone->setTransparency(0.95);//Make the clone that shows the solutions slightly transparent so we can still see the object below it.
    mProgressClone->showVirtualContacts(false);
    mProgressClone->setRenderGeometry(true);
    //solution clone is always added to scene graph
    World * w = mHand->getWorld();
    w->addRobot(mProgressClone, true);
    unsigned int numRob = w->getNumRobots();
    for (int r = 0; r < numRob; ++ r)
    {
        if (mProgressClone == w->getRobot(r))
            continue;
        w->toggleCollisions(false, mProgressClone, w->getRobot(r));
    }
    mProgressClone->getWorld()->toggleCollisions(true, mProgressClone);

    mProgressClone->setTran( mRefHand->getTran() );//CHANGED!  was mHand
}

void OnLinePlanner::createAndUseClone()
{
    SimAnnPlanner::createAndUseClone();    
    mHand->getWorld()->toggleCollisions(false, mGraspTester->getHand(), mHand);
    //so we can distinguish between the two clones
    mGraspTester->getHand()->setName( mGraspTester->getHand()->getName() + QString(" th") );//this hand is never put in scene graph, for behind the scenes stuff?
    mHand->setName( mHand->getName() + QString(" pl") );
    mHand->setTransparency(0.8);//Make the planner hand barely visible just so the user can see something is going on.

}

void OnLinePlanner::startThread()
{
    setState(INIT);
    if(!this->mMultiThread)
        SimAnnPlanner::startThread();
    mGraspTester->startPlanner();

    mRefHand->setTransparency(0.7);
    //this->mHand->moveToThread(this);
    //mHand->getWorld()->toggleCollisions(true, mHand);
    showClone(false);
    showSolutionClone(true);
    SimAnnPlanner::startPlanner();
    setRenderType(RENDER_ALWAYS);
    setState(RUNNING);
    DBGA("Started on-line planner");
}


void
OnLinePlanner::startPlanner()
{
    createAndUseClone();
    //mHand->getWorld()->toggleCollisions(true, mHand);
	DBGP("Starting on-line planner");
    mRefHand->setTransparency(0.7);
    SimAnnPlanner::startPlanner();
    mGraspTester->startPlanner();
    mGraspTester->setRenderType(RENDER_NEVER);
    showClone(true);
    showSolutionClone(true);

    DBGA("Started on-line planner");
}



void
OnLinePlanner::pausePlanner()
{
	mGraspTester->pausePlanner();
	msleep(1000);
	//if (mSolutionClone) mSolutionClone->breakContacts();
	SimAnnPlanner::pausePlanner();
	showClone(false);
}

double
OnLinePlanner::stateDistance(const GraspPlanningState *s1, const GraspPlanningState *s2)
{
	return distanceOutsideApproach(s1->getTotalTran(), s2->getTotalTran());
}

/*!	A helper function that gives the change between two transforms, but 
	disregards any change along the approach direction of the hand. I'm not
	really sure this is needed anymore, might be replaced in the future.
*/
double
OnLinePlanner::distanceOutsideApproach(const transf &solTran, const transf &handTran, bool useAlignment)
{
	double max_angle = M_PI / 4.0;
	double max_dist = 50.0;
	double f;
	//relative transform between the two
	transf changeTran = solTran * handTran.inverse();
    transf handApproach = mHand->getApproachTran() * handTran;
    transf solApproachTran = mHand->getApproachTran() * solTran;

    vec3 handApproachZ(handApproach.affine()[2], handApproach.affine()[5], handApproach.affine()[8]);
    vec3 solApproachZ(solApproachTran.affine()[2],solApproachTran.affine()[5], solApproachTran.affine()[8]);

	//DBGP("T1: " << solTran.translation());
	//DBGP("T2: " << handTran.translation());
	//DBGP("Change: " << changeTran.translation() );
	
	//get change in terms of approach direction
	changeTran = mHand->getApproachTran() * changeTran * mHand->getApproachTran().inverse();
  //double dotZ = (mHand->getApproachTran()*handTran).affine().row(2) * (mHand->getApproachTran()*solTran).affine().transpose().row(2);
	//get angular change
    double angle; vec3 axis;
	changeTran.rotation().ToAngleAxis(angle, axis);

	//get translation change
	vec3 approach = changeTran.translation();
	//change along approach direction does not count as distance
	//DBGP("Approach: " << approach);
	if (approach.z() < 0) {
		f = -1.0;
	} else {
		f = 1.0;
	}
  double angleMod = 1.0;

  if (handApproachZ%solApproachZ < 0)
    angleMod = 10.0;

	approach.z() = 0;
	double dist = approach.len();

	//compute final value
	if (angle > M_PI) angle -= 2*M_PI;
	if (angle < -M_PI) angle += 2*M_PI;
	angle = fabs(angle) / max_angle ;
	dist = dist / max_dist;
	//DBGP("Angle " << angle << "; dist " << dist << std::endl);
  double alignment_score = 1/angleMod;
  //if (useAlignment) alignment_score = .001+(axis.z() * angleMod);
  
    return f * std::max(angle, dist)/alignment_score;
}


void OnLinePlanner::setGraspAttribute(int i, const QString &attribute, double value)
{
    EGPlanner::setGraspAttribute(i, attribute, value);
    //this->updateSolutionList();
}

/*! Keeps the list of solutions sorted according to some metric */
void
OnLinePlanner::updateSolutionList()
{
  QMutexLocker lock(&mListAttributeMutex);
  
	transf stateTran, currentHandTran = mRefHand->getTran();

	std::list<GraspPlanningState*>::iterator it;
	//re-compute distance between current hand position and solutions. 
	for ( it = mBestList.begin(); it != mBestList.end(); it++ )	{
		stateTran = (*it)->getTotalTran();
		//compute distance between each solution and current hand position	
    double dist = distanceOutsideApproach(stateTran, currentHandTran, false);
		if (dist < 0) dist = -dist;        
    dist += 1000 * (1-(*it)->getAttribute("testResult"));    
		(*it)->setDistance(dist);
		if (mMarkSolutions) {
			if (dist<1) (*it)->setIVMarkerColor(1-dist, dist, 0);
			else (*it)->setIVMarkerColor(0 , 1, 1);
		}
	}
	
	//sort list according to distance from current hand position
	mBestList.sort(GraspPlanningState::compareStatesDistances);
	//keep only best in list
  std::list<GraspPlanningState *>::iterator it2 = mBestList.begin();
  
  for(int i = 0; it2 != mBestList.end();)
  {
      if ((*it2)->getAttribute("testResult") <= 0)
      break;
    
    if(i >= SOLUTION_BUFFER_SIZE)
    {
      delete *it2;
      std::list<GraspPlanningState *>::iterator it3 = it2;
      ++it2;
      mBestList.erase(it3);
    }
    else
      ++it2;
      
      ++i;
  }  
  
	while (mBestList.size() > 2*SOLUTION_BUFFER_SIZE) {
		delete mBestList.back();
		mBestList.pop_back();
	}
  
}

void 
OnLinePlanner::mainLoop()
{
	static clock_t lastCheck = clock();
	clock_t time = clock();
	double secs = (float)(time - lastCheck) / CLOCKS_PER_SEC;

	if (secs < 0.2) {
    if (secs < 0.1) {
		//perform grasp planning all the time
		graspLoop();
		return;
	}
	lastCheck = time;

	//every 0.2 seconds, perform the management part:

	//set as a reference transform for the search the transform of the reference hand (presumably controlled by
	//the user via a flock of birds)
	mCurrentState->setRefTran( mRefHand->getTran(), false );
	//this is to ensure this (potentially) illegal state does not make it into the best list
	mCurrentState->setLegal(false);
	//re-set the legal search range along the approach direction, so we don't search pointlessly inside the object
	if ( mCurrentState->getVariable("dist")) {
		Body *obj = mCurrentState->getObject();
		double maxDist = 200;
		mObjectDistance = mRefHand->getApproachDistance(obj,maxDist);
		if (mObjectDistance > maxDist) mObjectDistance = maxDist;
		mCurrentState->getPosition()->getVariable("dist")->setRange(-30,mObjectDistance);
		//make sure the current value is within range; otherwise simm ann can hang...
		mCurrentState->getPosition()->getVariable("dist")->setValue(mObjectDistance/2);
		mCurrentState->getPosition()->getVariable("dist")->setJump(0.33);
	}
	
	//is the planning part has produced new candidates, send them to the grasp tester
	std::list<GraspPlanningState*>::iterator it = mCandidateList.begin();
	while (it!=mCandidateList.end()) {
		//while there is space
		if ( mGraspTester->postCandidate(*it) ) {
			DBGP("Candidate posted");
			it = mCandidateList.erase(it);
		} else {
			DBGP("Tester thread buffer is full");
			break;
		}
	}

	//retrieve solutions from the tester
	GraspPlanningState *s;
    {
    QMutexLocker lock(&mListAttributeMutex);
	while ( (s = mGraspTester->popSolution()) != NULL ) {
		//hack - this is not ideal, but so far I don't have a better solution of how to keep track
		//of what hand is being used at what time
		s->changeHand( mRefHand, true );//CHANGED! to mSolutionClone from mRefHand
		mBestList.push_back(s);
    //graspItGUI->getIVmgr()->emitAnalyzeGrasp(s);
		if (mMarkSolutions) {
			mHand->getWorld()->getIVRoot()->addChild( s->getIVRoot() );
		}
	}
    }
	updateSolutionList();

	//now shape the real hand.
	//s = mInterface->updateHand(&mBestList);//this used to update the current hand to solutions that are found, no longer does this
//	if (mBestList.size() != 0) showGrasp(0);//CHANGED! this replaces line above
	//CHANGED! stuff below was taken out
//	if (s) {
//		if (mSolutionClone) s->execute(mSolutionClone);
//		if (mMarkSolutions) s->setIVMarkerColor(1,1,0);
//	}
    }
	DBGP("On-line main loop done");
   }



void
OnLinePlanner::graspLoop()
{
	//DBGP("Grasp loop started");
	//prepare input
	GraspPlanningState *input = NULL;
	if ( processInput() ) {
		input = mTargetState;
	}

	//call simulated annealing
	SimAnn::Result r = mSimAnn->iterate(mCurrentState,mEnergyCalculator,input);
	mCurrentStep = mSimAnn->getCurrentStep();

	if ( r == SimAnn::JUMP ) {
		assert(mCurrentState->isLegal());
		//we have a new state from the SimAnn
		if (mCurrentState->getEnergy() < 0 || mCurrentState->getEnergy() < mCurrentBest->getEnergy()) {
			DBGP("New candidate");
			GraspPlanningState *insertState = new GraspPlanningState(mCurrentState);
			//make solution independent of reference hand position
			insertState->setPositionType(SPACE_COMPLETE,true);
			insertState->setRefTran( mCurrentState->getObject()->getTran(), true);
			insertState->setItNumber( mCurrentStep );
			if (insertState->getEnergy() < mCurrentBest->getEnergy()) {
				mCurrentBest->copyFrom( insertState );
			}
			if (!addToListOfUniqueSolutions(insertState, &mCandidateList,0.4)) {
				DBGP("Similar to old candidate");
				delete insertState;
			} else {
				//graspItGUI->getIVmgr()->emitAnalyzeGrasp(mCandidateList.back());
				mCandidateList.sort(GraspPlanningState::compareStates);//CHANGED! was compareStates
				while (mCandidateList.size() > CANDIDATE_BUFFER_SIZE) {
					delete mCandidateList.back();
					mCandidateList.pop_back();
				}
			}
			DBGP("Added candidate");
		}
	}

    if (mCurrentStep % 500 == 0)
    {
        emit update();
        render(mProgressClone);
        //emit signalRender(this);        
    }



	//DBGP("Grasp loop done");
}




int
OnLinePlanner::getFCBufferSize()
{
	return mGraspTester->getNumCandidates();
}


//CHANGED! I added a special version of this to the OnLinePlanner so that it doesn't use the
// actual hand to show the results
void
OnLinePlanner::showGrasp(int i)
{
	assert (i>=0 && i<getListSize());
	const GraspPlanningState *s = getGrasp(i);
	s->execute(mSolutionClone);
	bool l; double e;
	mEnergyCalculator->analyzeCurrentPosture(s->getHand(), s->getObject(), l, e, false);
	DBGA("Re-computed energy: " << e);
}

void
OnLinePlanner::executeGrasp(int i)
{
	assert (i>=0 && i<getListSize());
//	mRefHand->getIVRoot()->removeChild(mRefHand->getIVRoot()->getByName("curveX"));
//	mRefHand->getIVRoot()->removeChild(mRefHand->getIVRoot()->getByName("curveZ"));
	const GraspPlanningState *s = getGrasp(i);
	s->execute(mRefHand);
	mRefHand->setTransparency(0);
	showSolutionClone(false);
	graspItGUI->getIVmgr()->emitProcessWorldPlanner(i);
}


