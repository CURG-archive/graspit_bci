#include "BCI/states/activateRefinementState.h"

#include <QGLWidget>
#include <QRegion>
#include <QPainter>

using bci_experiment::OnlinePlannerController;
using bci_experiment::world_element_tools::getWorld;
using bci_experiment::WorldController;

ActivateRefinementState::ActivateRefinementState(BCIControlWindow *_bciControlWindow, QState *parent) :
        HandRotationState("ActivateRefinementState", _bciControlWindow, parent) {
    addSelfTransition(BCIService::getInstance(), SIGNAL(plannerUpdated()), this, SLOT(onPlannerUpdated()));
    addSelfTransition(BCIService::getInstance(), SIGNAL(next()), this, SLOT(nextGrasp()));
    addSelfTransition(BCIService::getInstance(), SIGNAL(rotLat()), this, SLOT(setTimerRunning()));
    addSelfTransition(BCIService::getInstance(), SIGNAL(rotLong()), this, SLOT(setTimerRunning()));

    activeRefinementView = new ActiveRefinementView(bciControlWindow->currentFrame);
    activeRefinementView->hide();

    choiceTimer = new QTimer(this);
}

ActivateRefinementState::~ActivateRefinementState() {
    disconnectChoiceTimer();
    delete choiceTimer;
}

void ActivateRefinementState::connectChoiceTimer() {
    connect(choiceTimer, SIGNAL(timeout()), this, SLOT(sendOptionChoice()));
    choiceTimer->start(5000, true);
}

void ActivateRefinementState::disconnectChoiceTimer() {
    choiceTimer->stop();
    disconnect(choiceTimer, SIGNAL(timeout()), this, SLOT(sendOptionChoice()));
}


void ActivateRefinementState::onEntry(QEvent *e) {
    activeRefinementView->show();
    bciControlWindow->currentState->setText("Refinement State");
    OnlinePlannerController::getInstance()->setPlannerToRunning();
    OnlinePlannerController::getInstance()->startTimedUpdate();

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
}


void ActivateRefinementState::nextGrasp(QEvent *e) {
    if (OnlinePlannerController::getInstance()->getNumGrasps()) {
        const GraspPlanningState *nextGrasp = OnlinePlannerController::getInstance()->getGrasp(1);
        Hand *refHand = OnlinePlannerController::getInstance()->getRefHand();
        nextGrasp->execute(refHand);
        OnlinePlannerController::getInstance()->alignHand();
        //OnlinePlannerController::getInstance()->sortGrasps();

    }
}

void ActivateRefinementState::onPlannerUpdated(QEvent *e) {
    OnlinePlannerController::getInstance()->sortGrasps();
    const GraspPlanningState *bestGrasp = OnlinePlannerController::getInstance()->getGrasp(0);
    Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();
    const GraspPlanningState *nextGrasp = bestGrasp;
    if (OnlinePlannerController::getInstance()->getNumGrasps()) {
        nextGrasp = OnlinePlannerController::getInstance()->getGrasp(1);
    }

    if (bestGrasp) {
        activeRefinementView->showSelectedGrasp(hand, bestGrasp);
        QString graspID;
        bciControlWindow->currentState->setText("Refinement State - Grasp:" + graspID.setNum(bestGrasp->getAttribute("graspId")));
    }

    if (nextGrasp) {
        activeRefinementView->showNextGrasp(hand, nextGrasp);
    }

    OnlinePlannerController::getInstance()->analyzeNextGrasp();
}

//!*******************************************
//! Option Choice Paradigm related functions *
//!*******************************************

void ActivateRefinementState::generateImageOptions(bool debug) {
    for (std::vector<QImage *>::iterator it = imageOptions.begin(); it != imageOptions.end(); ++it) {
        delete *it;
    }
    for (std::vector<GraspPlanningState *>::iterator it = sentChoices.begin(); it != sentChoices.end(); ++it) {
        delete *it;
    }

    imageOptions.clear();
    imageDescriptions.clear();
    imageCosts.clear();
    sentChoices.clear();

    OnlinePlannerController * ctrl = OnlinePlannerController::getInstance();
    Hand * hand = ctrl->getGraspDemoHand();

    ctrl->sortGrasps();
    DBGA("Found " << ctrl->getNumGrasps() << " grasps");

    for (int i = 0; i < ctrl->getNumGrasps(); ++i) {
        GraspPlanningState * grasp = new GraspPlanningState(ctrl->getGrasp(i));
        if (grasp->getAttribute("testResult") < 0) {
            DBGA("Skipping grasp " << i << " because it has score " << grasp->getAttribute("testResult"));
            continue;
        }
        sentChoices.push_back(grasp);

        activeRefinementView->showSelectedGrasp(hand, grasp);

        ctrl->emitRender();

        QImage * img = activeRefinementView->getHandView()->getSnapShot();

        if (imageOptions.size() == 0) {
            // set the background for the first one to a different color
            QImage mask = img->createHeuristicMask(true);
            QColor maskColor;
            for (int y = 0; y < img->height(); ++y) {
                for (int x = 0; x < img->width(); ++x) {
                    maskColor = QColor(mask.pixel(x, y));
                    if (maskColor.black() == 0) {
                        img->setPixel(x, y, qRgb(100, 0, 100));
                    }
                }
            }
        }

        if (debug) {
            QString debugFileName = QString("active_refinement_img" + QString::number(imageOptions.size()) + ".png");
            img->save(debugFileName);
        }

        imageOptions.push_back(img);
        imageCosts.push_back(0.25);
        imageDescriptions.push_back(QString("GraspID: ") + QString::number(grasp->getAttribute("graspId")));
    }

    if (imageOptions.size() == 0) {
        DBGA("Rescheduling!");
        choiceTimer->start(2500);
        return;
    }
    activeRefinementView->showSelectedGrasp(hand, ctrl->getGrasp(0));
    choicesValid = true;
}

void ActivateRefinementState::respondOptionChoice(unsigned int option,
        float confidence,
        std::vector<float> &interestLevel) {
    if (!choicesValid) {
        DBGA("Respond received before choices!");
        return;
    }
    choicesValid = false;

    GraspPlanningState * grasp_opt = sentChoices[option - stringOptions.size()];

    double graspid = grasp_opt->getAttribute("graspId");

    OnlinePlannerController * ctrl = OnlinePlannerController::getInstance();

    int grasp_index = -1;

    for (int i = 0; i < ctrl->getNumGrasps(); ++i) {
        if (ctrl->getGrasp(i)->getAttribute("graspId") == graspid) {
            grasp_index = i;
            break;
        }
    }

    if (grasp_index < 0) {
        DBGA("Grasp not found in current grasps!!");
        DBGA("Rescheduling!");
        choiceTimer->start(2500);
        return;
    } else {
        const GraspPlanningState * grasp = ctrl->getGrasp(grasp_index);
        if (grasp_index == 0) {
            // picked the highlighted option!
            DBGA("Current grasp selected");
            Hand * refHand = OnlinePlannerController::getInstance()->getRefHand();
            grasp->execute(refHand);
            ctrl->alignHand();
            BCIService::getInstance()->emitGoToNextState1();
        } else {
            DBGA("Rescheduling!");
            choiceTimer->start(2500);
        }
    }

}
