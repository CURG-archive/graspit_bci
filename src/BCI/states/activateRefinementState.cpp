#include "BCI/states/activateRefinementState.h"

#include <QGLWidget>

using bci_experiment::OnlinePlannerController;
using bci_experiment::world_element_tools::getWorld;
using bci_experiment::WorldController;

ActivateRefinementState::ActivateRefinementState(BCIControlWindow *_bciControlWindow, QState *parent) :
        HandRotationState("ActivateRefinementState", _bciControlWindow, parent) {
    addSelfTransition(BCIService::getInstance(), SIGNAL(plannerUpdated()), this, SLOT(onPlannerUpdated()));
    addSelfTransition(BCIService::getInstance(), SIGNAL(next()), this, SLOT(nextGrasp()));
    addSelfTransition(OnlinePlannerController::getInstance(),SIGNAL(render()), this, SLOT(updateView()));

    //addSelfTransition(BCIService::getInstance(),SIGNAL(rotLat()), this, SLOT(setTimerRunning()));
    //addSelfTransition(BCIService::getInstance(),SIGNAL(rotLong()), this, SLOT(setTimerRunning()));

    activeRefinementView = new ActiveRefinementView(bciControlWindow->currentFrame);
    activeRefinementView->hide();

    choiceTimer = new QTimer(this);
    choiceTimer->setSingleShot(true);
}

ActivateRefinementState::~ActivateRefinementState() {
    disconnectChoiceTimer();
    delete choiceTimer;
}

void ActivateRefinementState::connectChoiceTimer() {
    connect(choiceTimer, SIGNAL(timeout()), this, SLOT(checkSendOptionChoice()));
    choiceTimer->start(5000);
}

void ActivateRefinementState::disconnectChoiceTimer() {
    choiceTimer->stop();
    disconnect(choiceTimer, SIGNAL(timeout()), this, SLOT(checkSendOptionChoice()));
}

void ActivateRefinementState::checkSendOptionChoice() {
    OnlinePlannerController *ctrl = OnlinePlannerController::getInstance();

    ctrl->sortGrasps();
    DBGA("Found " << ctrl->getNumGrasps() << " grasps");

    for (int i = 0; i < ctrl->getNumGrasps(); ++i) {
        GraspPlanningState *grasp = new GraspPlanningState(ctrl->getGrasp(i));
        if (grasp->getAttribute("testResult") < 0) {
            DBGA("Skipping grasp " << i << " because it has score " << grasp->getAttribute("testResult"));
            continue;
        }

        sendOptionChoice();
        return;
    }
    choiceTimer->start(2500);
    DBGA("Rescheduling!");
}


void ActivateRefinementState::onEntry(QEvent *e) {
    activeRefinementView->show();
    bciControlWindow->currentState->setText("Refinement State");
    OnlinePlannerController::getInstance()->setPlannerToRunning();
    //OnlinePlannerController::getInstance()->startTimedUpdate();
	OnlinePlannerController::getInstance()->blockGraspAnalysis(false);
    connectChoiceTimer();
}

void ActivateRefinementState::setTimerRunning() {
    if (!OnlinePlannerController::getInstance()->timedUpdateRunning)
        OnlinePlannerController::getInstance()->startTimedUpdate();
}

void ActivateRefinementState::onExit(QEvent *e) {
    activeRefinementView->hide();
    disconnectChoiceTimer();
    OnlinePlannerController::getInstance()->setPlannerToPaused();
    OnlinePlannerController::getInstance()->stopTimedUpdate();
    OnlinePlannerController::getInstance()->blockGraspAnalysis(true);
}


void ActivateRefinementState::nextGrasp(QEvent *e) {
    if (OnlinePlannerController::getInstance()->getNumGrasps()) {
        const GraspPlanningState *nextGrasp = OnlinePlannerController::getInstance()->getGrasp(1);
        Hand *refHand = OnlinePlannerController::getInstance()->getRefHand();
        nextGrasp->execute(refHand);
        OnlinePlannerController::getInstance()->alignHand();
        updateView();

    }
}

void ActivateRefinementState::updateView()
{
    OnlinePlannerController::getInstance()->sortGrasps();
    const GraspPlanningState *bestGrasp = OnlinePlannerController::getInstance()->getGrasp(0);
    Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();
    const GraspPlanningState *nextGrasp = bestGrasp;
    if(OnlinePlannerController::getInstance()->getNumGrasps())
    {
        nextGrasp = OnlinePlannerController::getInstance()->getGrasp(1);
    }

    if(nextGrasp)
    {
        activeRefinementView->showNextGrasp(hand, nextGrasp);
    }

    if(bestGrasp)
    {
        activeRefinementView->showSelectedGrasp(hand,bestGrasp);
        QString graspID;
        bciControlWindow->currentState->setText("Refinement State - Grasp:" + graspID.setNum(bestGrasp->getAttribute("graspId")) );
    }
    OnlinePlannerController::getInstance()->renderPending = false;

}

void ActivateRefinementState::onPlannerUpdated(QEvent * e)
{
    DBGA("ActivateRefinementState::onPlannerUpdated-- entered");
    updateView();
    OnlinePlannerController::getInstance()->analyzeNextGrasp();
}

//!*******************************************
//! Option Choice Paradigm related functions *
//!*******************************************

void ActivateRefinementState::generateImageOptions(bool debug) {
    for (std::vector<QImage *>::iterator it = imageOptions.begin(); it != imageOptions.end(); ++it) {
        delete *it;
    }

    imageOptions.clear();
    imageDescriptions.clear();
    imageCosts.clear();
    stringOptions.clear();
    sentChoices.clear();

    OnlinePlannerController *ctrl = OnlinePlannerController::getInstance();
    Hand *hand = ctrl->getGraspDemoHand();

    ctrl->sortGrasps();
    DBGA("Found " << ctrl->getNumGrasps() << " grasps");

    for (int i = 0; i < ctrl->getNumGrasps(); ++i) {
        const GraspPlanningState *grasp = ctrl->getGrasp(i);
        if (grasp->getAttribute("testResult") < 0) {
            DBGA("Skipping grasp " << i << " because it has score " << grasp->getAttribute("testResult"));
            continue;
        }
        sentChoices.push_back(grasp->getAttribute("graspId"));

        activeRefinementView->showSelectedGrasp(hand, grasp);

        ctrl->emitRender();


        QString debugFileName = "";
        if (debug) {
            debugFileName = QString("active_refinement_img" + QString::number(imageOptions.size()) + ".png");
        }

        QImage *img = graspItGUI->getIVmgr()->generateImage(activeRefinementView->getHandView()->getIVRoot(), debugFileName);

        if (imageOptions.size() == 0 && grasp->getAttribute("testResult") > 0) {
            ui_tools::setQImageBGColor(img, qRgb(0, 0, 255));
            if (debug) {
                img->save(debugFileName);
            }
        }

        imageOptions.push_back(img);
        imageCosts.push_back(0.25);
        imageDescriptions.push_back(QString("GraspID: ") + QString::number(grasp->getAttribute("graspId")));
    }

    if (imageOptions.size() == 0) {
        DBGA("Rescheduling!");
        choiceTimer->start(2500);
        return;
    } else {
        const static int MIN_IMAGES = 9;

        if (ctrl->getCurrentGraspIfExists() && imageOptions.size() < MIN_IMAGES) {
            QString debugFileName = "";
            if (debug) {
                debugFileName = QString("active_refinement_img_distractor.png");
            }

            const GraspPlanningState * currentGrasp = ctrl->getCurrentGraspIfExists();
            activeRefinementView->showSelectedGrasp(hand, currentGrasp);
            while (imageOptions.size() < MIN_IMAGES) {
                QImage *img = graspItGUI->getIVmgr()->generateImage(activeRefinementView->getHandView()->getIVObjectRoot(), debugFileName);
                imageOptions.push_back(img);
                imageCosts.push_back(0);
                imageDescriptions.push_back(QString("Distractor"));
            }
        }
    }
    activeRefinementView->showSelectedGrasp(hand, ctrl->getCurrentGraspIfExists());
    choicesValid = true;
}

void ActivateRefinementState::respondOptionChoice(unsigned int option,
        float confidence,
        std::vector<float> interestLevel) {
    if (!choicesValid) {
        DBGA("Respond received before choices!");
        return;
    }
    choicesValid = false;

    if (option >= stringOptions.size() + sentChoices.size()) {
        sendOptionChoice();
        return;
    }

    double graspid = sentChoices[option - stringOptions.size()];

    OnlinePlannerController *ctrl = OnlinePlannerController::getInstance();

    const GraspPlanningState *st = ctrl->getGraspByGraspId(graspid);

    if (!st) {
        DBGA("Grasp not found in current grasps!!");
        DBGA("Rescheduling!");
        choiceTimer->start(2500);
        return;
    } else {
        const GraspPlanningState *grasp = ctrl->setCurrentGraspId(graspid);
        assert(grasp->getAttribute("graspId") == graspid);

        DBGA("Current grasp selected");
        Hand *refHand = OnlinePlannerController::getInstance()->getRefHand();
        grasp->execute(refHand);
        ctrl->alignHand();

        bool was_blue_grasp = (option - stringOptions.size()) == 0;
        if (was_blue_grasp && grasp->getAttribute("testResult") > 0) {
            BCIService::getInstance()->emitGoToNextState1();
        } else {
            DBGA("Rescheduling!");
            choiceTimer->start(2500);
        }
    }

}
