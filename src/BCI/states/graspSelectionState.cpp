#include "BCI/states/graspSelectionState.h"
#include "BCI/bciService.h"
#include "BCI/onlinePlannerController.h"
#include "BCI/controller_scene/controller_scene_mgr.h"
#include "BCI/controller_scene/sprites.h"
#include <QPushButton>


using bci_experiment::OnlinePlannerController;


GraspSelectionState::GraspSelectionState(BCIControlWindow *_bciControlWindow, ControllerSceneManager *_csm, QState* parent):
    State("ObjectSelectionState", parent),
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

    addSelfTransition(OnlinePlannerController::getInstance()->currentPlanner,SIGNAL(update()), this, SLOT(onPlannerUpdated()));
    addSelfTransition(OnlinePlannerController::getInstance(),SIGNAL(render()), this, SLOT(onPlannerUpdated()));

    stateName = QString("Grasp Selection");
    graspSelectionView = new GraspSelectionView(bciControlWindow->currentFrame);
    graspSelectionView->hide();


    QTimer timer;
    QObject::connect(&timer, SIGNAL(timeout()), graspSelectionView, SLOT(incrementSelectedButton));
    timer.start(1000 / 30);

}

void GraspSelectionState::onEntry(QEvent *e)
{

    graspSelectionView->show();
    bciControlWindow->currentState->setText(stateName);
    //loads grasps from the database
    OnlinePlannerController::getInstance()->setPlannerToReady();
    OnlinePlannerController::getInstance()->analyzeNextGrasp();
    onPlannerUpdated();

    csm->clearTargets();

    std::shared_ptr<Target>  t1 = std::shared_ptr<Target> (new Target(csm->control_scene_separator,
                                                                       QString("sprites/target_background.png"),
                                                                       0.35,
                                                                       0.25,
                                                                       0.0, QString("Next\nGrasp")));

    std::shared_ptr<Target>  t2 = std::shared_ptr<Target> (new Target(csm->control_scene_separator,
                                                                       QString("sprites/target_background.png"),
                                                                       -1.1,
                                                                       0.25,
                                                                       0.0, QString("Refine\nGrasp")));
    std::shared_ptr<Target>  t3 = std::shared_ptr<Target> (new Target(csm->control_scene_separator,
                                                                       QString("sprites/target_background.png"),
                                                                       -1.1,
                                                                       -1.0,
                                                                       0.0, QString("Select\nGrasp")));

    std::shared_ptr<Target>  t4 = std::shared_ptr<Target> (new Target(csm->control_scene_separator,
                                                                       QString("sprites/target_background.png"),
                                                                       0.35,
                                                                       -1.0,
                                                                       0.0, QString("Go\nBack")));

    QObject::connect(t1.get(), SIGNAL(hit()), this, SLOT(onNext()));
    QObject::connect(t2.get(), SIGNAL(hit()), this, SLOT(emit_goToActivateRefinementState()));
    QObject::connect(t3.get(), SIGNAL(hit()), this, SLOT(emit_goToConfirmationState()));
    QObject::connect(t4.get(), SIGNAL(hit()), this, SLOT(emit_goToObjectSelectionState()));

    csm->addTarget(t1);
    csm->addTarget(t2);
    csm->addTarget(t3);
    csm->addTarget(t4);
}


void GraspSelectionState::onExit(QEvent *e)
{
    csm->clearTargets();
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
    qint64 minElapsedMSecs = 600;
    if(!activeTimer.isValid() || activeTimer.elapsed() >= minElapsedMSecs)
    {

        activeTimer.start();
        OnlinePlannerController::getInstance()->incrementGraspIndex();
        const GraspPlanningState * currentGrasp = OnlinePlannerController::getInstance()->getCurrentGrasp();
        Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();

        int next_grasp_index =OnlinePlannerController::getInstance()->currentGraspIndex + 1;
        if (next_grasp_index == OnlinePlannerController::getInstance()->getNumGrasps())
        {
            next_grasp_index = 0;
        }
        const GraspPlanningState *nextGrasp = OnlinePlannerController::getInstance()->getGrasp(next_grasp_index);
        if(nextGrasp)
        {
            graspSelectionView->showNextGrasp(hand, nextGrasp);
        }

        if(currentGrasp)
        {
            currentGrasp->execute(OnlinePlannerController::getInstance()->getRefHand());
            OnlinePlannerController::getInstance()->alignHand();
            graspSelectionView->showSelectedGrasp(hand, currentGrasp);
            QString graspID;
            bciControlWindow->currentState->setText(stateName +"- Grasp: " + graspID.setNum(currentGrasp->getAttribute("graspId")) );
        }

    }
    csm->setCursorPosition(-1,0,0);
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
    int next_grasp_index =OnlinePlannerController::getInstance()->currentGraspIndex + 1;
    if (next_grasp_index == OnlinePlannerController::getInstance()->getNumGrasps())
    {
        next_grasp_index = 0;
    }
    const GraspPlanningState *nextGrasp = OnlinePlannerController::getInstance()->getGrasp(next_grasp_index);
    if(nextGrasp)
    {
        graspSelectionView->showNextGrasp(hand, nextGrasp);
    }
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



