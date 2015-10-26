#include "BCI/states/stoppedExecutionState.h"

StoppedExecutionState::StoppedExecutionState(BCIControlWindow *_bciControlWindow, ControllerSceneManager *_csm, QState* parent)
    :State("StoppedExecutionState", parent), bciControlWindow(_bciControlWindow), csm(_csm)
{
    stoppedExecutionView = new StoppedExecutionView(bciControlWindow->currentFrame);
    stoppedExecutionView->hide();
}


void StoppedExecutionState::onEntry(QEvent *e)
{
    stoppedExecutionView->show();
    bciControlWindow->currentState->setText("Execution State");
}


void StoppedExecutionState::onExit(QEvent *e)
{
    stoppedExecutionView->hide();
}
