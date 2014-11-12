#ifndef EGPLANNERRENDERER_H
#define EGPLANNERRENDERER_H
#include <QObject>
#include "EGPlanner/egPlanner.h"
#include <graspitGUI.h>
#include <graspitGUI.h>
#include "ivmgr.h"
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>


class ForceRenderer : public QObject
{
    Q_OBJECT

public slots:
    void render(EGPlanner * planner)
    {
        SoQtExaminerViewer *ra = dynamic_cast<SoQtExaminerViewer *>(GraspItGUI::getInstance()->getIVmgr()->getViewer());
        if(ra)
        {
            ra->render();
            ra->getRenderAreaWidget()->update();
        }
    }
};

#endif // EGPLANNERRENDERER_H
