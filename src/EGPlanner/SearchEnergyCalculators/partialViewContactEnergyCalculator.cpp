

#include "partialViewContactEnergyCalculator.h"
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



PartialViewContactEnergyCalculator::PartialViewContactEnergyCalculator()
{

}

void PartialViewContactEnergyCalculator::setHandAndObject(Hand *hand, Body *object)
{
    mHand = hand;
    mObject = object;
}



 double PartialViewContactEnergyCalculator::computeEnergy(bool debug) const
 {
     VirtualContact *contact;
     vec3 contactToBodyPointVec, n, cn;
     vec3 objectNormal;

     double totalError = 0;

     //number of contacts that intersect the mesh at points
     // visible to the camera
     int num_visible = 0;

     for (int i=0; i<mHand->getGrasp()->getNumContacts(); i++)
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

             if(debug)
             {
                 std::cout << "dist: " << dist << " cn%n: " << cn % n << std::endl;
                 std::cout << "object normal: " << objectNormal << std::endl;
                 std::cout << "contact normal: " << cn << std::endl;
             }

             //if camera to body_point and bodypoint to vc normals
             //have common direction, then contact point is visible.
             if ( bodyContactPointInWorld % n > 0)
             {
                 num_visible += 1;
                 double d = 1 - cn % n;

                 //beter grasps have contacts closer to the object
                 totalError += dist;

                 //better grasps contacts normals are better aligned with the object normal at
                 //projected point of contact
                 totalError += d * 1000.0 / 2.0;
             }
             //we are not contacting a visible portion of the model.  add a minor
             //penalty or else all the virtual contacts will attempt to NOT be
             //aligned with the model
             else
             {
                 totalError += dist/100;
                 double d = 1 ;
                 totalError += d * 10.0 / 2.0;
             }

     }

     totalError /= num_visible;

     return totalError;
 }


