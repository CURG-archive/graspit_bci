#include "BCI/states/onlinePlanningState.h"
#include "BCI/controller_scene/controller_scene_mgr.h"
#include "BCI/controller_scene/sprites.h"

using bci_experiment::world_element_tools::getWorld;
using bci_experiment::OnlinePlannerController;


OnlinePlanningState::OnlinePlanningState(BCIControlWindow *_bciControlWindow, ControllerSceneManager *_csm, QState* parent):
    HandRotationState("OnlinePlanningState", _bciControlWindow, _csm, parent),
    bciControlWindow(_bciControlWindow),
    csm(_csm)
{

}

void OnlinePlanningState::onEntry(QEvent *e)
{
    csm->clearTargets();
    Target *t2 = new Target(csm->control_scene_separator, QString("sprites/target_next.png"), -1.1, 0.25, 0.0);
    Target *t3 = new Target(csm->control_scene_separator, QString("sprites/target_next.png"), -1.1, -1.0, 0.0);
    Target *t4 = new Target(csm->control_scene_separator, QString("sprites/target_next.png"), 0.35, -1.0, 0.0);

    QObject::connect(t2, SIGNAL(hit()), this, SLOT(onRotateHandLat()));
    QObject::connect(t3, SIGNAL(hit()), this, SLOT(onRotateHandLong()));
    QObject::connect(t4, SIGNAL(hit()), this, SLOT(emit_goToConfirmationState()));

    csm->addTarget(t2);
    csm->addTarget(t3);
    csm->addTarget(t4);
}


void OnlinePlanningState::onExit(QEvent *e)
{

}
