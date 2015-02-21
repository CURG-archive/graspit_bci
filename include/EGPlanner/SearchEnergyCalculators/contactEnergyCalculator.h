#ifndef _contactenergycalculator_h_
#define _contactenergycalculator_h_

#include <ostream>
#include <list>
#include <QObject>


class Hand;
class Body;
class QualityMeasure;
class GraspPlanningState;


class ContactEnergyCalculator : public QObject
{
    Q_OBJECT
protected:

    Hand *mHand;
    Body *mObject;



public:
    ContactEnergyCalculator();

    void setHandAndObject(Hand *h, Body *o);

    double computeEnergy(bool debug, bool palmOnly) const;

};


#endif
