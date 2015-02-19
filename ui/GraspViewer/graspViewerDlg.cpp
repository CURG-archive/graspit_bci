
#include "graspViewerDlg.h"

#include "grasp.h"
#include "body.h"
#include "robot.h"
#include "searchState.h"
#include "stdio.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>

#include <QLabel>

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
    graspNumLabel->setText(QString("Grasp: ") +
                           QString::number(graspIndex) +
                           QString("/") +
                           QString::number(grasps.size()));
    showGrasp();
}

void GraspViewerDlg::nextGraspButton_clicked()
{
    graspIndex++;

    if(graspIndex == grasps.size())
    {
        graspIndex--;
    }
    graspNumLabel->setText(QString("Grasp: ") +
                           QString::number(graspIndex) +
                           QString("/") +
                           QString::number(grasps.size()));
    showGrasp();
}

void GraspViewerDlg::showGrasp()
{
    double* pose = (grasps.at(graspIndex))->pose;
    double* joints = (grasps.at(graspIndex))->joint_values;
    double energy = (grasps.at(graspIndex))->energy;

    energyLabel->setText(QString("Energy: ") +
                         QString::number(energy));

    vec3 translation = vec3(pose[0], pose[1], pose[2]);
    Quaternion rotation = Quaternion(pose[3], pose[4], pose[5], pose[6]);
    transf handTransform= transf(rotation,translation);

    GraspPlanningState grasp = GraspPlanningState(mHand);
    grasp.setPositionType(SPACE_COMPLETE);
    grasp.mPosition->setTran(handTransform * mObject->getTran());
    grasp.mPosture->storeHandDOF(joints);
    grasp.execute(mHand);
}




void GraspViewerDlg::loadButton_clicked()
{
    std::ostringstream filename;
    QString dir = QString(getenv("GRASPS_PATH")) + QString("/");
    filename << dir.toStdString() << mObject->getName().toStdString() << "/grasps.txt";

    std::cout << filename.str() << std::endl;

    std::ifstream iFile;
    iFile.open(filename.str());

    if (iFile.is_open())
    {
        while(!iFile.eof())
        {
            char buf[200];

            iFile.getline(buf, sizeof(buf));

            if (strncmp(" graspId", buf, 8) != 0) {
                continue;
            }

            Grasp *g = new Grasp;

            const char* token[10] = {};

            //energy
            iFile.getline(buf, sizeof(buf));


            token[0] = strtok(buf, " ");
            token[1] = strtok(0, " ");
            g->energy = atof(token[1]);

            
            //joint angles
            iFile.getline(buf, sizeof(buf));
            //pose
            iFile.getline(buf, sizeof(buf));

            token[0] = strtok(buf, " ");
            strtok(0, " ");
            g->pose = new double[ 7 ];
            for (int i=0; i<7; i++) {
                token[i+1] = strtok(0, " ");
                g->pose[i] = atof(token[i+1]);
            }

            //joint values
            iFile.getline(buf, sizeof(buf));
            token[0] = strtok(buf, " ");
            g->joint_values = new double[ 8 ];
            for (int i=0; i<8; i++) {
                token[i+1] = strtok(0, " ");
                g->joint_values[i] = atof(token[i+1]);
            }

            grasps.push_back(g);

        }
        std::cout << "All grasps loaded..." <<std::endl;
    }
    else
    {
        std::cout << "Error reading grasps, file is not open" << std::endl;
    }

    iFile.close();

    //enable prev and next buttons, disable load button
    loadGrasps->setEnabled(FALSE);
    nextGrasp->setEnabled(TRUE);
    prevGrasp->setEnabled(TRUE);

    nextGraspButton_clicked();

}


void GraspViewerDlg::destroy()
{

}









