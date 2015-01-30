#include "activeRefinementView.h"
#include "ui_activeRefinementView.h"
#include "BCI/bciService.h"
#include "BCI/onlinePlannerController.h"

using bci_experiment::OnlinePlannerController;

ActiveRefinementView::ActiveRefinementView(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ActiveRefinementView) {
    ui->setupUi(this);

    connect(ui->buttonOk, SIGNAL(clicked()), this, SLOT(onOk()));
    connect(ui->buttonRotLat, SIGNAL(clicked()), this, SLOT(onRotLat()));
    connect(ui->buttonRotLong, SIGNAL(clicked()), this, SLOT(onRotLong()));
    connect(ui->buttonNextGrasp, SIGNAL(clicked()), this, SLOT(onNextGrasp()));

    SoQtExaminerViewer *mainViewer = graspItGUI->getIVmgr()->getViewer();

    Hand *h = OnlinePlannerController::getInstance()->getGraspDemoHand();
    QString viewName = QString("current best grasp");
    //handView = new HandView(mainViewer,h,*parentWindow, viewName);
    createHandView();
    showSelectedGrasp(h, NULL);
    showNextGrasp(h, NULL);
}


void ActiveRefinementView::createHandView() {
    Hand *h = OnlinePlannerController::getInstance()->getGraspDemoHand();

    SoQtExaminerViewer *mainViewer = graspItGUI->getIVmgr()->getViewer();
    QString viewName = QString("current best grasp");
    QString nextViewName = QString("next grasp");
    currentGraspView = new HandView(mainViewer, h, *this->ui->previewFrame, viewName);
    nextGraspView = new HandView(mainViewer, h, *this->ui->previewNextFrame, nextViewName);
}

void ActiveRefinementView::onOk() {
    BCIService::getInstance()->emitGoToNextState1();
}

void ActiveRefinementView::onRotLat() {
    BCIService::getInstance()->emitRotLat();
}

void ActiveRefinementView::onRotLong() {
    BCIService::getInstance()->emitRotLong();
}

void ActiveRefinementView::showEvent(QShowEvent *) {
    this->currentGraspView->updateGeom(*OnlinePlannerController::getInstance()->getGraspDemoHand());
}

void ActiveRefinementView::showSelectedGrasp(Hand *hand, const GraspPlanningState *graspPlanningState) {
    if (graspPlanningState) {
        currentGraspView->update(*graspPlanningState, *hand);
    }
}

void ActiveRefinementView::showNextGrasp(Hand *hand, const GraspPlanningState *graspPlanningState) {
    if (graspPlanningState) {
        nextGraspView->update(*graspPlanningState, *hand);
    }
}

void ActiveRefinementView::onNextGrasp() {
    BCIService::getInstance()->emitNext();
}

ActiveRefinementView::~ActiveRefinementView() {
    delete ui;
}

