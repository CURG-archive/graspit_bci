#ifndef CONFIRMATIONSTATE_H
#define CONFIRMATIONSTATE_H


#include "BCI/bciControlWindow.h"
#include "BCI/state.h"
#include "BCI/state_views/confirmationView.h"
class ConfirmationState:public State
{
    Q_OBJECT

public:
    ConfirmationState(BCIControlWindow *_bciControlWindow, QState* parent = 0 );

     virtual void onEntry(QEvent *e);
     virtual void onExit(QEvent *e);
    GraspPlanningState * sentChoice;
    void respondOptionChoice(unsigned int option, float confidence, std::vector<float> & interestLevel);
public slots:
    void onNextGrasp(QEvent *e=NULL);
private:
    ConfirmationView* confirmationView;
    BCIControlWindow *bciControlWindow;

protected:
    void generateImageOptions(bool debug);
};


#endif // CONFIRMATIONSTATE_H
