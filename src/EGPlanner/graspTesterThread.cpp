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
// $Id: graspTesterThread.cpp,v 1.15.2.1 2009/07/23 21:18:06 cmatei Exp $
//
//######################################################################

#include "world.h"
#include "body.h"
#include "robot.h"
#include "graspTesterThread.h"
#include "searchState.h"
#include "searchEnergy.h"
#include "grasp.h"

//#define GRASPITDBG
#include "debug.h"

GraspTester::GraspTester(Hand *h)
{
	mHand = h;
	init();
	mEnergyCalculator = new SearchEnergy();
	mEnergyCalculator->setType(ENERGY_STRICT_AUTOGRASP);
	mCurrentStep = 0; mMaxSteps = 1; //run forever
	mMaxCandidates = 20; mNumCandidates = 0;
}

GraspTester::~GraspTester()
{
	clearBuffers();
}

void
GraspTester::setEnergyType(SearchEnergyType)
{
	DBGA("Grasp tester only uses STRICT AUTPGRASP energy");
}

bool
GraspTester::resetPlanner()
{
	if (!EGPlanner::resetPlanner()) return false;
	clearBuffers();
	return true;
}

void
GraspTester::mainLoop()
{
	GraspPlanningState *s = popCandidate();

	if (!s) {
		DBGP("Empty buffer for tester");
		msleep(100);
		return;
	}
	s->changeHand(mHand,true);
	testGrasp(s);
	DBGP("TESTER: candidate has energy " << s->getEnergy());
	mHand->breakContacts();
	if (s->isLegal() && s->getEnergy() < -1.2) {
		//save the final grasping position that has resulted from autograsp
		s->setPositionType(SPACE_COMPLETE);
		s->setPostureType(POSE_DOF);
		//save the current transform with respect to the target object
    s->setRefTran(mHand->getGrasp()->getObject()->getTran());
		s->saveCurrentHandState();
		postSolution(s);
		DBGP("Tester posting a solution at iteration " << s->getItNumber());
	} else {
		DBGP("Tester removing candidate");
		delete s;
	}
}

GraspPlanningState*
GraspTester::popSolution()
{
	GraspPlanningState *s = NULL;
	mListMutex.lock();
	if ( !mSolutionList.empty() ) {
		s = mSolutionList.front();
		mSolutionList.pop_front();
	}
	mListMutex.unlock();
	return s;
}

void
GraspTester::postSolution(GraspPlanningState *s)
{
	mListMutex.lock();
	mSolutionList.push_back(s);
	mListMutex.unlock();
}

GraspPlanningState*
GraspTester::popCandidate()
{
	GraspPlanningState *s = NULL;
	mListMutex.lock();
	if ( !mCandidateList.empty() ) {
		s = mCandidateList.front();
		mCandidateList.pop_front();
	}
	mNumCandidates = (int)mCandidateList.size();
	mListMutex.unlock();
	return s;
}

bool
GraspTester::postCandidate(GraspPlanningState *s)
{
	mListMutex.lock();
	if ( (int)mCandidateList.size() > mMaxCandidates ) {
		DBGP("Tester buffer size: " << mCandidateList.size());
		mListMutex.unlock();
		return false;
	}
	mCandidateList.push_back(s);
	mNumCandidates = (int)mCandidateList.size();
	mListMutex.unlock();
	return true;
}

void
GraspTester::clearBuffers()
{
	std::list<GraspPlanningState*>::iterator it;
	mListMutex.lock();

	for (it = mCandidateList.begin(); it!= mCandidateList.end(); it++) {
		delete (*it);
	}
	mCandidateList.clear();
	mNumCandidates = 0;
	for (it = mSolutionList.begin(); it!= mSolutionList.end(); it++) {
		delete (*it);
	}
	mSolutionList.clear();
	mListMutex.unlock();
}

void
GraspTester::testGrasp(GraspPlanningState *s)
{
	bool legal; double energy;
	//test will leave the hand in the tested state, if it is legal!
	mEnergyCalculator->analyzeState(legal, energy, s, false);
	if (!legal) {
		DBGA("Illegal state in tester thread!");
		s->setLegal(false);
		DBGA("outputting collisions");

		s->execute();		
		CollisionReport colReport;
		std::vector<Body *> body_list;
		mHand->getBodyList(&body_list);
		mHand->getWorld()->getCollisionReport(&colReport, &body_list);
		CollisionReport::iterator it = colReport.begin();
		while(it!=colReport.end()) {
		  DBGA("Collision between " << it->first->getName().toStdString() << " and " << it->second->getName().toStdString() );
		  ++it;
		}
		return;
	}
    mHand->saveState();
    double pregrasp_dist = -50.0;
    mHand->autoGrasp(false, -2.0, true);
    transf newTran = translate_transf(vec3(0,0,pregrasp_dist) * mHand->getApproachTran()) * mHand->getTran();
    mHand->moveTo(newTran, 100*Contact::THRESHOLD, M_PI/36.0);
    //mHand->approachToContact(pregrasp_dist, false);
    CollisionReport colReport;
    std::vector<Body *> body_list;
    mHand->getBodyList(&body_list);
    mHand->getWorld()->getCollisionReport(&colReport, &body_list);
    if(colReport.size() || energy > -2.0)
    {
        s->setAttribute("testResult",-2);
        DBGP("Tester state failed energy: " << energy << " GraspID: " << s->getAttribute("graspId"));
    }
    else
    {
        s->setAttribute("testResult",0);
        DBGP("Tester succeeded! energy: " << energy << " GraspID: " << s->getAttribute("graspId"));
    }
	s->setEnergy(energy);
    mHand->restoreState();
}
