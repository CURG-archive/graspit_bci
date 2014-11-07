
#include "BCI/BCIStateMachine.h"

#include "BCI/states/objectSelectionState.h"
#include "BCI/states/graspSelectionState.h"
#include "BCI/states/placementLocationSelectionState.h"
#include "BCI/states/confirmationState.h"
#include "BCI/states/onlinePlanningState.h"
#include "BCI/states/activateRefinementState.h"
#include "BCI/states/objectRecognitionState.h"
#include "BCI/states/executionState.h"
#include "BCI/states/stoppedExecutionState.h"

#include "BCI/bciService.h"

BCIStateMachine::BCIStateMachine(BCIControlWindow *_bciControlWindow, BCIService *_bciService):
    bciControlWindow(_bciControlWindow),
    bciService(_bciService)
{

    ObjectSelectionState *objectSelectionState = new ObjectSelectionState(bciControlWindow);
    GraspSelectionState *initialGraspSelectionState = new GraspSelectionState(bciControlWindow);
    //OnlinePlanningState * onlinePlanningState = new OnlinePlanningState(bciControlWindow);
    ActivateRefinementState *activateRefinementState = new ActivateRefinementState(bciControlWindow);
    GraspSelectionState *finalGraspSelectionState = new GraspSelectionState(bciControlWindow);
    ConfirmationState *confirmationState = new ConfirmationState(bciControlWindow);
    ExecutionState *executionState = new ExecutionState(bciControlWindow);
    StoppedExecutionState *stoppedExecutionState = new StoppedExecutionState(bciControlWindow);


    objectSelectionState->addTransition(bciService,SIGNAL(goToNextState1()), initialGraspSelectionState);
    objectSelectionState->addSelfTransition(bciService, SIGNAL(exec()), objectSelectionState,SLOT(onSelect()));
    objectSelectionState->addSelfTransition(bciService, SIGNAL(next()), objectSelectionState,SLOT(onNext()));

    initialGraspSelectionState->addTransition(bciService, SIGNAL(goToNextState2()), activateRefinementState);
    initialGraspSelectionState->addTransition(bciService, SIGNAL(goToPreviousState()), objectSelectionState);
    initialGraspSelectionState->addTransition(bciService, SIGNAL(goToNextState1()), confirmationState);
    initialGraspSelectionState->addTransition(bciService, SIGNAL(exec()), confirmationState);
    initialGraspSelectionState->addTransition(bciService, SIGNAL(next()), activateRefinementState);


    activateRefinementState->addTransition(bciService, SIGNAL(goToNextState1()), finalGraspSelectionState);
    activateRefinementState->addTransition(bciService, SIGNAL(exec()), finalGraspSelectionState);


    finalGraspSelectionState->addTransition(bciService, SIGNAL(goToNextState1()),confirmationState);
    finalGraspSelectionState->addTransition(bciService, SIGNAL(exec()),confirmationState);
    finalGraspSelectionState->addSelfTransition(bciService,SIGNAL(goToNextState2()), finalGraspSelectionState, SLOT(onNext()));
    finalGraspSelectionState->addSelfTransition(bciService,SIGNAL(next()), finalGraspSelectionState, SLOT(onNext()));
    QString nextButtonLabel("Next Grasp");
    finalGraspSelectionState->setNextButtonLabel(nextButtonLabel);
    finalGraspSelectionState->stateName = "Final Selection";

    //onlinePlanningState->addTransition(bciService, SIGNAL(goToNextState1()), finalGraspSelectionState);

    confirmationState->addTransition(bciService, SIGNAL(goToNextState1()), executionState);
    confirmationState->addTransition(bciService, SIGNAL(goToNextState2()), initialGraspSelectionState);
    confirmationState->addTransition(bciService, SIGNAL(exec()), executionState);
    confirmationState->addTransition(bciService, SIGNAL(next()), initialGraspSelectionState);

    executionState->addTransition(bciService, SIGNAL(goToNextState1()), stoppedExecutionState);
    executionState->addTransition(bciService, SIGNAL(exec()), stoppedExecutionState);


    stoppedExecutionState->addTransition(bciService, SIGNAL(goToNextState1()), executionState);
    stoppedExecutionState->addTransition(bciService, SIGNAL(goToNextState2()), objectSelectionState);
    stoppedExecutionState->addTransition(bciService, SIGNAL(exec()), executionState);
    stoppedExecutionState->addTransition(bciService, SIGNAL(next()), objectSelectionState);

    stateMachine.addState(objectSelectionState);
    stateMachine.addState(initialGraspSelectionState);
    stateMachine.addState(activateRefinementState);
    stateMachine.addState(finalGraspSelectionState);
    //stateMachine.addState(onlinePlanningState);
    stateMachine.addState(confirmationState);
    stateMachine.addState(executionState);
    stateMachine.addState(stoppedExecutionState);

    stateMachine.setInitialState(objectSelectionState);
}

void BCIStateMachine::start()
{        
    stateMachine.start();
    bciControlWindow->currentFrame->show();
}

