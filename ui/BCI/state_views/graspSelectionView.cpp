#include "graspSelectionView.h"
#include "ui_graspSelectionView.h"
#include "BCI/bciService.h"
#include "debug.h"

using bci_experiment::OnlinePlannerController;

GraspSelectionView::GraspSelectionView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GraspSelectionView)
{
    ui->setupUi(this);

    currentGraspSpinner = new QtWaitingSpinner(10,7,3,15,ui->renderArea);
    nextGraspSpinner = new QtWaitingSpinner(10,7,3,15,ui->nextRenderArea);

    SoQtExaminerViewer *mainViewer = graspItGUI->getIVmgr()->getViewer();
    Hand * h = OnlinePlannerController::getInstance()->getGraspDemoHand();
    QFrame *parentWindow = this->ui->renderArea;
    QString viewName = QString("current best grasp");
    selectedHandView = new HandView(mainViewer,h,*parentWindow, viewName);

    QFrame *nextParentWindow = this->ui->nextRenderArea;
    QString nextViewName = QString("next grasp");
    nextHandView = new HandView(mainViewer,h,*nextParentWindow, nextViewName);

    currentGraspSpinner->setSpeed(1.5);
    currentGraspSpinner->start();
    currentGraspSpinner->move(currentGraspSpinner->parentWidget()->geometry().center()/2.0);

    nextGraspSpinner->setSpeed(1.5);
    nextGraspSpinner->start();
    nextGraspSpinner->move(currentGraspSpinner->parentWidget()->geometry().center()/2.0);
}

void GraspSelectionView::showEvent(QShowEvent *)
{
    Hand * h = OnlinePlannerController::getInstance()->getGraspDemoHand();
    selectedHandView->updateGeom(*OnlinePlannerController::getInstance()->getGraspDemoHand());
    showSelectedGrasp(h,NULL);
    showNextGrasp(h, NULL);
}


void GraspSelectionView::showSelectedGrasp(Hand *hand ,const GraspPlanningState *graspPlanningState)
{
    currentGraspSpinner->hide();

    if(graspPlanningState)
    {
        selectedHandView->update(*graspPlanningState, *hand);
        DBGA("GraspSelectionView::showSelectedGrasp::Showing grasp:" << graspPlanningState->getItNumber());
    }
    else
    {
        DBGA("GraspSelectionView::showSelectedGrasp::No grasp");
    }
}

void GraspSelectionView::showNextGrasp(Hand *hand ,const GraspPlanningState *graspPlanningState)
{
    nextGraspSpinner->hide();

    if(graspPlanningState)
    {
        nextHandView->update(*graspPlanningState, *hand);
        DBGA("GraspSelectionView::showNextGrasp::Showing grasp:" << graspPlanningState->getItNumber());
    }
    else
    {
        DBGA("GraspSelectionView::showNextGrasp::No grasp");
    }
}


GraspSelectionView::~GraspSelectionView()
{
    delete ui;
}
