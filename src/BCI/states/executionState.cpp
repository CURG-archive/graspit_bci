#include "BCI/states/executionState.h"
#include "BCI/controller_scene/controller_scene_mgr.h"
#include "BCI/controller_scene/sprites.h"
#include "BCI/onlinePlannerController.h"

ExecutionState::ExecutionState(BCIControlWindow *_bciControlWindow, ControllerSceneManager *_csm, QState* parent)
    : State("ExecutionState", parent), bciControlWindow(_bciControlWindow),
      csm(_csm)
{
    executionView = new ExecutionView(bciControlWindow->currentFrame);
    executionView->hide();
}


void ExecutionState::onEntry(QEvent *e)
{
    executionView->show();
    bciControlWindow->currentState->setText("Execution");

    BCIService::getInstance()->executeGrasp(OnlinePlannerController::getInstance()->getCurrentGrasp(),NULL,NULL);

    csm->clearTargets();

    std::shared_ptr<Target>  t1 = std::shared_ptr<Target> (new Target(csm->control_scene_separator,
                                                                       QString("sprites/target_background.png"),
                                                                       -1.1, 0.25, 0.0, QString("STOP!")));

    QObject::connect(t1.get(), SIGNAL(hit()), this, SLOT(emit_goToStoppedExecutionState()));

    csm->addTarget(t1);
}

void ExecutionState::onExit(QEvent *e)
{
    executionView->hide();
}
