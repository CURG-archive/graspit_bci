#include "BCI/worldController.h"
#include "BCI/bciService.h"
#include "debug.h"
#include "BCI/utils/worldElementTools.h"
#include "BCI/utils/uiTools.h"



namespace bci_experiment {
	using world_element_tools::getWorld;

    WorldController *WorldController::worldController = NULL;

    WorldController *WorldController::getInstance() {
        if (!worldController) {
            worldController = new WorldController();
        }

        return worldController;
    }

    WorldController::WorldController(QObject *parent) :
            QObject(parent) {
    }

    void WorldController::highlightAllBodies() {
        ui_tools::highlightAll();
    }

    void WorldController::unhighlightAllBodies() {
        ui_tools::unhighlightAll();
    }

    void WorldController::highlightCurrentBody(GraspableBody *currentTarget) {
        ui_tools::highlightCurrentGraspableBody(currentTarget);
    }


}
