#include "BCI/states/graspSelectionState.h"
#include "BCI/bciService.h"
#include "BCI/onlinePlannerController.h"
#include <QPushButton>

using bci_experiment::OnlinePlannerController;


GraspSelectionState::GraspSelectionState(BCIControlWindow *_bciControlWindow, QState* parent):
    HandRotationState("GraspSelectionState",_bciControlWindow, parent)
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
    addSelfTransition(BCIService::getInstance(),SIGNAL(plannerUpdated()), this, SLOT(onPlannerUpdated()));
    connect(this, SIGNAL(entered()), OnlinePlannerController::getInstance(), SLOT(setPlannerToReady()));    
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
    //OnlinePlannerController::getInstance()->setPlannerToReady();
    //called so that view will show best grasp from database
    onPlannerUpdated();
}


void GraspSelectionState::onExit(QEvent *e)
{
    graspSelectionView->hide();
}

void GraspSelectionState::setNextButtonLabel(QString &label)
{
    QString name("buttonRefineGrasp");
    QPushButton * buttonRefineGrasp = graspSelectionView->findChild<QPushButton *>((name));
    buttonRefineGrasp->setText(label);
}



//Currently unused
void GraspSelectionState::onNext()
{
    OnlinePlannerController::getInstance()->incrementGraspIndex();
    const GraspPlanningState * currentGrasp = OnlinePlannerController::getInstance()->getCurrentGrasp();
    Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();
    OnlinePlannerController::getInstance()->stopTimedUpdate();
   if(currentGrasp)
   {
       currentGrasp->execute(OnlinePlannerController::getInstance()->getRefHand());
       OnlinePlannerController::getInstance()->alignHand();
       graspSelectionView->showSelectedGrasp(hand ,currentGrasp);
       QString graspID;
       bciControlWindow->currentState->setText(stateName +"- Grasp: " + graspID.setNum(currentGrasp->getAttribute("graspId")) );
   }

}

void GraspSelectionState::onPlannerUpdated()
{
    const GraspPlanningState *bestGrasp = OnlinePlannerController::getInstance()->getGrasp(0);
    Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();
    OnlinePlannerController::getInstance()->sortGrasps();

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



