#include "BCI/states/placementLocationSelectionState.h"



PlacementLocationSelectionState::PlacementLocationSelectionState(BCIControlWindow *_bciControlWindow,ControllerSceneManager *_csm, QState* parent)
    :State("PlacementLocationSelectionState", parent), bciControlWindow(_bciControlWindow), csm(_csm)
{

}

void PlacementLocationSelectionState::onEntry(QEvent *e)
{
    placementLocationSelectionView = new PlacementLocationSelectionView(bciControlWindow->currentFrame);
    bciControlWindow->currentFrame->show();

    bciControlWindow->currentState->setText("Placement Location Selection State");

}



void PlacementLocationSelectionState::onExit(QEvent *e)
{
    Q_UNUSED(e);
    bciControlWindow->currentFrame->removeChild(placementLocationSelectionView);
}
