#ifndef _heatmapenergycalculator_h_
#define _heatmapenergycalculator_h_

#include <ostream>
#include <list>
#include <QObject>

#include "matvec3D.h"
#include <flann/flann.hpp>
#include "image_geometry/pinhole_camera_model.h"

class Hand;
class Body;
class QualityMeasure;
class GraspPlanningState;


class HeatmapEnergyCalculator : public QObject
{
    Q_OBJECT
protected:

    Hand *mHand;
    Body *mObject;
    std::string capturedMeshDir;

    bool heatmaps_inited;
    int num_heatmaps;
    int height;
    int width;

    int num_grasp_priors;
    int num_joints;

    std::vector<std::vector<std::vector<double> > > heatmaps;
    std::vector<std::vector<std::vector<double> > > rgbd;
    image_geometry::PinholeCameraModel mCameraModel;

    flann::Index<flann::L2<double> > *grasp_priors_index_;
    flann::Matrix<double> *grasp_priors_matrix;

    void initCameraModel();

    void readHeatmapConfig();
    void readGraspPriorConfig();

    void updateGraspPriors();
    void updateHeatmaps();
    void updateRGBD();
    void saveImage(int heatmap_index, int height_intersect, int width_intersect) const;

    int getGraspType() const;

public:
    HeatmapEnergyCalculator();

    void setHandAndObject(Hand *h, Body *o);
    void setDir(QString dir);

    double heatmapProjectionEnergy(bool debug) const;

};


#endif
