#ifndef EXECUTIONSTATE_H
#define EXECUTIONSTATE_H

#include "BCI/state.h"
#include "BCI/bciControlWindow.h"
#include "BCI/state_views/executionView.h"

class ExecutionState: public State
{
   Q_OBJECT

public:
    ExecutionState(BCIControlWindow *_bciControlWindow, ControllerSceneManager *_csm, QState* parent=0);

    virtual void onEntry(QEvent *e);
    virtual void onExit(QEvent *e);

public slots:
    void emit_goToStoppedExecutionState(){emit goToStoppedExecutionState();}

signals:
    void goToStoppedExecutionState();

private:
    BCIControlWindow *bciControlWindow;
    ExecutionView *executionView;
    ControllerSceneManager *csm;
};

#endif // EXECUTIONSTATE_H
