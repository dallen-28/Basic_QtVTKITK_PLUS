#pragma once
// Local includes
#include "DataRepository.h"

// VTK includes
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkActor.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkPLYReader.h>
#include <vtkOBJReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkMetaImageReader.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// QT includes
#include <qfileinfo.h>
#include <qstring.h>
#include <qobject.h>
#include <qerrormessage.h>
#include <qtimer.h>

class VisualizationController
{
    VisualizationController(std::string);
    ~VisualizationController();

    void LoadMesh(std::string);
    void LoadVolume(std::string);
    void StartTracker();

    void UpdateTransferFunction();


private:

    double *RotationMatrixToEulerAngles(vtkMatrix4x4* R);
    void UpdateTracker();
    void SetCameraUsingWitMotionTracker();
    void SetCamera2UsingWitMotionTracker();

    // Plus config File
    std::string configFile;

    // Qt Objects
    QTimer                                      *trackerTimer;

    // VTK Scene
    vtkSmartPointer<vtkRenderer>                ren;
    vtkSmartPointer<vtkRenderer>                ren2;
    vtkSmartPointer<vtkActor>                   surfaceMesh;
    vtkSmartPointer<vtkVolume>                  volume;
    vtkSmartPointer<vtkTransform>               cameraTransform;
    vtkSmartPointer<vtkTransform>               camera2Transform;          


    // Data Repository
    DataRepository                      *dataRepository;
};
