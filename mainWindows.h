/*=========================================================================

Program:   basic_qtVTK_AIGS
Module:    $RCSfile: mainWindows.h,v $
Creator:   Elvis C. S. Chen <chene@robarts.ca>
Language:  C++
Author:    $Author: Elvis Chen $
Date:      $Date: 2018/05/28 12:01:30 $
Version:   $Revision: 0.99 $

==========================================================================

Copyright (c) Elvis C. S. Chen, elvis.chen@gmail.com

Use, modification and redistribution of the software, in source or
binary forms, are permitted provided that the following terms and
conditions are met:

1) Redistribution of the source code, in verbatim or modified
form, must retain the above copyright notice, this license,
the following disclaimer, and any notices that refer to this
license and/or the following disclaimer.

2) Redistribution in binary form must include the above copyright
notice, a copy of this license and the following disclaimer
in the documentation or with other materials provided with the
distribution.

3) Modified copies of the source code must be clearly marked as such,
and must not be misrepresented as verbatim copies of the source code.

THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE SOFTWARE "AS IS"
WITHOUT EXPRESSED OR IMPLIED WARRANTY INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE.  IN NO EVENT SHALL ANY COPYRIGHT HOLDER OR OTHER PARTY WHO MAY
MODIFY AND/OR REDISTRIBUTE THE SOFTWARE UNDER THE TERMS OF THIS LICENSE
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, LOSS OF DATA OR DATA BECOMING INACCURATE
OR LOSS OF PROFIT OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF
THE USE OR INABILITY TO USE THE SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES.

=========================================================================*/


#ifndef __MAINWIDGET_H__
#define __MAINWIDGET_H__

#pragma once

// Local includes
#include <QMainWindow>
#include "ui_basic_QtVTK_AIGS.h"
#include "VisualizationController.h"

// C++ includes
#include <tuple>
#include <vector>

// Qt includes
#include <qstring.h>
#include <qprocess.h>
#include <qcombobox.h>
#include <QPlusDeviceSetSelectorWidget.h>
#include <Qwidget.h>
#include <QTimer.h>

// Vtk includes
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkSmartPointer.h>


/* GUI Application for rendering a volume with VTK and DRR generation with ITK
 and Plus for tracking the hardware components */

class basic_QtVTK : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    // constructor/destructor
    basic_QtVTK();
    ~basic_QtVTK() {};
    void Render();

    protected slots:

    /* Connect to devices described in the argument configuration file in response by clicking on the Connect button
    param aConfigFile DeviceSet configuration file path and name */
    void ConnectToDevicesByConfigFile(std::string);

    public slots:

    virtual void SlotExit();

    void LoadMesh();
    void LoadFiducialPts();
    void EditMeshColor();
    void EditRendererBackgroundColor();
    void ScreenShot();
    void StartTracker(bool);
    void UpdateTrackerInfo();
    void StylusCalibration(bool);
    void CollectSinglePointPhantom();
    void CollectDRR();
    void ResetPhantomCollectedPoints();
    void DeleteOnePhantomCollectedPoints();
    void PerformPhantomRegistration();
    void AboutThisProgram();

protected:
    /* Device Set Selector Widget */
    QPlusDeviceSetSelectorWidget           *deviceSetSelectorWidget;

    /* Local Config File Name */
    std::string                             configFile;

private:
    void CreateTrackerLogo();
    void CreateLinearZStylusActor();
    void SetupQTObjects();


    VisualizationController *visualizationController;
    QTimer                  *trackerTimer;

};

#endif // of __MAINWIDGET_H__