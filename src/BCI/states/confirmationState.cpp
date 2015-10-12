
#include "BCI/states/confirmationState.h"
#include  <QSignalTransition>
#include "world.h"
#include "BCI/onlinePlannerController.h"
#include "BCI/bciService.h"

using bci_experiment::world_element_tools::getWorld;
using bci_experiment::OnlinePlannerController;

ConfirmationState::ConfirmationState(BCIControlWindow *_bciControlWindow,QState* parent):
        State("ConfirmationState", parent),bciControlWindow(_bciControlWindow)
{    
    confirmationView = new ConfirmationView(bciControlWindow->currentFrame);
    this->addSelfTransition(BCIService::getInstance(),SIGNAL(rotLat()), this, SLOT(onNextGrasp()));
    confirmationView->hide();
}


void ConfirmationState::onEntry(QEvent *e)
{
    const GraspPlanningState *grasp = OnlinePlannerController::getInstance()->getCurrentGrasp();
    Hand * hand = OnlinePlannerController::getInstance()->getGraspDemoHand();
    confirmationView->setCurrentGrasp(hand,grasp);
    confirmationView->show();
    bciControlWindow->currentState->setText("Confirmation");
    OnlinePlannerController::getInstance()->setPlannerToPaused();
}

void ConfirmationState::onNextGrasp(QEvent *e)
{

}

void ConfirmationState::onExit(QEvent * e)
{
    Q_UNUSED(e);
    confirmationView->hide();
}


