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
#include <vtkMatrix4x4.h>
#include <vtkColorTransferFunction.h>
#include <vtkColorTransferFunctionItem.h>
#include <vtkPiecewiseFunction.h>
#include <vtkImageData.h>
#include <vtkAlgorithm.h>
#include <vtkTransform.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkCamera.h>

// QT includes
#include <qfileinfo.h>
#include <qstring.h>
#include <qobject.h>
#include <qerrormessage.h>
#include <qtimer.h>

// basic_QtVTK Forward Declaration
class basic_QtVTK;

class VisualizationController
{
public:
    enum
    {
        Fluoro = 1,
        Xray,
        Bone
    };

    VisualizationController(basic_QtVTK*);
    ~VisualizationController();

    void LoadVolumes(std::string);
    void StartTracker();
    void UpdateTracker();
    void UpdateTransferFunction(int);
    void Zoom(int);

    // Main Window needs public access to these
    vtkSmartPointer<vtkRenderer>                    ren;
    vtkSmartPointer<vtkRenderer>                    ren2;


private:
    double *RotationMatrixToEulerAngles(vtkMatrix4x4* R);
    void SetCamerasUsingWitMotionTracker();
    void LoadMesh(std::string);
    void LoadVolume(std::string);
    void SetToFluoro();
    void SetToXray();
    void SetToBone();
    
    // Camera vectors
    double up[4];
    double out[4];
    double zoomFactor;

    // Plus config File
    std::string configFile;

    // Qt Objects
    QTimer                                      *trackerTimer;

    // VTK Scene
    vtkSmartPointer<vtkActor>                   surfaceMesh;
    vtkSmartPointer<vtkVolume>                  volume;
    vtkSmartPointer<vtkTransform>               cameraTransform;
    vtkSmartPointer<vtkTransform>               camera2Transform;   

    // Transfer Function
    vtkSmartPointer<vtkColorTransferFunction>   colorFun;
    vtkSmartPointer<vtkPiecewiseFunction>       opacityFun;
    vtkSmartPointer<vtkVolumeProperty>          volumeProperty;
    vtkSmartPointer<vtkSmartVolumeMapper>       volumeMapper;
    

    // Data Repository
    DataRepository                      *dataRepository;
    basic_QtVTK                         *mainWindow;
};
