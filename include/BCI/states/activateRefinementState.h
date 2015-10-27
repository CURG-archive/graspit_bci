#ifndef STARTSTATE_H
#define STARTSTATE_H

#include "BCI/states/handRotationState.h"

#include <QSignalTransition>
#include "debug.h"
#include "BCI/bciControlWindow.h"
#include "BCI/state_views/activeRefinementView.h"

class ActivateRefinementState: public HandRotationState
{
    Q_OBJECT

public:
    ActivateRefinementState(BCIControlWindow *_bciControlWindow, ControllerSceneManager *_csm, QState* parent = 0 );


public slots:
    virtual void onEntry(QEvent *e);
    virtual void onExit(QEvent *e);
    virtual void onPlannerUpdated(QEvent *e = NULL);
    void emit_goToConfirmationState();

    virtual void nextGrasp(QEvent *e = NULL);
    virtual void updateView();
    void setTimerRunning();

protected:
    ActiveRefinementView *activeRefinementView;

private:
    ControllerSceneManager *csm;


};


#endif // STARTSTATE_H


