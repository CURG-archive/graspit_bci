//######################################################################
//
// GraspIt!
// Copyright (C) 2002-2009  Columbia University in the City of New York.
// All rights reserved.
//
// GraspIt! is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GraspIt! is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GraspIt!.  If not, see <http://www.gnu.org/licenses/>.
//
// Author(s): Matei T. Ciocarlie
//
// $Id: egPlannerDlg.cpp,v 1.9.2.1 2009/07/23 21:18:13 cmatei Exp $
//
//######################################################################

#include <algorithm>

#include "egPlannerDlg.h"

#include <QGridLayout>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QValidator>
#include <QFileDialog>
#include "search.h"
#include "searchState.h"
#include "body.h"
#include "robot.h"
#include "egPlanner.h"
#include "eigenGrasp.h"

#include "world.h"
#include "graspitGUI.h"
#include "ivmgr.h"
#include "contactExaminerDlg.h"
#include "onLinePlanner.h"
#include "timeTest.h"
#include "guidedPlanner.h"
#include "loopPlanner.h"
#include <QPainter>
#include <QDesktopWidget>

//Graspit DB stuff
#include "DBase/graspit_db_grasp.h"
#include "DBase/graspit_db_model.h"


//#define GRASPITDBG
#include "debug.h"

#include "bciStageFrame.h"
#include <fstream>

#define X_WINDOW_OFFSET 0

namespace bci_experiment{

  World * getWorld()
  {
    return graspItGUI->getIVmgr()->getWorld();
  }
  //! Gets first object that matches the given name.
  Body * getObjectByName(const QString & objectName)
  {    
    World * w = getWorld();
    Body * b = NULL;
    for(int i = 0; i < w->getNumBodies(); ++i)
      {
	if(w->getBody(i)->getName() == objectName)
	  b = w->getBody(i);
      }
    return b;
  }

  //! Brief helper to add an object to the world from relative directory, model type, and model filename.
  Body * addToWorld(const QString & relativeModelDir, const QString & modelType, const QString & modelFilename)
  {
    QString bodyFile = QString(getenv("GRASPIT")) + "/" +  relativeModelDir + modelFilename;
    std::cout << "body string: "<< bodyFile.toStdString() << std::endl;
    return graspItGUI->getIVmgr()->getWorld()->importBody(modelType, bodyFile);
  }

  //! If the named body doesn't exist, add it to the world
  Body * addBodyIfUnique(const QString & bodyName)
  {
    Body * newBody = getObjectByName(bodyName);
    if(newBody)
      return newBody;
    newBody = addToWorld("models/objects/","Body", bodyName+".xml");
    
    return newBody;
  }
  
  void highlightBody(Body * b, SbColor color)
  {    
    b->getIVMat()->emissiveColor.setIgnored(false);
    b->getIVMat()->emissiveColor.setValue(color);
    b->getIVMat()->transparency.setIgnored(true);
  }
  
  void unhighlightBody(Body * b)
  {
    b->getIVMat()->emissiveColor.setIgnored(true);
    b->getIVMat()->transparency.setIgnored(false);
  }

  int getGraspableBodyIndex(Body * b)
  {
    World * w = getWorld();
    int bodyIndex = -1;
    while(bodyIndex < w->getNumGB() - 1)
      {	
	++bodyIndex;
	if(w->getGB(bodyIndex) == b)
	  break;
      }
    return bodyIndex;
  }

  GraspableBody * getNextGraspableBody(GraspableBody * b)
  {
    GraspableBody * nextBody = NULL;
    World * w = getWorld();    
    if(b)
      {
	int bodyIndex = getGraspableBodyIndex(b);
	if(bodyIndex == -1)
	  {
	    std::cout << "highlightNextGraspableBody -- ERROR -- attempted to get next of unregistered body -- \n";
	      return NULL;
	  }
	int nextBodyIndex = (bodyIndex + 1) % w->getNumGB();
	nextBody = getWorld()->getGB(nextBodyIndex);
      }
    return nextBody;
  }

  GraspableBody * highlightNextGraspableBody(GraspableBody *b)
  {
    if(!b)
      return b;
    GraspableBody * nextBody = getNextGraspableBody(b);
    if(nextBody)
      {
        highlightBody(b, SbColor(1,0,0));
	      highlightBody(nextBody, SbColor(0,1,0));
      }
    return nextBody;
  }
  
  bool unhighlightAll()
  {
    for(int i = 0; i < getWorld()->getNumGB(); ++i)
    {
      unhighlightBody(getWorld()->getGB(i));
    }
    return true;
  }

  bool highlightAll()
  {
    for(int i = 0; i < getWorld()->getNumGB(); ++i)
    {
      highlightBody(getWorld()->getGB(i), SbColor(1,0,0));
    }
    return true;
  }
  



  void moveBody(Body * b, const transf & relTran)
  {
    b->setTran(b->getTran() * relTran);
  }
  
  void moveAllBodies(const transf & relTran)
  {
    for(int b_ind = 0; b_ind < getWorld()->getNumGB(); ++b_ind)
      {
	      moveBody(getWorld()->getGB(b_ind), relTran);
      }
  }
  

  Body * getOrAddExperimentTable()
  {
    QString bodyName("experiment_table");
    Body * tableBody= getObjectByName(bodyName);
    if(tableBody)
      return tableBody;
    return addToWorld("models/objects/","Obstacle", bodyName+".xml");
  }

  void setObjectCentral(Body * b)
  {
    Body * table = getOrAddExperimentTable();
    moveAllBodies(table->getTran().inverse());
    transf centralize(Quaternion::IDENTITY, -b->getTran().translation());        
    moveAllBodies(centralize);
    
    table->setTran(centralize);
    SoNodeList l;
    unsigned int listLen = SoTransform::getByName("PointCloudTransform", l);
    if(listLen == 1)
    {
      SoTransform * tran = static_cast<SoTransform*>(l[0]);
      centralize.toSoTransform(tran);
    }    
  }

  void sendString(const QString & s)
  {
    emit graspItGUI->getIVmgr()->emitSendString(s);
  }
  
  void sendSetTarget(Body * b)
  {    
    QString setTargetString = "setTarget " + b->getName();
    sendString(setTargetString);    
  }

  void disableNontargetCollisions(Hand * h, GraspableBody * target)
  {
    World * w = getWorld();
    for (int i = 0; i < w->getNumGB(); ++i)
      {
	if(w->getGB(i) != target)
	  w->toggleCollisions(false, w->getGB(i), h);
      }
    w->toggleCollisions(true, target, h);
  }
  
    
  

  void setTableObjectCollisions(bool setting)
  {
    World * w = getWorld();
    Body * experiment_table = getOrAddExperimentTable();
    for (int i = 0; i < w->getNumGB(); ++i)
      w->toggleCollisions(setting, experiment_table, w->getGB(i));
  }

  void disableTableObjectCollisions()
  {
    setTableObjectCollisions(false);
  }


  void viewBodies(std::vector<Body *> & body_vec)
  {
    SoGroup * so = new SoGroup(); 
    so->ref();
    for(unsigned int i = 0; i < body_vec.size(); ++i)      
      so->addChild(body_vec[i]->getIVRoot());
    graspItGUI->getIVmgr()->getViewer()->getCamera()->viewAll(so, graspItGUI->getIVmgr()->getViewer()->getViewportRegion(), 1.2);
    so->removeAllChildren();
    so->unref();
  }

  

  void viewGraspableBodies()
  {
    std::vector<Body *> body_vec;
    for(int gi = 0; gi < getWorld()->getNumGB(); ++gi)
      body_vec.push_back(getWorld()->getGB(gi));
    viewBodies(body_vec);
  }

  void viewHand(Hand * h)
  {    
    graspItGUI->getIVmgr()->getViewer()->getCamera()->viewAll(h->getIVRoot(), graspItGUI->getIVmgr()->getViewer()->getViewportRegion(), 0.5);    
  }

  void viewTarget(Body * b)
  {    
    graspItGUI->getIVmgr()->getViewer()->getCamera()->viewAll(b->getIVRoot(), graspItGUI->getIVmgr()->getViewer()->getViewportRegion(), 1.0);    
    Body * tableBody= getObjectByName("experiment_table");
    SbVec3f table_z = tableBody->getTran().affine().transpose().row(2).toSbVec3f();
    
    
    
    graspItGUI->getIVmgr()->getViewer()->getCamera()->pointAt(SbVec3f(0,0,0), table_z);
  }

  void resetHandCollisions(Hand * h, bool setting, std::vector<bool> & collisionStatus)
  {
    World * w = getWorld();
    
    Body * experiment_table = getOrAddExperimentTable();
    collisionStatus.push_back(!w->collisionsAreOff(h, experiment_table));
    w->toggleCollisions(setting, h, experiment_table);

    for (int i = 0; i < w->getNumGB(); ++i)
    {
      collisionStatus.push_back(!w->collisionsAreOff(h, w->getGB(i)));
      w->toggleCollisions(setting, h, w->getGB(i));
    }
  }

  bool setCollisionState(Hand * h, std::vector<bool> & collisionStatus)
  {
    World * w = getWorld();    
    if(collisionStatus.size() != w->getNumGB() + 1)
      return false;
    
    Body * experiment_table = getOrAddExperimentTable();
    w->toggleCollisions(collisionStatus[0], h, experiment_table);
    for (int i = 0; i < w->getNumGB(); ++i)
    {      
      w->toggleCollisions(collisionStatus[i+1], h, w->getGB(i));
    }
    return true;
  }

  int getNumHandCollisions(Hand * h)
  {
    CollisionReport colReport;
    std::vector<Body *> body_list;
    h->getBodyList(&body_list);
    getWorld()->getCollisionReport(&colReport, &body_list);
    return colReport.size();    
  }

  bool testPreGraspCollisions(Hand * h, float pregrasp_dist)
  {
    h->autoGrasp(false, -2.0, true);
    h->approachToContact(pregrasp_dist, false);
    return (getNumHandCollisions(h));
  }

  bool testGraspCollisions(Hand * h, const GraspPlanningState * s)
  {
    bool result = false;
    std::vector<bool> currentCollisionState;
    resetHandCollisions(h, true, currentCollisionState);
    s->execute(h);
    World * w = getWorld();
    w->toggleCollisions(false, h, s->getObject());
    if(getNumHandCollisions(h))
      result = true;
    if(testPreGraspCollisions(h, -50.0))
      result = true;

    setCollisionState(h, currentCollisionState);
    return result;    
  }


  bool setPointcloudTransparency(double transparency)
  {
    SoMaterial * mat = static_cast<SoMaterial *>(SoMaterial::getByName("PointCloudColorMaterial"));
    if(!mat)
      return false;
    mat->transparency = transparency;
    return true;
  }

  void printTestResult(const GraspPlanningState & s)
  {
    std::cout << "graspID: " << s.getAttribute("graspId") <<  " Test Result: " <<s.getAttribute("testResult") << "\n";
  }

} 

void EigenGraspPlannerDlg::exitButton_clicked()
{
  QDialog::accept();
}

void EigenGraspPlannerDlg::init()
{ 

  this->setWindowState(Qt::WindowMinimized);
  energyBox->insertItem("Hand Contacts");
  energyBox->insertItem("Potential Quality");
  energyBox->insertItem("Contacts AND Quality");
  energyBox->insertItem("Autograsp Quality");
  energyBox->insertItem("Guided Autograsp");
  energyBox->setCurrentItem(1);//CHANGED!
  initShortcut = new QShortcut (Qt::Key_Semicolon, this);
  connect(initShortcut, SIGNAL(activated()), this, SLOT(plannerNext()));
  plannerTypeBox->insertItem("Sim. Ann.");
  plannerTypeBox->insertItem("Loop");
  plannerTypeBox->insertItem("Multi-Threaded");
  plannerTypeBox->insertItem("Online");
  plannerTypeBox->insertItem("Assisted Control");
  plannerTypeBox->insertItem("Time Test");
  plannerTypeBox->setCurrentItem(3);//CHANGED!

  plannerInitButton->setEnabled(TRUE);
  plannerResetButton->setEnabled(FALSE);
  plannerStartButton->setEnabled(FALSE);
  instantEnergyButton->setEnabled(FALSE);

  //useVirtualHandBox->setChecked(FALSE);
  //showSolutionBox->setChecked(TRUE);
  //showCloneBox->setChecked(TRUE);
  //onlineDetailsGroup->setEnabled(TRUE);//CHANGED!

  QString n;
  QIntValidator* vAnnSteps = new QIntValidator(1,500000,this);
  annStepsEdit->setValidator(vAnnSteps);
  n.setNum(70000);
  annStepsEdit->setText(n);

  spaceSearchBox->insertItem("Complete");
  spaceSearchBox->insertItem("Axis-angle");
  spaceSearchBox->insertItem("Ellipsoid");
  spaceSearchBox->insertItem("Approach");
  spaceSearchBox->setCurrentItem(1);

  prevGraspButton->setEnabled(FALSE);
  nextGraspButton->setEnabled(FALSE);
  bestGraspButton->setEnabled(FALSE);
  executeGraspButton->setEnabled(FALSE);

  variableBox->setColumnLayout(0, Qt::Vertical);

  varGridLayout = new QGridLayout( variableBox->layout(),1,5 );
  varGridLayout->setSpacing(5);
  varGridLayout->setAlignment(Qt::AlignTop);
  varGridLayout->addMultiCellWidget(spaceSearchLabel,0,0,0,1);
  varGridLayout->addMultiCellWidget(spaceSearchBox,0,0,2,4);

  varGridLayout->addWidget( new QLabel("On", variableBox),1,0 );
  varGridLayout->addWidget( new QLabel("Name", variableBox),1,1 );
  varGridLayout->addWidget( new QLabel("Input", variableBox),1,2 );
  varGridLayout->addWidget( new QLabel("Target", variableBox),1,3 );
  varGridLayout->addWidget( new QLabel("Confidence", variableBox),1,4 );

  inputGloveBox->setEnabled(FALSE);
  inputLoadButton->setEnabled(FALSE);
 
  graspItGUI->getMainWindow()->mWindow->setWindowState(Qt::WindowMinimized);
  fprintf(stderr,"INIT DONE \n");
}

void EigenGraspPlannerDlg::redrawCircles()
{
  circleDraw->update();
  
}

void EigenGraspPlannerDlg::destroy()
{
  delete mHandObjectState;
  if (mPlanner) delete mPlanner;

  //cleanup
  for (unsigned int i=0; i<varNames.size(); i++) {
    //  varMainLayout->removeItem(varLayouts[i]);

    varGridLayout->remove(varNames[i]);
    varGridLayout->remove(varCheck[i]);
    varGridLayout->remove(varInput[i]);
    varGridLayout->remove(varTarget[i]);
    varGridLayout->remove(varConfidence[i]);

    delete varNames[i];
    delete varCheck[i];
    delete varInput[i];
    delete varTarget[i];
    delete varConfidence[i];

  }
  varNames.clear();
  varCheck.clear();
  varInput.clear();
  varConfidence.clear();
  varTarget.clear();
  varLayouts.clear();
}
/*
 *  Realigns the hand so that it is facing the object and at a reasonable distance
 */
void realignHand(Hand * h)
{
  double approachDist;
  h->quickOpen(1.0);
  if(!h->getGrasp()->getObject())
    approachDist = 300;
  else
    {          
      approachDist = 300 - h->getTran().translation().len();
     
    }
  h->approachToContact(-approachDist, true);
  graspItGUI->getIVmgr()->align();
}


/*
 * Set up the internal structures to use a new object. Sets up the internal hand object state
 * and the grasp. If a planner is running, it does not do anything sensible. 
 */
void EigenGraspPlannerDlg::updateObject(GraspableBody * b)
{
  mObject = b;
  mHand->getGrasp()->setObjectNoUpdate(mObject);
  mHandObjectState->setObject(mObject);
  mHandObjectState->setRefTran(mObject->getTran());

}

void EigenGraspPlannerDlg::setMembers( Hand *h, GraspableBody *b )
{
  mPlanner = NULL;
  mHand = h;
  mHandObjectState = new GraspPlanningState(mHand);
  updateObject(b); 
  // mHand->getGrasp()->setGravity(true);
  mHand->getGrasp()->setGravity(false); 
  mHandObjectState->setPositionType(SPACE_AXIS_ANGLE);
  mHandObjectState->reset();
  setVariableLayout();

  if (mHand->getNumVirtualContacts() > 0) {
    setContactsBox->setChecked(TRUE);
  }

  updateVariableLayout();
  updateInputLayout();  
  QRect geom;
  if(qApp->desktop()->screenCount() > 1)
    geom = QRect(1280.0,0,1280,1024);
  else
    geom = QRect(0.0,0,1280,1024);
  bciStageFrame = new BciStageFrame();  
  viewWindow = new HandViewWindow(parentWidget(), mHand, geom,graspItGUI->getIVmgr()->getViewer()->getSceneGraph(), bciStageFrame);
  graspItGUI->getMainWindow()->mWindow->resize(10,10);
  viewWindow->getViewWindow()->setActiveWindow();

  
  bciStageFrame->setBCIState(&graspItGUI->getIVmgr()->bciPlanningState, INITIALIZATION_PHASE);
  
  
  //bciStageFrame->move(geom.x() + geom.width()/2 - bciStageFrame->width()/2, 0);
 

  QTimer::singleShot(100, this, SLOT(plannerTimedUpdate()));
}

// ----------------------------------- Search State and variable layout management -------------------------------

void EigenGraspPlannerDlg::setVariableLayout()
{
  //cleanup
  for (unsigned int i=0; i<varNames.size(); i++) {
    varGridLayout->remove(varNames[i]);
    varGridLayout->remove(varCheck[i]);
    varGridLayout->remove(varInput[i]);
    varGridLayout->remove(varTarget[i]);
    varGridLayout->remove(varConfidence[i]);
    delete varNames[i];
    delete varCheck[i];
    delete varInput[i];
    delete varTarget[i];
    delete varConfidence[i];
  }
  varNames.clear();
  varCheck.clear();
  varInput.clear();
  varTarget.clear();
  varConfidence.clear();
  varLayouts.clear();

  int size = mHand->getEigenGrasps()->getSize() + 7;

  for (int i=0; i<size; i++) {
    QLabel *name = new QLabel("foo",variableBox);
    QCheckBox *check = new QCheckBox(variableBox);
    connect(check, SIGNAL(clicked()), this, SLOT(variableCheckBoxChanged()));
    QCheckBox *inputCheck = new QCheckBox(variableBox);
    connect(inputCheck, SIGNAL(clicked()), this, SLOT(variableInputChanged()));
    QLabel *target = new QLabel("N/A",variableBox);
    QSlider *slider = new QSlider(0,100,10,0,Qt::Horizontal,variableBox);
    connect(slider, SIGNAL(sliderReleased()), this, SLOT(variableInputChanged()));

    slider->setLineStep(1);
    slider->setMaximumWidth(50);
    varGridLayout->addWidget(check,2+i,0);
    varGridLayout->addWidget(name,2+i,1);
    varGridLayout->addWidget(inputCheck,2+i,2);
    varGridLayout->addWidget(target,2+i,3);
    varGridLayout->addWidget(slider,2+i,4);


    varCheck.push_back( check );
    varNames.push_back( name );
    varInput.push_back( inputCheck );
    varTarget.push_back( target );
    varConfidence.push_back( slider );
  }

  //signals for world interface to controlling progression of planner
  //connect(graspItGUI->getIVmgr()->getWorld(), SIGNAL(exec()), this, SLOT(plannerExec()));
  connect(graspItGUI->getIVmgr()->getWorld(), SIGNAL(exec()), this, SLOT(processExec()));
  connect(graspItGUI->getIVmgr()->getWorld(), SIGNAL(next()), this, SLOT(processNext()));
  connect(graspItGUI->getIVmgr()->getWorld(), SIGNAL(targetBodyChanged(GraspableBody *)), this, SLOT(updateObject(GraspableBody *)) );
  connect(graspItGUI->getIVmgr()->getWorld(), SIGNAL(resetStateMachine()), this, SLOT(resetStateMachine()) );

}

void EigenGraspPlannerDlg::updateVariableLayout() 
{
  int i;
  for (i=0; i<mHandObjectState->getNumVariables(); i++) {
    varNames[i]->setEnabled(TRUE);
    varNames[i]->setText( mHandObjectState->getVariable(i)->getName() );
    varCheck[i]->setEnabled(TRUE);
    if (mHandObjectState->getVariable(i)->isFixed()) varCheck[i]->setChecked(false);
    else varCheck[i]->setChecked(true);
  }

  for (i=mHandObjectState->getNumVariables(); i < mHand->getEigenGrasps()->getSize() + 7; i++) {
    varNames[i]->setEnabled(FALSE);
    varNames[i]->setText( "n/a" );
    varCheck[i]->setChecked(false);
    varCheck[i]->setEnabled(FALSE);
  }
}

void EigenGraspPlannerDlg::updateInputLayout()
{
  int i;
  for (i=0; i<mHandObjectState->getNumVariables(); i++) {
    if ( !mPlanner || !(mPlanner->isReady() || mPlanner->isActive()) ) {
      varInput[i]->setEnabled(FALSE);
      varInput[i]->setChecked(false);
      varTarget[i]->setText("N/A");
      varTarget[i]->setEnabled(FALSE);
      varConfidence[i]->setValue( 0 );
      varConfidence[i]->setEnabled(FALSE);
    } else {
      GraspPlanningState *t = mPlanner->getTargetState();
      varInput[i]->setEnabled(TRUE);
      QString n;
      n.setNum(t->getVariable(i)->getValue(),'f',3);
      varTarget[i]->setText(n);
      varConfidence[i]->setValue( t->getVariable(i)->getConfidence() * 100 );
      if ( t->getVariable(i)->isFixed() ) {
        varInput[i]->setChecked(TRUE);
        varTarget[i]->setEnabled(TRUE);
        varConfidence[i]->setEnabled(TRUE);
      } else {
        varInput[i]->setChecked(FALSE);
        varTarget[i]->setEnabled(FALSE);
        varConfidence[i]->setEnabled(FALSE);
      }
      if (mHandObjectState->getVariable(i)->getName() == "Tx" || mHandObjectState->getVariable(i)->getName() == "Ty") {
        varInput[i]->setChecked(true);
        varConfidence[i]->setEnabled(true);
        varConfidence[i]->setValue(70);
        mHandObjectState->getVariable(i)->setConfidence(.70);
      }
    }
  }

  for (i=mHandObjectState->getNumVariables(); i < mHand->getEigenGrasps()->getSize() + 7; i++) 
    {
      varInput[i]->setEnabled(FALSE);
      varInput[i]->setChecked(false);
      varTarget[i]->setText("N/A");
      varTarget[i]->setEnabled(FALSE);
      varConfidence[i]->setValue( 0 );
      varConfidence[i]->setEnabled(FALSE);
    }
}

void EigenGraspPlannerDlg::variableInputChanged()
{
  assert(mPlanner);
  GraspPlanningState *t = mPlanner->getTargetState();
  assert(t);
  for (int i=0; i<mHandObjectState->getNumVariables(); i++) {
    if (varInput[i]->isChecked()) {
      varTarget[i]->setEnabled(TRUE);
      varConfidence[i]->setEnabled(TRUE);
      t->getVariable(i)->setFixed(true);
      t->getVariable(i)->setConfidence( ((double)varConfidence[i]->value()) / 100.0);
      DBGP("Conf: " << ((double)varConfidence[i]->value()) / 100.0);
    }
    else {
      varTarget[i]->setEnabled(FALSE);
      t->getVariable(i)->setFixed(false);
      t->getVariable(i)->setConfidence(0);
      varConfidence[i]->setValue(0);
      varConfidence[i]->setEnabled(FALSE);
    }
  }
}

void EigenGraspPlannerDlg::variableCheckBoxChanged()
{
  for (int i=0; i<mHandObjectState->getNumVariables(); i++) {
    if (varCheck[i]->isChecked()) mHandObjectState->getVariable(i)->setFixed(false);
    else mHandObjectState->getVariable(i)->setFixed(true);
  }
  //force a reset of the planner
  plannerStartButton->setEnabled(FALSE);
}

void EigenGraspPlannerDlg::spaceSearchBox_activated( const QString &s )
{
  if ( s==QString("Complete") ) {
    mHandObjectState->setPositionType(SPACE_COMPLETE);
    mHandObjectState->setRefTran( mObject->getTran() );
  }  else if ( s==QString("Axis-angle") ) {
    mHandObjectState->setPositionType(SPACE_AXIS_ANGLE);
    mHandObjectState->setRefTran( mObject->getTran() );
  } else if ( s==QString("Ellipsoid") ) {
    mHandObjectState->setPositionType(SPACE_ELLIPSOID);
    mHandObjectState->setRefTran( mObject->getTran() );
  } else if ( s==QString("Approach") ) {
    mHandObjectState->setPositionType(SPACE_APPROACH);
    mHandObjectState->setRefTran( mHand->getTran() );
  } else {
    fprintf(stderr,"WRONG SEARCH TYPE IN DROP BOX!\n");
  }
  mHandObjectState->reset();
  updateVariableLayout();
  //force a reset of the planner
  if (mPlanner) mPlanner->invalidateReset();
  updateStatus();
}

//------------------------------------- Show Results stuff ---------------------------------

void EigenGraspPlannerDlg::prevGraspButton_clicked()
{
  mDisplayState--;
  updateResults(true, false);
}

void EigenGraspPlannerDlg::bestGraspButton_clicked()
{
  if (!mPlanner) return;
  mDisplayState = 0;
  updateResults(true, false);
}

void EigenGraspPlannerDlg::nextGraspButton_clicked()
{
  mDisplayState++;
  updateResults(true, false);
};

//If there is a planner and a valid grasp, execute it
//by activating the online planner's execute 
void EigenGraspPlannerDlg::executeGraspButton_clicked()
{

  if (mPlanner && mPlanner->getListSize() > 0)
    {
      updateResults(true, true);
      std::cout<< " Posture in executeGraspButton_clicked():";
      const_cast<GraspPlanningState * >(mPlanner->getGrasp(mDisplayState))->getPosture()->print();
      onlineGraspButton_clicked();
    }
}

void EigenGraspPlannerDlg::plannerUpdate()
{
  assert(mPlanner);
  //  mDisplayState = 0;
  updateResults(false, false);
  //if we are using the CyberGlove for input this updates the target values
  if (inputGloveBox->isChecked()) {
    updateInputLayout();
  }
}

void EigenGraspPlannerDlg::updateResults(bool render, bool execute)
{
  {
  assert(mPlanner);
  assert(viewWindow);  
  if (execute) assert( mPlanner->getType() == PLANNER_ONLINE);

  QString nStr;
  nStr.setNum(mPlanner->getCurrentStep());
  currentStepLabel->setText(QString("Current step: ") + nStr);

  nStr.setNum(mPlanner->getRunningTime(),'f',0);
  timeLabel->setText(QString("Time used: ") + nStr + QString(" sec."));

  int d = mPlanner->getListSize();
  int rank, size, iteration; double energy;


  //CHANGED! so now it cycles
  if (d==0) {
    mDisplayState = rank = size = energy = iteration = 0; render = false;
  } else if (mDisplayState < 0){
    mDisplayState = 9;
  } else if ( mDisplayState >= d) {
    mDisplayState = 0;
  } 

  if ( d!=0 ){
    const GraspPlanningState *s = mPlanner->getGrasp( mDisplayState);
    rank = mDisplayState+1;
    size = d;
    iteration = s->getItNumber();
    energy = s->getEnergy();
  }

  /*
    FILE *f = fopen("foo.txt","w");
    for (int i=0; i<mPlanner->getListSize(); i++) {
    for(int j=i+1; j<mPlanner->getListSize(); j++) {
    float d = mPlanner->getGrasp(i)->distance( mPlanner->getGrasp(j) );
    fprintf(stderr,"%d -- %d: %f\n",i+1,j+1,d);
    }
    fprintf(stderr,"\n");
    mPlanner->getGrasp(i)->writeToFile(f);
    }
    fclose(f);
  */

  QString n1,n2;
  n1.setNum(rank);
  n2.setNum(size);
  rankLabel->setText("Rank: " + n1 + "/" + n2);
  n1.setNum(iteration);
  iterationLabel->setText("Iteration: " + n1);
  n1.setNum(energy,'f',3);
  energyLabel->setText("Energy: " + n1);

  if (render) {
    mPlanner->showGrasp(mDisplayState);
    //mHand->getWorld()->findAllContacts();
    //mHand->getGrasp()->update();
    mPlanner->getGrasp(mDisplayState)->printState();
    //mHand->autoGrasp(true);
  }

  if (execute) {
    OnLinePlanner *op = (OnLinePlanner*)mPlanner;
    op->executeGrasp(mDisplayState);
  }
  if(d!=0)
    {
      bool unlock = false;
      if (!mPlanner->mListAttributeMutex.locked())
      {
        mPlanner->mListAttributeMutex.lock();
        unlock = true;
      }
      for(int mDisplayStateNum = 0; mDisplayStateNum < d; ++mDisplayStateNum){
	//  std::cout << "Resizing View\n";
	    viewWindow->addView(*const_cast<GraspPlanningState*>(mPlanner->getGrasp((mDisplayState + mDisplayStateNum)%10)), mDisplayStateNum);
	// viewWindow->getViewWindow()->setFocus();
	// viewWindow->getViewWindow()->update();      
      }
      if (unlock)
        mPlanner->mListAttributeMutex.unlock();
    }
  }//end lock scope

  graspItGUI->getIVmgr()->emitAnalyzeNextGrasp();
  //viewWindow->setCurrentView(mDisplayState);
}

// ----------------------------- Settings management ---------------------------


void EigenGraspPlannerDlg::updateStatus()
{
  PlannerState s = DONE;
  if (mPlanner) s = mPlanner->getState();
  DBGP("Update Layout");
  switch(s) {
  case INIT:
    plannerInitButton->setEnabled(FALSE);
    plannerResetButton->setEnabled(TRUE);
    plannerStartButton->setEnabled(FALSE);
    plannerStartButton->setText(">");

    prevGraspButton->setEnabled(FALSE);
    nextGraspButton->setEnabled(FALSE);
    bestGraspButton->setEnabled(FALSE);
    executeGraspButton->setEnabled(FALSE);
    mObject->showFrictionCones(true);

    inputGloveBox->setEnabled(FALSE);
    inputLoadButton->setEnabled(FALSE);
    onlineDetailsGroup->setEnabled(TRUE);

    break;
  case READY:
    {
      plannerInitButton->setEnabled(FALSE);
      plannerResetButton->setEnabled(TRUE);
      plannerStartButton->setEnabled(TRUE);
      plannerStartButton->setText(">");

      prevGraspButton->setEnabled(TRUE);
      nextGraspButton->setEnabled(TRUE);
      bestGraspButton->setEnabled(TRUE);
      executeGraspButton->setEnabled(TRUE);
      mObject->showFrictionCones(true);

      inputGloveBox->setEnabled(TRUE);
      inputLoadButton->setEnabled(TRUE);
      if (mPlanner->getType() == PLANNER_ONLINE) {
	onlineDetailsGroup->setEnabled(TRUE);
	showSolutionBox->setChecked(TRUE);
	showCloneBox->setChecked(TRUE);
      }
      else onlineDetailsGroup->setEnabled(FALSE);
    }
    break;
  case RUNNING:
    plannerInitButton->setEnabled(FALSE);
    plannerResetButton->setEnabled(FALSE);
    plannerStartButton->setEnabled(TRUE);
    plannerStartButton->setText("||");

    prevGraspButton->setEnabled(FALSE);
    nextGraspButton->setEnabled(FALSE);
    bestGraspButton->setEnabled(FALSE);
    executeGraspButton->setEnabled(FALSE);
    executeGraspButton->setEnabled(FALSE);
    mObject->showFrictionCones(false);
    break;
  default:
    plannerInitButton->setEnabled(TRUE);
    plannerResetButton->setEnabled(FALSE);
    plannerStartButton->setEnabled(FALSE);
    plannerStartButton->setText(">");

    prevGraspButton->setEnabled(FALSE);
    nextGraspButton->setEnabled(FALSE);
    bestGraspButton->setEnabled(FALSE);
    executeGraspButton->setEnabled(FALSE);
    mObject->showFrictionCones(true);

    inputGloveBox->setEnabled(FALSE);
    inputLoadButton->setEnabled(FALSE);
    onlineDetailsGroup->setEnabled(FALSE);
    break;
  }
  updateInputLayout();
}

void EigenGraspPlannerDlg::energyBox_activated( const QString & )
{
  //force a reset of the planner
  if (mPlanner) mPlanner->invalidateReset();
  updateStatus();
}

void EigenGraspPlannerDlg::setContactsBox_toggled( bool checked)
{
  if (checked) {
    if ( mHand->getNumVirtualContacts() == 0 ) {
      //if we are asking to use pre-set contacts, but none are available, pop up the dialog
      //for loading virtual contacts
      ContactExaminerDlg dlg(this);
      dlg.exec();
    }
    if (mHand->getNumVirtualContacts() == 0) {
      //if we still have no virtual contacts, un-check the box
      setContactsBox->setChecked(false);
    }
  }
  if (mPlanner) mPlanner->invalidateReset();
  updateStatus();
}

void EigenGraspPlannerDlg::readPlannerSettings()
{
  assert(mPlanner);
  //energy type
  QString s = energyBox->currentText();
  if ( s == QString("Hand Contacts") ) {
    mPlanner->setEnergyType(ENERGY_CONTACT);
  } else if ( s == QString("Potential Quality") ) {
    mPlanner->setEnergyType(ENERGY_POTENTIAL_QUALITY);
  } else if ( s == QString("Autograsp Quality") ) {
    mPlanner->setEnergyType(ENERGY_AUTOGRASP_QUALITY);
  } else if ( s == QString("Contacts AND Quality") ) {
    mPlanner->setEnergyType(ENERGY_CONTACT_QUALITY);
  } else if ( s == QString("Guided Autograsp") ) {
    mPlanner->setEnergyType(ENERGY_GUIDED_AUTOGRASP);
  } else {
    fprintf(stderr,"WRONG ENERGY TYPE IN DROP BOX!\n");
  }

  //contact type
  if ( setContactsBox->isChecked() ) {
    mPlanner->setContactType(CONTACT_PRESET);
  } else {
    mPlanner->setContactType(CONTACT_LIVE);
  }

  //steps
  int steps = annStepsEdit->text().toInt();
  mPlanner->setMaxSteps(steps);
}

void EigenGraspPlannerDlg::plannerComplete()
{
  updateStatus();
  bestGraspButton_clicked();
}



//----------------------------------- Planner start / stop control stuff ---------------------------
void EigenGraspPlannerDlg::processNext()
{
  bciStageFrame->blinkArea1();
  QTimer::singleShot(200, this, SLOT(plannerNext()));
}

void EigenGraspPlannerDlg::plannerNext() 
{
  switch(graspItGUI->getIVmgr()->bciPlanningState)
    {
    case INITIALIZATION_PHASE:
      {
       graspItGUI->getIVmgr()->getViewer()->viewAll();
	     graspItGUI->getIVmgr()->emitRunObjectRecognition();
       mHand->setTransparency(1);
	     break;
      }
    case OBJECT_SELECTION_PHASE:
      {
        GraspableBody * gb = bci_experiment::highlightNextGraspableBody(mObject);
	if(!gb)
	  return;
	updateObject(gb);
	break;
      }
    case INITIAL_REVIEW_PHASE:
      {
	if (!mPlanner)
	  return;
	// show next grasp in big window using clone hand
	nextGraspButton_clicked();       
	break;
      }
      
    case START_PHASE:    
      {
	bciStageFrame->setBCIState(&graspItGUI->getIVmgr()->bciPlanningState, CONFIRM_PHASE);
	break;
      }
    case PLANNING_PHASE:
      {	
	// show next grasp in big window using clone hand
	nextGraspButton_clicked();
	break;
      }
    case FINAL_REVIEW_PHASE:
      {
	// show next grasp in big window using clone hand
	nextGraspButton_clicked();        
        break;
      }
    case CONFIRM_PHASE://fall through
    case EXECUTION_PHASE:
      {        
	//return to planning phase
	//move hand back to planning distance
	realignHand(mHand);        
	plannerStart_clicked();
	bciStageFrame->setBCIState(&graspItGUI->getIVmgr()->bciPlanningState, PLANNING_PHASE);
	break;
      }
    }
}
 


void EigenGraspPlannerDlg::processExec()
{
  bciStageFrame->blinkArea2();
  QTimer::singleShot(200, this, SLOT(plannerExec()));
}



void EigenGraspPlannerDlg::initializeDbInterface()
{
  if (!mDbMgr)
    mDbMgr = new db_planner::SqlDatabaseManager("tonga.cs.columbia.edu", 5432,
						"postgres","roboticslab","armdb",
						new GraspitDBModelAllocator,
						new GraspitDBGraspAllocator(mHand));
}


void EigenGraspPlannerDlg::initializeHandviewWindow()
{
	//Open view window
	if(viewWindow)
	  {       
      viewWindow->clearViews();
	    viewWindow->initViews(mHand);
	  }             
}


void EigenGraspPlannerDlg::loadGraspsToHandviewWindow()
{
    
  // Get corresponding model from database
  std::vector<db_planner::Model*> modelList;    
  mDbMgr->ModelList(&modelList,db_planner::FilterList::USE_FILE_NAME,
    '/' + mHand->getGrasp()->getObject()->getFilename().split('/').back());
  // If no model can be found return
  if (modelList.size()==0)
  {
    std::cout << "No Models Found \n";
    return;
  }
  // Using the found model, retrieve the grasps
  std::vector<db_planner::Grasp*> grasps;
  mDbMgr->GetGrasps(*modelList[modelList.size()-1], GraspitDBGrasp::getHandDBName(mHand).toStdString(), &grasps);
  HandObjectState hs(mHand);
  hs.setPositionType(SPACE_COMPLETE);
  hs.setPostureType(POSE_DOF);
  hs.saveCurrentHandState();
  // Load the grasps into the grasp planner list.        
  unsigned int numGrasps = std::min<unsigned int>(grasps.size(), 10);
  for (unsigned int gNum = 0; gNum < numGrasps; ++gNum)
    {          
      GraspPlanningState *s = new GraspPlanningState(static_cast<GraspitDBGrasp *>
						     (grasps[gNum])->getFinalGraspPlanningState());
      
      s->setObject(mHand->getGrasp()->getObject());
      s->setRefTran(mHand->getGrasp()->getObject()->getTran());
      float testResult = -2*bci_experiment::testGraspCollisions(mHand, s);
      s->addAttribute("graspId", gNum);
      s->addAttribute("testResult", testResult);
      //bci_experiment::printTestResult(*s);
      mPlanner->addSolution(s);
  }
  if (numGrasps){
    graspItGUI->getIVmgr()->emitAnalyzeNextGrasp();        
    std::cout<< "emitted analyze grasp\n";
  }
  hs.execute(mHand);
  dynamic_cast<OnLinePlanner *>(mPlanner)->updateSolutionList();
  
  for(int i = 0; i < std::min<unsigned int>(mPlanner->getListSize(), 10); ++i)
  {
      viewWindow->addView(*const_cast<GraspPlanningState*>(mPlanner->getGrasp(i)), i);     
      viewWindow->getViewWindow()->show();        
      viewWindow->getViewWindow()->update();
  } 

  viewWindow->getViewWindow()->sizeIncrement();

}


void EigenGraspPlannerDlg::initializeTarget()
{
  bci_experiment::sendSetTarget(mObject);
  bci_experiment::disableNontargetCollisions(mHand, mObject);
  bci_experiment::disableTableObjectCollisions();  

  //Initialize Handview Window
  initializeHandviewWindow();
  
  //start planner        
  if (mPlanner)
    plannerReset_clicked();      
  plannerInit_clicked();
  
  
  // Download grasps from database
  initializeDbInterface();
  
  
  // Load the grasps into the grasp planner list.        
  loadGraspsToHandviewWindow();
  mPlanner->getGrasp(0)->execute(mHand);
  //Draw new first grasp
  updateResults(true, false);
  
  std::ofstream time_out("grasp_timing.txt", std::ios_base::out | std::ios_base::app);
  time_out << "Object: " << mObject->getName().toStdString() << "\n";  
  timer.start();
  graspItGUI->getIVmgr()->setFocus();    
  graspItGUI->getIVmgr()->setActiveWindow();

  
}


void EigenGraspPlannerDlg::plannerExec()
{
  switch(graspItGUI->getIVmgr()->bciPlanningState)
    {
    case INITIALIZATION_PHASE:
      {
	//If there are no objects, don't allow it to go on.
	if(bci_experiment::getWorld()->getNumGB() == 0)
	  return;
		
	//Next Phase
	bciStageFrame->setBCIState(&graspItGUI->getIVmgr()->bciPlanningState, OBJECT_SELECTION_PHASE);
		
	GraspableBody * gb = bci_experiment::getWorld()->getGB(0);
  bci_experiment::highlightAll();
  bci_experiment::setPointcloudTransparency(.2);
	bci_experiment::highlightBody(gb, SbColor(0,1,0));
  if(gb)
	  updateObject(gb);  
	
	break;
      }
    case OBJECT_SELECTION_PHASE:
      {
  bci_experiment::unhighlightAll();
  bci_experiment::setPointcloudTransparency(0);
	bci_experiment::setObjectCentral(mObject);
	initializeTarget();
	bciStageFrame->setBCIState(&graspItGUI->getIVmgr()->bciPlanningState, INITIAL_REVIEW_PHASE);  
	realignHand(mHand);        
  bci_experiment::viewHand(mHand);
  mHand->setTransparency(0.0);
	break;
      }

    case INITIAL_REVIEW_PHASE:
      {        
      
	//Select current grasp to start from
	if(mPlanner->getListSize() > 0)
	  mPlanner->getGrasp(mDisplayState)->execute(mHand);
      
	realignHand(mHand);  
	//next grasp in big window and on hand
	bciStageFrame->setBCIState(&graspItGUI->getIVmgr()->bciPlanningState, START_PHASE);
	break;
      }
    case START_PHASE:
      {
      
	plannerStart_clicked();
	bciStageFrame->setBCIState(&graspItGUI->getIVmgr()->bciPlanningState, PLANNING_PHASE);
	break;
      }
    case PLANNING_PHASE:
      {
	// show next grasp in big window
	stopPlanner();
	bciStageFrame->setBCIState(&graspItGUI->getIVmgr()->bciPlanningState, FINAL_REVIEW_PHASE);
	break;
      }
    case FINAL_REVIEW_PHASE:
      {
        
        //next grasp in big window and on hand
	bciStageFrame->setBCIState(&graspItGUI->getIVmgr()->bciPlanningState, CONFIRM_PHASE);
        break;
      }
    case CONFIRM_PHASE:
      {
	bciStageFrame->setBCIState(&graspItGUI->getIVmgr()->bciPlanningState, EXECUTION_PHASE);
        bciStageFrame->setActiveWindow();
       
      }
    case EXECUTION_PHASE:
      {        
        int time = timer.elapsed();
        std::ofstream time_out("grasp_timing.txt", std::ios_base::out | std::ios_base::app);
        
        time_out << " " << time <<std::endl;
        executeGraspButton_clicked();
        break;  
      }
        
    }
}

void EigenGraspPlannerDlg::resetStateMachine()
{
  if (mPlanner)
  {
    stopPlanner();
    delete mPlanner;
    mPlanner = NULL;
  }
  if(viewWindow)
    viewWindow->clearViews();

  realignHand(mHand);
  bciStageFrame->setBCIState(&graspItGUI->getIVmgr()->bciPlanningState, INITIALIZATION_PHASE);
}

void EigenGraspPlannerDlg::plannerInit_clicked()
{  
  QString s = plannerTypeBox->currentText();
  if (s == QString("Sim. Ann.")) {
    if (mPlanner) delete mPlanner;
    mPlanner = new SimAnnPlanner(mHand);
    ((SimAnnPlanner*)mPlanner)->setModelState(mHandObjectState);
    energyBox->setEnabled(TRUE);
  } else if (s == QString("Loop")) {
    if (mPlanner) delete mPlanner;
    mPlanner = new LoopPlanner(mHand);
    ((LoopPlanner*)mPlanner)->setModelState(mHandObjectState);
    energyBox->setEnabled(TRUE);
  } else if (s == QString("Multi-Threaded")) {
    if (mPlanner) delete mPlanner;
    mPlanner = new GuidedPlanner(mHand);
    ((GuidedPlanner*)mPlanner)->setModelState(mHandObjectState);
    energyBox->setCurrentItem(2);
    energyBox->setEnabled(FALSE);


  } else if (s == QString("Online") ) {
    if (mPlanner) delete mPlanner;
    mPlanner = new OnLinePlanner(mHand);
    ((OnLinePlanner*)mPlanner)->setModelState(mHandObjectState);
    energyBox->setEnabled(TRUE);
    energyBox->setCurrentItem(2);
    QString n;
    n.setNum(2000);
    annStepsEdit->setText(n);
    QObject::connect(mPlanner,SIGNAL(update()),this,SLOT(onlinePlannerUpdate())); 
  }


  else if ( s == QString("Time Test") ) {
    if (mPlanner) delete mPlanner;
    mPlanner = new MTTester(mHand);
  } else {
    fprintf(stderr,"Unknown planner type requested\n");
    return;
  } 
  graspItGUI->getIVmgr()->getWorld()->setCurrentPlanner(mPlanner);



  QObject::connect(mPlanner,SIGNAL(update()),this,SLOT(plannerUpdate()));
  QObject::connect(mPlanner,SIGNAL(complete()),this,SLOT(plannerComplete()));

  updateStatus();
  plannerReset_clicked();  
}

void EigenGraspPlannerDlg::plannerReset_clicked() 
{
  assert(mPlanner);
  readPlannerSettings();
  mPlanner->resetPlanner();
  updateStatus();
}

void EigenGraspPlannerDlg::startPlanner()
{
  //mPlanner->startThread(); 
  mPlanner->startPlanner();
  updateStatus();
}

void EigenGraspPlannerDlg::stopPlanner()
{
  mPlanner->pausePlanner();
  updateStatus();
}

void EigenGraspPlannerDlg::plannerStart_clicked()
{	
  
  if (!mPlanner->isActive()){
    startPlanner();
  } else {
    stopPlanner();
  }
}

void EigenGraspPlannerDlg::plannerTypeBox_activated( const QString & )
{
  if (mPlanner) {
    mHand->getWorld()->setCurrentPlanner(NULL);
    delete mPlanner;
    mPlanner = NULL;
  }
  updateStatus();
}


//----------------------------------- Dedicated on-line planner control ---------------------------

void EigenGraspPlannerDlg::autoGraspBox_clicked()
{

}

//this slot does the updating that's specific to the online planner
void EigenGraspPlannerDlg::onlinePlannerUpdate()
{
  assert( mPlanner->getType() == PLANNER_ONLINE);
  OnLinePlanner *op = (OnLinePlanner*)mPlanner;
  QString num;

  double objDist = op->getObjectDistance();
  num.setNum(objDist,'f',0);
  objDistLabel->setText("Object distance: "  + num);
  /*
    double solDist = op->getSolutionDistance();
    if (solDist >= 0) num.setNum(solDist,'f',0);
    else num.setAscii("N/A");
    solDistLabel->setText("Solution distance: " + num);
  */
  ActionType s = op->getAction();
  switch (s) {
  case ACTION_PLAN:
    num.setAscii("PLANNING");
    break;
  case ACTION_GRASP:
    num.setAscii("GRASPING");
    break;
  case ACTION_OPEN:
    num.setAscii("OPEN");
    break;
  default:
    num.setAscii("N/A");
    break;
  }
  onlineStatusLabel->setText("Status: " + num);

  num.setNum( op->getSABufferSize() );
  saBufferLabel->setText("SimAnn buffer: " + num);
  num.setNum( op->getFCBufferSize() );
  fcBufferLabel->setText("FC Thread buffer: " + num); 
}

void EigenGraspPlannerDlg::onlineGraspButton_clicked()
{
  assert( mPlanner->getType() == PLANNER_ONLINE);
  OnLinePlanner *op = (OnLinePlanner*)mPlanner;
  op->action(ACTION_GRASP);
  onlinePlannerUpdate();
}

void EigenGraspPlannerDlg::onlineReleaseButton_clicked()
{
  assert( mPlanner->getType() == PLANNER_ONLINE);
  OnLinePlanner *op = (OnLinePlanner*)mPlanner;
  op->action(ACTION_OPEN);
  onlinePlannerUpdate();
}


void EigenGraspPlannerDlg::onlinePlanButton_clicked()
{
  assert( mPlanner->getType() == PLANNER_ONLINE);
  OnLinePlanner *op = (OnLinePlanner*)mPlanner;
  op->action(ACTION_PLAN);
  onlinePlannerUpdate(); 
}


void EigenGraspPlannerDlg::instantEnergyButton_clicked()
{
  assert(mPlanner);
  // mPlanner->instantEnergy();
}


void EigenGraspPlannerDlg::showCloneBox_toggled( bool c)
{
  assert( mPlanner->getType() == PLANNER_ONLINE);
  OnLinePlanner *op = (OnLinePlanner*)mPlanner;
  op->showClone(c);  
}


void EigenGraspPlannerDlg::showSolutionBox_toggled( bool c)
{	
  assert( mPlanner->getType() == PLANNER_ONLINE);
  OnLinePlanner *op = (OnLinePlanner*)mPlanner;
  op->showSolutionClone(c);
}

void EigenGraspPlannerDlg::useVirtualHandBox_clicked()
{
  /*
    assert( mPlanner->getType() == PLANNER_ONLINE);
    OnLinePlanner *op = (OnLinePlanner*)mPlanner;
    bool c = useVirtualHandBox->isChecked();
    op->controlVirtualHand(c);

  */
}

void EigenGraspPlannerDlg::useRealBarrettBox_toggled( bool s)
{
  assert( mPlanner->getType() == PLANNER_ONLINE);
  OnLinePlanner *op = (OnLinePlanner*)mPlanner;
  op->useRealBarrettHand(s);
}

//---------------------------------------- Input selection -----------------------------------------

void EigenGraspPlannerDlg::inputGloveBox_toggled( bool on)
{
  assert(mPlanner);
  mPlanner->setInput(INPUT_GLOVE, on);
}

void EigenGraspPlannerDlg::inputLoadButton_clicked()
{
  assert(mPlanner);
  QString fn = QFileDialog::getOpenFileName(this, QString(),  QString(getenv("GRASPIT"))+QString("/models/grasps"),
					    "Grasp Files (*.txt)" );

  if ( fn.isEmpty() ) {
    return;
  }

  FILE *fp = fopen(fn.latin1(), "r");
  bool success = true;
  if (!fp) {
    DBGA("Failed to open input file!");
    success = false;
  }else if (!mPlanner->getTargetState()->readFromFile(fp)) {
    DBGA("Failed to read target from input file!");
    success = false;
  } else {
    DBGA("Target values loaded successfully");
  }
  fclose(fp);
  mPlanner->setInput(INPUT_FILE, success);
  updateInputLayout();
}


void EigenGraspPlannerDlg::plannerTimedUpdate()
{
  if(mPlanner && viewWindow)
  {
    if(graspItGUI->getIVmgr()->bciPlanningState == INITIAL_REVIEW_PHASE)
    {
      dynamic_cast<OnLinePlanner *>(mPlanner)->updateSolutionList();
      updateResults(true, false);
    }
  }
  QTimer::singleShot(1000, this, SLOT(plannerTimedUpdate()));
}







