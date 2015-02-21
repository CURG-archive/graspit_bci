

#include "contactEnergyCalculator.h"
#include <time.h>

#include "robot.h"
#include "barrett.h"
#include "body.h"
#include "grasp.h"
#include "contact.h"
#include "world.h"
#include "quality.h"
#include "graspitGUI.h"
#include "ivmgr.h"
#include "matrix.h"

#include "debug.h"

#include <fstream>
#include <stdlib.h>

#include <QGLWidget>
#include <QRegion>
#include <QPainter>



ContactEnergyCalculator::ContactEnergyCalculator()
{

}

void ContactEnergyCalculator::setHandAndObject(Hand *hand, Body *object)
{
    mHand = hand;
    mObject = object;
}


 double ContactEnergyCalculator::computeEnergy(bool debug, bool palmOnly) const
 {
     VirtualContact *contact;
     vec3 contactToBodyPointVec, n, cn;

     vec3 objectNormal;
     double totalError = 0;

//     int PALM_INDEX = 0;
//     int FINGER_1_INDEX = 7;
//     int FINGER_2_INDEX = 11;
//     int FINGER_3_INDEX = 15;


     int limit = mHand->getGrasp()->getNumContacts();

     if(palmOnly)
     {
         //first for contacts are palm
         limit = 4;
     }

     for (int i=0; i<limit; i++)
     {
             contact = (VirtualContact*)mHand->getGrasp()->getContact(i);
             contact->getObjectDistanceAndNormal(mObject, &contactToBodyPointVec, &objectNormal);

             double dist = contactToBodyPointVec.len();

             position vc_in_world = contact->getWorldLocation();

             vec3 bodyContactPointInWorld;
             bodyContactPointInWorld.set(vc_in_world.x() - contactToBodyPointVec.x(),
                                         vc_in_world.y() - contactToBodyPointVec.y(),
                                         vc_in_world.z() - contactToBodyPointVec.z());

             cn = contact->getWorldNormal();
             n = normalise(objectNormal);

             double d = 1 - cn % n;

             if(debug)
             {
                 std::cout << "dist: " << dist << " cn%n: " << cn % n << std::endl;
                 std::cout << "object normal: " << objectNormal << std::endl;
                 std::cout << "contact normal: " << cn << std::endl;
             }

             //beter grasps have contacts closer to the object
             totalError += dist;

             //better grasps contacts normals are better aligned with the object normal at
             //projected point of contact
             totalError += d * 1000.0 / 2.0;
     }

     totalError /= limit;

     return totalError;

 }


