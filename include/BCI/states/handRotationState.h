#ifndef HANDROTATIONSTATE_H
#define HANDROTATIONSTATE_H

#include "BCI/state.h"
#include <QSignalTransition>
#include "BCI/onlinePlannerController.h"
#include "BCI/bciControlWindow.h"


class HandRotationState : public State {
Q_OBJECT

public:
    HandRotationState(const QString name, BCIControlWindow *_bciControlWindow, QState *parent = 0);

    void setRotationAllowed(bool allowed);

    bool getRotationAllowed() {
        return rotationAllowed;
    }


protected:
    BCIControlWindow *bciControlWindow;
    QSignalTransition *rotateLatTransition;
    QSignalTransition *rotateLongTransition;
    bool rotationAllowed;

public slots:

    virtual void onRotateHandLat();

    virtual void onRotateHandLong();

    virtual void onHandRotationStateEntry();


};

#endif // HANDROTATIONSTATE_H
