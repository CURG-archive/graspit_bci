#include "activeRefinementView.h"
#include "ui_activeRefinementView.h"
#include "BCI/bciService.h"
#include "BCI/onlinePlannerController.h"

using bci_experiment::OnlinePlannerController;

ActiveRefinementView::ActiveRefinementView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ActiveRefinementView)
{
    ui->setupUi(this);

    createHandView();
}



void ActiveRefinementView::createHandView()
{
    Hand * h = OnlinePlannerController::getInstance()->getGraspDemoHand();

    SoQtExaminerViewer *mainViewer = graspItGUI->getIVmgr()->getViewer();
    QString viewName = QString("current best grasp");
    QString nextViewName = QString("next grasp");
    currentGraspView = new HandView(mainViewer, h, *this->ui->previewFrame, viewName);
    nextGraspView = new HandView(mainViewer, h, *this->ui->previewNextFrame, nextViewName);
}

void ActiveRefinementView::showEvent(QShowEvent *)
{
    Hand * h = OnlinePlannerController::getInstance()->getGraspDemoHand();
    this->currentGraspView->updateGeom(*OnlinePlannerController::getInstance()->getGraspDemoHand());
    showSelectedGrasp(h,NULL);
    showNextGrasp(h,NULL);
}

void ActiveRefinementView::showSelectedGrasp(Hand *hand, const GraspPlanningState *graspPlanningState)
{
    if(graspPlanningState)
    {
        currentGraspView->update(*graspPlanningState, *hand);
    }
}

void ActiveRefinementView::showNextGrasp(Hand * hand,  const GraspPlanningState *graspPlanningState)
{
    if(graspPlanningState)
    {
        nextGraspView->update(*graspPlanningState, *hand);
    }
}

ActiveRefinementView::~ActiveRefinementView()
{
    delete ui;
}

