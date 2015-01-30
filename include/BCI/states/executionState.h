#ifndef EXECUTIONSTATE_H
#define EXECUTIONSTATE_H

#include "BCI/state.h"
#include "BCI/bciControlWindow.h"
#include "BCI/state_views/executionView.h"

class ExecutionState : public State {
public:
    ExecutionState(BCIControlWindow *_bciControlWindow, QState *parent = 0);

    virtual void onEntry(QEvent *e);

    virtual void onExit(QEvent *e);

    virtual void respondOptionChoice(unsigned int option, float confidence, std::vector<float> &interestLevel);

private:
    BCIControlWindow *bciControlWindow;
    ExecutionView *executionView;

protected:
    virtual void generateImageOptions(bool debug);

};

#endif // EXECUTIONSTATE_H
