#include "BCI/states/activateRefinementState.h"
#include "BCI/bciService.h"
#include "BCI/controller_scene/controller_scene_mgr.h"
#include "BCI/controller_scene/sprites.h"

using bci_experiment::OnlinePlannerController;
using bci_experiment::world_element_tools::getWorld;
using bci_experiment::WorldController;

ActivateRefinementState::ActivateRefinementState(BCIControlWindow *_bciControlWindow, ControllerSceneManager *_csm, QState* parent):
    HandRotationState("ActivateRefinementState",_bciControlWindow, _csm, parent),
    csm(_csm)
{
    addSelfTransition(OnlinePlannerController::getInstance()->currentPlanner, SIGNAL(update()), this, SLOT(onPlannerUpdated()));
    addSelfTransition(BCIService::getInstance(), SIGNAL(next()), this, SLOT(nextGrasp()));
    addSelfTransition(OnlinePlannerController::getInstance(),SIGNAL(render()), this, SLOT(updateView()));

    //addSelfTransition(BCIService::getInstance(),SIGNAL(rotLat()), this, SLOT(setTimerRunning()));
    //addSelfTransition(BCIService::getInstance(),SIGNAL(rotLong()), this, SLOT(setTimerRunning()));

    activeRefinementView = new ActiveRefinementView(bciControlWindow->currentFrame);
    activeRefinementView->hide();
}



void ActivateRefinementState::onEntry(QEvent *e)
{
    activeRefinementView->show();

    //not sure I need this line or not.
    //onPlannerUpdated();
    bciControlWindow->currentState->setText("Refinement State");
    OnlinePlannerController::getInstance()->setPlannerToRunning();
    //OnlinePlannerController::getInstance()->startTimedUpdate();
    OnlinePlannerController::getInstance()->blockGraspAnalysis(false);

    csm->clearTargets();
    Target *t2 = new Target(csm->control_scene_separator, QString("sprites/rotateLat.png"), -1.1, 0.25, 0.0);
    Target *t3 = new Target(csm->control_scene_separator, QString("sprites/rotateLong.png"), -1.1, -1.0, 0.0);
    Target *t4 = new Target(csm->control_scene_separator, QString("sprites/target_next.png"), 0.35, -1.0, 0.0);

    QObject::connect(t2, SIGNAL(hit()), this, SLOT(onRotateHandLat()));
    QObject::connect(t3, SIGNAL(hit()), this, SLOT(onRotateHandLong()));
    QObject::connect(t4, SIGNAL(hit()), this, SLOT(emit_goToConfirmationState()));

    csm->addTarget(t2);
    csm->addTarget(t3);
    csm->addTarget(t4);

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
    OnlinePlannerController::getInstance()->blockGraspAnalysis(true);
}


void ActivateRefinementState::emit_goToConfirmationState()
{
    BCIService::getInstance()->emitGoToNextState1();
}


void ActivateRefinementState::nextGrasp(QEvent *e)
{
    if(OnlinePlannerController::getInstance()->getNumGrasps())
    {
        const GraspPlanningState *nextGrasp = OnlinePlannerController::getInstance()->getGrasp(1);
        Hand *refHand = OnlinePlannerController::getInstance()->getRefHand();
        nextGrasp->execute(refHand);
        OnlinePlannerController::getInstance()->alignHand();
        updateView();
    }
}

void ActivateRefinementState::updateView()
{
    OnlinePlannerController::getInstance()->sortGrasps();
    const GraspPlanningState *bestGrasp = OnlinePlannerController::getInstance()->getGrasp(0);
    Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();
    const GraspPlanningState *nextGrasp = bestGrasp;
    if(OnlinePlannerController::getInstance()->getNumGrasps())
    {
        nextGrasp = OnlinePlannerController::getInstance()->getGrasp(1);
    }

    if(nextGrasp)
    {
        activeRefinementView->showNextGrasp(hand, nextGrasp);
    }

    if(bestGrasp)
    {
        activeRefinementView->showSelectedGrasp(hand,bestGrasp);
        QString graspID;
        bciControlWindow->currentState->setText("Refinement State - Grasp:" + graspID.setNum(bestGrasp->getAttribute("graspId")) );
    }
    OnlinePlannerController::getInstance()->renderPending = false;

}

void ActivateRefinementState::onPlannerUpdated(QEvent * e)
{
    DBGA("ActivateRefinementState::onPlannerUpdated-- entered");
    updateView();
    OnlinePlannerController::getInstance()->analyzeNextGrasp();
}


