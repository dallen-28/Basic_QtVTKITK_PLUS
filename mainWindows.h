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

#include <vtkSmartPointer.h>
#include <QMainWindow>
#include "ui_basic_QtVTK_AIGS.h"
#include "PlusConfigure.h"
#include "ITKFluoroImage.h"
#include "PlusTrackedFrame.h"
#include "ArduinoTracker.h"

// C++ includes
#include <tuple>
#include <vector>

// Qt includes
#include <qstring.h>
#include <qprocess.h>

// VTK forward declaration
class vtkActor;
class vtkGenericOpenGLRenderWindow;
class vtkImageCanvasSource2D;
class vtkLogoRepresentation;
class vtkLogoWidget;
class vtkNDITracker;
class vtkPoints;
class vtkRenderer;
class vtkTrackerTool;
class vtkVolume;
class vtkSmartVolumeMapper;
class vtkPiecewiseFunction;
class vtkColorTransferFunction;
class vtkVolumeProperty;
class vtkTransform;
class vtkLandmarkTransform;
class vtkMatrix4x4;

class vtkPlusNDITracker;
class vtkPlusTracker;
class vtkPlusDevice;
class vtkPlusVirtualMixer;
class vtkPlusChannel;
class vtkPlusTransformRepository;
class PlusTrackedFrame;
class PlusTransformName;
class vtkPlusProbeCalibrationAlgo;

class QComboBox;
class QProcess;
class QPlusDeviceSetSelectorWidget;
class QProcess;
class QTimer;
class QWidget;
class vtkPlusDataCollector;
class vtkPlusOpenIGTLinkServer;
class vtkPlusTransformRepository;
class vtkPlusDataCollector;
class vtkPlusDataSource;
class vtkPlusMicrochipTracker;
class vtkPlusWitMotionTracker;
class vtkPlusGenericSerialDevice;

class QTimer;

//! an enum type to specify the type of tracked objects
enum enumTrackedObjectTypes {
    enNeedle = 0,
    enStylus,
    enLaserPointer,
    enLaserPlane,
    enUSProbe,
    enPhantom,
    enCalibrationBlock,
    enOthers,
    enTrackedObject_Max
};

// port number, rom file name
typedef std::tuple< int, QString, enumTrackedObjectTypes > trackedObjectTypes;

/* GUI Application for rendering a volume with VTK and DRR generation with ITK
 and plus for tracking the hardware components */

class basic_QtVTK : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    // constructor/destructor
    basic_QtVTK();
    ~basic_QtVTK() {};

    protected slots:

    /* Connect to devices described in the argument configuration file in response by clicking on the Connect button
    param aConfigFile DeviceSet configuration file path and name */
    void ConnectToDevicesByConfigFile(std::string);

    public slots:

    virtual void slotExit();

    void loadMesh();
    void loadFiducialPts();
    void editMeshColor();
    void editRendererBackgroundColor();
    void screenShot();
    void startTracker(bool);
    void updateTrackerInfo();
    void stylusCalibration(bool);
    void collectSinglePointPhantom();
    void collectDRR();
    void resetPhantomCollectedPoints();
    void deleteOnePhantomCollectedPoints();
    void performPhantomRegistration();
    void setCameraUsingNDITracker();
    void setCameraUsingWitMotionTracker();
    void setCamera2UsingWitMotionTracker();

    void aboutThisProgram();

protected:
    /*! Device set selector widget */
    QPlusDeviceSetSelectorWidget*         m_DeviceSetSelectorWidget;

    /*! Store local config file name */
    std::string                           m_LocalConfigFile;

    /* Store config root element */
    vtkSmartPointer<vtkXMLDataElement>    m_configRootElement;

public:
    // centralized location to initiate all vtk objects
    void createVTKObjects();

    // connect VTK pipelines
    void setupVTKObjects();

    // connect Qt pipelines (slots/signals/actions)
    void setupQTObjects();

    // clean up
    void cleanVTKObjects();


private:
    void createTrackerLogo();
    void createLinearZStylusActor();

private:
    // QT Objects
    QTimer                                              *trackerTimer;

    // VTK Objects
    vtkSmartPointer<vtkActor>                           stylusActor;
    vtkSmartPointer<vtkActor>                           actor;
    vtkSmartPointer<vtkVolume>                          volume;
    vtkSmartPointer<vtkVolume>                          fluoroVolume;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow>       renWin;
    vtkSmartPointer<vtkRenderer>                        ren;
    vtkSmartPointer<vtkRenderer>                        ren2;
    vtkSmartPointer<vtkImageCanvasSource2D>             trackerDrawing;
    vtkSmartPointer<vtkLogoRepresentation>              trackerLogoRepresentation;
    vtkSmartPointer<vtkLogoWidget>                      trackerLogoWidget;
    vtkSmartPointer<vtkTransform>                       cameraTransform;
    vtkSmartPointer<vtkTransform>                       camera2Transform;

    // Tracker Devices
    vtkPlusNDITracker                                   *myNDITracker;
    vtkPlusWitMotionTracker                             *myAccelerometer;
    vtkPlusWitMotionTracker                             *myAccelerometer2;
    vtkPlusVirtualMixer                                 *myMixer;
    vtkPlusDevice                                       *trackerDevice;
    vtkPlusDevice                                       *trackerDevice2;
    vtkPlusDevice                                       *mixerDevice;

    // Tracker Related Objects
    vtkSmartPointer<vtkPlusDataCollector>               dataCollector;
    vtkSmartPointer<vtkPlusTransformRepository>         transformRepository;
    vtkPlusChannel                                      *trackerChannel;
    vtkPlusChannel                                      *trackerChannel2;
    PlusTrackedFrame                                    trackerFrame;
    PlusTrackedFrame                                    trackerFrame2;

    // Transform Names
    PlusTransformName                                   stylusToTrackerName;
    PlusTransformName                                   referenceToTrackerName;
    PlusTransformName                                   stylusTipToReferenceName;
    PlusTransformName                                   stylusTipToTrackerName;
    PlusTransformName                                   accelerometerToTrackerName;
    PlusTransformName                                   accelerometer2ToTrackerName;

    // Transforms
    vtkSmartPointer<vtkMatrix4x4>                       accelerometerToTracker;
    vtkSmartPointer<vtkMatrix4x4>                       accelerometer2ToTracker;
    vtkSmartPointer<vtkMatrix4x4>                       accelerometerToCT;
    double up[4] = { 1,0,0,0 };
    double out[4];
    vtkSmartPointer<vtkMatrix4x4>                       stylusToTracker;
    vtkSmartPointer<vtkMatrix4x4>                       referenceToTracker;       
    vtkSmartPointer<vtkMatrix4x4>                       stylusTipToReference;
    vtkSmartPointer<vtkMatrix4x4>                       stylusTipToTracker;
    vtkSmartPointer<vtkTransform>                       phantomTransform;
    vtkSmartPointer<vtkLandmarkTransform>               phantomRegTransform;

    // Points for fiducial registration
    vtkSmartPointer<vtkPoints>                          fiducialPts;
    vtkSmartPointer<vtkPoints>                          targetPoints;

    int                                                 screenShotFileNumber;
    bool                                                isTrackerInitialized, isStylusCalibrated;
    int                                                 numTrackedTools;
    int                                                 logoWidgetX, logoWidgetY;

};

#endif // of __MAINWIDGET_H__