

#include "BCI/controller_scene/controller_scene_mgr.h"
#include "BCI/controller_scene/sprites.h"
#include "debug.h"
#include "Inventor/nodes/SoAnnotation.h"
#include <Inventor/SoDB.h>

ControllerSceneManager::ControllerSceneManager(SoAnnotation *control_scene_separator_)
    : control_scene_separator(control_scene_separator_),
      state(CursorState::SPINNING)
{

    cursor = new Cursor(control_scene_separator, QString("sprites/cursor_scaled.png"), -1.0, 0.0, 0.0);
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
}

void ControllerSceneManager::update()
{
    cursor->update(state);
    for(int i = 0; i < targets.size(); i++)
    {
        targets[i]->update(state);
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

