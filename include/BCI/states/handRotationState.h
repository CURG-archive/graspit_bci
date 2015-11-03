#ifndef HANDROTATIONSTATE_H
#define HANDROTATIONSTATE_H

#include "BCI/state.h"
#include <QSignalTransition>
#include "BCI/onlinePlannerController.h"
#include "BCI/bciControlWindow.h"



class HandRotationState:public State
{
    Q_OBJECT

public:

    HandRotationState(const QString name ,BCIControlWindow *_bciControlWindow, ControllerSceneManager *_csm,QState* parent = 0);

protected:
    BCIControlWindow *bciControlWindow;
    QSignalTransition * rotateLatTransition;
    QSignalTransition * rotateLongTransition;
    bool rotationAllowed;

    ControllerSceneManager *csm;

public slots:
    virtual void onRotateHandLat();
    virtual void onRotateHandLong();
    virtual void onHandRotationStateEntry();

//    void onRotateHandLat();
//    void onRotateHandLong();
//    void onHandRotationStateEntry();

    void setRotationAllowed(bool allowed);
    bool getRotationAllowed(){return rotationAllowed;}

};

#endif // HANDROTATIONSTATE_H
