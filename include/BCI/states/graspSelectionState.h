#ifndef GRASPSELECTIONSTATE_H
#define GRASPSELECTIONSTATE_H


#include "BCI/states/handRotationState.h"

#include <QSignalTransition>
#include "debug.h"
#include "BCI/bciControlWindow.h"
#include "BCI/state_views/graspSelectionView.h"

class GraspSelectionState: public HandRotationState
{
    Q_OBJECT

public:
    GraspSelectionState(BCIControlWindow *_bciControlWindow, QState* parent = 0 );


    virtual void onEntry(QEvent *e);
    virtual void onExit(QEvent *e);

protected:
    GraspSelectionView *graspSelectionView;

public slots:
    void onNext();
    void onPlannerUpdated();
};


#endif // GRASPSELECTIONSTATE_H
