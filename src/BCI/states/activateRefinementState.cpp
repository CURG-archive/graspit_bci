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
    std::shared_ptr<Target>  t1 = std::shared_ptr<Target> (new Target(csm->control_scene_separator,
                                                                       QString("sprites/target_next.png"),
                                                                       0.35, .25, 0.0));
    std::shared_ptr<Target>  t2 = std::shared_ptr<Target> (new Target(csm->control_scene_separator,
                                                                       QString("sprites/rotateLat.png"),
                                                                       -1.1, 0.25, 0.0));
    std::shared_ptr<Target>  t3 = std::shared_ptr<Target> (new Target(csm->control_scene_separator,
                                                                       QString("sprites/rotateLong.png"),
                                                                        -1.1, -1.0, 0.0));
    std::shared_ptr<Target>  t4 = std::shared_ptr<Target> (new Target(csm->control_scene_separator,
                                                                       QString("sprites/target_confirm_grasp.png"),
                                                                        0.35, -1.0, 0.0));

    QObject::connect(t1.get(), SIGNAL(hit()), this, SLOT(nextGrasp()));
    QObject::connect(t2.get(), SIGNAL(hit()), this, SLOT(onRotateHandLat()));
    QObject::connect(t3.get(), SIGNAL(hit()), this, SLOT(onRotateHandLong()));
    QObject::connect(t4.get(), SIGNAL(hit()), this, SLOT(emit_goToConfirmationState()));

    csm->addTarget(t1);
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
    csm->clearTargets();
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
    csm->setCursorPosition(-1,0,0);
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
    updateView();
    OnlinePlannerController::getInstance()->analyzeNextGrasp();
}


