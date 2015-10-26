#ifndef ONLINESELECTIONSTATE_H
#define ONLINESELECTIONSTATE_H


#include "BCI/states/handRotationState.h"
#include "BCI/bciControlWindow.h"

class OnlinePlanningState : public HandRotationState
{
public:
    OnlinePlanningState(BCIControlWindow *_bciControlWindow,ControllerSceneManager *_csm, QState* parent = 0);
    virtual void onEntry(QEvent * e);
    virtual void onExit(QEvent * e);

    BCIControlWindow *bciControlWindow;

protected:
    ControllerSceneManager *csm;


public slots:
    void emit_goToConfirmationState(){emit goToConfirmationState();}

signals:
    void goToConfirmationState();
};

#endif // ONLINESELECTIONSTATE_H
