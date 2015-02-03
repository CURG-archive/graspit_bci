#include "BCI/states/executionState.h"

#include "BCI/onlinePlannerController.h"

ExecutionState::ExecutionState(BCIControlWindow *_bciControlWindow, QState *parent)
        : State("ExecutionState", parent), bciControlWindow(_bciControlWindow) {
    executionView = new ExecutionView(bciControlWindow->currentFrame);
    executionView->hide();
}


void ExecutionState::onEntry(QEvent *e) {
    executionView->show();
    bciControlWindow->currentState->setText("Execution");

    BCIService::getInstance()->executeGrasp(OnlinePlannerController::getInstance()->getCurrentGrasp(), NULL, NULL);
}


void ExecutionState::onExit(QEvent *e) {
    executionView->hide();
}


//!*******************************************
//! Option Choice Paradigm related functions *
//!*******************************************

void ExecutionState::generateImageOptions(bool debug) {
    imageOptions.clear();
    imageDescriptions.clear();
    imageCosts.clear();
    size_t numDistractors;
    stringOptions.push_back(QString("Restart Execution"));
    stringOptions.push_back(QString("Stop Execution"));
    for (size_t i; i < numDistractors; ++i) {
        stringOptions.push_back(QString("Distractor Image"));
    }
    for (size_t i; i < stringOptions.size(); ++i) {
        imageDescriptions.push_back(stringOptions[i]);
        imageCosts.push_back(.25);
    }
    generateStringImageOptions(debug);
}


void
ExecutionState::respondOptionChoice(unsigned int option, float confidence, std::vector<float> interestLevel) {
    //! Fixme
}

