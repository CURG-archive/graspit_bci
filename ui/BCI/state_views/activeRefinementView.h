#ifndef ACTIVEREFINEMENTVIEW_H
#define ACTIVEREFINEMENTVIEW_H

#include <QWidget>
#include "BCI/handView.h"

class GraspPlanningState;

namespace Ui {
class ActiveRefinementView;
}

class ActiveRefinementView : public QWidget
{
    Q_OBJECT
    
public:
    explicit ActiveRefinementView(QWidget *parent = 0);
    void showSelectedGrasp(Hand *hand,const GraspPlanningState *graspPlanningState);
    void showNextGrasp(Hand *hand, const GraspPlanningState *graspPlanningState);
    ~ActiveRefinementView();

    
protected:
    virtual void showEvent(QShowEvent *);
private:
    HandView *currentGraspView;
    HandView *nextGraspView;
    Ui::ActiveRefinementView *ui;
    void createHandView();
};

#endif // ACTIVEREFINEMENTVIEW_H
