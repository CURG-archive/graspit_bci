#include "world.h"
#include "robot.h"
#include "searchState.h"
#include "BCI/worldElementTools.h"
#include "BCI/plannerTools.h"
#include "DBPlanner/db_manager.h"
#include "EGPlanner/egPlanner.h"


namespace bci_experiment{
namespace planner_tools{

bool testPreGraspCollisions(Hand * h, float pregrasp_dist)
{
  h->autoGrasp(false, -2.0, true);
  h->approachToContact(pregrasp_dist, false);
  return (getNumHandCollisions(h));
}


bool testGraspCollisions(Hand * h, const GraspPlanningState * s)
{
  bool result = false;
  std::vector<bool> currentCollisionState;
  resetHandCollisions(h, true, currentCollisionState);
  s->execute(h);
  World * w = getWorld();
  w->toggleCollisions(false, h, s->getObject());
  if(getNumHandCollisions(h))
    result = true;
  if(testPreGraspCollisions(h, -50.0))
    result = true;

  setCollisionState(h, currentCollisionState);
  return result;
}


int getNumHandCollisions(Hand * h)
{
  CollisionReport colReport;
  std::vector<Body *> body_list;
  h->getBodyList(&body_list);
  getWorld()->getCollisionReport(&colReport, &body_list);
  return colReport.size();
}

bool setCollisionState(Hand * h, std::vector<bool> & collisionStatus)
{
  World * w = getWorld();
  if(collisionStatus.size() != w->getNumGB() + 1)
    return false;

  Body * experiment_table = getOrAddExperimentTable();
  w->toggleCollisions(collisionStatus[0], h, experiment_table);
  for (int i = 0; i < w->getNumGB(); ++i)
  {
    w->toggleCollisions(collisionStatus[i+1], h, w->getGB(i));
  }
  return true;
}


void resetHandCollisions(Hand * h, bool setting,
                         std::vector<bool> & collisionStatus)
{
  World * w = getWorld();

  Body * experiment_table = getOrAddExperimentTable();
  collisionStatus.push_back(!w->collisionsAreOff(h, experiment_table));
  w->toggleCollisions(setting, h, experiment_table);

  for (int i = 0; i < w->getNumGB(); ++i)
  {
    collisionStatus.push_back(!w->collisionsAreOff(h, w->getGB(i)));
    w->toggleCollisions(setting, h, w->getGB(i));
  }
}



EGPlanner * getCurrentPlanner()
{
    getWorld()->getCurrentPlanner();
}


void sendString(const QString & s)
{
  emit graspItGUI->getIVmgr()->emitSendString(s);
}


void sendSetTarget(Body * b)
{
  QString setTargetString = "setTarget " + b->getName();
  sendString(setTargetString);
}


void importGraspsFromDBMgr( EGPlanner * mPlanner,
                            db_planner::DatabaseManager * mDbMgr)
{
    // Get corresponding model from database
    std::vector<db_planner::Model*> modelList;
    mDbMgr->ModelList(&modelList,db_planner::FilterList::USE_FILE_NAME,
      '/' + mHand->getGrasp()->getObject()->getFilename().split('/').back());


    // If no model can be found return
    if (modelList.size()==0)
    {
      std::cout << "No Models Found \n";
      return;
    }


    // Using the found model, retrieve the grasps
    std::vector<db_planner::Grasp*> grasps;
    mDbMgr->GetGrasps(*modelList[modelList.size()-1], GraspitDBGrasp::getHandDBName(mHand).toStdString(), &grasps);
    HandObjectState hs(mHand);
    hs.setPositionType(SPACE_COMPLETE);
    hs.setPostureType(POSE_DOF);
    hs.saveCurrentHandState();


    // Load the grasps into the grasp planner list.
    unsigned int numGrasps = std::min<unsigned int>(grasps.size(), 10);
    for (unsigned int gNum = 0; gNum < numGrasps; ++gNum)
      {
        GraspPlanningState *s = new GraspPlanningState(static_cast<GraspitDBGrasp *>
                               (grasps[gNum])->getFinalGraspPlanningState());

        s->setObject(mHand->getGrasp()->getObject());
        s->setRefTran(mHand->getGrasp()->getObject()->getTran());
        float testResult = -2*bci_experiment::testGraspCollisions(mHand, s);
        s->addAttribute("graspId", gNum);
        s->addAttribute("testResult", testResult);
        s->addAttribute("testTime", 0);
        //bci_experiment::printTestResult(*s);
        mPlanner->addSolution(s);
    }

}

}
}