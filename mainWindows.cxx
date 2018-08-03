/*=========================================================================

Program:   basic_qtVTK_AIGS
Module:    $RCSfile: mainWindows.cxx,v $
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

// local includes
#include "mainWindows.h"
#include "SerialPort.h"
#include "VisualizationController.h"

// VTK includes
#include <QVTKWidget.h>
#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkButtonWidget.h>
#include <vtkCamera.h>
#include <vtkConeSource.h>
#include <vtkCylinderSource.h>
#include <vtkCoordinate.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkImageData.h>
#include <vtkImageViewer2.h>
#include <vtkImageExtractComponents.h>
#include <vtkLineSource.h>
#include <vtkLogoRepresentation.h>
#include <vtkLogoWidget.h>
#include <vtkMatrix4x4.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkMetaImageReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPNGWriter.h>
#include <vtkPNGReader.h>
#include <vtkImageReader2.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataReader.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSimplePointsReader.h>
#include <vtkSimplePointsWriter.h>
#include <vtkSmartPointer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkSTLReader.h>
#include <vtkTexturedButtonRepresentation2D.h>
#include <vtkTransform.h>
#include <vtkLandmarkTransform.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPieceWiseFunction.h>
#include <vtkTubeFilter.h>
#include <vtkWindowToImageFilter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkSphereSource.h>

// ITK includes
#include <itkEuler3DTransform.h>

// PLUS includes
#include <QPlusDeviceSetSelectorWidget.h>
#include <QPlusStatusIcon.h>
#include <vtkPlusDataCollector.h>
#include <PlusCommon.h>
#include <vtkPlusOpenIGTLinkServer.h>
#include <vtkPlusTransformRepository.h>
#include <vtkPlusGenericSerialDevice.h>
#include <vtkPlusNDITracker.h>
#include <vtkPlusOpticalMarkerTracker.h>
#include <PlusXmlUtils.h>
#include <vtkPlusDataSource.h>
#include <vtkPlusProbeCalibrationAlgo.h>
#include <vtkPlusMicrochipTracker.h>
#include <vtkPlusWitMotionTracker.h>
#include <vtkPlusVirtualMixer.h>


// system
#include <chrono>
#include <thread>

// tracker
#include <vtkNDITracker.h>
#include <vtkTrackerTool.h>

// QT includes
#include <QColorDialog>
#include <QDebug>
#include <QErrorMessage>
#include <QFileDialog>
#include <QLCDNumber>
#include <QMessageBox>
#include <QTimer>
#include <QProcess>

// cmath
#include <cmath>

// PI
const float M_PI = 4 * atan(1);

basic_QtVTK::basic_QtVTK()
{
    this->setupUi(this);
    this->trackerWidget->hide();

    // Disable tracker button until Volume is loaded
    //this->trackerButton->setDisabled(true);

    // Initialize VTK Objects 
    this->visualizationController = new VisualizationController(this);

    // Connect VTK with Qt
    this->openGLWidget->GetRenderWindow()->AddRenderer(this->visualizationController->ren);
    this->openGLWidget2->GetRenderWindow()->AddRenderer(this->visualizationController->ren2);

    // Set to Blue
    this->visualizationController->ren->SetBackground(.1, .2, .4);

    // Set to White
    this->visualizationController->ren2->SetBackground(.9, .9, .9);
    SetupQTObjects();

    /*
    vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    this->openGLWidget->SetRenderWindow(renWin);
    this->openGLWidget->GetRenderWindow()->AddRenderer(ren);
    ren->SetBackground(.1, .2, .4);*/
    this->Render();

}
void basic_QtVTK::Render()
{
    this->openGLWidget->GetRenderWindow()->Render();
    this->openGLWidget2->GetRenderWindow()->Render();
}


void basic_QtVTK::SetupQTObjects()
{
    connect(action_Background_Color, SIGNAL(triggered()), this, SLOT(EditRendererBackgroundColor()));
    connect(action_Quit, SIGNAL(triggered()), this, SLOT(SlotExit()));
    connect(actionLoad_Mesh, SIGNAL(triggered()), this, SLOT(LoadMesh()));
    connect(actionMesh_Color, SIGNAL(triggered()), this, SLOT(EditMeshColor()));
    connect(actionScreen_Shot, SIGNAL(triggered()), this, SLOT(ScreenShot()));
    connect(actionthis_program, SIGNAL(triggered()), this, SLOT(AboutThisProgram()));
    connect(trackerButton, SIGNAL(toggled(bool)), this, SLOT(StartTracker(bool)));
    connect(pivotButton, SIGNAL(toggled(bool)), this, SLOT(StylusCalibration(bool)));
    connect(actionLoad_Fiducial, SIGNAL(triggered()), this, SLOT(LoadFiducialPts()));
    connect(resetPhantomPtButton, SIGNAL(clicked()), this, SLOT(ResetPhantomCollectedPoints()));
    connect(deleteOnePhantomPtButton, SIGNAL(clicked()), this, SLOT(DeleteOnePhantomCollectedPoints()));
    connect(phantomRegistrationButton, SIGNAL(clicked()), this, SLOT(PerformPhantomRegistration()));
    connect(collectSinglePtButton, SIGNAL(clicked()), this, SLOT(CollectSinglePointPhantom()));
    connect(CollectDRRButton, SIGNAL(clicked()), this, SLOT(CollectDRR()));

    QPlusStatusIcon* statusIcon = new QPlusStatusIcon(NULL);
    statusIcon->SetMaxMessageCount(3000);
    statusbar->insertWidget(0, statusIcon);

    // QPlusDeviceSetSelectorWidget
    deviceSetSelectorWidget = new QPlusDeviceSetSelectorWidget(NULL);
    deviceSetSelectorWidget->SetConfigurationDirectory(QStringLiteral("C:\\d\\pb\\PlusLibData\\ConfigFiles"));
    deviceSetSelectorWidget->SetConnectButtonText("Start");
    deviceSetSelectorWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    deviceSetSelectorWidget->setMaximumWidth(200);
    verticalLayout_4->addWidget(deviceSetSelectorWidget);
    connect(deviceSetSelectorWidget, SIGNAL(ConnectToDevicesByConfigFileInvoked(std::string)), this, SLOT(ConnectToDevicesByConfigFile(std::string)));
    //connect(deviceSetSelectorWidget, SIGNAL(DeviceSetSelected(std::string)), this, SLOT(ConnectToDevicesByConfigFile(std::string)));
}

// Read config file and connect to devices
void basic_QtVTK::ConnectToDevicesByConfigFile(std::string aConfigFile)
{
    this->trackerButton->setDisabled(false);
    this->configFile = aConfigFile;
    this->LoadMesh();
}


// This method starts the Tracker Device
// 
void basic_QtVTK::StartTracker(bool checked)
{

    LOG_INFO("START TRACKER");
    
    this->visualizationController->StartTracker();

    // create a QTimer
    trackerTimer = new QTimer(this);
    connect(trackerTimer, SIGNAL(timeout()), this, SLOT(UpdateTrackerInfo()));
    trackerTimer->start(0);


}
void basic_QtVTK::UpdateTrackerInfo()
{
    this->visualizationController->UpdateTracker();
    this->Render();
}

void basic_QtVTK::LoadFiducialPts()
{
    // fiducial is stored as lines of 3 floats
    /*QString fname = QFileDialog::getOpenFileName(this,
        tr("Open fiducial file"),
        QDir::currentPath(),
        "PolyData File (*.xyz)");
    vtkNew<vtkSimplePointsReader> reader;
    reader->SetFileName(fname.toStdString().c_str());
    reader->Update();

    fiducialPts = reader->GetOutput()->GetPoints();
    fiducialPts->Modified();

    qDebug() << "# of fiducial:" << fiducialPts->GetNumberOfPoints();
    statusBar()->showMessage(tr("Loaded fiducial file"));*/
}


void basic_QtVTK::LoadMesh()
{
    this->visualizationController->LoadVolumes(this->configFile);
    this->Render();
}


void basic_QtVTK::StylusCalibration(bool checked)
{
    // Implement pivot/spin calibration in PLUS
}


void basic_QtVTK::CreateTrackerLogo()
{
}


void basic_QtVTK::ScreenShot()
{
  
}


void basic_QtVTK::EditRendererBackgroundColor()
{
    
}


void basic_QtVTK::EditMeshColor()
{
    // use this slot for selecting Transfer function preset
}


void basic_QtVTK::SlotExit()
{
    qApp->exit();
}


void basic_QtVTK::AboutThisProgram()
{
    QMessageBox::about(this, tr("About basic_QtVTK"),
        tr("This is a demostration for Qt/VTK/AIGS integration\n\n"
            "By: \n\n"
            "Elvis C.S. Chen\t\t"
            "chene@robarts.ca"));
}


void basic_QtVTK::CreateLinearZStylusActor()
{

}

void basic_QtVTK::CollectDRR()
{

    LOG_INFO("Collect DRR Button not implemented in this version\n");

}

/*!
* A QT Slot to collect a single ponts on the phantom
*/
void basic_QtVTK::CollectSinglePointPhantom()
{
    LOG_INFO("Collect");
}


void basic_QtVTK::ResetPhantomCollectedPoints()
{
    LOG_INFO("reset");
}


void basic_QtVTK::DeleteOnePhantomCollectedPoints()
{

    LOG_INFO("delete");
}


void basic_QtVTK::PerformPhantomRegistration()
{
    LOG_INFO("register");

}
