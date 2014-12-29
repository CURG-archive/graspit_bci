#include "BCI/states/activateRefinementState.h"
#include "BCI/bciService.h"

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
}



void ActivateRefinementState::onEntry(QEvent *e)
{
    activeRefinementView->show();
    bciControlWindow->currentState->setText("Refinement State");    
    OnlinePlannerController::getInstance()->setPlannerToRunning();
    OnlinePlannerController::getInstance()->startTimedUpdate();
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
    if(OnlinePlannerController::getInstance()->getNumGrasps())
    {
        const GraspPlanningState *nextGrasp = OnlinePlannerController::getInstance()->getGrasp(1);
        Hand *refHand = OnlinePlannerController::getInstance()->getRefHand();
        nextGrasp->execute(refHand);
        OnlinePlannerController::getInstance()->alignHand();
        //OnlinePlannerController::getInstance()->sortGrasps();

    }
}

void ActivateRefinementState::onPlannerUpdated(QEvent * e)
{
    OnlinePlannerController::getInstance()->sortGrasps();
    const GraspPlanningState *bestGrasp = OnlinePlannerController::getInstance()->getGrasp(0);    
    Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();
    const GraspPlanningState *nextGrasp = bestGrasp;    
    if(OnlinePlannerController::getInstance()->getNumGrasps())
    {
        nextGrasp = OnlinePlannerController::getInstance()->getGrasp(1);
    }

    if(bestGrasp)
    {
        activeRefinementView->showSelectedGrasp(hand,bestGrasp);
        QString graspID;
        bciControlWindow->currentState->setText("Refinement State - Grasp:" + graspID.setNum(bestGrasp->getAttribute("graspId")) );
    }

    if(nextGrasp)
    {
        activeRefinementView->showNextGrasp(hand, nextGrasp);
    }

    OnlinePlannerController::getInstance()->analyzeNextGrasp();
}


