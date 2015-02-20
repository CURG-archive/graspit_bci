#include "BCI/states/confirmationState.h"
#include "BCI/handView.h"

using bci_experiment::world_element_tools::getWorld;
using bci_experiment::OnlinePlannerController;

ConfirmationState::ConfirmationState(BCIControlWindow *_bciControlWindow, QState *parent) :
        State("ConfirmationState", parent), sentChoice(NULL), bciControlWindow(_bciControlWindow) {
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
    if (OnlinePlannerController::getInstance()->getCurrentGrasp()) {
        if (sentChoice)
            delete sentChoice;

        sentChoice = new GraspPlanningState(*OnlinePlannerController::getInstance()->getCurrentGrasp());
        sentChoice->setAttribute("graspId", OnlinePlannerController::getInstance()->getCurrentGrasp()->getAttribute("graspId"));
    }

    for (unsigned int i = 0; i < imageOptions.size(); ++i) {
        delete imageOptions[i];
    }


    imageOptions.clear();
    imageDescriptions.clear();
    imageCosts.clear();

    stringOptions.push_back(QString("Go Back - RePlan"));
    imageDescriptions.push_back(QString("Go Back - RePlan"));
    size_t numDistractors = 8;

    for (size_t i = 0; i < numDistractors; ++i) {
        stringOptions.push_back(QString("Distractor"));
        imageDescriptions.push_back(QString("Distractor"));
    }

    generateStringImageOptions(debug);

    //! Get grasp planning state
    QString debugFileName = "";
    if (debug)
        debugFileName = QString("confirmation_state_grasp.png");

    QImage *img = graspItGUI->getIVmgr()->generateImage(confirmationView->getHandView()->getIVRoot(), debugFileName);
    imageOptions.push_back(img);
    imageCosts.push_back(.25);
    imageDescriptions.push_back(QString("GraspID: ") + QString::number(sentChoice->getAttribute("graspId")));


    //! Add distractor images here?
}

void
ConfirmationState::respondOptionChoice(unsigned int option, float confidence, std::vector<float> interestLevel) {
    //if (!sentChoice)
    //    return;
    DBGA("ConfirmationSTate::respondOptionChoice - Entered");
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
