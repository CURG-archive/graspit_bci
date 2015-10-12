
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


    objectSelectionState->addStateTransition(bciService,SIGNAL(goToNextState1()), initialGraspSelectionState);
    objectSelectionState->addSelfTransition(bciService, SIGNAL(exec()), objectSelectionState,SLOT(onSelect()));
    objectSelectionState->addSelfTransition(bciService, SIGNAL(rotLat()), objectSelectionState,SLOT(onNext()));

    initialGraspSelectionState->addStateTransition(bciService, SIGNAL(goToNextState2()), activateRefinementState);
    initialGraspSelectionState->addStateTransition(bciService, SIGNAL(goToPreviousState()), objectSelectionState);
    initialGraspSelectionState->addStateTransition(bciService, SIGNAL(goToNextState1()), confirmationState);
    initialGraspSelectionState->addStateTransition(bciService, SIGNAL(exec()), confirmationState);
    initialGraspSelectionState->addStateTransition(bciService, SIGNAL(next()), activateRefinementState);
    initialGraspSelectionState->setRotationAllowed(false);
    initialGraspSelectionState->setButtonLabel( "buttonRotateLong", "Next Grasp");
    initialGraspSelectionState->setButtonLabel( "buttonRotateLat", "Change Target Object");
    initialGraspSelectionState->addSelfTransition(bciService,SIGNAL(rotLong()), initialGraspSelectionState, SLOT(onNext()));
    initialGraspSelectionState->addStateTransition(bciService,SIGNAL(rotLat()), objectSelectionState);

    activateRefinementState->addStateTransition(bciService, SIGNAL(goToNextState1()), finalGraspSelectionState);
    activateRefinementState->addStateTransition(bciService, SIGNAL(exec()), finalGraspSelectionState);


    finalGraspSelectionState->addStateTransition(bciService, SIGNAL(goToNextState1()),confirmationState);
    finalGraspSelectionState->addStateTransition(bciService, SIGNAL(exec()),confirmationState);
    finalGraspSelectionState->addSelfTransition(bciService,SIGNAL(goToNextState2()), finalGraspSelectionState, SLOT(onNext()));
    finalGraspSelectionState->addSelfTransition(bciService,SIGNAL(next()), finalGraspSelectionState, SLOT(onNext()));

    finalGraspSelectionState->setButtonLabel("buttonRefineGrasp", "Next Grasp");
    finalGraspSelectionState->stateName = "Final Selection";

    //onlinePlanningState->addStateTransition(bciService, SIGNAL(goToNextState1()), finalGraspSelectionState);

    confirmationState->addStateTransition(bciService, SIGNAL(goToNextState1()), executionState);
    confirmationState->addStateTransition(bciService, SIGNAL(goToNextState2()), initialGraspSelectionState);
    confirmationState->addStateTransition(bciService, SIGNAL(exec()), executionState);
    confirmationState->addStateTransition(bciService, SIGNAL(next()), initialGraspSelectionState);

    executionState->addStateTransition(bciService, SIGNAL(goToNextState1()), stoppedExecutionState);
    executionState->addStateTransition(bciService, SIGNAL(exec()), stoppedExecutionState);


    stoppedExecutionState->addStateTransition(bciService, SIGNAL(goToNextState1()), executionState);
    stoppedExecutionState->addStateTransition(bciService, SIGNAL(goToNextState2()), objectSelectionState);
    stoppedExecutionState->addStateTransition(bciService, SIGNAL(exec()), executionState);
    stoppedExecutionState->addStateTransition(bciService, SIGNAL(next()), objectSelectionState);

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

