#include "BCI/states/graspSelectionState.h"
#include "BCI/bciService.h"
#include "BCI/onlinePlannerController.h"
#include "BCI/controller_scene/controller_scene_mgr.h"
#include "BCI/controller_scene/sprites.h"
#include <QPushButton>


using bci_experiment::OnlinePlannerController;


GraspSelectionState::GraspSelectionState(BCIControlWindow *_bciControlWindow, ControllerSceneManager *_csm, QState* parent):
    HandRotationState("GraspSelectionState",_bciControlWindow, _csm, parent)
{
    /* What should next do?

      It should update the online planner controller's idea of the *CURRENT* grasp

      This has two control consequences - If the grasp is sent to the robot, this is the
      grasp that is sent. If the planner is started, this grasp is used as approach
      direction of the seed.

      It also affects the visualization - This should change which grasp is shown by the demonstration hand
      and which grasp is shown most prominantly in any grasp preview pane.
    */

    //addSelfTransition(BCIService::getInstance(),SIGNAL(next()), this, SLOT(onNext()));
    addSelfTransition(OnlinePlannerController::getInstance()->currentPlanner,SIGNAL(update()), this, SLOT(onPlannerUpdated()));
    addSelfTransition(OnlinePlannerController::getInstance(),SIGNAL(render()), this, SLOT(onPlannerUpdated()));
    addSelfTransition(BCIService::getInstance(), SIGNAL(rotLat()), this, SLOT(onPlannerUpdated()));
    addSelfTransition(BCIService::getInstance(), SIGNAL(rotLong()), this, SLOT(onPlannerUpdated()));
    stateName = QString("Grasp Selection");
    graspSelectionView = new GraspSelectionView(bciControlWindow->currentFrame);
    graspSelectionView->hide();

}


void GraspSelectionState::onEntry(QEvent *e)
{

    graspSelectionView->show();
    bciControlWindow->currentState->setText(stateName);
    //loads grasps from the database
    OnlinePlannerController::getInstance()->setPlannerToReady();
    //called so that view will show best grasp from database
    //OnlinePlannerController::getInstance()->connectPlannerUpdate(true);
    OnlinePlannerController::getInstance()->analyzeNextGrasp();
    onPlannerUpdated();

    //not sure i want/need this
    //OnlinePlannerController::getInstance()->setPlannerToRunning();

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
    //OnlinePlannerController::getInstance()->connectPlannerUpdate(false);
}


bool GraspSelectionState::setButtonLabel(QString buttonName, QString label)
{
    QPushButton * button = graspSelectionView->findChild<QPushButton *>((buttonName));
    if(button)
    {
        button->setText(label);
        return true;
    }
    return false;
}



//Currently unused
void GraspSelectionState::onNext()
{
    static QTime activeTimer;
    qint64 minElapsedMSecs = 1200;
    if(!activeTimer.isValid() || activeTimer.elapsed() >= minElapsedMSecs)
    {

        activeTimer.start();
        OnlinePlannerController::getInstance()->incrementGraspIndex();
        const GraspPlanningState * currentGrasp = OnlinePlannerController::getInstance()->getCurrentGrasp();
        Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();

        if(currentGrasp)
        {
            currentGrasp->execute(OnlinePlannerController::getInstance()->getRefHand());
            OnlinePlannerController::getInstance()->alignHand();
            graspSelectionView->showSelectedGrasp(hand, currentGrasp);
            QString graspID;
            bciControlWindow->currentState->setText(stateName +"- Grasp: " + graspID.setNum(currentGrasp->getAttribute("graspId")) );
        }
    }

}

void GraspSelectionState::onPlannerUpdated()
{
    static QTime activeTimer;
    qint64 minElapsedMSecs = 300;
    if(!activeTimer.isValid() || activeTimer.elapsed() >= minElapsedMSecs)
    {
    DBGA("GraspSelectionState::onPlannerUpdated: " << this->name().toStdString());
    OnlinePlannerController::getInstance()->sortGrasps();
    OnlinePlannerController::getInstance()->resetGraspIndex();
    const GraspPlanningState *bestGrasp = OnlinePlannerController::getInstance()->getCurrentGrasp();
    Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();

    if(bestGrasp)
    {
        graspSelectionView->showSelectedGrasp(hand,bestGrasp);
        QString graspID;
        bciControlWindow->currentState->setText(stateName + ": Grasp: " + graspID.setNum(bestGrasp->getAttribute("graspId")) );
    }
    else
    {
        DBGA("GraspSelectionState::onPlannerUpdated::No best grasp found");
    }
    OnlinePlannerController::getInstance()->analyzeNextGrasp();
    }
    OnlinePlannerController::getInstance()->renderPending = false;
}

void GraspSelectionState::onRotateHandLat()
{
    HandRotationState::onRotateHandLat();
}

void GraspSelectionState::onRotateHandLong()
{
    HandRotationState::onRotateHandLong();
}



