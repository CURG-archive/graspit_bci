#ifndef HANDVIEW_H_
#define HANDVIEW_H_

#include <Q3MainWindow>
#include <QHboxLayout>
#include <QVBoxLayout>
//#include <QMainWindow>
#include <QString>
#include <QObject>
#include <QtGui/QShortcut>
#include <set>

#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <exception>
#include <stdexcept>
#include <typeinfo>

#include <q3listbox.h>
#include <QApplication>
#include <QThread>

#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/SoOffscreenRenderer.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoGetMatrixAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoBoxHighlightRenderAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/draggers/SoHandleBoxDragger.h>
#include <Inventor/draggers/SoRotateDiscDragger.h>
#include <Inventor/draggers/SoCenterballDragger.h>
#include <Inventor/draggers/SoTranslate1Dragger.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoKeyboardEvent.h>
#include <Inventor/fields/SoSFTime.h>
#include <Inventor/manips/SoHandleBoxManip.h>
#include <Inventor/manips/SoCenterballManip.h>
#include <Inventor/nodekits/SoWrapperKit.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoBlinker.h>
#include <Inventor/nodes/SoColorIndex.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoCoordinate4.h>
#include <Inventor/nodes/SoComplexity.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoDrawStyle.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/nodes/SoFont.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoIndexedFaceSet.h>
#include <Inventor/nodes/SoLight.h>
#include <Inventor/nodes/SoLightModel.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoNurbsCurve.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoPickStyle.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoSelection.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoFaceSet.h>
#include <Inventor/nodes/SoTransformSeparator.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/nodes/SoFile.h>
#include <Inventor/sensors/SoIdleSensor.h>
#include <Inventor/sensors/SoNodeSensor.h>
#include <Inventor/SoSceneManager.h>
#include <Inventor/Qt/SoQt.h>
#include <QTimer>

#include "pointers.dat"
#include "ivmgr.h"
#include "SoArrow.h"
#include "SoTorquePointer.h"
#include "SoComplexShape.h"
#include "robot.h"
#include "joint.h"
#include "humanHand.h"
#include "body.h"
#include "contact.h"
#include "grasp.h"
#include "world.h"
#include "mainWindow.h"
#include "matvec3D.h"
#include "graspitGUI.h"
#include "humanHand.h"
#include "body.h"

#include "EGPlanner/searchState.h"
class SoQtRenderArea;
class SoQtExaminerViewer;
class SoCoordinate3;
class SoIndexedFaceSet;
class SoSeparator;
class QWidget;
class GWS;
class HandObjectState;
class Hand;

class HandView
{
private:
  //! A pointer to the QT widget holding the view proj wirn
  QFrame *viewWin;
  
  //! A pointer to the viewer for this window
  SoQtRenderArea *viewViewer;

  //! Root of local scene graph for view
  SoSeparator * IVRoot;

  //! Root of hand geometry for graph
  SoSeparator * IVHandGeometry;
  //! Root of 
  SoSeparator * IVObjectGeometry;

  //! The unique identifier of the view
  QString viewName_;

  //! The Examiner View used to initializr the views
  SoQtExaminerViewer * mainViewer_;
  SoCamera * camera_;

  //! Copy the hand geometry
  void setupIVHandGeometry(Hand * h);
  int stateID_;
public:

  //! Constructor for the HandView
  HandView(SoQtExaminerViewer *mainViewer, Hand * h, QFrame &parentWindow, QString viewName);
   
  //! Destructor
  ~HandView();
 
 
  //! Update handles the processing of viewWindows as they get moved about
  bool update(HandObjectState & s, Hand & cloneHand);

  //! Getter for RenderArea
  SoQtRenderArea * getRenderArea(){return viewViewer;}
  
  //! Getter for viewName
  QString getName();

  //! Getter for the IVRoot
  SoSeparator * getIVRoot();

  //! Getter for the viewViewer
  SoQtRenderArea * getViewViewer();
  
  //! Setter for the viewViewer
  void setViewViewer(SoQtRenderArea * newView);
  
  //! Getter for the viewWindow
  QFrame * getViewWindow();
};
  


class HandViewWindow{

  //! viewsList
  std::vector<HandView *> views;

  //! The layout for the thumbnail views
  QGridLayout * grid;
  QHBoxLayout * hbox;
  QVBoxLayout * vbox1;
  QVBoxLayout * vbox2;
  //! The container for the layout for the preview (created so that we
  //   can use more than one preview window if so desired with different graphics applied)
  QGridLayout * previewGrid;
  QWidget * stageFrame_;
  //! MaxViewSize - protect against over access on number of loaded thumbnail views
  unsigned int maxViewSize;

  //! The copied hand we use as a temporary storage as we load thumbnail and preview - views
  Hand * cloneHand;

  //! Index of current Handview stored in the previewWindow
  int currentPreview;

  QRect geom_;

public:
  //! the window containing the thumbnail views
  QFrame * handViewWindow;
  QFrame * viewHolder;
  //! ConstructorNUL
  HandViewWindow(QWidget * parent, Hand * h, const QRect & geom, SoNode * IVRoot = NULL, QWidget * stageFrame = NULL);
  void initViews(Hand * h);
  ~HandViewWindow(){
    for (int i = 0; i < views.size(); ++i)
      delete views[i];
    delete cloneHand; 
    delete handViewWindow;
  }
  //! Actually adds views at loop of initialization
  bool addView(HandObjectState & s, int i);
  void clearViews();
  //! Simple getter for the viewWindow associated with our object
  QFrame* getViewWindow(){
    return handViewWindow;
  }

 /* //! To determine which view appears in the preview window
  void updateList();
*/
  //! Skip directly to the desired thumbnail frame
  void setCurrentView(int num);

  //! Returns the current grasp (as seen in the preview window)
  int getCurrentGrasp();
};


#endif
