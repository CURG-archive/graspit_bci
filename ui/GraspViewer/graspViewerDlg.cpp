
#include "graspViewerDlg.h"

#include "grasp.h"
#include "body.h"
#include "robot.h"
#include "searchState.h"
#include "stdio.h"
#include <fstream>
#include <sstream>


void GraspViewerDlg::setMembers( Hand *h, GraspableBody *b )
{
  mHand = h;
  mObject = b;

  mHand->getGrasp()->setObjectNoUpdate(mObject);

  mHandObjectState = new GraspPlanningState(mHand);
  mHandObjectState->setObject(mObject);
  mHandObjectState->setRefTran(mObject->getTran());
}


void GraspViewerDlg::prevGraspButton_clicked()
{
    graspIndex--;

    if (graspIndex < 0)
    {
        graspIndex++ ;
    }

    showGrasp();
}


void GraspViewerDlg::nextGraspButton_clicked()
{
    graspIndex++;

    if(graspIndex = poses.size())
    {
        graspIndex--;
    }

    showGrasp();
}

void GraspViewerDlg::showGrasp()
{
    double* pose = poses.at(graspIndex);
    double* joints = jointValues.at(graspIndex);

    vec3 translation = vec3(pose[0], pose[1], pose[2]);
    Quaternion rotation = Quaternion(pose[3], pose[4], pose[5], pose[6]);
    transf handTransform= transf(rotation,translation);

    GraspPlanningState grasp = GraspPlanningState(mHand);
    grasp.setPositionType(SPACE_COMPLETE);
    grasp.mPosition->setTran(handTransform);
    grasp.mPosture->storeHandDOF(joints);
    grasp.execute(mHand);
}




void GraspViewerDlg::loadButton_clicked()
{
    std::ostringstream filename;
    filename <<  mObject->getName().toStdString() << "/grasps.txt";

    std::cout << filename.str() << std::endl;

    std::ifstream iFile;
    iFile.open(filename.str());

    double value;
    double *joint_values = new double[ 8 ];
    double *pose = new double[ 7 ];
    if (iFile.is_open())
    {
        for(int j=0; j < 8; j++)
        {
            iFile >> value;
            joint_values[j] =  value;
        }

        for(int j=0; j < 7; j++)
        {
            iFile >> value;
            pose[j] =  value;
        }
    }
    else
    {
        std::cout << "Error reading grasps, file is not open" << std::endl;
    }

    iFile.close();
}


void GraspViewerDlg::destroy()
{

}









