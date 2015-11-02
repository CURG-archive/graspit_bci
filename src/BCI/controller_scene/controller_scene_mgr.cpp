

#include "BCI/controller_scene/controller_scene_mgr.h"
#include "BCI/controller_scene/sprites.h"
#include "debug.h"
#include "Inventor/nodes/SoAnnotation.h"
#include <Inventor/SoDB.h>
#include "BCI/bciService.h"
#include "Inventor/nodes/SoEventCallback.h"
#include <Inventor/events/SoMouseButtonEvent.h>


ControllerSceneManager *ControllerSceneManager::current_control_scene_manager =NULL;


ControllerSceneManager::ControllerSceneManager(SoAnnotation *control_scene_separator_)
    : control_scene_separator(control_scene_separator_),
      state(CursorState::SPINNING)
{
    SoEventCallback *mouseEventCB = new SoEventCallback;
    mouseEventCB->addEventCallback(SoMouseButtonEvent::getClassTypeId(), handleMouseButtonEvent);
    control_scene_separator->addChild(mouseEventCB);

    cursor = new Cursor(control_scene_separator, QString("sprites/cursor_scaled.png"), -.8, -1.0, 0.0);
    current_control_scene_manager = this;
}

void ControllerSceneManager::addTarget(std::shared_ptr<Target> t)
{
    this->lock();
    targets.push_back(t);
    this->unlock();
}

void ControllerSceneManager::clearTargets()
{

    this->lock();
    for(int i = 0; i < targets.size(); i++)
    {
        targets[i]->valid = false;
        SoDB::writelock();
        control_scene_separator->removeChild(targets[i]->sprite_root);
        SoDB::writeunlock();
    }
    targets.clear();
    this->setCursorPosition(-1,0,0);
    this->unlock();
}

void ControllerSceneManager::setCursorPosition(double x, double y, double theta)
{
    cursor->setXYTheta(x, y, theta );
}

void ControllerSceneManager::update()
{
    short renderAreaWidth = BCIService::getInstance()->bciRenderArea->getSize()[0];
    short renderAreaHeight = BCIService::getInstance()->bciRenderArea->getSize()[1];

    if(this->try_lock())
    {
        cursor->update(state, renderAreaWidth, renderAreaHeight);
        for(int i = 0; i < targets.size(); i++)
        {
            targets[i]->update(state, renderAreaWidth, renderAreaHeight);
            if (targets[i]->intersects(cursor->bounding_rect))
            {
                targets[i]->setHit();
            }
        }
        this->unlock();
    }


}



void ControllerSceneManager::handleMouseButtonEvent(void *, SoEventCallback *eventCB)
{
  const SoEvent *event = eventCB->getEvent();
  if (SO_MOUSE_RELEASE_EVENT(event,BUTTON1))
  {

      short renderAreaWidth = BCIService::getInstance()->bciRenderArea->getSize()[0];
      short renderAreaHeight = BCIService::getInstance()->bciRenderArea->getSize()[1];
      double x = event->getPosition().getValue()[0]/500.0 - renderAreaWidth/1000.0;
      double y = event->getPosition().getValue()[1]/500.0 - renderAreaHeight/1000.0;

//      std::cout << "width: " << renderAreaWidth << std::endl;
//      std::cout << "height: " << renderAreaHeight << std::endl;
//      std::cout << "click_X: " << event->getPosition().getValue()[0] << std::endl;
//      std::cout << "click_Y: " << event->getPosition().getValue()[1] << std::endl;
//      std::cout << "x: " << x << std::endl;
//      std::cout << "y: " << y << std::endl;
      current_control_scene_manager->setCursorPosition(x, y, 0);
  }


}




void ControllerSceneManager::setState(int _state)
{
        state = _state;
}

