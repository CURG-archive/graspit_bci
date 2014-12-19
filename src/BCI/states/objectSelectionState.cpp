#include "BCI/states/objectSelectionState.h"
#include "BCI/bciService.h"
#include "BCI/state_views/objectSelectionView.h"
#include "graspitGUI.h"
#include <QGLWidget>

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
         onVisionFinished();
    }
    else
    {
        visionRunning = false;
        bciControlWindow->currentState->setText("Object Selection: Failed");
        onVisionFinished();
    }
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

    sendOptionChoice();
}

void ObjectSelectionState::respondOptionChoice(unsigned int option, double confidence, std::vector<double> interestLevel)
{
    DBGA("selecting object " << option << " with confidence " << confidence);

#warning THIS IS SUPER HACKY DONT USE THIS PLEASEEEEE
    DBGA("THIS IS SUPER HACKY DONT USE THIS PLEASEEEEE");

    OnlinePlannerController::getInstance()->incrementCurrentTarget();
    for (int i = 0; i < option; ++i) {
        GraspableBody *gb = OnlinePlannerController::getInstance()->incrementCurrentTarget();
        WorldController::getInstance()->highlightCurrentBody(gb);
    }
    onSelect();
}

void ObjectSelectionState::generateImageOptions(bool debug)
{
    for (unsigned int i = 0; i < imageOptions.size(); ++i)
    {
        delete imageOptions[i];
    }
    imageOptions.clear();
    imageDescriptions.clear();
    imageCosts.clear();

    stringOptions.push_back(QString("Rerun Object Detection"));

    generateStringImageOptions(debug);

    assert(stringOptions.size() == imageOptions.size());
    for (unsigned int i = 0; i < imageOptions.size(); ++i) {
        imageDescriptions.push_back(stringOptions[i]);
        imageCosts.push_back(0.5);
    }

    for(int i = 0; i < graspItGUI->getIVmgr()->getWorld()->getNumGB(); ++i)
    {
        GraspableBody *newTarget = OnlinePlannerController::getInstance()->incrementCurrentTarget();
        WorldController::getInstance()->highlightCurrentBody(newTarget);
        QString debugFileName="";
        if(debug)
            debugFileName=QString("img" + QString::number(imageOptions.size()) + ".png");
        QImage * img = graspItGUI->getIVmgr()->generateImage(NULL, debugFileName);

        imageOptions.push_back(img);
        imageCosts.push_back(.25);
        imageDescriptions.push_back(QString("Select target:") + newTarget->getName());
    }

    assert(imageOptions.size() == imageCosts.size());
    assert(imageOptions.size() == imageDescriptions.size());
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
