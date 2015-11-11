#ifndef CONTROLLERSCENEMGR_H
#define CONTROLLERSCENEMGR_H


#include <vector>
#include "qobject.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <Inventor/nodes/SoCallback.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/nodes/SoEventCallback.h>
class Cursor;
class Target;
class SoAnnotation;

enum CursorState{SPINNING = 0, MOVING_SLOW = 1, MOVING_FAST = 2};



class ControllerSceneManager: public QObject {

    Q_OBJECT ;


protected:

    int state;
    Cursor *cursor;
    std::vector<std::shared_ptr<Target>> targets;
    std::vector<std::shared_ptr<Target>> temp_targets;

    boost::recursive_mutex mtx_;

    void lock() {
        mtx_.lock();
    }
    void unlock() {
        mtx_.unlock();
    }
    bool try_lock() {
        return mtx_.try_lock();
    }



public:
    ControllerSceneManager(SoAnnotation *control_scene_separator_);

    virtual ~ControllerSceneManager() {};
    void addTarget(std::shared_ptr<Target> t);
    void clearTargets();
    void setCursorPosition(double x, double y,  double theta);
    static void handleMouseButtonEvent(void *,SoEventCallback *eventCB);


    static ControllerSceneManager *current_control_scene_manager;
    SoAnnotation * control_scene_separator;

public slots:

    void update();
    void setState(int state);



};



#endif // CONTROLLERSCENEMGR_H
