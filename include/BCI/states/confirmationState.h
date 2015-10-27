#ifndef CONFIRMATIONSTATE_H
#define CONFIRMATIONSTATE_H


#include "BCI/bciControlWindow.h"
#include "BCI/state.h"
#include "BCI/state_views/confirmationView.h"
class ConfirmationState:public State
{
    Q_OBJECT

public:
    ConfirmationState(BCIControlWindow *_bciControlWindow,ControllerSceneManager *_csm, QState* parent = 0 );

     virtual void onEntry(QEvent *e);
     virtual void onExit(QEvent *e);


public slots:
    void onNextGrasp(QEvent *e=NULL);
private:
    ConfirmationView* confirmationView;
    BCIControlWindow *bciControlWindow;
    ControllerSceneManager *csm;

public slots:

    void emit_goToExecutionState(){emit goToExecutionState();}
    void emit_goToPreviousState(){emit goToPreviousState();}

signals:
    void goToExecutionState();
    void goToPreviousState();
};


#endif // CONFIRMATIONSTATE_H
