#ifndef GRASPSELECTIONSTATE_H
#define GRASPSELECTIONSTATE_H


#include "BCI/states/handRotationState.h"

#include <QSignalTransition>
#include "debug.h"
#include "BCI/bciControlWindow.h"
#include "BCI/state_views/graspSelectionView.h"
#include <vector>

class GraspSelectionState : public HandRotationState {
Q_OBJECT

public:
    GraspSelectionState(BCIControlWindow *_bciControlWindow, QState *parent = 0);

    QString stateName;

    virtual void onEntry(QEvent *e);

    virtual void onExit(QEvent *e);

    virtual bool setButtonLabel(QString buttonName, QString label);

protected:
    GraspSelectionView *graspSelectionView;
    std::vector<GraspPlanningState *> sentChoices;

    virtual void generateImageOptions(bool debug = true);

    bool choiceReady();

    bool choicesValid;
public slots:

    void onNext();

    void onPlannerUpdated();

    virtual void onRotateHandLat();

    virtual void onRotateHandLong();

    virtual void respondOptionChoice(unsigned int option, float confidence, std::vector<float> interestLevel);

};


#endif // GRASPSELECTIONSTATE_H
