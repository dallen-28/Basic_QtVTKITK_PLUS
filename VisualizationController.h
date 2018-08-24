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
#include <vtkPNGReader.h>
#include <vtkImageViewer2.h>
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
#include <vtkConeSource.h>
#include <vtkImageResize.h>
#include <vtkImageActor.h>
#include <vtkProperty.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkCylinderSource.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>
#include <vtkIntArray.h>
#include <vtkDataArray.h>
#include <vtkDataSetAttributes.h>
#include <vtkVariant.h>
#include <vtkMetaImageWriter.h>
#include <vtkDelimitedTextWriter.h>

//stdlib includes
#include <math.h>

// QT includes
#include <qfileinfo.h>
#include <qstring.h>
#include <qobject.h>
#include <qerrormessage.h>
#include <qtimer.h>

// PI
const double M_PI = atan(1)*4.0;

// basic_QtVTK Forward Declaration
class basic_QtVTK;

class VisualizationController
{
public:
    enum
    {
        Fluoro = 1,
        Xray,
        Bone,
        Heart
    };

    VisualizationController();
    ~VisualizationController();

    // 
    void LoadVolumes(std::string);
    void StartTracker();
    void UpdateTracker();
    void UpdateTransferFunction(int);
    void Zoom(int);
    void ZoomFOV(int);
    void EditMeshColour(int, int, int);

    // Main Window needs public access to these
    vtkSmartPointer<vtkRenderer>                    ren;
    vtkSmartPointer<vtkRenderer>                    ren2;
    vtkSmartPointer<vtkRenderer>                    foregroundRenderer;


private:
    double *RotationMatrixToEulerAngles(vtkMatrix4x4* R);
    void SetCamerasUsingWitMotionTracker();
    void LoadMesh(std::string);
    void LoadVolume(std::string);
    void GetSegmentationPoints(std::string, double);
    void SetToFluoro();
    void SetToXray();
    void SetToBone();
    void SetToHeartFluoro();
    void DisplayCoordinateAxes();
    
    // Camera vectors
    double up[4];
    double out[4];
    double zoomFactor;
    double fieldOfViewZoomFactor;

    // Plus config File
    std::string configFile;

    // Qt Objects
    QTimer                                      *trackerTimer;

    // VTK Scene
    vtkSmartPointer<vtkActor>                   surfaceMesh;
    vtkSmartPointer<vtkVolume>                  volume;
    vtkSmartPointer<vtkIntArray>                ids;
    vtkSmartPointer<vtkTransform>               cameraTransform;
    vtkSmartPointer<vtkTransform>               camera2Transform;   

    // Field of View
    vtkSmartPointer<vtkImageViewer2>            imageViewer;
    int                                         fieldOfViewCenter;

    // Transfer Function
    vtkSmartPointer<vtkColorTransferFunction>   colorFun;
    vtkSmartPointer<vtkPiecewiseFunction>       opacityFun;
    vtkSmartPointer<vtkVolumeProperty>          volumeProperty;
    vtkSmartPointer<vtkSmartVolumeMapper>       volumeMapper;


    // Data Repository
    DataRepository                      *dataRepository;
};
