#include "BCI/states/graspSelectionState.h"
#include "BCI/bciService.h"
#include "BCI/onlinePlannerController.h"
#include <QPushButton>

using bci_experiment::OnlinePlannerController;


GraspSelectionState::GraspSelectionState(BCIControlWindow *_bciControlWindow, QState* parent):
    HandRotationState("GraspSelectionState",_bciControlWindow, parent)
{
    /* What should next do?

      It should update the online planner controller's idea of the *CURRENT* grasp

      This has two control consequences - If the grasp is sent to the robot, this is the
      grasp that is sent. If the planner is started, this grasp is used as approach
      direction of the seed.

      It also affects the visualization - This should change which grasp is shown by the demonstration hand
      and which grasp is shown most prominantly in any grasp preview pane.
    */

    //addSelfTransition(BCIService::getInstance(),SIGNAL(next()), this, SLOT(onNext()));
    addSelfTransition(BCIService::getInstance(),SIGNAL(plannerUpdated()), this, SLOT(onPlannerUpdated()));
    connect(this, SIGNAL(entered()), OnlinePlannerController::getInstance(), SLOT(setPlannerToReady()));
    addSelfTransition(BCIService::getInstance(), SIGNAL(rotLat()), this, SLOT(onPlannerUpdated()));
    addSelfTransition(BCIService::getInstance(), SIGNAL(rotLong()), this, SLOT(onPlannerUpdated()));
    stateName = QString("Grasp Selection");
    graspSelectionView = new GraspSelectionView(bciControlWindow->currentFrame);
    graspSelectionView->hide();

}


void GraspSelectionState::onEntry(QEvent *e)
{
    choicesValid = true;

    graspSelectionView->show();
    bciControlWindow->currentState->setText(stateName);

    //loads grasps from the database
    //OnlinePlannerController::getInstance()->setPlannerToReady();
    //called so that view will show best grasp from database
    onPlannerUpdated();
}


void GraspSelectionState::onExit(QEvent *e)
{
    graspSelectionView->hide();
}

void GraspSelectionState::setNextButtonLabel(QString &label)
{
    QString name("buttonRefineGrasp");
    QPushButton * buttonRefineGrasp = graspSelectionView->findChild<QPushButton *>((name));
    buttonRefineGrasp->setText(label);
}



//Currently unused
void GraspSelectionState::onNext()
{
    OnlinePlannerController::getInstance()->incrementGraspIndex();
    const GraspPlanningState * currentGrasp = OnlinePlannerController::getInstance()->getCurrentGrasp();
    Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();
    OnlinePlannerController::getInstance()->stopTimedUpdate();
   if(currentGrasp)
   {
       currentGrasp->execute(OnlinePlannerController::getInstance()->getRefHand());
       OnlinePlannerController::getInstance()->alignHand();
       graspSelectionView->showSelectedGrasp(hand ,currentGrasp);
       QString graspID;
       bciControlWindow->currentState->setText(stateName +"- Grasp: " + graspID.setNum(currentGrasp->getAttribute("graspId")) );
   }

}

void GraspSelectionState::onPlannerUpdated()
{
    const GraspPlanningState *bestGrasp = OnlinePlannerController::getInstance()->getGrasp(0);
    Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();
    OnlinePlannerController::getInstance()->sortGrasps();

    if(bestGrasp)
    {
        graspSelectionView->showSelectedGrasp(hand,bestGrasp);
        QString graspID;
        bciControlWindow->currentState->setText(stateName + ": Grasp: " + graspID.setNum(bestGrasp->getAttribute("graspId")) );
    }
    else
    {
        DBGA("GraspSelectionState::onPlannerUpdated::No best grasp found");
    }
    OnlinePlannerController::getInstance()->analyzeNextGrasp();
    if(choiceReady())
        sendOptionChoice();
}

bool GraspSelectionState::choiceReady()
{
    if(choicesValid && OnlinePlannerController::getInstance()->getNumGrasps())
    {
        for(int i = 0; i < OnlinePlannerController::getInstance()->getNumGrasps(); ++i)
        {
            const GraspPlanningState * currentGrasp = OnlinePlannerController::getInstance()->getGrasp(i);
            if(currentGrasp->getAttribute("testResult") == 0.0)
                return false;
        }
        choicesValid = false;
        return true;
    }
    else
    {
        return false;
    }

}

void GraspSelectionState::generateImageOptions(bool debug)
{
    for (unsigned int i = 0; i < imageOptions.size(); ++i)
    {
        delete imageOptions[i];
    }

    for (unsigned int i = 0; i < sentChoices.size(); ++i)
    {
        delete sentChoices[i];
    }

    imageOptions.clear();
    imageDescriptions.clear();
    imageCosts.clear();
    sentChoices.clear();

    stringOptions.push_back(QString("Select Different Object"));

    generateStringImageOptions(debug);

    imageDescriptions.push_back(stringOptions[0]);
    imageCosts.push_back(.5);
    const GraspPlanningState * currentGrasp;
    for (int i = 0; i < OnlinePlannerController::getInstance()->getNumGrasps(); ++i)
    {
        onNext();
        currentGrasp = OnlinePlannerController::getInstance()->getGrasp(i);
        sentChoices.push_back(new GraspPlanningState(currentGrasp));

        QString debugFileName="";
        if(debug)
            debugFileName=QString("img" + QString::number(imageOptions.size()) + ".png");
        QImage * img = graspItGUI->getIVmgr()->generateImage(
                    graspSelectionView->getHandView()->getIVRoot(), debugFileName);

        imageOptions.push_back(img);
        imageCosts.push_back(.25);
        imageDescriptions.push_back(QString("GraspID: ") + QString::number(currentGrasp->getAttribute("graspId")) );
    }
}



void GraspSelectionState::respondOptionChoice(unsigned int option, double confidence, std::vector<double> interestLevel)
{
    const GraspPlanningState * currentGrasp = NULL;
    if(option == 0)
    {
        //Go back to object selection state.
        BCIService::getInstance()->emitGoToPreviousState();
        return;
    }

    for(int i = 0; i < OnlinePlannerController::getInstance()->getNumGrasps(); ++i)
    {

        if(OnlinePlannerController::getInstance()->getGrasp(i)->getAttribute("graspId") == sentChoices[option - stringOptions.size()]->getAttribute("graspId"))
            currentGrasp = OnlinePlannerController::getInstance()->getGrasp(i);
            break;
    }
    if(!currentGrasp)
    {
        DBGA("GraspSelectionState::respondOptionChoice -- Failed to find chosen grasp in grasp list");
        return;
    }

    Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();
    OnlinePlannerController::getInstance()->stopTimedUpdate();
    if(currentGrasp)
    {
       while(OnlinePlannerController::getInstance()->getCurrentGrasp() != currentGrasp)
           onNext();
    }
    //Go to grasp refinement state
    BCIService::getInstance()->emitNext();
}






