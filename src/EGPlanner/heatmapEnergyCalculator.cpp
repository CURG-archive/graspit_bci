

#include "EGPlanner/heatmapEnergyCalculator.h"
#include <time.h>
#include <sys/stat.h> //for checking if directory already exists in saveImage and mkdir

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



HeatmapEnergyCalculator::HeatmapEnergyCalculator():
    heatmaps_inited(false)
{
    initCameraModel();
}

void HeatmapEnergyCalculator::setHandAndObject(Hand *hand, Body *object)
{
    mHand = hand;
    mObject = object;
}

void HeatmapEnergyCalculator::initCameraModel()
{
    sensor_msgs::CameraInfo cam_info;

    double D[] = {0.0, 0.0, 0.0, 0.0, 0.0};
    std::vector<double> D_vec (D, D + sizeof(D) / sizeof(double) );

    boost::array<double, 9> K =  {{525.0, 0.0, 319.5, 0.0, 525.0, 239.5, 0.0, 0.0, 1.0}};
    boost::array<double, 9> R =  {{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}};
    boost::array<double, 12> P =  {{525.0, 0.0, 319.5, 0.0, 0.0, 525.0, 239.5, 0.0, 0.0, 0.0, 1.0, 0.0}};

    cam_info.height = 480;
    cam_info.width = 640;
    cam_info.distortion_model = "plumb_bob";
    cam_info.D = D_vec;
    cam_info.K = K;
    cam_info.R = R;
    cam_info.P = P;
    cam_info.binning_x = 0;
    cam_info.binning_y = 0;
    cam_info.roi.x_offset = 0;
    cam_info.roi.y_offset = 0;
    cam_info.roi.height = 0;
    cam_info.roi.width = 0;
    cam_info.roi.do_rectify = false;

    mCameraModel.fromCameraInfo(cam_info);
}

void HeatmapEnergyCalculator::setDir(QString dir)
{
    std::cout << "setting dir" << std::endl;
    if(!heatmaps_inited)
    {
        char* graspit_path;
        graspit_path = getenv ("GRASPIT");

        std::ostringstream pathStream;
        pathStream  << graspit_path << "/models/captured_meshes/"  << dir.toStdString().c_str() << "/";
        capturedMeshDir = pathStream.str();

        updateGraspPriors();
        updateHeatmaps();
        updateRGBD();

        heatmaps_inited = true;
    }
}


void HeatmapEnergyCalculator::updateGraspPriors()
{
    readGraspPriorConfig();

    double value;
    std::ifstream iFile;

    std::vector<double*> grasp_priors;

    for(int i=0; i < num_grasp_priors; i++)
    {
        std::ostringstream filename;
        filename << capturedMeshDir << "grasp_priors/" << i << ".txt";
        iFile.open(filename.str());

        if (iFile.is_open())
        {
            double *joint_values = new double[ num_joints ];

            for (int j=0; j <num_joints; j++)
            {
                iFile >> value;
                joint_values[j] = value;
            }

            grasp_priors.push_back(joint_values);
        }
        else
        {
            std::cout<< "FAILED TO OPEN FILE: " <<  filename.str() << std::endl;
        }

        iFile.close();

    }

    grasp_priors_matrix = new flann::Matrix<double>(grasp_priors[0], num_grasp_priors, num_joints);
    grasp_priors_index_ = new flann::Index< flann::L2<double> >(*grasp_priors_matrix, flann::KDTreeIndexParams(1));
    grasp_priors_index_->buildIndex();
}

void HeatmapEnergyCalculator::readHeatmapConfig()
{
    std::ifstream configFile;
    std::ostringstream filename;
    filename << capturedMeshDir << "heatmapsConfig.txt" ;
    std::ifstream iFile;
    iFile.open(filename.str());
    if (iFile.is_open())
    {
        iFile >> num_heatmaps >> height >> width;
    }
    else
    {
        std::cout << "Error reading heatmapsConfig.txt, file is not open" << std::endl;
    }

    std::cout << "num_heatmaps: " << num_heatmaps << std::endl;
    std::cout << "height: " << height << std::endl;
    std::cout << "width: " << width << std::endl;
}

void HeatmapEnergyCalculator::readGraspPriorConfig()
{
    std::ifstream configFile;
    std::ostringstream filename;
    filename << capturedMeshDir << "graspPriorsConfig.txt" ;
    std::ifstream iFile;
    iFile.open(filename.str());

    if (iFile.is_open())
    {
        iFile >> num_grasp_priors >> num_joints;
    }
    else
    {
        std::cout << "Error reading graspPriorsConfig.txt, file is not open" << std::endl;
    }

    std::cout << "num_grasp_priors: " << num_grasp_priors << std::endl;
    std::cout << "num_joints: " << num_joints << std::endl;
}


void HeatmapEnergyCalculator::updateHeatmaps()
{
    readHeatmapConfig();

    heatmaps.resize(num_heatmaps);
    for(int i=0; i < num_heatmaps; i++){

        heatmaps[i].resize(height);

        for(int j=0; j < height; j++){
            heatmaps[i][j].resize(width);
        }
    }

    double value;
    std::ifstream iFile;
    for(int i=0; i < num_heatmaps; i++)
    {
        std::ostringstream filename;
        filename << capturedMeshDir << "heatmaps/" << i << ".txt";

        iFile.open(filename.str());
        if (iFile.is_open())
        {
            for(int j=0; j < height; j++)
            {
                for(int k=0; k < width; k++)
                {
                    iFile >> value;
                    heatmaps[i][j][k] =  value;
                }
            }
        }
        else
        {
            std::cout << "Error reading heatmaps, file is not open" << std::endl;
        }

        iFile.close();
    }
}

void HeatmapEnergyCalculator::updateRGBD()
{
    rgbd.resize(4);
    for(int i=0; i < 4; i++){

        rgbd[i].resize(480);

        for(int j=0; j < 480; j++){
            rgbd[i][j].resize(640);
        }
    }

    double value;
    std::ifstream iFile;

    std::ostringstream filename;
    filename << capturedMeshDir << "rgbd.txt";

    iFile.open(filename.str());
    if (iFile.is_open())
    {
        for(int i = 0; i < 4; i++)
            {
            for(int j=0; j < 480; j++)
            {
                for(int k=0; k < 640; k++)
                {
                    iFile >> value;
                    rgbd[i][j][k] =  value*255;
                }
            }
        }
    }
    else
    {
        std::cout << "Error reading rgbd, file is not open" << std::endl;
    }

    iFile.close();
}


int HeatmapEnergyCalculator::getGraspType(std::vector<std::vector<int> > neighbors, std::vector<std::vector<double> > neighbor_distances) const
 {
     std::vector<double> query_joint_state;

     double *joint_values = new double[ mHand->getNumJoints() ];

     mHand->getJointValues(joint_values);

     for (int joint_index=0; joint_index < mHand->getNumJoints(); ++joint_index)
     {
         query_joint_state.push_back(joint_values[joint_index]);
     }

     //std::vector<std::vector<int> > neighbors;
     flann::Matrix<double> query_pose_mat(&query_joint_state[0], 1, mHand->getNumJoints());

     //std::vector<std::vector<double> >neighbor_distances;
     flann::SearchParams params(32, 0.0, true);
     double max_neighbor_distance = 30.0;

     grasp_priors_index_->radiusSearch(query_pose_mat, neighbors,  neighbor_distances, max_neighbor_distance, params);

     //no grasp types near this configuration
     //so retrun -1, we are currently in a hand configuraton unlike any of the grasp types
     if (neighbors.empty() || neighbors.at(0).empty())
     {
        return -1;
     }
     else
     {
         return neighbors[0][0];
     }
 }

 double HeatmapEnergyCalculator::heatmapProjectionEnergy(bool debug) const
 {
     double heatmap_quality = 0;
     VirtualContact *contact;

     std::vector<std::vector<int> > neighbors;
     std::vector<std::vector<double> > neighbor_distances;

     int grasp_type = getGraspType(neighbors, neighbor_distances);

     if(debug)
     {
         std::cout << "grasp_type: " << grasp_type << std::endl;
     }

     //we are not close to any grasp type
     //so we are not in a good hand configuration.
     if (grasp_type == -1)
     {
         return 1000;
     }

     for (int i=0; i<mHand->getGrasp()->getNumContacts(); i++)
     {

         int PALM_INDEX = 0;
         int FINGER_1_INDEX = 7;
         int FINGER_2_INDEX = 11;
         int FINGER_3_INDEX = 15;


         //NEED TO ENSURE THAT WE HAVE PALM OR F1, F2, F3, VC
         if(i==PALM_INDEX || i == FINGER_1_INDEX || i == FINGER_2_INDEX || i ==FINGER_3_INDEX)
         {
             contact = (VirtualContact*)mHand->getGrasp()->getContact(i);

             //Need to get contact location in relation to camera:
             position worldPoint = contact->getWorldLocation();

             //Need to get uv from image_geometry camera model
             cv::Point3d worldPointCV(-worldPoint.x()/1000,-worldPoint.y()/1000,worldPoint.z()/1000);
             cv::Point2d uv = mCameraModel.project3dToPixel(worldPointCV);

             int heatmap_index = 4*grasp_type ;

             if (i == PALM_INDEX)
             {
                 heatmap_index += 0;
             }
             else if(i==FINGER_1_INDEX)
             {
                 heatmap_index += 1;
             }
             else if(i==FINGER_2_INDEX)
             {
                 heatmap_index += 2;
             }
             else if(i==FINGER_3_INDEX)
             {
                 heatmap_index += 3;
             }

             int width_index = int(uv.x)+ int((640-width)/2) ; //uv.x is between 0 and 640
             int height_index = int(uv.y)+ int((480-height)/2) ;  //uv.y is between 0 and 480

             if(debug)
             {
                 std::cout << "World Point in Meters: " << worldPointCV << std::endl;
                 std::cout << "UV for rgbd image: " << uv << std::endl;
                 std::cout << "Heatmap index is: " << heatmap_index << std::endl;
                 std::cout << "width: " << width_index  << " height: " << height_index<< std::endl;
             }


             if (heatmaps[heatmap_index].size() > height_index && height_index > 0)
             {
                 if (heatmaps[heatmap_index][height_index].size() > width_index && width_index > 0)
                 {
                     heatmap_quality += heatmaps[heatmap_index][480-height_index][640-width_index];

                     if (debug)
                     {
                         std::cout << "HeatMapQuality: " << heatmaps[heatmap_index][480-height_index][640-width_index] << std::endl;
                         saveImage(heatmap_index, height_index, width_index);
                     }

                 }
                 else
                 {
                     heatmap_quality += 100;
                 }

             }
             else{
                 heatmap_quality += 100;
             }

         }
     }

     return heatmap_quality;

 }


 void HeatmapEnergyCalculator::saveImage(int heatmap_index, int height_intersect, int width_intersect) const
 {
     std::string saveImgDir = capturedMeshDir + "imgs/";
     struct stat status = {0};
     if ( stat(saveImgDir.c_str(), &status) == -1 ) {
         mkdir(saveImgDir.c_str(), 0700);
     }


     QImage *img = new QImage(width, height, QImage::Format_RGB16);

     for (int height_index = 0; height_index < height; ++height_index)
     {
         for (int width_index = 0; width_index < width; ++width_index)
         {
            double value = heatmaps[heatmap_index][height_index][width_index];
            img->setPixel(width_index, height_index,qRgb(value, value, value) );
         }
     }

     VirtualContact *contact;
     for (int index=0; index<mHand->getGrasp()->getNumContacts(); index++)
     {

         contact = (VirtualContact*)mHand->getGrasp()->getContact(index);
         //Need to get contact location in relation to camera:
         position worldPoint = contact->getWorldLocation();

         //Need to get uv from image_geometry camera model
         cv::Point3d worldPointCV(-worldPoint.x()/1000,-worldPoint.y()/1000,worldPoint.z()/1000);
         cv::Point2d uv = mCameraModel.project3dToPixel(worldPointCV);

         int width_index = int(uv.x)+int((640-width)/2) ; //uv.x is between 0 and 640
         int height_index = int(uv.y)+int((480-height)/2) ;  //uv.y is between 0 and 480

         for(int i=-4; i < 4; i++)
         {
             for(int j=-4; j < 4; j++)
             {
                 img->setPixel(640-width_index + i, 480-height_index+ j, qRgb(0, 0, 254));
             }
         }
     }

     for(int i=-4; i < 4; i++)
     {
         for(int j=-4; j < 4; j++)
         {
             img->setPixel(640- width_intersect + i, 480 -height_intersect+ j, qRgb(254, 0, 0));
         }
     }


     QString debugFileName = QString(saveImgDir.c_str()) + QString("heatmap_" + QString::number(heatmap_index) + ".png");
     img->save(debugFileName);
 }


