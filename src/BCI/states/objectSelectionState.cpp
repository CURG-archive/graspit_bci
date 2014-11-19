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

void ObjectSelectionState::generateImageOptions(bool debug)
{
    imageOptions.clear();
    imageDescriptions.clear();
    imageCosts.clear();

    stringOptions.push_back(QString("Rerun Object Detection"));

    generateStringImageOptions(debug);
    imageDescriptions.push_back(stringOptions[0]);
    imageCosts.push_back(.5);

    for(int i = 0; i < graspItGUI->getIVmgr()->getWorld()->getNumGB(); ++i)
    {
        GraspableBody *newTarget = OnlinePlannerController::getInstance()->getCurrentTarget();
        WorldController::getInstance()->highlightCurrentBody(newTarget);
        OnlinePlannerController::getInstance()->emitRender();
        QGLWidget * glwidget = static_cast<QGLWidget *>(graspItGUI->getIVmgr()->getViewer()->getGLWidget());
        QImage fb = glwidget->grabFrameBuffer();
        QImage * img = new QImage(fb);
        imageOptions.push_back(img);
        imageCosts.push_back(.25);
        imageDescriptions.push_back(QString("Select target:") + newTarget->getName());
        if(debug)
            img->save(QString("img") + QString::number(imageOptions.size() - 1) + QString(".png"));
    }

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
