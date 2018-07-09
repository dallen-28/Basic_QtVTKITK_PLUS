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
const std::string TrackerType = "Arduino";

char* portName1 = "\\\\.\\COM6";
char* portName2 = "\\\\.\\COM9";


template< class PReader > vtkPolyData *readAnPolyData(const char *fname) {
    vtkSmartPointer< PReader > reader =
        vtkSmartPointer< PReader >::New();
    reader->SetFileName(fname);
    reader->Update();
    reader->GetOutput()->Register(reader);
    return(vtkPolyData::SafeDownCast(reader->GetOutput()));
}

basic_QtVTK::basic_QtVTK()
{
    this->setupUi(this);
    this->trackerWidget->hide();

    createVTKObjects();
    setupVTKObjects();
    setupQTObjects();

    // Initialize ArduinoTrackers
    accelerometer1 = new ArduinoTracker(portName1);
    accelerometer2 = new ArduinoTracker(portName2);

    ren->ResetCameraClippingRange();
    this->openGLWidget->GetRenderWindow()->Render();
    this->openGLWidget2->GetRenderWindow()->Render();
}


void basic_QtVTK::createVTKObjects()
{
    actor = vtkSmartPointer<vtkActor>::New();
    actor2 = vtkSmartPointer<vtkActor>::New();

    vtkConeSource *cone = vtkConeSource::New();
    vtkConeSource *cone2 = vtkConeSource::New();

    vtkPolyDataMapper *mapper = vtkPolyDataMapper::New();
    vtkPolyDataMapper *mapper2 = vtkPolyDataMapper::New();

    mapper->SetInputConnection(cone->GetOutputPort());
    mapper2->SetInputConnection(cone2->GetOutputPort());
    actor->SetMapper(mapper);
    actor2->SetMapper(mapper2);

    volume = vtkSmartPointer<vtkVolume>::New();
    //myTracker = vtkSmartPointer< vtkNDITracker >::New();
    myTracker = vtkSmartPointer< vtkPlusNDITracker >::New();

    ren = vtkSmartPointer<vtkRenderer>::New();
    ren2 = vtkSmartPointer<vtkRenderer>::New();
    phantomRegTransform = vtkSmartPointer<vtkLandmarkTransform>::New();
    phantomTransform = vtkSmartPointer<vtkTransform>::New();
    targetPoints = vtkSmartPointer<vtkPoints>::New();
    renWin = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    stylusActor = vtkSmartPointer<vtkActor>::New();
    trackerDrawing = vtkSmartPointer<vtkImageCanvasSource2D>::New();
    trackerLogoRepresentation = vtkSmartPointer<vtkLogoRepresentation>::New();
    trackerLogoWidget = vtkSmartPointer<vtkLogoWidget>::New();
    
    dataCollector = vtkSmartPointer<vtkPlusDataCollector>::New();
    transformRepository = vtkSmartPointer<vtkPlusTransformRepository>::New();

    // Initialize Transforms
    stylusToTracker = vtkSmartPointer<vtkMatrix4x4>::New();
    referenceToTracker = vtkSmartPointer<vtkMatrix4x4>::New();
    stylusTipToReference = vtkSmartPointer<vtkMatrix4x4>::New();
    stylusTipToTracker = vtkSmartPointer<vtkMatrix4x4>::New();
    calibration = vtkSmartPointer<vtkPlusProbeCalibrationAlgo>::New();
    
    
    ren->AddActor(actor);
    ren->AddActor(actor2);
    actor->SetPosition(0, 0, 5);
}


void basic_QtVTK::cleanVTKObjects()
{
    // if needed
    if (isTrackerInitialized)
        myTracker->StopRecording();
}


void basic_QtVTK::setupVTKObjects()
{
    // number of screenshot
    screenShotFileNumber = 0;

    // tracker
    this->isTrackerInitialized = isStylusCalibrated = false;
    trackedObjects.push_back(std::make_tuple(4, QString("C://users//danie//documents//NDI_roms//8700340.rom"), enumTrackedObjectTypes::enStylus)); // NDI 3 sphere linear stylus
    trackedObjects.push_back(std::make_tuple(5, QString("C://users//danie//documents//NDI_roms//8700302.rom"), enumTrackedObjectTypes::enOthers)); // NDI 4 sphere planar

    this->openGLWidget->SetRenderWindow(renWin);

    // VTK Renderer
    ren->SetBackground(.1, .2, .4);

    // connect VTK with Qt
    this->openGLWidget->GetRenderWindow()->AddRenderer(ren);
    this->openGLWidget2->GetRenderWindow()->AddRenderer(ren2);
}


void basic_QtVTK::setupQTObjects()
{
    connect(action_Background_Color, SIGNAL(triggered()), this, SLOT(editRendererBackgroundColor()));
    connect(action_Quit, SIGNAL(triggered()), this, SLOT(slotExit()));
    connect(actionLoad_Mesh, SIGNAL(triggered()), this, SLOT(loadMesh()));
    connect(actionMesh_Color, SIGNAL(triggered()), this, SLOT(editMeshColor()));
    connect(actionScreen_Shot, SIGNAL(triggered()), this, SLOT(screenShot()));
    connect(actionthis_program, SIGNAL(triggered()), this, SLOT(aboutThisProgram()));
    connect(trackerButton, SIGNAL(toggled(bool)), this, SLOT(startTracker(bool)));
    connect(pivotButton, SIGNAL(toggled(bool)), this, SLOT(stylusCalibration(bool)));
    connect(actionLoad_Fiducial, SIGNAL(triggered()), this, SLOT(loadFiducialPts()));
    connect(resetPhantomPtButton, SIGNAL(clicked()), this, SLOT(resetPhantomCollectedPoints()));
    connect(deleteOnePhantomPtButton, SIGNAL(clicked()), this, SLOT(deleteOnePhantomCollectedPoints()));
    connect(phantomRegistrationButton, SIGNAL(clicked()), this, SLOT(performPhantomRegistration()));
    connect(collectSinglePtButton, SIGNAL(clicked()), this, SLOT(collectSinglePointPhantom()));
    connect(CollectDRRButton, SIGNAL(clicked()), this, SLOT(collectDRR()));

    QPlusStatusIcon* statusIcon = new QPlusStatusIcon(NULL);
    statusIcon->SetMaxMessageCount(3000);
    statusbar->insertWidget(0, statusIcon);

    // QPlusDeviceSetSelectorWidget
    m_DeviceSetSelectorWidget = new QPlusDeviceSetSelectorWidget(NULL);
    m_DeviceSetSelectorWidget->SetConfigurationDirectory(QStringLiteral("C:\\d\\pb\\PlusLibData\\ConfigFiles"));
    m_DeviceSetSelectorWidget->SetConnectButtonText("Launch Server");
    m_DeviceSetSelectorWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_DeviceSetSelectorWidget->setMaximumWidth(200);
    verticalLayout_4->addWidget(m_DeviceSetSelectorWidget);
    connect(m_DeviceSetSelectorWidget, SIGNAL(ConnectToDevicesByConfigFileInvoked(std::string)), this, SLOT(ConnectToDevicesByConfigFile(std::string)));
}

// Read config file and connect to devices
void basic_QtVTK::ConnectToDevicesByConfigFile(std::string aConfigFile)
{
    qDebug() << "Connect using configuration file: " << aConfigFile.c_str();

    configRootElement = vtkSmartPointer<vtkXMLDataElement>::Take(vtkXMLUtilities::ReadElementFromFile(aConfigFile.c_str()));

    // Read configuration
    if (PlusXmlUtils::ReadDeviceSetConfigurationFromFile(configRootElement, aConfigFile.c_str()) == PLUS_FAIL)
    {
        LOG_ERROR("Unable to read configuration from file" << aConfigFile.c_str());
        exit;
    }

    vtkPlusConfig::GetInstance()->SetDeviceSetConfigurationData(configRootElement);

    if (dataCollector->ReadConfiguration(configRootElement) != PLUS_SUCCESS)
    {
        LOG_ERROR("Configuration incorrect for vtkPlusDataCollector.");
        exit;
    }

    // Set transform names
    stylusToTrackerName.SetTransformName("StylusToTracker");
    referenceToTrackerName.SetTransformName("ReferenceToTracker");
    stylusTipToReferenceName.SetTransformName("StylusTipToReference");
    stylusTipToTrackerName.SetTransformName("StylusTipToTracker");
}


// This method starts the Tracker Device
// 
void basic_QtVTK::startTracker(bool checked)
{
    if (checked)
    {
        if (TrackerType == "Arduino")
        {
            trackerTimer = new QTimer(this);
            connect(trackerTimer, SIGNAL(timeout()), this, SLOT(UpdateVolume1()));
            trackerTimer->start(0);
            //this->openGLWidget->GetInteractor()->Disable();
            return;
        }


        // if tracker is not initialized, do so now
        if (!isTrackerInitialized)
        {
            // Get tracker
            if (dataCollector->GetDevice(trackerDevice, "TrackerDevice") != PLUS_SUCCESS)
            {
                LOG_ERROR("Unable to locate the device with ID = \"TrackerDevice\". Check config file.");
                exit;
            }

            // Cast Device to type vtkPlusNDITracker
            myTracker = dynamic_cast<vtkPlusNDITracker*>(trackerDevice);

            if (myTracker == NULL)
            {
                LOG_ERROR("Tracking device is not NDI Polaris. Could not connect.");
                exit(EXIT_FAILURE);
            }

            // Connect to devices
            std::cout << "Connecting to NDI Polaris through COM" << myTracker->GetSerialPort();
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
            if (transformRepository->ReadConfiguration(configRootElement) != PLUS_SUCCESS)
            {
                LOG_ERROR("Configuration incorrect for vtkPlusTransformRepository.");
                exit(EXIT_FAILURE);
            }
            if (myTracker->GetOutputChannelByName(trackerChannel, "TrackerStream") != PLUS_SUCCESS)
            {
                LOG_ERROR("Unable to locate the channel with Id=\"TrackerStream\". Check config file.");
                exit(EXIT_FAILURE);
            }

            myTracker->SetBaudRate(115200); /*!< Set the baud rate sufficiently high. */
            int nMax = myTracker->GetNumberOfTools();
        }
    }

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
  
}


void basic_QtVTK::updateTrackerInfo()
{
    if (isTrackerInitialized)
    {
        trackerChannel->GetTrackedFrame(trackerFrame);
        transformRepository->SetTransforms(trackerFrame);
        
        bool isProbeMatrixValid = false;
        // Check if probe is visible  
        if (transformRepository->GetTransform(stylusToTrackerName, stylusToTracker, &isProbeMatrixValid) == PLUS_SUCCESS && isProbeMatrixValid)
        {
            // connected and withing good tracking accuracy. shown in green
            trackerDrawing->SetDrawColor(0, 255, 0);
            trackerDrawing->FillBox(1, 16, 1, 11);
        }
        else
        {
            trackerDrawing->SetDrawColor(255, 0, 0);
            trackerDrawing->FillBox(1, 16, 1, 11);
        }

        bool isReferenceMatrixValid = false;
        // Check if reference is visible
        if (transformRepository->GetTransform(referenceToTrackerName, referenceToTracker, &isReferenceMatrixValid) == PLUS_SUCCESS && isReferenceMatrixValid)
        {
            // connected and withing good tracking accuracy. shown in green
            trackerDrawing->SetDrawColor(0, 255, 0);
            trackerDrawing->FillBox(17, 33, 1, 11);
        }
        else
        {
            trackerDrawing->SetDrawColor(255, 0, 0);
            trackerDrawing->FillBox(17, 33, 1, 11);
        }

        //this->volume->SetUserMatrix(stylusToTracker);


        /*for (int i = 0; i < (int)trackedObjects.size(); i++)
        {
            if (tools[i]->IsMissing())
            {
                // not connected, shown in blue
                trackerDrawing->SetDrawColor(0, 0, 255);
                trackerDrawing->FillBox(logoWidgetX*i + i + 1, logoWidgetX*(i + 1) + i, 1, logoWidgetY + 1);
            }
            else if (tools[i]->IsOutOfVolume())
            {
                // connected, visible but not accurate, shown in yellow
                trackerDrawing->SetDrawColor(255, 255, 0);
                trackerDrawing->FillBox(logoWidgetX*i + i + 1, logoWidgetX*(i + 1) + i, 1, logoWidgetY + 1);
            }
            else if (tools[i]->IsOutOfView())
            {
                // connected, visible, but outside of the tracking volume. Shown in red
                trackerDrawing->SetDrawColor(255, 0, 0);
                trackerDrawing->FillBox(logoWidgetX*i + i + 1, logoWidgetX*(i + 1) + i, 1, logoWidgetY + 1);
            }
            else
            {
                // connected and withing good tracking accuracy. shown in green
                trackerDrawing->SetDrawColor(0, 255, 0);
                trackerDrawing->FillBox(logoWidgetX*i + i + 1, logoWidgetX*(i + 1) + i, 1, logoWidgetY + 1);
            }
        }*/

        bool isValid = false;
        transformRepository->GetTransform(stylusTipToTrackerName, stylusTipToTracker, &isValid);

        this->setCameraUsingTracker();

        vtkSmartPointer<vtkTransform> refTransform = vtkSmartPointer<vtkTransform>::New();
        refTransform->SetMatrix(referenceToTracker);

        phantomTransform->Identity();
        phantomTransform->PostMultiply();
        phantomTransform->Concatenate(phantomRegTransform->GetMatrix());
        phantomTransform->Concatenate(refTransform);
        volume->SetUserTransform(phantomTransform);

        //this->actor->SetUserMatrix(stylusTipToReference);
        //vtkSmartPointer<vtkTransform> tran = vtkSmartPointer<vtkTransform>::New();

        //stylusActor->SetUserTransform(tools[toolIdx]->GetTransform());
        //tran->SetMatrix(stylusToTracker);
        //stylusActor->SetUserTransform(tran);

        //volume->SetUserMatrix(referenceToTracker);

        trackerDrawing->Update();
        this->openGLWidget->GetRenderWindow()->Render();

    }
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

void basic_QtVTK::UpdateVolume1()
{
    if (this->accelerometer1->arduino->isConnected())
    {
        this->accelerometer1->ReceiveData();
        this->accelerometer2->ReceiveData();

        vtkNew<vtkTransform> volumeTransform;
        volumeTransform->Identity();
        volumeTransform->PostMultiply();    

        vtkNew<vtkTransform> volumeTransform2;
        volumeTransform2->Identity();
        volumeTransform2->PostMultiply();

        //volumeTransform->Translate(0, 0, 174.0);

        volumeTransform->RotateX(this->accelerometer1->orientation[1] - 90);
        volumeTransform->RotateY(this->accelerometer1->orientation[0]);
        volumeTransform->RotateZ(this->accelerometer1->orientation[2] + 90);

        volumeTransform2->RotateX(this->accelerometer2->orientation[1] - 90);
        volumeTransform2->RotateY(this->accelerometer2->orientation[0]);
        volumeTransform2->RotateZ(this->accelerometer2->orientation[2] + 90);

        actor->SetUserTransform(volumeTransform);
        actor2->SetUserTransform(volumeTransform2);

        this->openGLWidget->GetRenderWindow()->Render();

    }
}

void basic_QtVTK::loadMesh()
{
    QString fname = QFileDialog::getOpenFileName(this,
        tr("Open phantom mesh"),
        QDir::currentPath(),
        "PolyData File (*.vtk *.stl *.ply *.obj *.vtp *.mha)");

    // std::cerr << fname.toStdString().c_str() << std::endl;

    vtkPolyData *data;

    vtkAlgorithm *reader = nullptr;
    vtkImageData *input = nullptr;

    QFileInfo info(fname);
    bool knownFileType = true;

    // parse the file extension and use the appropriate reader
    if (info.suffix() == QString(tr("vtk")))
    {
        data = readAnPolyData<vtkPolyDataReader >(fname.toStdString().c_str());
    }
    else if (info.suffix() == QString(tr("stl")) || info.suffix() == QString(tr("stlb")))
    {
        data = readAnPolyData<vtkSTLReader>(fname.toStdString().c_str());
    }
    else if (info.suffix() == QString(tr("ply")))
    {
        data = readAnPolyData<vtkPLYReader>(fname.toStdString().c_str());
    }
    else if (info.suffix() == QString(tr("obj")))
    {
        data = readAnPolyData<vtkOBJReader>(fname.toStdString().c_str());
    }
    else if (info.suffix() == QString(tr("vtp")))
    {
        data = readAnPolyData<vtkXMLPolyDataReader>(fname.toStdString().c_str());
    }
    else if (info.suffix() == QString(tr("mha")))
    {
        // Intialize ITK object
        this->fluoroImage = new ITKFluoroImage(fname.toStdString(), 0.0, 0.0, 0.0);

        vtkMetaImageReader *metaReader = vtkMetaImageReader::New();
        metaReader->SetFileName(fname.toUtf8());
        //metaReader->SetFileName("C://users//danie//Documents//CTChest.mha");
        metaReader->Update();
        input = metaReader->GetOutput();
        reader = metaReader;

        // Verify that we actually have a volume
        int dim[3];
        input->GetDimensions(dim);
        if (dim[0] < 2 ||
            dim[1] < 2 ||
            dim[2] < 2)
        {
            cout << "Error loading data!" << endl;
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        knownFileType = false;
    }

    if (knownFileType)
    {
        // do something only if we know the file type
        vtkNew<vtkPolyDataMapper> mapper;
        vtkNew<vtkSmartVolumeMapper> volumeMapper;
        vtkNew<vtkVolumeProperty> property;
        vtkNew<vtkColorTransferFunction> colorFun;
        vtkNew<vtkPiecewiseFunction> opacityFun;

        property->SetColor(colorFun);
        property->SetScalarOpacity(opacityFun);
        property->SetInterpolationTypeToLinear();

        colorFun->AddRGBPoint(-3024, 0, 0, 0, 0.5, 0.0);
        colorFun->AddRGBPoint(-16, 0.73, 0.25, 0.30, 0.49, .61);
        colorFun->AddRGBPoint(641, .90, .82, .56, .5, 0.0);
        colorFun->AddRGBPoint(3071, 1, 1, 1, .5, 0.0);

        opacityFun->AddPoint(-3024, 0, 0.5, 0.0);
        opacityFun->AddPoint(-16, 0, .49, .61);
        opacityFun->AddPoint(641, .72, .5, 0.0);
        opacityFun->AddPoint(3071, .71, 0.5, 0.0);

        volumeMapper->SetBlendModeToComposite();
        property->ShadeOn();
        property->SetAmbient(0.1);
        property->SetDiffuse(0.9);
        property->SetSpecular(0.2);
        property->SetSpecularPower(10.0);
        property->SetScalarOpacityUnitDistance(0.8919);

        if (info.suffix() == QString(tr("mha")))
        {
            volumeMapper->SetInputConnection(reader->GetOutputPort());

            volume->SetMapper(volumeMapper);
            volume->SetProperty(property);
            ren->AddVolume(volume);
            ren2->AddActor(this->fluoroImage->ExportToVTK());
        }
        
        else
        {
            mapper->SetInputData(data);
            actor->SetMapper(mapper);
            ren->AddActor(actor);
        }

        // reset the camera according to visible actors
        ren->GetActiveCamera()->ParallelProjectionOff();

        
        ren->ResetCamera();
        ren->ResetCameraClippingRange();
        this->openGLWidget->GetRenderWindow()->Render();

        vtkInteractorStyleImage *inStyle = vtkInteractorStyleImage::New();
        this->openGLWidget2->GetInteractor()->SetInteractorStyle(inStyle);

        ren2->ResetCamera();
        ren2->ResetCameraClippingRange();
        this->openGLWidget2->GetRenderWindow()->Render();


    }
    else
    {
        QErrorMessage *em = new QErrorMessage(this);
        em->showMessage("Input file format not supported");
    }
}


void basic_QtVTK::stylusCalibration(bool checked)
{
    // assumes that there is only 1 stylus among all the tracked objects

    // make sure the tracker is initialized/found first.
    if (isTrackerInitialized)
    {      

        // find out which port is the stylus
        int stylusPort = -1;
        int toolIdx = -1;
        for (int i = 0; i < (int)trackedObjects.size(); i++)
        {
            int port = std::get<0>(trackedObjects[i]);
            enumTrackedObjectTypes myType = std::get<2>(trackedObjects[i]);

            if (myType == enumTrackedObjectTypes::enStylus)
            {
                stylusPort = port;
                toolIdx = i;
            }
        }

        if (checked)
        {
            qDebug() << "Starting pivot calibration";
            double m[16] = { 1,0,0,0,
                0,1,0,0,
                0,0,1,0,
                0,0,0,1 };
            vtkNew<vtkMatrix4x4> matrix;
            matrix->DeepCopy(m);

            qDebug() << "stylus port:" << stylusPort << "index:" << toolIdx;
            tools[toolIdx]->SetCalibrationMatrix(matrix);
            tools[toolIdx]->InitializeToolTipCalibration();
            tools[toolIdx]->SetCollectToolTipCalibrationData(1);
        }
        else
        {
            tools[toolIdx]->SetCollectToolTipCalibrationData(0);
            this->stylusTipRMS->display(tools[toolIdx]->DoToolTipCalibration());
            isStylusCalibrated = true;
            qDebug() << "Pivot calibration finished";
            // tools[toolIdx]->Print(std::cerr);
            createLinearZStylusActor();
        }
    }
    else
    {
        // if tracker is not initialized, do nothing and uncheck the button
        pivotButton->setChecked(false);
    }
}


void basic_QtVTK::createTrackerLogo()
{
    logoWidgetX = 16;
    logoWidgetY = 10;

    int nObjects = (int)trackedObjects.size();

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
        ren2->SetBackground((double)r / 255.0, (double)g / 255.0, (double)b / 255.0);


        this->openGLWidget->GetRenderWindow()->Render();
    }
}


void basic_QtVTK::editMeshColor()
{
    QColor color = QColorDialog::getColor(Qt::gray, this);

    if (color.isValid())
    {
        int r, g, b;
        color.getRgb(&r, &g, &b);
        actor->GetProperty()->SetColor((double)r / 255.0, (double)g / 255.0, (double)b / 255.0);


        this->openGLWidget->GetRenderWindow()->Render();
    }
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
    int stylusPort = -1;
    int toolIdx = -1;
    for (int i = 0; i < (int)trackedObjects.size(); i++)
    {
        int port = std::get<0>(trackedObjects[i]);
        enumTrackedObjectTypes myType = std::get<2>(trackedObjects[i]);

        if (myType == enumTrackedObjectTypes::enStylus)
        {
            stylusPort = port;
            toolIdx = i;
        }
    }

    trackerChannel->GetTrackedFrame(trackerFrame);
    transformRepository->SetTransforms(trackerFrame);


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

// Calculates rotation matrix to euler angles
// The result is the same as MATLAB except the order
// of the euler angles ( x and z are swapped ).
double* basic_QtVTK::rotationMatrixToEulerAngles(vtkMatrix4x4* R)
{

    double* a = new double[3];

    float sy = sqrt(R->Element[0][0]*R->Element[0][0] + R->Element[1][0] * R->Element[1][0]);

    bool singular = sy < 1e-6; // If

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

void basic_QtVTK::collectDRR()
{

    vtkMatrix4x4* matr;

    // If acceloremeter is tracking get orientation from volume rather then camera
    //if (arduino && arduino->isConnected())
    //{
    //    matr = this->volume->GetMatrix();
    //}
    //else
    //{
        matr = this->ren->GetActiveCamera()->GetModelViewTransformMatrix();
    //}
    //vtkMatrix4x4* matr = this->volume->GetMatrix();
    double *or1 = this->rotationMatrixToEulerAngles(matr);

    qDebug() << "Camera Orientation: " << or1[0] << " " << or1[1] << " " << or1[2]; 
   

    this->ren2->RemoveActor(this->fluoroImage->ExportToVTK());
    //this->fluoroImage->UpdateDRR(-or1[0], or1[1], or1[2]);
    ITKFluoroImage *image2 = new ITKFluoroImage("C:\\users\\danie\\documents\\CTChest.mha", -or1[0], or1[1], or1[2]);

    this->ren2->AddActor(this->fluoroImage->ExportToVTK());
    this->openGLWidget2->GetRenderWindow()->Render();

}

/*!
* A QT Slot to collect a single ponts on the phantom
*/
void basic_QtVTK::collectSinglePointPhantom()
{
    if (1) /*!< Make sure the stylus is calibrated first */
    {
        if (myTracker->IsRecording()) /*!< Make sure the traker is tracking */
        {
            if (1 /*tools[0]->IsOutOfView == 0 && tools[1]->IsOutOfView == 0*/)  /*! make sure both items are visible */
            {
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
            else
            {
                statusBar()->showMessage(tr("One item or both is not visible"), 5000);
            }
        }
        else
        {
            statusBar()->showMessage(tr("Tracker not tracking"), 5000);
        }
    }
    else
    {
        statusBar()->showMessage(tr("Stylus NOT calibrated"), 5000);
    }
}


void basic_QtVTK::resetPhantomCollectedPoints()
{

    qDebug() << "reset";
}


void basic_QtVTK::deleteOnePhantomCollectedPoints()
{

    qDebug() << "delete";
    this->ren2->RemoveActor(this->fluoroImage->ExportToVTK());
    this->openGLWidget2->GetRenderWindow()->Render();
}


void basic_QtVTK::performPhantomRegistration()
{
    qDebug() << "register";

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

void basic_QtVTK::setCameraUsingTracker()
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
