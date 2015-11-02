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
// Author(s): Andrew T. Miller 
//
// $Id: main.cpp,v 1.12.2.2 2009/07/24 15:33:45 cmatei Exp $
//
//######################################################################

/*! \mainpage GraspIt! Developer Documentation
  \image html logo.jpg

  These pages document the GraspIt! source code. Please remember this is
  research code. There are still plenty of pieces of code that are unfinished
  and several bugs that need to be fixed.

  More information and original source code for the packages included with
  GraspIt! can be found in the following places:

  - <b>qhull:</b> http://www.qhull.org
  - <b>maxdet:</b> http://www.stanford.edu/~boyd/old_software/MAXDET.html
  - <b>tinyxml:</b> http://www.grinninglizard.com/tinyxml/
*/

/*! \file
  \brief Program execution starts here.  Server is started, main window is built, and the interactive loop is started.
 */

#define GRASPITDBG

#include <iostream>
#include <graspitApp.h>
#include "graspitGUI.h"
#include "graspitServer.h"
#include "graspitProtobufServer.h"
#include "mainWindow.h"

#include <QtGui>"

#include "ros/ros.h"

#include "debug.h"

#ifdef Q_WS_WIN
#include <windows.h>
#include <wincon.h>
#endif

int main(int argc, char **argv)
{
#ifdef GRASPITDBG
#ifdef Q_WS_WIN
  AllocConsole(); 
  freopen("conin$", "r", stdin); 
  freopen("conout$", "w", stdout); 
  freopen("conout$", "w", stderr); 
  //ios::sync_with_stdio();
#endif
#endif

  ROS_INFO("Initializing Ros Node");
  ros::init(argc, argv, "ros_bci_node");
  ROS_INFO("Ros Node Initialized");

  GraspItApp app(argc, argv);
 
  app.showSplash();
  QApplication::setOverrideCursor( Qt::waitCursor );

  GraspItGUI gui(argc,argv);

  QPushButton * spinButton = new QPushButton("Spin");
  QPushButton * slowButton = new QPushButton("Move Slow");
  QPushButton * fastButton = new QPushButton("Move Fast");

  spinButton->setDefault(true);
  slowButton->setDefault(true);
  fastButton->setDefault(true);

  QDialogButtonBox *cursorControlBox = new QDialogButtonBox(Qt::Vertical);
  cursorControlBox->setCaption(QString("Cursor Control Box"));

  cursorControlBox->addButton(spinButton, QDialogButtonBox::ActionRole);
  cursorControlBox->addButton(slowButton, QDialogButtonBox::ActionRole);
  cursorControlBox->addButton(fastButton, QDialogButtonBox::ActionRole);
  cursorControlBox->resize(QSize(200,100));
  cursorControlBox->show();

  QObject::connect(spinButton, SIGNAL(clicked()), gui.getIVmgr(), SLOT(updateControlSceneState0()));
  QObject::connect(slowButton, SIGNAL(clicked()), gui.getIVmgr(), SLOT(updateControlSceneState1()));
  QObject::connect(fastButton, SIGNAL(clicked()), gui.getIVmgr(), SLOT(updateControlSceneState2()));

  QTimer timer;
  QObject::connect(&timer, SIGNAL(timeout()), gui.getIVmgr(), SLOT(updateControlScene()));
  timer.start(1000 / 30);

  //This is the GraspIt TCP server. It can be used to connect to GraspIt from
  //external programs, such as Matlab.
  //On some machines, the Q3Socket segfaults at exit, so this is commented out by
  //default
  GraspItServer server(4765);
  unsigned int portNumber = 4766;

  GraspitProtobufServer protobufServer(portNumber);



  app.setMainWidget(gui.getMainWindow()->mWindow);
  QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

  app.closeSplash();
  QApplication::restoreOverrideCursor();


  if (!gui.terminalFailure()) {
	  gui.startMainLoop();
  }


  return 0;
}
