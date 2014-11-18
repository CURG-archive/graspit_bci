#include "BCI/onlinePlannerController.h"

#include "BCI/bciService.h"
#include "debug.h"
#include "EGPlanner/graspTesterThread.h"

using bci_experiment::world_element_tools::getWorld;

namespace bci_experiment
{

    bool isMainThread(QObject * obj)
    {
        if(obj->thread() != GraspItGUI::getInstance()->getIVmgr()->thread())
        {
            DBGA("Object not in main thread");
            return false;
        }

        if(QThread::currentThread() != GraspItGUI::getInstance()->getIVmgr()->thread())
        {
            DBGA("Current thread is not main thread");
            return false;
        }
	return true;
    }

    void disableShowContacts()
    {
        for(int i = 0; i < getWorld()->getNumBodies(); ++i)
        {
            getWorld()->getBody(i)->showFrictionCones(false);
        }
    }

    QMutex OnlinePlannerController::createLock;
    OnlinePlannerController * OnlinePlannerController::onlinePlannerController = NULL;

    OnlinePlannerController* OnlinePlannerController::getInstance()
    {
        if(createLock.locked())
        {
            assert(0);
            return NULL;
        }
        QMutexLocker lock(&createLock);
        if(!onlinePlannerController)
        {            
            onlinePlannerController = new OnlinePlannerController();
	    //            assert(isMainThread(onlinePlannerController));
        }

        return onlinePlannerController;
    }



    OnlinePlannerController::OnlinePlannerController(QObject *parent) :
        QObject(parent),
        mDbMgr(NULL),
        currentTarget(NULL),
        currentGraspIndex(0),
        graspDemonstrationHand(NULL)
    {
        currentPlanner = planner_tools::createDefaultPlanner();
        connect(currentPlanner, SIGNAL(update()), this, SLOT(emitRender()));
    }


    bool OnlinePlannerController::analyzeApproachDir()
    {
        Hand * refHand(currentPlanner->getRefHand());
        GraspPlanningState * graspPlanningState = new GraspPlanningState(refHand);

        graspPlanningState->setPostureType(POSE_DOF, false);
        graspPlanningState->saveCurrentHandState();
        //graspItGUI->getIVmgr()->emitAnalyzeApproachDir(graspPlanningState);
        return true;
    }


    void
    OnlinePlannerController::plannerTimedUpdate()
    {
        float currentTime = QDateTime::currentDateTime().toTime_t();
        /* If there is a planner and the planner has found some solutions
        * do some tests.
        */
        if(currentPlanner->getListSize())
        {
            // Notify someone to analyze the current approach direction
            analyzeApproachDir();
            analyzeNextGrasp();
            // If the planner is itself not updating the order of the solution list
            if(!currentPlanner->isRunning())
            {
                currentPlanner->showGrasp(0);
                // We should trigger a redraw here somehow of the grasp views
                //updateResults(true, false);
            }
        }
        BCIService::getInstance()->onPlannerUpdated();
        if(timedUpdateRunning)
        {
            QTimer::singleShot(1000, this, SLOT(plannerTimedUpdate()));
        }
        DBGA("OnlinePlannercontroller::plannerTimedUpdate:: timeelapsed: " << currentTime - QDateTime::currentDateTime().toTime_t());
    }




    void OnlinePlannerController::initializeDbInterface()
    {

        if (!mDbMgr && currentPlanner->getHand())
        {
            GraspitDBModelAllocator *graspitDBModelAllocator = new GraspitDBModelAllocator();
            GraspitDBGraspAllocator *graspitDBGraspAllocator = new GraspitDBGraspAllocator(currentPlanner->getHand());

            mDbMgr = new db_planner::SqlDatabaseManager("tonga.cs.columbia.edu", 5432,
                              "postgres","roboticslab","armdb",graspitDBModelAllocator,graspitDBGraspAllocator);

          planner_tools::importGraspsFromDBMgr(currentPlanner, mDbMgr);
        }
    }




    void OnlinePlannerController::initializeTarget()
    {
        setAllowedPlanningCollisions();
        bool targetsOff = getWorld()->collisionsAreOff(currentPlanner->getHand(), currentPlanner->getHand()->getGrasp()->getObject());

        disableShowContacts();
        //start planner
        targetsOff = getWorld()->collisionsAreOff(currentPlanner->getHand(), currentPlanner->getHand()->getGrasp()->getObject());

        currentPlanner->resetPlanner();
        targetsOff = getWorld()->collisionsAreOff(currentPlanner->getHand(), currentPlanner->getHand()->getGrasp()->getObject());
        // Download grasps from database and load them in to the planner
        initializeDbInterface();
        targetsOff = getWorld()->collisionsAreOff(currentPlanner->getHand(), currentPlanner->getHand()->getGrasp()->getObject());
        currentPlanner->updateSolutionList();
        targetsOff = getWorld()->collisionsAreOff(currentPlanner->getHand(), currentPlanner->getHand()->getGrasp()->getObject());
        // Set the hand to it's highest ranked grasp
        if(currentPlanner->getListSize())
        {
            if(!currentPlanner->getGrasp(0)->execute(currentPlanner->getHand()))
                currentPlanner->getGrasp(0)->execute(currentPlanner->getRefHand());
            else
                currentPlanner->getRefHand()->setTran(currentPlanner->getHand()->getTran());
        }
        targetsOff = getWorld()->collisionsAreOff(currentPlanner->getHand(), currentPlanner->getHand()->getGrasp()->getObject());
        // Realign the hand with respect to the object, moving the hand back to its
        // pregrasp pose. Use the real hand because it's collisions are set appropriately

        //world_element_tools::realignHand(currentPlanner->getHand());

        //Now transfer that position to the reference hand.


    }



    bool OnlinePlannerController::hasRecognizedObjects()
    {
       return world_element_tools::getWorld()->getNumGB();
    }


    GraspableBody* OnlinePlannerController::getCurrentTarget()
    {
        if(!currentTarget && getWorld()->getNumGB())
        {
            setCurrentTarget(getWorld()->getGB(0));
        }
        return currentTarget;
    }

    GraspableBody* OnlinePlannerController::incrementCurrentTarget()
    {
        GraspableBody *newTarget = world_element_tools::getNextGraspableBody(currentTarget);
        setCurrentTarget(newTarget);
        return currentTarget;
    }

    void OnlinePlannerController::setCurrentTarget(GraspableBody *gb)
    {
        if(currentTarget && currentTarget != gb)
            disconnect(currentTarget, SIGNAL(destroyed()),this,SLOT(targetRemoved()));

        if(gb)
        {
            currentTarget = gb;

            currentPlanner->getTargetState()->setObject(currentTarget);
            currentPlanner->setModelState(currentPlanner->getTargetState());


            currentPlanner->getRefHand()->getGrasp()->setObjectNoUpdate(currentTarget);
            currentPlanner->getHand()->getGrasp()->setObjectNoUpdate(currentTarget);
            OnlinePlannerController::getGraspDemoHand()->getGrasp()->setObjectNoUpdate(currentTarget);
            currentPlanner->getGraspTester()->getHand()->getGrasp()->setObjectNoUpdate(currentTarget);


            connect(currentTarget, SIGNAL(destroyed()), this, SLOT(targetRemoved()));
        }

    }


    void OnlinePlannerController::drawGuides()
    {
        if(!getHand() || !getCurrentTarget())
            DBGA("OnlinePlannerController::drawGuides::Error - Tried to draw guides with no hand or no target");

        ui_tools::updateCircularGuides(currentPlanner->getRefHand(), getCurrentTarget());
    }

    void OnlinePlannerController::destroyGuides()
    {
        ui_tools::destroyGuideSeparator();
    }

    void OnlinePlannerController::alignHand()
    {
        if(!getHand() || !getCurrentTarget())
            DBGA("OnlinePlannerController::alignHand::Error - Tried to align hand with no hand or no target");
        Hand * alignedHand = getHand();
        if(currentPlanner)
            alignedHand = currentPlanner->getRefHand();
        world_element_tools::realignHand(alignedHand);
        drawGuides();
    }

    bool OnlinePlannerController::setPlannerToStopped()
    {
        currentPlanner->stopPlanner();
        return true;
    }

    bool OnlinePlannerController::setPlannerToPaused()
    {
        currentPlanner->pausePlanner();
        return true;
    }

    bool OnlinePlannerController::setPlannerTargets()
    {
        // Set the target for the planner state
          currentPlanner->getTargetState()->setObject(currentTarget);
          currentPlanner->setModelState(currentPlanner->getTargetState());


          currentPlanner->getRefHand()->getGrasp()->setObjectNoUpdate(currentTarget);
          currentPlanner->getHand()->getGrasp()->setObjectNoUpdate(currentTarget);
          OnlinePlannerController::getGraspDemoHand()->getGrasp()->setObjectNoUpdate(currentTarget);
          currentPlanner->getGraspTester()->getHand()->getGrasp()->setObjectNoUpdate(currentTarget);


    }

    bool OnlinePlannerController::setAllowedPlanningCollisions()
    {

        world_element_tools::disableTableObjectCollisions();
        //world_element_tools::setNontargetCollisions(currentPlanner->getRefHand(), currentTarget, false);
        getWorld()->toggleCollisions(false, currentPlanner->getRefHand());
        world_element_tools::setNonLinkCollisions(currentPlanner->getHand(), true);
        world_element_tools::setNonLinkCollisions(currentPlanner->getGraspTester()->getHand(), true);
        //world_element_tools::setNonLinkCollisions(getGraspDemoHand(), true);
	return true;
    }


    bool OnlinePlannerController::setPlannerToRunning()
    {
        if(currentTarget != currentPlanner->getTargetState()->getObject() ||
                currentPlanner->getState() == INIT)
            setPlannerToReady();

        if(currentPlanner->getState()==READY)
        {
            currentPlanner->startThread();
            plannerTimedUpdate();
        }
        return true;
    }

    //puts planner in ready state
    bool OnlinePlannerController::setPlannerToReady()
    {
        if(currentTarget && (!mDbMgr || currentPlanner->getState() != READY || currentTarget != currentPlanner->getHand()->getGrasp()->getObject()))
        {
            initializeTarget();
            bool targetsOff = getWorld()->collisionsAreOff(currentPlanner->getHand(), currentPlanner->getHand()->getGrasp()->getObject());
            plannerTimedUpdate();
        }
      else{
	DBGA("OnlinePlannerController::setPlannerToReady: ERROR Attempted to set planner to ready without valid target");
      }

        return true;
    }




    void OnlinePlannerController::rotateHandLong()
    {

        float stepSize = M_PI/100.0;
        transf robotTran = currentPlanner->getRefHand()->getTran();
        transf objectTran = currentTarget->getTran();


        transf rotationTrans = (robotTran * objectTran.inverse()) * transf(Quaternion(stepSize, vec3::Z), vec3(0,0,0));
        transf newTran = rotationTrans *  objectTran;
        currentPlanner->getRefHand()->moveTo(newTran, WorldElement::ONE_STEP, WorldElement::ONE_STEP);
        drawGuides();
    }

    void OnlinePlannerController::rotateHandLat()
    {
        float stepSize = M_PI/100.0;

        transf robotTran = currentPlanner->getRefHand()->getTran();
        transf objectTran = currentTarget->getTran();


        transf rotationTrans = (robotTran * objectTran.inverse()) * transf(Quaternion(stepSize, vec3::X), vec3(0,0,0));
        transf newTran = rotationTrans *  objectTran;
        currentPlanner->getRefHand()->moveTo(newTran, WorldElement::ONE_STEP, WorldElement::ONE_STEP);
        drawGuides();
    }

    void OnlinePlannerController::incrementGraspIndex()
    {
        currentGraspIndex = (currentGraspIndex + 1)%(currentPlanner->getListSize());
    }

    Hand * OnlinePlannerController::getRefHand()
    {
        return currentPlanner->getRefHand();
    }


    Hand * OnlinePlannerController::getHand()
    {
        return currentPlanner->getHand();
    }

    bool isCloneOf(Robot * r1, Robot * r2)
    {
        return r1->getBase()->getIVGeomRoot()->getChild(0) == r2->getBase()->getIVGeomRoot()->getChild(0);
    }

    Hand * OnlinePlannerController::getGraspDemoHand()
    {
        if (!currentPlanner)
        {
            DBGA("OnlinePlannerController::getGraspDemoHand:Attempted to get demonstration hand with no planner set");
            return NULL;
        }
        /*if(graspDemonstrationHand && !(isCloneOf(currentPlanner->getRefHand(), graspDemonstrationHand)))
        {
            getWorld()->destroyElement(graspDemonstrationHand);
            graspDemonstrationHand = NULL;
        }
        if(!graspDemonstrationHand)
        {
            graspDemonstrationHand = new Hand(getWorld(),currentPlanner->getRefHand()->getName() + "grasp_demo_hand");
            graspDemonstrationHand->cloneFrom(currentPlanner->getRefHand());
            getWorld()->removeElementFromSceneGraph(graspDemonstrationHand);
        }*/
        graspDemonstrationHand = static_cast<OnLinePlanner *>(currentPlanner)->getSolutionClone();
        return graspDemonstrationHand;

    }

    const GraspPlanningState * OnlinePlannerController::getGrasp(int index)
    {
        if(currentPlanner->getListSize() > 0 && index < currentPlanner->getListSize())
        {
            return currentPlanner->getGrasp(index);
        }
        return NULL;

    }

    unsigned int OnlinePlannerController::getNumGrasps()
    {
        if (currentPlanner)
            return currentPlanner->getListSize();
        return 0;
    }

    const GraspPlanningState * OnlinePlannerController::getCurrentGrasp()
    {
        return getGrasp(currentGraspIndex);
    }

    bool OnlinePlannerController::stopTimedUpdate()
    {
        timedUpdateRunning = false;
        return false;
    }

    bool OnlinePlannerController::startTimedUpdate()
    {
        timedUpdateRunning = true;
        plannerTimedUpdate();
        return true;
    }

    bool OnlinePlannerController::toggleTimedUpdate()
    {
        if(timedUpdateRunning)
            stopTimedUpdate();
        else
            startTimedUpdate();
        return timedUpdateRunning;
    }

    class MutexTryLocker {
      QMutex &m_;
      bool locked_;
    public:
      MutexTryLocker(QMutex &m) : m_(m), locked_(m.tryLock()) {}
      ~MutexTryLocker() { if (locked_) m_.unlock(); }
      bool isLocked() const { return locked_; }
      void unlock(){if (locked_) m_.unlock(); locked_ = false;}
    };

    void OnlinePlannerController::analyzeNextGrasp()
    {

        DBGA("Analyzing next grasp");
        //Planner exists
      if(!currentPlanner)
      {
          DBGA("OnlinePlannerController::analyzeNextGrasp:: Attempted to analyze grasp with no planner set");
          return;
      }


        // Lock planner's grasp list

      MutexTryLocker lock(currentPlanner->mListAttributeMutex);
      if(!lock.isLocked())
      {
          QTimer::singleShot(100, this, SLOT(analyzeNextGrasp()));
          return;
      }
      DBGA("Took the planner lock");

      //Check if any test is still pending
      //Go through all grasps
      //Ordering of grasps may have changed based on the demonstrated hand pose,
      //so we must examine all grasps to ensure that none of them are currently being evaluated.
      int firstUnevaluatedIndex = -1;
      float currentTime = QDateTime::currentDateTime().toTime_t();
      float expirationTime =  currentTime - 10;
      for(int i = 0; i < currentPlanner->getListSize(); ++i)
      {
          //If a grasp hasn't been evaluated
          const GraspPlanningState * gs = currentPlanner->getGrasp(i);
          if(gs->getAttribute("testResult") == 0.0)
          {
              if(firstUnevaluatedIndex < 0)
                  firstUnevaluatedIndex = i;
              //And an evaluation request was emitted for it less than some time ago
              if(gs->getAttribute("testTime") > expirationTime)
              {
                  DBGA("OnlinePlannerController::analyzeNextGrasp::Last attempt to analyze this grasp was too recent.");
                  //Don't emit another request to analyze.
                  return;
              }              
          }
      }
      if (firstUnevaluatedIndex < 0)
      {
          DBGA("OnlinePlannerController::analyzeNextGrasp::No unevaluated grasps to analyze");
          return;
      }

      const GraspPlanningState * graspToEvaluate = currentPlanner->getGrasp(firstUnevaluatedIndex);
      assert(graspToEvaluate->getAttribute("testResult") == 0.0);
      //Request analysis and ask to be called gain when analysis is completed.

      DBGA("Analyze Grasp time elapsed: " << currentTime -  QDateTime::currentDateTime().toTime_t());
      if(BCIService::getInstance()->checkGraspReachability(graspToEvaluate, this, SLOT(analyzeNextGrasp())))
        currentPlanner->setGraspAttribute(firstUnevaluatedIndex, "testTime",  QDateTime::currentDateTime().toTime_t());
      lock.unlock();

      DBGA("checkGraspReachability: " << currentTime -  QDateTime::currentDateTime().toTime_t());
    }

   void OnlinePlannerController::addToWorld(const QString model_filename, const QString object_name, const QString object_pose_string)
    {
        std::stringstream s;
        s << object_pose_string.toStdString();
        transf object_pose;
        s >> object_pose;

        QString body_file = QString(getenv("GRASPIT")) + "/" +  "models/objects/" + model_filename;
        Body *b = graspItGUI->getIVmgr()->getWorld()->importBody("GraspableBody", body_file);
        if(!b)
        {
            QString body_file = QString(getenv("GRASPIT")) + "/" +  "models/object_database/" + model_filename;
            b = graspItGUI->getIVmgr()->getWorld()->importBody("GraspableBody", body_file);
        }

        if(b)
        {
            b->setTran(object_pose);
            b->setName(object_name);
        }

    }

   void OnlinePlannerController::clearObjects()
   {
       while(getWorld()->getNumGB() > 0)
           getWorld()->destroyElement(getWorld()->getGB(0), true);

   }

   void OnlinePlannerController::targetRemoved()
   {
        currentTarget = NULL;
        getCurrentTarget();
   }

   void OnlinePlannerController::sortGrasps()
   {
       currentPlanner->updateSolutionList();
   }

}
