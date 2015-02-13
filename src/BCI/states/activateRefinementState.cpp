#include "BCI/states/activateRefinementState.h"
#include "BCI/bciService.h"
#include <fstream>
#include "stdio.h"
#include "qobject.h"


using bci_experiment::OnlinePlannerController;
using bci_experiment::world_element_tools::getWorld;
using bci_experiment::WorldController;

ActivateRefinementState::ActivateRefinementState(BCIControlWindow *_bciControlWindow,QState* parent):
    HandRotationState("ActivateRefinementState",_bciControlWindow, parent)
{
    addSelfTransition(BCIService::getInstance(), SIGNAL(plannerUpdated()), this, SLOT(onPlannerUpdated()));
    addSelfTransition(BCIService::getInstance(), SIGNAL(next()), this, SLOT(nextGrasp()));
    addSelfTransition(BCIService::getInstance(),SIGNAL(rotLat()), this, SLOT(setTimerRunning()));
    addSelfTransition(BCIService::getInstance(),SIGNAL(rotLong()), this, SLOT(setTimerRunning()));

    activeRefinementView = new ActiveRefinementView(bciControlWindow->currentFrame);
    activeRefinementView->hide();

    double *jv0 = new double[ 8 ];
    double *jv1 = new double[ 8 ];

    double *p0 = new double[ 8 ];
    double *p1 = new double[ 8 ];

    for(int i=0; i < 7;i++){
        jv0[i] = .4;
        jv1[i] = 0;
        p0[i] = 0;
        p1[i] = 0;
    }

    p0[3] = 1;
    p1[3] = 1;

    poses.push_back(p0);
    poses.push_back(p1);
    jvs.push_back(jv0);
    jvs.push_back(jv1);

    grasp_index = 0;

}



void ActivateRefinementState::onEntry(QEvent *e)
{
    activeRefinementView->show();
    bciControlWindow->currentState->setText("Refinement State");    
    //OnlinePlannerController::getInstance()->setPlannerToRunning();
    //OnlinePlannerController::getInstance()->startTimedUpdate();
}

void ActivateRefinementState::setTimerRunning()
{
    if(!OnlinePlannerController::getInstance()->timedUpdateRunning)
        OnlinePlannerController::getInstance()->startTimedUpdate();
}

void ActivateRefinementState::onExit(QEvent *e)
{
    activeRefinementView->hide();
     OnlinePlannerController::getInstance()->setPlannerToPaused();
    OnlinePlannerController::getInstance()->stopTimedUpdate();
}


void ActivateRefinementState::nextGrasp(QEvent *e)
{
    //if(OnlinePlannerController::getInstance()->getNumGrasps())
    //{
        //const GraspPlanningState *nextGrasp = OnlinePlannerController::getInstance()->getGrasp(0);
        Body *b = world_element_tools::getWorld()->getBody(0);

        double *joint_values = new double[ 8 ];
        double *pose = new double[ 7 ];
        std::ostringstream filename;
        //filename << b->getName().toStdString() << "/graspExample.txt";
        filename <<  "graspExample.txt";

        std::cout << filename.str() << std::endl;
        std::ifstream iFile;
        iFile.open(filename.str());
        double value;
        if (iFile.is_open())
        {
            for(int j=0; j < 8; j++)
            {
                iFile >> value;
                joint_values[j] =  value;
            }

            for(int j=0; j < 7; j++)
            {
                iFile >> value;
                pose[j] =  value;
            }
        }
        else
        {
            std::cout << "Error reading heatmaps, file is not open" << std::endl;
        }
        iFile.close();


        Quaternion r = Quaternion(pose[3], pose[4], pose[5], pose[6]);
        vec3 t = vec3(pose[0], pose[1], pose[2]);
        transf tpos= transf(r,t);

        Hand *refHand = OnlinePlannerController::getInstance()->getRefHand();
        GraspPlanningState grasp = GraspPlanningState(refHand);
        grasp.setPositionType(SPACE_COMPLETE);

        //refHand->setJointValuesAndUpdate(joint_values);
        //refHand->updateDOFFromJoints(joint_values);
        //grasp.saveCurrentHandState();
        //grasp.setPostureType(SPACE_COMPLETE);
        grasp.mPosition->setTran(tpos);
        grasp.mPosture->storeHandDOF(joint_values);

        std::cout << "num_variables: " << grasp.mPosture->getNumVariables() << std::endl;


        grasp.execute(refHand);

        grasp_index ++;

    //}
}

void ActivateRefinementState::onPlannerUpdated(QEvent * e)
{

}


