
#include "BCI/states/confirmationState.h"
#include  <QSignalTransition>
#include "world.h"
#include "BCI/onlinePlannerController.h"
#include "BCI/bciService.h"
#include "BCI/controller_scene/controller_scene_mgr.h"
#include "BCI/controller_scene/sprites.h"

using bci_experiment::world_element_tools::getWorld;
using bci_experiment::OnlinePlannerController;

ConfirmationState::ConfirmationState(BCIControlWindow *_bciControlWindow, ControllerSceneManager *_csm, QState* parent):
        State("ConfirmationState", parent),bciControlWindow(_bciControlWindow),
        csm(_csm)
{    
    confirmationView = new ConfirmationView(bciControlWindow->currentFrame);
    confirmationView->hide();
}


void ConfirmationState::onEntry(QEvent *e)
{
    const GraspPlanningState *grasp = OnlinePlannerController::getInstance()->getCurrentGrasp();
    Hand * hand = OnlinePlannerController::getInstance()->getGraspDemoHand();
    confirmationView->setCurrentGrasp(hand,grasp);
    confirmationView->show();
    bciControlWindow->currentState->setText("Confirmation State");
    OnlinePlannerController::getInstance()->setPlannerToPaused();

    csm->clearTargets();

    Target *t1 = new Target(csm->control_scene_separator, QString("sprites/target_next.png"), 0.35, 0.25, 0.0);
    Target *t2 = new Target(csm->control_scene_separator, QString("sprites/target_next.png"), -1.1, 0.25, 0.0);

    QObject::connect(t1, SIGNAL(hit()), this, SLOT(emit_goToExecutionState()));
    QObject::connect(t2, SIGNAL(hit()), this, SLOT(emit_goToPreviousState()));

    csm->addTarget(t1);
    csm->addTarget(t2);

}


void ConfirmationState::onExit(QEvent * e)
{
    Q_UNUSED(e);
    confirmationView->hide();
    csm->clearTargets();
}


