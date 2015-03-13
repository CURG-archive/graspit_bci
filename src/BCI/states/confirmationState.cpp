#include "BCI/states/confirmationState.h"
#include "BCI/handView.h"

using bci_experiment::world_element_tools::getWorld;
using bci_experiment::OnlinePlannerController;

ConfirmationState::ConfirmationState(BCIControlWindow *_bciControlWindow, QState *parent) :
        State("ConfirmationState", parent), sentChoice(-1), bciControlWindow(_bciControlWindow) {
    confirmationView = new ConfirmationView(bciControlWindow->currentFrame);
    this->addSelfTransition(BCIService::getInstance(), SIGNAL(rotLat()), this, SLOT(onNextGrasp()));
    confirmationView->hide();
}


void ConfirmationState::onEntry(QEvent *e) {
    const GraspPlanningState *grasp = OnlinePlannerController::getInstance()->getCurrentGrasp();
    Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();
    confirmationView->setCurrentGrasp(hand, grasp);
    confirmationView->show();
    bciControlWindow->currentState->setText("Confirmation");
    OnlinePlannerController::getInstance()->setPlannerToPaused();
    sendOptionChoice();
}

void ConfirmationState::onNextGrasp(QEvent *e) {

}

void ConfirmationState::onExit(QEvent *e) {
    Q_UNUSED(e);
    confirmationView->hide();
}


//!*******************************************
//! Option Choice Paradigm related functions *
//!*******************************************

void ConfirmationState::generateImageOptions(bool debug) {
    OnlinePlannerController * ctrl = OnlinePlannerController::getInstance();
    const GraspPlanningState *currentGrasp = ctrl->getCurrentGraspIfExists();
    if (currentGrasp) {
        sentChoice = currentGrasp->getAttribute("graspId");
    } else {
        sentChoice = -1;
        return;
    }

    for (unsigned int i = 0; i < imageOptions.size(); ++i) {
        delete imageOptions[i];
    }


    imageOptions.clear();
    imageDescriptions.clear();
    imageCosts.clear();
    stringOptions.clear();

    stringOptions.push_back(QString("Go Back - RePlan"));
    imageDescriptions.push_back(QString("Go Back - RePlan"));
    imageCosts.push_back(0.25);
    generateStringImageOptions();

    size_t numDistractors = 8;

    QString debugFileName = "";
    QImage *img;

    if (debug) {
        debugFileName = QString("confirmation_state_distractor.png");
    }

    currentGrasp = ctrl->getCurrentGraspIfExists();
    for (size_t i = 0; i < numDistractors; ++i) {
        img = graspItGUI->getIVmgr()->generateImage(confirmationView->getHandView()->getIVObjectRoot(), debugFileName);
        imageOptions.push_back(img);
        imageCosts.push_back(0);
        imageDescriptions.push_back(QString("Distractor"));
    }

    //! Get grasp planning state
    if (debug) {
        debugFileName = QString("confirmation_state_grasp.png");
    }

    img = graspItGUI->getIVmgr()->generateImage(confirmationView->getHandView()->getIVRoot(), debugFileName);
    imageOptions.push_back(img);
    imageCosts.push_back(.25);
    imageDescriptions.push_back(QString("GraspID: ") + QString::number(sentChoice));
    //! Add distractor images here?
}

void
ConfirmationState::respondOptionChoice(unsigned int option, float confidence, std::vector<float> interestLevel) {
    DBGA("ConfirmationState::respondOptionChoice - Entered");
    DBGA("Received : " << option << " " << interestLevel.size() - 1);
    if (option == 0) {
        // The 0th option will emit go back to replan and emit the next signal
        BCIService::getInstance()->emitNext();
    }
    else if (option == interestLevel.size() - 1) {
        // The last option will go to exec and execute the grasp
        BCIService::getInstance()->emitExec();
    }
    else {
        // Everything else will do nothing and just send again
        sendOptionChoice();
    }
}
