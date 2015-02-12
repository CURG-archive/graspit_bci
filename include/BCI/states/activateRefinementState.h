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
    ActivateRefinementState(BCIControlWindow *_bciControlWindow, QState* parent = 0 );
    std::vector<double*> jvs;
    std::vector<double*> poses;
    int grasp_index;

public slots:
    virtual void onEntry(QEvent *e);
    virtual void onExit(QEvent *e);
    virtual void onPlannerUpdated(QEvent *e = NULL);
    virtual void nextGrasp(QEvent *e = NULL);
    void setTimerRunning();
protected:
    ActiveRefinementView *activeRefinementView;


};


#endif // STARTSTATE_H


