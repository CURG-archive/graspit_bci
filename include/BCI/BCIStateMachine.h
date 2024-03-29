#ifndef BCISTATEMACHINE_H
#define BCISTATEMACHINE_H

#include <QState>
#include <QStateMachine>
#include <QSignalTransition>

#include "state.h"
#include "BCI/bciControlWindow.h"


#include "ivmgr.h"

class ControllerSceneManager;

class BCIStateMachine
{
public:
    BCIStateMachine(BCIControlWindow *_bciControlWindow, BCIService *_bciService);
    void start();


private:
    BCIControlWindow *bciControlWindow;
    BCIService *bciService;
    ControllerSceneManager *csm;

    QStateMachine stateMachine;


};

#endif // BCISTATEMACHINE_H
