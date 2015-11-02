#include "BCI/states/objectSelectionState.h"
#include "BCI/bciService.h"
#include "BCI/state_views/objectSelectionView.h"

#include "BCI/controller_scene/controller_scene_mgr.h"
#include "BCI/controller_scene/sprites.h"

#include "graspitGUI.h"
#include <QGLWidget>


using bci_experiment::world_element_tools::getWorld;
using bci_experiment::OnlinePlannerController;
using bci_experiment::WorldController;


ObjectSelectionState::ObjectSelectionState(BCIControlWindow *_bciControlWindow, ControllerSceneManager *_csm,
                                           QState* parent):
    State("ObjectSelectionState", parent),
    bciControlWindow(_bciControlWindow),
    csm(_csm),
    visionRunning(false)
{
    objectSelectionView = new ObjectSelectionView(this,bciControlWindow->currentFrame);
    objectSelectionView->hide();
    this->addSelfTransition(getWorld(), SIGNAL(numElementsChanged()), this, SLOT(onNewObjectFound()));
    this->addSelfTransition(BCIService::getInstance(),SIGNAL(next()), this, SLOT(onRunVision()));
}


void ObjectSelectionState::onEntry(QEvent *e)
{
    objectSelectionView->show();

    WorldController::getInstance()->highlightAllBodies();
    GraspableBody *currentTarget = OnlinePlannerController::getInstance()->getCurrentTarget();

    BCIService::getInstance()->getCameraOrigin(NULL,NULL);
    //Don't draw guides in this phase
    OnlinePlannerController::getInstance()->stopTimedUpdate();
    OnlinePlannerController::getInstance()->destroyGuides();
    WorldController::getInstance()->highlightCurrentBody(currentTarget);    
    OnlinePlannerController::getInstance()->setSceneLocked(false);
    OnlinePlannerController::getInstance()->showRobots(false);
    if(BCIService::getInstance()->runObjectRetreival(this, SLOT(onVisionFinished())))
    {
        visionRunning = true;
        bciControlWindow->currentState->setText("Object Selection: Running Recognition");
        onVisionFinished();
        bciControlWindow->setBackgroundColor(QColor(255,0,0));
    }
    else
    {
        visionRunning = false;
        bciControlWindow->currentState->setText("Object Selection: Failed");
        onVisionFinished();
    }
    OnlinePlannerController::getInstance()->blockGraspAnalysis(false);

    csm->clearTargets();
    std::shared_ptr<Target>  t1 = std::shared_ptr<Target> (new Target(csm->control_scene_separator,
                                                                       QString("sprites/target_next.png"),
                                                                      -1.1, -1.0, 0.0));

    std::shared_ptr<Target>  t2 = std::shared_ptr<Target> (new Target(csm->control_scene_separator,
                                                                       QString("sprites/target_select.png"),
                                                                      0.35, -1.0, 0.0));

    QObject::connect(t1.get(), SIGNAL(hit()), this, SLOT(onNext()));
    QObject::connect(t2.get(), SIGNAL(hit()), this, SLOT(onSelect()));

    csm->addTarget(t1);
    csm->addTarget(t2);
}

void ObjectSelectionState::onRunVision(QEvent * e)
{
    if(!visionRunning)
    {
        if(BCIService::getInstance()->runObjectRecognition(this, SLOT(onVisionFinished())))
        {
            bciControlWindow->currentState->setText("Object Selection: Running Recognition");
            visionRunning = true;
            bciControlWindow->setBackgroundColor(QColor(255,0,0));        }
        else
            bciControlWindow->currentState->setText("Object Selection: Failed");
    }
}

void ObjectSelectionState::onVisionFinished()
{
    bciControlWindow->currentState->setText("Object Selection: Running Finished");
    graspItGUI->getIVmgr()->blinkBackground();
    visionRunning = false;
    bciControlWindow->setBackgroundColor(QColor(255,255,255));
    //sendOptionChoice();
}

void ObjectSelectionState::onExit(QEvent *e)
{
    bciControlWindow->setBackgroundColor(QColor(255,255,255));
    WorldController::getInstance()->unhighlightAllBodies();
    OnlinePlannerController::getInstance()->setSceneLocked(true);
    objectSelectionView->hide();

    OnlinePlannerController::getInstance()->showRobots(true);
    csm->clearTargets();
}


void ObjectSelectionState::onNext()
{
    static QTime activeTimer;
    qint64 minElapsedMSecs = 1200;
    if(!activeTimer.isValid() || activeTimer.elapsed() >= minElapsedMSecs)
    {

        activeTimer.start();
        GraspableBody *newTarget = OnlinePlannerController::getInstance()->incrementCurrentTarget();
        WorldController::getInstance()->highlightCurrentBody(newTarget);
    }
}


void ObjectSelectionState::onSelect()
{
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
