#ifndef CONTROLLERSCENEMGR_H
#define CONTROLLERSCENEMGR_H


#include <vector>
#include "qobject.h"

class Cursor;
class Target;
class SoAnnotation;

enum CursorState{SPINNING = 0, MOVING_SLOW = 1, MOVING_FAST = 2};

class ControllerSceneManager: public QObject {

    Q_OBJECT ;


protected:

    int state;
    Cursor *cursor;
    std::vector<Target *> targets;



public:
    ControllerSceneManager(SoAnnotation *control_scene_separator_);

    virtual ~ControllerSceneManager() {};
    void addTarget(Target *t);
    void clearTargets();

    SoAnnotation * control_scene_separator;

public slots:

    void update();
    void setState(int state);

};

#endif // CONTROLLERSCENEMGR_H
