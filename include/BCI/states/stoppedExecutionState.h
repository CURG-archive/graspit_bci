#ifndef STOPPEDEXECUTIONSTATE_H
#define STOPPEDEXECUTIONSTATE_H


#include "BCI/state.h"
#include "BCI/bciControlWindow.h"
#include "BCI/state_views/stoppedExecutionView.h"


class StoppedExecutionState:public State
{
public:
    StoppedExecutionState(BCIControlWindow *_bciControlWindow,ControllerSceneManager *_csm, QState* parent = 0 );
     virtual void onEntry(QEvent *e);
     virtual void onExit(QEvent *e);

private:
    BCIControlWindow *bciControlWindow;
    StoppedExecutionView *stoppedExecutionView;
    ControllerSceneManager *csm;
};

#endif // STOPPEDEXECUTIONSTATE_H
