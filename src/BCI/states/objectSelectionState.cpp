#include "BCI/states/objectSelectionState.h"
#include "BCI/bciService.h"
#include "BCI/state_views/objectSelectionView.h"
#include "graspitGUI.h"

using bci_experiment::world_element_tools::getWorld;
using bci_experiment::OnlinePlannerController;
using bci_experiment::WorldController;


ObjectSelectionState::ObjectSelectionState(BCIControlWindow *_bciControlWindow,
                                           QState* parent):
    State("ObjectSelectionState", parent),
    bciControlWindow(_bciControlWindow),
    visionRunning(false)

{
    objectSelectionView = new ObjectSelectionView(this,bciControlWindow->currentFrame);
    objectSelectionView->hide();
    this->addSelfTransition(getWorld(), SIGNAL(numElementsChanged()), this, SLOT(onNewObjectFound()));
    this->addSelfTransition(BCIService::getInstance(),SIGNAL(rotLat()), this, SLOT(onRunVision()));

}


void ObjectSelectionState::onEntry(QEvent *e)
{
    objectSelectionView->show();
    WorldController::getInstance()->highlightAllBodies();
    GraspableBody *currentTarget = OnlinePlannerController::getInstance()->getCurrentTarget();
    BCIService::getInstance()->getCameraOrigin(NULL,NULL);

    WorldController::getInstance()->highlightCurrentBody(currentTarget);


    if(BCIService::getInstance()->runObjectRetreival(this, SLOT(onVisionFinished())))
    {
        visionRunning = true;
        bciControlWindow->currentState->setText("Object Selection: Running Recognition");
    }
    else
        bciControlWindow->currentState->setText("Object Selection: Failed");
}

void ObjectSelectionState::onRunVision(QEvent * e)
{

    if(!visionRunning)
    {
        if(BCIService::getInstance()->runObjectRecognition(this, SLOT(onVisionFinished())))
        {
            bciControlWindow->currentState->setText("Object Selection: Running Recognition");
            visionRunning = true;
        }
        else
            bciControlWindow->currentState->setText("Object Selection: Failed");
    }

}

void ObjectSelectionState::onVisionFinished()
{
    bciControlWindow->currentState->setText("Object Selection: Running Finished");
    graspItGUI->getIVmgr()->blinkBackground();
    visionRunning = false;
}

void ObjectSelectionState::onExit(QEvent *e)
{
    WorldController::getInstance()->unhighlightAllBodies();
    objectSelectionView->hide();
}


void ObjectSelectionState::onNext()
{
    GraspableBody *newTarget = OnlinePlannerController::getInstance()->incrementCurrentTarget();
    WorldController::getInstance()->highlightCurrentBody(newTarget);
}


void ObjectSelectionState::onSelect()
{
    //if (visionRunning)
    //{
     //   SbColor warnColor(1,.3,.3);
     //   graspItGUI->getIVmgr()->blinkBackground(30, 2, warnColor);
     //   return;
    //}
    BCIService::getInstance()->emitGoToNextState1();
}

void ObjectSelectionState::onNewObjectFound()
{
    GraspableBody *currentTarget = OnlinePlannerController::getInstance()->getCurrentTarget();

    if(currentTarget)
    {
        WorldController::getInstance()->highlightCurrentBody(currentTarget);
    }

}
