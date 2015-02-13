
#ifndef _graspviewerdlg_h_
#define _graspviewerdlg_h_

#include <QDialog>

#include "ui_graspViewerDlg.h"


class GraspPlanningState;
class GraspableBody;
class Hand;


class GraspViewerDlg : public QDialog, public Ui::GraspViewerDlgUI
{
	Q_OBJECT

private:

    std::vector<double*> jointValues;
    std::vector<double*> poses;
    std::vector<double> energy;

    int graspIndex;

    GraspPlanningState *mHandObjectState;
    GraspableBody *mObject;
    Hand *mHand;

    void showGrasp();

public:
  GraspViewerDlg(QWidget *parent = 0) :
        QDialog(parent)

  {
      setupUi(this);
  }
	
  ~GraspViewerDlg(){destroy();}

public slots:
  void prevGraspButton_clicked();
  void nextGraspButton_clicked();
  void loadButton_clicked();
  void setMembers( Hand *h, GraspableBody *b );
  void destroy();

};

#endif
