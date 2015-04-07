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
}


void GraspSelectionState::onExit(QEvent *e)
{
    graspSelectionView->hide();
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
}

void GraspSelectionState::onRotateHandLat()
{
    HandRotationState::onRotateHandLat();
}

void GraspSelectionState::onRotateHandLong()
{
    HandRotationState::onRotateHandLong();
}



