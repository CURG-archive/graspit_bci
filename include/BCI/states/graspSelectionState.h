#ifndef GRASPSELECTIONSTATE_H
#define GRASPSELECTIONSTATE_H


#include "BCI/state.h"

#include <QSignalTransition>
#include "debug.h"
#include "BCI/bciControlWindow.h"
#include "BCI/state_views/graspSelectionView.h"

class GraspSelectionState:public State
{
    Q_OBJECT

public:
    GraspSelectionState(BCIControlWindow *_bciControlWindow, ControllerSceneManager *_csm, QState* parent = 0 );


    virtual void onEntry(QEvent *e);
    virtual void onExit(QEvent *e);

private:
    BCIControlWindow *bciControlWindow;
    GraspSelectionView *graspSelectionView;
    ControllerSceneManager *csm;

public slots:
    void onNext();
    void onPlannerUpdated();

    void emit_goToActivateRefinementState(){emit goToActivateRefinementState();}
    void emit_goToObjectSelectionState(){emit goToObjectSelectionState();}
    void emit_goToConfirmationState(){emit goToConfirmationState();}

signals:
    void goToActivateRefinementState();
    void goToObjectSelectionState();
    void goToConfirmationState();

};


#endif // GRASPSELECTIONSTATE_H
