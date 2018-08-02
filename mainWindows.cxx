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
    this->trackerButton->setDisabled(true);

    SetupQTObjects();

    this->Render();

}
void basic_QtVTK::Render()
{
    this->openGLWidget->GetRenderWindow()->Render();
    this->openGLWidget2->GetRenderWindow()->Render();
}

void basic_QtVTK::SetupVTKObjects()
{

    // tracker
    this->openGLWidget->SetRenderWindow(renWin);

    // VTK Renderer
    ren->SetBackground(.1, .2, .4);

    // Set to white for Fluoro Image
    ren2->SetBackground(.9, .9, .9);

    // connect VTK with Qt
    this->openGLWidget->GetRenderWindow()->AddRenderer(ren);
    this->openGLWidget2->GetRenderWindow()->AddRenderer(ren2);
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
}

// Read config file and connect to devices
void basic_QtVTK::ConnectToDevicesByConfigFile(std::string aConfigFile)
{
    this->trackerButton->setDisabled(false);

    // Initialize visualizationcontroller
    this->visualizationController
}


// This method starts the Tracker Device
// 
void basic_QtVTK::startTracker(bool checked)
{
    // Put below code into method such as follows:
    // this->DataCollector->SetUpDevices(m_configFile);

    /*arduinoTracker1 = new ArduinoTracker("\\\\.\\COM9");
    arduinoTracker2 = new ArduinoTracker("\\\\.\\COM10");
    // create a QTimer
    trackerTimer = new QTimer(this);
    connect(trackerTimer, SIGNAL(timeout()), this, SLOT(updateTrackerInfo()));
    trackerTimer->start(0);
    return;*/

    if (checked)
    {
        // if tracker is not initialized, do so now
        if (!isTrackerInitialized)
        {
            // Get tracker1
            if (dataCollector->GetDevice(trackerDevice, "TrackerDevice") != PLUS_SUCCESS)
            {
                LOG_ERROR("Unable to locate the device with ID = \"TrackerDevice\". Check config file.");
                exit;
            }

            // Cast Device to type vtkPlusNDITracker
            myNDITracker = dynamic_cast<vtkPlusNDITracker*>(trackerDevice);

            if (myNDITracker == NULL)
            {
                LOG_INFO("Tracking device is not NDI Polaris.");
                myAccelerometer = dynamic_cast<vtkPlusWitMotionTracker*>(trackerDevice);
                
            }

            // Get Tracker2
            if (dataCollector->GetDevice(trackerDevice2, "TrackerDevice2") != PLUS_SUCCESS)
            {
                LOG_ERROR("Unable to locate the device with ID = \"TrackerDevice2\". Check config file.");
                exit;
            }

            myAccelerometer2 = dynamic_cast<vtkPlusWitMotionTracker*>(trackerDevice2);

            // Get Mixer
            if (dataCollector->GetDevice(mixerDevice, "TrackedVideoDevice") != PLUS_SUCCESS)
            {
                LOG_ERROR("Unable to locate the device with ID = \"TrackerVideoDevice\". Check config file.");
                exit;
            }

            myMixer = dynamic_cast<vtkPlusVirtualMixer*>(mixerDevice);

      
            // Connect to devices
            //std::cout << "Connecting to Device through COM" << myTracker->GetSerialPort();
            if (dataCollector->Connect() != PLUS_SUCCESS)
            {
                std::cout << ".................... [FAILED]" << std::endl;
                LOG_ERROR("Failed to connect to devices!");
                exit(EXIT_FAILURE);
            }
            if (dataCollector->Start() != PLUS_SUCCESS)
            {
                LOG_ERROR("Failed to start Data collection!");
                exit(EXIT_FAILURE);
            }
            if (transformRepository->ReadConfiguration(m_configRootElement) != PLUS_SUCCESS)
            {
                LOG_ERROR("Configuration incorrect for vtkPlusTransformRepository.");
                exit(EXIT_FAILURE);
            }
            if (myNDITracker != NULL)
            {
                if (myNDITracker->GetOutputChannelByName(trackerChannel, "TrackerStream") != PLUS_SUCCESS)
                {
                    LOG_ERROR("Unable to locate the channel with Id=\"TrackerStream\". Check config file.");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                if (myMixer->GetOutputChannelByName(trackerChannel, "MergedChannel") != PLUS_SUCCESS)
                {
                    LOG_ERROR("Unable to locate the channel with Id=\"TrackerChannel\". Check config file.");
                    exit(EXIT_FAILURE);
                }
                // Get InputChannels
                /*if (myAccelerometer->GetOutputChannelByName(trackerChannel, "TrackerChannel") != PLUS_SUCCESS)
                {
                    LOG_ERROR("Unable to locate the channel with Id=\"TrackerChannel\". Check config file.");
                    exit(EXIT_FAILURE);
                }
                if (myAccelerometer2->GetOutputChannelByName(trackerChannel2, "TrackerChannel2") != PLUS_SUCCESS)
                {
                    LOG_ERROR("Unable to locate the channel with Id=\"TrackerChannel2\". Check config file.");
                    exit(EXIT_FAILURE);
                }*/
                
                
            }
         
            //myTracker->SetBaudRate(115200); /*!< Set the baud rate sufficiently high. */
            //int nMax = myTracker->GetNumberOfTools();
        }
    }

    // else stop tracker
    else
    {
      trackerTimer->stop();
      return;
    }

    // Set Camera FocalPoint
    this->ren->GetActiveCamera()->SetFocalPoint(0, -100, -174);
    this->ren2->GetActiveCamera()->SetFocalPoint(0, -100, -174);

    myAccelerometer->SetBaudRate(19200);

    qDebug() << "Tracker Initialized";
    statusBar()->showMessage("Tracker Initialized", 5000);
    isTrackerInitialized = true;

    // enable the logo widget to display the status of each tracked object
    this->createTrackerLogo();
    trackerLogoWidget->On();
    this->openGLWidget->GetRenderWindow()->Render();

    // create a QTimer
    trackerTimer = new QTimer(this);
    connect(trackerTimer, SIGNAL(timeout()), this, SLOT(updateTrackerInfo()));
    trackerTimer->start(0);
    // test commitasd

}


void basic_QtVTK::updateTrackerInfo()
{
    // this->DataCollector->UpdateTrackerInfo();
    /* */
    if (!isTrackerInitialized)
    {
        LOG_ERROR("Attempting to update Tracker when tracker is not initialized");
        exit(EXIT_FAILURE);
    }

    trackerChannel->GetTrackedFrame(trackerFrame);
    transformRepository->SetTransforms(trackerFrame);
        
    bool isValid = false;
    transformRepository->GetTransform(accelerometerToTrackerName, accelerometerToTracker, &isValid);
    transformRepository->GetTransform(accelerometer2ToTrackerName, accelerometer2ToTracker, &isValid);

    //this->arduinoTracker1->ReceiveData();
    //this->arduinoTracker2->ReceiveData();

    //cameraTransform->Identity();
    //cameraTransform->PostMultiply();
    //cameraTransform->Translate(0, 0, 174);
    //cameraTransform->Concatenate(accelerometerToTracker);
    //cameraTransform->Concatenate(accelerometerToCT);

    /*camera2Transform->Identity();
    camera2Transform->PostMultiply();
    camera2Transform->Translate(0, 0, 174);
    camera2Transform->RotateX(this->arduinoTracker2->orientation[0]);
    camera2Transform->RotateY(this->arduinoTracker2->orientation[1]);
    //camera2Transform->RotateZ(this->arduinoTracker2->orientation[2]);
    //camera2Transform->Concatenate(accelerometer2ToTracker);

    volume->SetUserTransform(cameraTransform);
    //fluoroVolume->SetUserTransform(camera2Transform);*/

    this->setCamera2UsingWitMotionTracker();
    this->setCameraUsingWitMotionTracker();
    
    this->openGLWidget->GetRenderWindow()->Render();
    this->openGLWidget2->GetRenderWindow()->Render();

}


void basic_QtVTK::loadFiducialPts()
{
    // fiducial is stored as lines of 3 floats
    QString fname = QFileDialog::getOpenFileName(this,
        tr("Open fiducial file"),
        QDir::currentPath(),
        "PolyData File (*.xyz)");
    vtkNew<vtkSimplePointsReader> reader;
    reader->SetFileName(fname.toStdString().c_str());
    reader->Update();

    fiducialPts = reader->GetOutput()->GetPoints();
    fiducialPts->Modified();

    qDebug() << "# of fiducial:" << fiducialPts->GetNumberOfPoints();
    statusBar()->showMessage(tr("Loaded fiducial file"));
}


void basic_QtVTK::loadMesh()
{
    this->visualizationController->LoadMesh();
}


void basic_QtVTK::stylusCalibration(bool checked)
{
    // Implement pivot/spin calibration in PLUS
}


void basic_QtVTK::createTrackerLogo()
{
    logoWidgetX = 16;
    logoWidgetY = 10;

    // hardcoded for now
    int nObjects = 2;

    trackerDrawing->SetScalarTypeToUnsignedChar();
    trackerDrawing->SetNumberOfScalarComponents(3);
    trackerDrawing->SetExtent(0, logoWidgetX*nObjects + nObjects, 0, logoWidgetY + 2, 0, 0);
    // Clear the image  
    trackerDrawing->SetDrawColor(255, 255, 255);
    trackerDrawing->FillBox(0, logoWidgetX*nObjects + nObjects, 0, logoWidgetY + 2);
    trackerDrawing->Update();

    trackerLogoRepresentation->SetImage(trackerDrawing->GetOutput());
    trackerLogoRepresentation->SetPosition(.45, 0);
    trackerLogoRepresentation->SetPosition2(.1, .1);
    trackerLogoRepresentation->GetImageProperty()->SetOpacity(0.5);
    trackerLogoWidget->SetRepresentation(trackerLogoRepresentation);

    trackerLogoWidget->SetInteractor(this->openGLWidget->GetInteractor());
}


void basic_QtVTK::screenShot()
{
    // output the screen to PNG files.
    //
    // the file names are 0.png, 1.png, ..., etc.
    //
    QString fname = QString::number(screenShotFileNumber) + QString(tr(".png"));
    screenShotFileNumber++;

    vtkNew<vtkWindowToImageFilter> w2i;
    w2i->SetInput(this->openGLWidget->GetRenderWindow());
    w2i->ReadFrontBufferOff();
    w2i->SetInputBufferTypeToRGBA();

    vtkNew<vtkImageExtractComponents> iec;
    iec->SetInputConnection(w2i->GetOutputPort());
    iec->SetComponents(0, 1, 2);

    vtkNew<vtkPNGWriter> writer;
    writer->SetFileName(fname.toStdString().c_str());
    writer->SetInputConnection(iec->GetOutputPort());
    writer->Write();
}


void basic_QtVTK::editRendererBackgroundColor()
{
    QColor color = QColorDialog::getColor(Qt::gray, this);

    if (color.isValid())
    {
        int r, g, b;
        color.getRgb(&r, &g, &b);
        ren->SetBackground((double)r / 255.0, (double)g / 255.0, (double)b / 255.0);

        // Set Fluoro Background to white
        ren2->SetBackground(255.0, 255.0, 255.0);


        this->openGLWidget->GetRenderWindow()->Render();
    }
}


void basic_QtVTK::editMeshColor()
{
    // use this slot for selecting Transfer function preset
}


void basic_QtVTK::slotExit()
{
    cleanVTKObjects(); // if needed
    qApp->exit();
}


void basic_QtVTK::aboutThisProgram()
{
    QMessageBox::about(this, tr("About basic_QtVTK"),
        tr("This is a demostration for Qt/VTK/AIGS integration\n\n"
            "By: \n\n"
            "Elvis C.S. Chen\t\t"
            "chene@robarts.ca"));
}


void basic_QtVTK::createLinearZStylusActor()
{
    double radius = 1.5; // mm
    int nSides = 36;
    // find out which port is the stylus

    trackerChannel->GetTrackedFrame(trackerFrame);
    transformRepository->SetTransforms(trackerFrame);

    try
    {
      throw new std::invalid_argument("asd");
    }
    catch (std::invalid_argument ex)
    {
      printf(ex.what());
      exit;
    }

    PlusTransformName name;
    name.SetTransformName("StylusTipToStylus");

    vtkSmartPointer<vtkMatrix4x4> matr = vtkSmartPointer<vtkMatrix4x4>::New();


    bool isProbeMatrixValid = false;
    // Check if probe is visible  
    transformRepository->GetTransform(name, matr, &isProbeMatrixValid);



    //vtkMatrix4x4 *calibMatrix = tools[toolIdx]->GetCalibrationMatrix();
    vtkMatrix4x4 *calibMatrix = matr;

    double *pos, *outpt;
    pos = new double[4];
    outpt = new double[4];

    pos[0] = pos[1] = 0.0;
    pos[2] = pos[3] = 1.0;
    calibMatrix->MultiplyPoint(pos, outpt);
    double l = sqrt(outpt[0] * outpt[0] + outpt[1] * outpt[1] + outpt[2] * outpt[2]);
    pos[0] = outpt[0] / l;
    pos[1] = outpt[1] / l;
    pos[2] = outpt[2] / l; // pos now is the normalized direction of the stylus

    vtkNew<vtkAppendPolyData> append;
    double coneHeight = 25.0;

    vtkNew<vtkLineSource> line;
    line->SetPoint1(coneHeight*pos[0], coneHeight*pos[1], coneHeight*pos[2]);
    line->SetPoint2(outpt[0], outpt[1], outpt[2]);

    vtkNew<vtkTubeFilter> tube;
    tube->SetInputConnection(line->GetOutputPort());
    tube->SetRadius(radius);
    tube->SetNumberOfSides(nSides);


    vtkNew<vtkConeSource> cone;
    cone->SetHeight(coneHeight);
    cone->SetRadius(radius);
    cone->SetDirection(-pos[0], -pos[1], -pos[2]);
    cone->SetResolution(nSides);
    cone->SetCenter(.5*coneHeight*pos[0], .5*coneHeight*pos[1], .5*coneHeight*pos[2]);
    //cone->SetCenter(outpt[0] - coneHeight*pos[0], outpt[1] - coneHeight*pos[1], outpt[2] - coneHeight*pos[2]);

    append->AddInputConnection(tube->GetOutputPort());
    append->AddInputConnection(cone->GetOutputPort());

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(append->GetOutputPort());
    stylusActor->SetMapper(mapper);

    vtkSmartPointer<vtkTransform> tran = vtkSmartPointer<vtkTransform>::New();

    //stylusActor->SetUserTransform(tools[toolIdx]->GetTransform());

    bool isValid = false;
    transformRepository->GetTransform(stylusToTrackerName, stylusToTracker, &isValid);

    tran->SetMatrix(stylusToTracker);

    stylusActor->SetUserTransform(tran);

    vtkNew<vtkNamedColors> color;
    stylusActor->GetProperty()->SetColor(color->GetColor3d("zinc_white").GetRed(),
        color->GetColor3d("zinc_white").GetGreen(),
        color->GetColor3d("zinc_white").GetBlue());

    //ren->AddActor(stylusActor);

    delete[] pos;
    delete[] outpt;

}

void basic_QtVTK::collectDRR()
{

    LOG_INFO("Collect DRR Button not implemented in this version\n");

}

/*!
* A QT Slot to collect a single ponts on the phantom
*/
void basic_QtVTK::collectSinglePointPhantom()
{
    if (!myNDITracker->IsRecording())
    {
        LOG_ERROR("NDI Tracker not recording");
        return;
    }
    
    // If either tool is out of view, return false
    vtkNew<vtkTransform> transform;               
    vtkNew<vtkTransform> stylusTransform;
    vtkNew<vtkTransform> refTransform;
    stylusTransform->SetMatrix(stylusTipToTracker);
    refTransform->SetMatrix(referenceToTracker);

    transform->PostMultiply();
    transform->Identity();
    transform->Concatenate(stylusTransform);
    transform->Concatenate(refTransform->GetLinearInverse());
    transform->Update();

    double pos[3];
    transform->GetPosition(pos);
    std::cout << pos[0] << " " << pos[1] << " " << pos[2] << std::endl;
    targetPoints->InsertNextPoint(pos[0], pos[1], pos[2]);
    targetPoints->Modified();
    numCollected->display((int)targetPoints->GetNumberOfPoints());
}


void basic_QtVTK::resetPhantomCollectedPoints()
{
    LOG_INFO("reset");
}


void basic_QtVTK::deleteOnePhantomCollectedPoints()
{

    LOG_INFO("delete");
}


void basic_QtVTK::performPhantomRegistration()
{
    LOG_INFO("register");

    vtkNew<vtkTransform> refTransform;
    refTransform->SetMatrix(referenceToTracker);

    phantomRegTransform->SetTargetLandmarks(targetPoints);
    phantomRegTransform->SetSourceLandmarks(fiducialPts);
    phantomRegTransform->SetModeToRigidBody();
    phantomRegTransform->Update();
    phantomRegTransform->Print(std::cerr);
    //isPhantomCalibrated = true;

    phantomTransform->Identity();
    phantomTransform->PostMultiply();
    phantomTransform->Concatenate(phantomRegTransform->GetMatrix());
    phantomTransform->Concatenate(refTransform);
    volume->SetUserTransform(phantomTransform);
    //actor->SetUserTransform(phantomTransform);

    qDebug() << phantomTransform->GetPosition() << endl;

    this->openGLWidget->GetRenderWindow()->Render();

}

void basic_QtVTK::setCameraUsingNDITracker()
{

    double zdir[4] = { 0.0, 0.0, -1.0, 0.0 };
    double up[4] = { 1, 0, 0, 0 };
    double orig[4] = { 0.0, 0.0, 0.0, 1.0 };

    double outDir[4], pos[4], updir[4], phantomOrig[4];

    vtkSmartPointer<vtkTransform> tran = vtkSmartPointer<vtkTransform>::New();
    tran->SetMatrix(stylusTipToTracker);

    tran->MultiplyPoint(zdir, outDir);
    tran->MultiplyPoint(orig, pos);
    tran->MultiplyPoint(up, updir);
    tran->MultiplyPoint(orig, phantomOrig);

    //this->volume->SetUserTransform(this->tools[1]->GetTransform());

    // setting up camera
    this->ren->GetActiveCamera()->SetPosition(pos[0], pos[1], pos[2]);
    this->ren->GetActiveCamera()->SetViewUp(updir[0], updir[1], updir[2]);
    this->ren->GetActiveCamera()->SetFocalPoint(pos[0] + 100 * outDir[0], pos[1] + 100 * outDir[1], pos[2] + 100 * outDir[2]);

    // Uncomment to set the focal point to the phantom origin
    //ren->GetActiveCamera()->SetFocalPoint(phantomOrig[0], phantomOrig[1], phantomOrig[2]);
    

}

void basic_QtVTK::setCameraUsingWitMotionTracker()
{
    vtkNew<vtkTransform> tran;
    tran->Identity();
    tran->PostMultiply();
    double *a = rotationMatrixToEulerAngles(accelerometer2ToTracker);
    a[1] = a[1] * 180 / M_PI;

    cameraTransform->PostMultiply();
    cameraTransform->Identity();
    cameraTransform->Translate(0, 0, -800);
    cameraTransform->Concatenate(accelerometerToTracker);
    cameraTransform->Concatenate(accelerometerToCT);
    cameraTransform->Translate(0, 0, -174);
    cameraTransform->Translate(0, 0, (-a[1] - 350)*0.5);
    cameraTransform->Update();

    this->ren->GetActiveCamera()->SetPosition(cameraTransform->GetPosition());
    this->ren->GetActiveCamera()->SetFocalPoint(0, -100, (-a[1] - 350)*0.5);
    cameraTransform->MultiplyPoint(up, out);
    this->ren->GetActiveCamera()->SetViewUp(out[0], out[1], out[2]);

    //this->ren->GetActiveCamera()->SetFocalPoint(0, -100, -174);
    //this->ren2->GetActiveCamera()->SetFocalPoint(0, -100, -174);

}
void basic_QtVTK::setCamera2UsingWitMotionTracker()
{
    vtkNew<vtkTransform> tran;
    tran->Identity();
    tran->PostMultiply();
    double *a = rotationMatrixToEulerAngles(accelerometer2ToTracker);
    a[1] = a[1] * 180 / M_PI;

    camera2Transform->PostMultiply();
    camera2Transform->Identity();
    camera2Transform->Translate(0, 0, -800);
    camera2Transform->Concatenate(accelerometerToTracker);
    camera2Transform->Concatenate(accelerometerToCT);
    camera2Transform->Translate(0, 0, -174);
    camera2Transform->Translate(0, 0, (-a[1] - 350)*0.5);
    camera2Transform->Update();

    this->ren2->GetActiveCamera()->SetPosition(camera2Transform->GetPosition());
    this->ren2->GetActiveCamera()->SetFocalPoint(0, -100, (-a[1] - 350)*0.5);
    camera2Transform->MultiplyPoint(up, out);
    this->ren2->GetActiveCamera()->SetViewUp(out[0], out[1], out[2]);

    //this->ren->GetActiveCamera()->SetFocalPoint(0, -100, -174);
    //this->ren2->GetActiveCamera()->SetFocalPoint(0, -100, -174);

}
// Calculates rotation matrix to euler angles
// The result is the same as MATLAB except the order
// of the euler angles ( x and z are swapped ).
double* basic_QtVTK::rotationMatrixToEulerAngles(vtkMatrix4x4* R)
{

  double* a = new double[3];

  float sy = sqrt(R->Element[0][0] * R->Element[0][0] + R->Element[1][0] * R->Element[1][0]);

  bool singular = sy < 1e-6; 

  float x, y, z;
  if (!singular)
  {
    x = atan2(R->Element[2][1], R->Element[2][2]);
    y = atan2(-R->Element[2][0], sy);
    z = atan2(R->Element[1][0], R->Element[0][0]);
  }
  else
  {
    x = atan2(-R->Element[1][2], R->Element[1][1]);
    y = atan2(-R->Element[2][0], sy);
    z = 0;
  }

  a[0] = x;
  a[1] = y;
  a[2] = z;

  return a;
}
