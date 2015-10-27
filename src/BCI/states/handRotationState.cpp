#include "BCI/states/handRotationState.h"
#include "BCI/onlinePlannerController.h"
#include "BCI/bciService.h"


using bci_experiment::world_element_tools::getWorld;
using bci_experiment::OnlinePlannerController;
using bci_experiment::WorldController;


HandRotationState::HandRotationState(QString name , BCIControlWindow *_bciControlWindow, ControllerSceneManager *_csm, QState* parent):
      State(name, parent), bciControlWindow(_bciControlWindow), csm(_csm), rotationAllowed(true)
{
    rotateLatTransition = addSelfTransition(BCIService::getInstance(),SIGNAL(rotLat()), this, SLOT(onRotateHandLat()));
    rotateLongTransition = addSelfTransition(BCIService::getInstance(),SIGNAL(rotLong()), this, SLOT(onRotateHandLong()));
    addSelfTransition(this,SIGNAL(entered()), this, SLOT(onHandRotationStateEntry()));
    DBGA("HandRotationState");
}

void HandRotationState::setRotationAllowed(bool allowed)
{
    rotationAllowed = allowed;

    if(allowed)
    {
        if(!this->transitions().contains(rotateLatTransition))
            this->addStateTransition(rotateLatTransition);
        if(!this->transitions().contains(rotateLongTransition))
            this->addStateTransition(rotateLongTransition);

    }
    else
    {
        if(this->transitions().contains(rotateLatTransition))
            this->removeTransition(rotateLatTransition);
        if(this->transitions().contains(rotateLongTransition))
            this->removeTransition(rotateLongTransition);
    }
}


void HandRotationState::onRotateHandLong()
{
    if(rotationAllowed)
        OnlinePlannerController::getInstance()->rotateHandLong();
}

void HandRotationState::onRotateHandLat()
{
    if(rotationAllowed)
        OnlinePlannerController::getInstance()->rotateHandLat();
}

void HandRotationState::onHandRotationStateEntry()
{
    OnlinePlannerController::getInstance()->alignHand();
}
