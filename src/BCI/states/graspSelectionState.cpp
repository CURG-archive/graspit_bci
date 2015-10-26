#include "BCI/states/graspSelectionState.h"
#include "BCI/bciService.h"
#include "BCI/onlinePlannerController.h"
#include "BCI/controller_scene/controller_scene_mgr.h"
#include "BCI/controller_scene/sprites.h"

using bci_experiment::OnlinePlannerController;


GraspSelectionState::GraspSelectionState(BCIControlWindow *_bciControlWindow, ControllerSceneManager *_csm, QState* parent):
    State("GraspSelectionState", parent),
    bciControlWindow(_bciControlWindow),
    csm(_csm)

{
    /* What should next do?

      It should update the online planner controller's idea of the *CURRENT* grasp

      This has two control consequences - If the grasp is sent to the robot, this is the
      grasp that is sent. If the planner is started, this grasp is used as approach
      direction of the seed.

      It also affects the visualization - This should change which grasp is shown by the demonstration hand
      and which grasp is shown most prominantly in any grasp preview pane.
    */

    addSelfTransition(BCIService::getInstance(),SIGNAL(next()), this, SLOT(onNext()));
    addSelfTransition(BCIService::getInstance(),SIGNAL(plannerUpdated()), this, SLOT(onPlannerUpdated()));
    connect(this, SIGNAL(entered()), OnlinePlannerController::getInstance(), SLOT(setPlannerToReady()));
    graspSelectionView = new GraspSelectionView(bciControlWindow->currentFrame);
    graspSelectionView->hide();

}


void GraspSelectionState::onEntry(QEvent *e)
{

    graspSelectionView->show();
    bciControlWindow->currentState->setText("Grasp Selection State");

    //loads grasps from the database
    //OnlinePlannerController::getInstance()->setPlannerToReady();
    //called so that view will show best grasp from database
    onPlannerUpdated();
    OnlinePlannerController::getInstance()->setPlannerToRunning();

    csm->clearTargets();

    Target *t1 = new Target(csm->control_scene_separator, QString("sprites/target_next.png"), 0.35, 0.25, 0.0);
    Target *t2 = new Target(csm->control_scene_separator, QString("sprites/target_next.png"), -1.1, 0.25, 0.0);
    Target *t3 = new Target(csm->control_scene_separator, QString("sprites/target_next.png"), -1.1, -1.0, 0.0);
    Target *t4 = new Target(csm->control_scene_separator, QString("sprites/target_next.png"), 0.35, -1.0, 0.0);

    QObject::connect(t1, SIGNAL(hit()), this, SLOT(onNext()));
    QObject::connect(t2, SIGNAL(hit()), this, SLOT(emit_goToActivateRefinementState()));
    QObject::connect(t3, SIGNAL(hit()), this, SLOT(emit_goToConfirmationState()));
    QObject::connect(t4, SIGNAL(hit()), this, SLOT(emit_goToObjectSelectionState()));

    csm->addTarget(t1);
    csm->addTarget(t2);
    csm->addTarget(t3);
    csm->addTarget(t4);
}


void GraspSelectionState::onExit(QEvent *e)
{
    graspSelectionView->hide();
    csm->clearTargets();
}

void GraspSelectionState::onNext()
{
    OnlinePlannerController::getInstance()->incrementGraspIndex();
}

void GraspSelectionState::onPlannerUpdated()
{
    const GraspPlanningState *bestGrasp = OnlinePlannerController::getInstance()->getGrasp(0);
    Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();

    if(bestGrasp)
    {
        graspSelectionView->showSelectedGrasp(hand,bestGrasp);
        QString graspID;
        bciControlWindow->currentState->setText("Planner Updated: " + graspID.setNum(bestGrasp->getAttribute("graspId")) );
    }
    OnlinePlannerController::getInstance()->analyzeNextGrasp();

}



