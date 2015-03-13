#ifndef ACTIVATESTATE_H
#define ACTIVATESTATE_H

#include "BCI/states/handRotationState.h"

#include <QSignalTransition>
#include "debug.h"
#include "BCI/bciControlWindow.h"
#include "BCI/state_views/activeRefinementView.h"

class ActivateRefinementState : public HandRotationState {
Q_OBJECT

public:
    ActivateRefinementState(BCIControlWindow *_bciControlWindow, QState *parent = 0);
    virtual ~ActivateRefinementState();

public slots:

    virtual void onEntry(QEvent *e);

    virtual void onExit(QEvent *e);

    virtual void onPlannerUpdated(QEvent *e = NULL);

    virtual void nextGrasp(QEvent *e = NULL);

    void setTimerRunning();

    void checkSendOptionChoice();

    virtual void respondOptionChoice(unsigned int option, float confidence, std::vector<float> interestLevel);

protected:
    ActiveRefinementView *activeRefinementView;

    QTimer * choiceTimer;
    std::vector<double> sentChoices;
    bool choicesValid;

    virtual void generateImageOptions(bool debug = true);

private:
    void connectChoiceTimer();
    void disconnectChoiceTimer();

};


#endif // ACTIVATESTATE_H


