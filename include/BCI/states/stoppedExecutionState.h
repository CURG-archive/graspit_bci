#ifndef STOPPEDEXECUTIONSTATE_H
#define STOPPEDEXECUTIONSTATE_H


#include "BCI/state.h"
#include "BCI/bciControlWindow.h"
#include "BCI/state_views/stoppedExecutionView.h"


class StoppedExecutionState:public State
{
        Q_OBJECT
public:
    StoppedExecutionState(BCIControlWindow *_bciControlWindow,ControllerSceneManager *_csm, QState* parent = 0 );
     virtual void onEntry(QEvent *e);
     virtual void onExit(QEvent *e);

public slots:
    void onContinueExecutionClicked(){emit goToExecutionState();};
    void onStartOverClicked(){emit goToObjectSelectionState();};

signals:
    void goToExecutionState();
    void goToObjectSelectionState();


private:
    BCIControlWindow *bciControlWindow;
    StoppedExecutionView *stoppedExecutionView;
    ControllerSceneManager *csm;
};

#endif // STOPPEDEXECUTIONSTATE_H
