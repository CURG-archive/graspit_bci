

#include "BCI/controller_scene/controller_scene_mgr.h"
#include "BCI/controller_scene/sprites.h"
#include "debug.h"
#include "Inventor/nodes/SoAnnotation.h"
#include <Inventor/SoDB.h>
#include "BCI/bciService.h"



ControllerSceneManager::ControllerSceneManager(SoAnnotation *control_scene_separator_)
    : control_scene_separator(control_scene_separator_),
      state(CursorState::SPINNING)
{

    //cursor = new Cursor(control_scene_separator, QString("sprites/cursor_scaled.png"), -0.05, 0.0, 0.0);
    cursor = new Cursor(control_scene_separator, QString("sprites/cursor_scaled.png"), -.8, -1.0, 0.0);
    //cursor = new Cursor(control_scene_separator, QString("sprites/cursor_scaled.png"), -0.1, 0.05, 0.0);

}

void ControllerSceneManager::addTarget(Target *t)
{
    targets.push_back(t);
}

void ControllerSceneManager::clearTargets()
{
    for(int i = 0; i < targets.size(); i++)
    {
        SoDB::writelock();
        control_scene_separator->removeChild(targets[i]->sprite_root);
        SoDB::writeunlock();
    }
    targets.clear();
//    Target *t  = new Target(control_scene_separator, QString("sprites/cursor_scaled.png"), 0.05, 0.0, 0.0);
//    Target *t2  = new Target(control_scene_separator, QString("sprites/cursor_scaled.png"), 0.025, 0.025, 0.0);
//    targets.push_back(t);
//    targets.push_back(t2);
}

void ControllerSceneManager::update()
{
    float pixels_per_inch = BCIService::getInstance()->bciRenderArea->getViewportRegion().getPixelsPerInch();
    short renderAreaWidth = BCIService::getInstance()->bciRenderArea->getSize()[0];
    short renderAreaHeight = BCIService::getInstance()->bciRenderArea->getSize()[1];

    cursor->update(state, pixels_per_inch, renderAreaWidth, renderAreaHeight);
    for(int i = 0; i < targets.size(); i++)
    {
        targets[i]->update(state, pixels_per_inch, renderAreaWidth, renderAreaHeight);
        if (targets[i]->intersects(cursor->bounding_rect))
        {
            targets[i]->setHit();
            std::cout<< "Collision!!!" << std::endl;
        }
    }
}

void ControllerSceneManager::setState(int _state)
{
        state = _state;
}

