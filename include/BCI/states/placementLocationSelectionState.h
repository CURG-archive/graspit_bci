#ifndef PLACEMENTLOCATIONSELECTIONSTATE_H
#define PLACEMENTLOCATIONSELECTIONSTATE_H

#include "BCI/state.h"
#include "BCI/bciControlWindow.h"
#include "BCI/state_views/placementLocationSelectionView.h"

class PlacementLocationSelectionState:public State
{
public:
    PlacementLocationSelectionState(BCIControlWindow *_bciControlWindow,ControllerSceneManager *_csm, QState* parent = 0 );
     virtual void onEntry(QEvent *e);
     virtual void onExit(QEvent *e);

private:
    BCIControlWindow *bciControlWindow;
    PlacementLocationSelectionView *placementLocationSelectionView;
    ControllerSceneManager *csm;

};

#endif // PLACEMENTLOCATIONSELECTIONSTATE_H
