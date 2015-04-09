#include "BCI/states/graspSelectionState.h"
#include <QPushButton>
#include <QGLWidget>

using bci_experiment::OnlinePlannerController;


GraspSelectionState::GraspSelectionState(BCIControlWindow *_bciControlWindow, QState *parent) :
        HandRotationState("GraspSelectionState", _bciControlWindow, parent) {
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


void GraspSelectionState::onEntry(QEvent *e) {
    choicesValid = true;

    graspSelectionView->show();
    bciControlWindow->currentState->setText(stateName);
    //loads grasps from the database
    OnlinePlannerController::getInstance()->setPlannerToReady();
    //called so that view will show best grasp from database
    //OnlinePlannerController::getInstance()->connectPlannerUpdate(true);
    OnlinePlannerController::getInstance()->analyzeNextGrasp();
    onPlannerUpdated();
}


void GraspSelectionState::onExit(QEvent *e) {
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
void GraspSelectionState::onNext() {
    static QTime activeTimer;
    qint64 minElapsedMSecs = 1200;
    if (!activeTimer.isValid() || activeTimer.elapsed() >= minElapsedMSecs) {

        activeTimer.start();
        OnlinePlannerController::getInstance()->incrementGraspIndex();
        const GraspPlanningState *currentGrasp = OnlinePlannerController::getInstance()->getCurrentGrasp();
        Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();

        if (currentGrasp) {
            currentGrasp->execute(OnlinePlannerController::getInstance()->getRefHand());
            OnlinePlannerController::getInstance()->alignHand();
            graspSelectionView->showSelectedGrasp(hand, currentGrasp);
            OnlinePlannerController::getInstance()->emitRender();
            QString graspID;
            bciControlWindow->currentState->setText(stateName + "- Grasp: " + graspID.setNum(currentGrasp->getAttribute("graspId")));
        }
    }

}

void GraspSelectionState::onPlannerUpdated()
{
    static QTime activeTimer;
    static const qint64 minElapsedMSecs = 300;
    if(!activeTimer.isValid() || activeTimer.elapsed() >= minElapsedMSecs)
    {
        DBGA("GraspSelectionState::onPlannerUpdated: " << this->name().toStdString());
        OnlinePlannerController::getInstance()->sortGrasps();
        OnlinePlannerController::getInstance()->resetGraspIndex();
        const GraspPlanningState *bestGrasp = OnlinePlannerController::getInstance()->getCurrentGrasp();
        Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();

        if (bestGrasp) {
            graspSelectionView->showSelectedGrasp(hand, bestGrasp);
            QString graspID;
            bciControlWindow->currentState->setText(stateName + ": Grasp: " + graspID.setNum(bestGrasp->getAttribute("graspId")));
        }
        else {
            DBGA("GraspSelectionState::onPlannerUpdated::No best grasp found");
        }
        OnlinePlannerController::getInstance()->analyzeNextGrasp();

        if (choiceReady()) {
            sendOptionChoice();
        }
        OnlinePlannerController::getInstance()->renderPending = false;
    }
}


void GraspSelectionState::onRotateHandLat() {
    HandRotationState::onRotateHandLat();
}

void GraspSelectionState::onRotateHandLong() {
    HandRotationState::onRotateHandLong();
}



//!*******************************************
//! Option Choice Paradigm related functions *
//!*******************************************


bool GraspSelectionState::choiceReady() {
    if (choicesValid && OnlinePlannerController::getInstance()->getNumGrasps()) {
        for (int i = 0; i < OnlinePlannerController::getInstance()->getNumGrasps(); ++i) {
            const GraspPlanningState *currentGrasp = OnlinePlannerController::getInstance()->getGrasp(i);
#if 0 // TODO: figure out what this is for
            if (currentGrasp->getAttribute("testResult") == 0.0) {}
                return false;
            }
#endif
        }
        choicesValid = false;
        return true;
    }
    else {
        if (choicesValid) {
            DBGA("No choice found");
        } else {
            DBGA("Choices already sent!");
        }
        return false;
    }

}

void GraspSelectionState::generateImageOptions(bool debug) {
    for (unsigned int i = 0; i < imageOptions.size(); ++i) {
        delete imageOptions[i];
    }

    DBGA("Generating image options");

    imageOptions.clear();
    imageDescriptions.clear();
    imageCosts.clear();
    sentChoices.clear();
    stringOptions.clear();
    std::vector<vec3> approachDirs;
    stringOptions.push_back(QString("Select Different Object"));

    OnlinePlannerController *ctrl = OnlinePlannerController::getInstance();
    Hand *hand = ctrl->getGraspDemoHand();

    ctrl->sortGrasps();
    generateStringImageOptions(debug);
    imageDescriptions.push_back(stringOptions[0]);
    //imageCosts.push_back(.5);
    approachDirs.push_back(vec3(0,0,0));
    while (!graspSelectionView->getHandView()) {
        DBGA("waiting for handview")
        sleep(1);
    }

    const GraspPlanningState *currentGrasp;
    for (int i = 0; i < ctrl->getNumGrasps(); ++i) {
        currentGrasp = ctrl->getGrasp(i);
        currentGrasp->execute(hand);
        graspSelectionView->showSelectedGrasp(hand, currentGrasp);

        QString debugFileName = "";

        if (debug) {
            debugFileName = QString("grasp_selection_img" + QString::number(imageOptions.size()) + ".png");
        }

        QImage *img = graspItGUI->getIVmgr()->generateImage(graspSelectionView->getHandView()->getIVRoot(), debugFileName);

        sentChoices.push_back(currentGrasp->getAttribute("graspId"));

        imageOptions.push_back(img);
	// Instead of explicitly setting the image cost, cache the approachDirs so that they can be calculated later. 
	approachDirs.push_back(currentGrasp->getTotalTran().affine().transpose().row(2));
        //imageCosts.push_back(.25);
        imageDescriptions.push_back(QString("GraspID: ") + QString::number(currentGrasp->getAttribute("graspId")));
    }

    const static int MIN_IMAGES = 9;

    if (ctrl->getCurrentGraspIfExists() && imageOptions.size() < MIN_IMAGES) {
        QString debugFileName = "";
        if (debug) {
            debugFileName = QString("grasp_selection_img_distractor.png");
        }

        currentGrasp = ctrl->getCurrentGraspIfExists();
        graspSelectionView->showSelectedGrasp(hand, currentGrasp);
        while (imageOptions.size() < MIN_IMAGES) {
            QImage *img = graspItGUI->getIVmgr()->generateImage(graspSelectionView->getHandView()->getIVObjectRoot(), debugFileName);
            imageOptions.push_back(img);
            //imageCosts.push_back(0);
	    approachDirs.push_back(vec3(0,0,0));
            imageDescriptions.push_back(QString("Distractor"));
        }
    }
    OnlinePlannerController::getInstance()->generateGraspSimilarity(approachDirs, imageCosts);
}

void GraspSelectionState::respondOptionChoice(unsigned int option, float confidence, std::vector<float> interestLevel) {
    const GraspPlanningState *currentGrasp = NULL;
    OnlinePlannerController *ctrl = OnlinePlannerController::getInstance();
    if (option == 0) {
        //Go back to object selection state.
        BCIService::getInstance()->emitGoToPreviousState();
        return;
    }

    for (int i = 0; i < OnlinePlannerController::getInstance()->getNumGrasps(); ++i) {
        // For testing purposes, if sentChoices are empty,use the index into the grasp list
        if (!sentChoices.size()) {
            currentGrasp = ctrl->getGrasp(option);
        }
        else {
            // Otherwise use the send choices
            double graspid = sentChoices[option - stringOptions.size()];
            currentGrasp = ctrl->getGraspByGraspId(graspid);
        }
    }
    if (!currentGrasp) {
        DBGA("GraspSelectionState::respondOptionChoice -- Failed to find chosen grasp in grasp list");
        sendOptionChoice();
        return;
    }

    ctrl->stopTimedUpdate();

    currentGrasp->execute(ctrl->getRefHand());
    ctrl->alignHand();
    ctrl->sortGrasps();
    ctrl->resetGraspIndex();
    graspSelectionView->showSelectedGrasp(ctrl->getGraspDemoHand(), currentGrasp);
    ctrl->emitRender();
    DBGA("Found Current Grasp");
    BCIService::getInstance()->emitNext();
}
