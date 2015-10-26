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
    addSelfTransition(BCIService::getInstance(),SIGNAL(plannerUpdated()), this, SLOT(onPlannerUpdated()));
    connect(this, SIGNAL(entered()),OnlinePlannerController::getInstance(), SLOT(setPlannerToRunning()));
    connect(this, SIGNAL(exited()), OnlinePlannerController::getInstance(), SLOT(setPlannerToPaused()));

    activeRefinementView = new ActiveRefinementView(bciControlWindow->currentFrame);
    activeRefinementView->hide();
}


void ActivateRefinementState::onEntry(QEvent *e)
{
    activeRefinementView->show();
    bciControlWindow->currentState->setText("Active Refinement State");
    onPlannerUpdated();

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


void ActivateRefinementState::onExit(QEvent *e)
{
    activeRefinementView->hide();
}


void ActivateRefinementState::emit_goToConfirmationState()
{
    BCIService::getInstance()->emitGoToNextState1();
}


void ActivateRefinementState::onPlannerUpdated(QEvent * e)
{
    const GraspPlanningState *bestGrasp = OnlinePlannerController::getInstance()->getGrasp(0);
    Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();

    if(bestGrasp)
    {
        activeRefinementView->showSelectedGrasp(hand,bestGrasp);
        QString graspID;
        bciControlWindow->currentState->setText("Planner Updated: " + graspID.setNum(bestGrasp->getAttribute("graspId")) );
    }
    OnlinePlannerController::getInstance()->analyzeNextGrasp();
}


