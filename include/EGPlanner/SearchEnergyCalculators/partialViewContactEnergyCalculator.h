#ifndef _partialviewcontactenergycalculator_h_
#define _partialviewcontactenergycalculator_h_

#include <ostream>
#include <list>
#include <QObject>


class Hand;
class Body;
class QualityMeasure;
class GraspPlanningState;


class PartialViewContactEnergyCalculator : public QObject
{
    Q_OBJECT
protected:

    Hand *mHand;
    Body *mObject;



public:
    PartialViewContactEnergyCalculator();

    void setHandAndObject(Hand *h, Body *o);

    double computeEnergy(bool debug) const;

};


#endif
