#include "BCI/states/activateRefinementState.h"
#include "BCI/bciService.h"

using bci_experiment::OnlinePlannerController;
using bci_experiment::world_element_tools::getWorld;
using bci_experiment::WorldController;

ActivateRefinementState::ActivateRefinementState(BCIControlWindow *_bciControlWindow,QState* parent):
    HandRotationState("ActivateRefinementState",_bciControlWindow, parent)
{
    addSelfTransition(BCIService::getInstance(), SIGNAL(plannerUpdated()), this, SLOT(onPlannerUpdated()));
    addSelfTransition(BCIService::getInstance(), SIGNAL(next()), this, SLOT(nextGrasp()));
    connect(this, SIGNAL(entered()),OnlinePlannerController::getInstance(), SLOT(setPlannerToRunning()));
    connect(this, SIGNAL(exited()), OnlinePlannerController::getInstance(), SLOT(setPlannerToPaused()));

    activeRefinementView = new ActiveRefinementView(bciControlWindow->currentFrame);
    activeRefinementView->hide();
}



void ActivateRefinementState::onEntry(QEvent *e)
{
    activeRefinementView->show();
    bciControlWindow->currentState->setText("Refinement State");
    onPlannerUpdated();
}


void ActivateRefinementState::onExit(QEvent *e)
{
    activeRefinementView->hide();
}


void ActivateRefinementState::nextGrasp(QEvent *e)
{
    if(OnlinePlannerController::getInstance()->getNumGrasps())
    {
        const GraspPlanningState *nextGrasp = OnlinePlannerController::getInstance()->getGrasp(1);
        Hand *refHand = OnlinePlannerController::getInstance()->getRefHand();
        nextGrasp->execute(refHand);
        OnlinePlannerController::getInstance()->alignHand();
        OnlinePlannerController::getInstance()->sortGrasps();
    }
}

void ActivateRefinementState::onPlannerUpdated(QEvent * e)
{
    OnlinePlannerController::getInstance()->sortGrasps();
    const GraspPlanningState *bestGrasp = OnlinePlannerController::getInstance()->getGrasp(0);    
    Hand *hand = OnlinePlannerController::getInstance()->getGraspDemoHand();
    const GraspPlanningState *nextGrasp = bestGrasp;
    if(OnlinePlannerController::getInstance()->getNumGrasps())
    {
        nextGrasp = OnlinePlannerController::getInstance()->getGrasp(1);
    }

    if(bestGrasp)
    {
        activeRefinementView->showSelectedGrasp(hand,bestGrasp);
        QString graspID;
        bciControlWindow->currentState->setText("Refinement State - Grasp:" + graspID.setNum(bestGrasp->getAttribute("graspId")) );
    }

    if(nextGrasp)
    {
        activeRefinementView->showNextGrasp(hand, nextGrasp);
    }

    OnlinePlannerController::getInstance()->analyzeNextGrasp();
}

void ActivateRefinementState::generateImageOptions(bool debug)
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

    stringOptions.push_back(QString("Stop Planner"));

    generateStringImageOptions(debug);

    imageDescriptions.push_back(stringOptions[0]);
    imageCosts.push_back(.5);
    const GraspPlanningState * currentGrasp;
    Hand * hand = OnlinePlannerController::getInstance()->getGraspDemoHand();
    for (int i = 0; i < OnlinePlannerController::getInstance()->getNumGrasps(); ++i)
    {
        currentGrasp = OnlinePlannerController::getInstance()->getGrasp(i);
        activeRefinementView->showSelectedGrasp(hand, currentGrasp);
        sentChoices.push_back(new GraspPlanningState(currentGrasp));

        QString debugFileName="";
        if(debug)
            debugFileName=QString("img" + QString::number(imageOptions.size()) + ".png");
        QImage * img = graspItGUI->getIVmgr()->generateImage(
                    activeRefinementView->currentGraspView->getIVRoot(), debugFileName);
        if(i ==0)
            decorateOption(img);

        imageOptions.push_back(img);
        imageCosts.push_back(.25);
        imageDescriptions.push_back(QString("GraspID: ") + QString::number(currentGrasp->getAttribute("graspId")) );
    }
}

void ActivateRefinementState::respondOptionChoice(unsigned int option, double confidence, std::vector<double> interestLevel)
{
    OnlinePlannerController::getInstance()->stopTimedUpdate();
    const GraspPlanningState * currentGrasp = NULL;
    if(option == 0)
    {
        //Go to next state
        BCIService::getInstance()->emitExec();
        return;
    }

    int i;
    for(i = 0; i < OnlinePlannerController::getInstance()->getNumGrasps(); ++i)
    {
        if(OnlinePlannerController::getInstance()->getGrasp(i)->getAttribute("graspId") == sentChoices[option - stringOptions.size()]->getAttribute("graspId"))
            currentGrasp = OnlinePlannerController::getInstance()->getGrasp(i);
            break;
    }
    if(!currentGrasp)
    {
        DBGA("ActivateRefinementSelectionState::respondOptionChoice -- Failed to find chosen grasp in grasp list");
        return;
    }


    currentGrasp->execute(OnlinePlannerController::getInstance()->getRefHand());
    OnlinePlannerController::getInstance()->alignHand();

    if(option == 1)
    {
        //Grasp accepted. Move to confirmation state
        BCIService::getInstance()->emitNext();
        return;
    }

    //Otherwise, plan for a few seconds and send options again
    QTimer::singleShot(5000, this, SLOT(sendOptionChoice()));
    return;
}

#include <QPainter>
#include <QBitmap>
void ActivateRefinementState::decorateOption(QImage * imageOption)
{
    QBitmap mask = QBitmap::fromImage(imageOption->createMaskFromColor(Qt::white));
    QPainter p(imageOption);

    p.setClipRegion(QRegion(mask));
    p.fillRect(imageOption->rect(), Qt::yellow);
}
