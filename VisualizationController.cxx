#include "VisualizationController.h"

template< class PReader > vtkPolyData *readAnPolyData(const char *fname) {
    vtkSmartPointer< PReader > reader =
        vtkSmartPointer< PReader >::New();
    reader->SetFileName(fname);
    reader->Update();
    reader->GetOutput()->Register(reader);
    return(vtkPolyData::SafeDownCast(reader->GetOutput()));
}

// Initialize the VTK scene objects
VisualizationController::VisualizationController(basic_QtVTK* mainWindow)
{
    // We need to keep a reference to the mainWindow
    this->mainWindow = mainWindow;

    this->surfaceMesh = vtkSmartPointer<vtkActor>::New();
    this->volume = vtkSmartPointer<vtkVolume>::New();
    this->ren = vtkSmartPointer<vtkRenderer>::New();
    this->ren2 = vtkSmartPointer<vtkRenderer>::New();
    this->foregroundRenderer = vtkSmartPointer<vtkRenderer>::New();
    this->cameraTransform = vtkSmartPointer<vtkTransform>::New();
    this->camera2Transform = vtkSmartPointer<vtkTransform>::New();
    
    // Set camera up direction to +x
    this->up[0] = 1.0;
    this->up[1] = 0.0;
    this->up[2] = 0.0;
    this->up[3] = 0.0;

    // Render field of view image in the foreground (Get data from config file)
    vtkSmartPointer<vtkPNGReader> reader1 = vtkSmartPointer<vtkPNGReader>::New();
    reader1->SetFileName("C:\\users\\danie\\Documents\\FieldOfView.png");
    imageViewer = vtkSmartPointer<vtkImageViewer2>::New();
    imageViewer->SetInputConnection(reader1->GetOutputPort());
    imageViewer->SetRenderer(this->foregroundRenderer);
    fieldOfViewCenter = 2500;

    this->colorFun = vtkSmartPointer<vtkColorTransferFunction>::New();
    this->opacityFun = vtkSmartPointer<vtkPiecewiseFunction>::New();
    this->volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
    this->volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();

    this->volumeProperty->SetColor(colorFun);
    this->volumeProperty->SetScalarOpacity(opacityFun);
    this->volumeProperty->SetInterpolationTypeToLinear();

}
void VisualizationController::LoadVolumes(std::string configFile)
{
    this->configFile = configFile;

    this->dataRepository = new DataRepository(this->configFile);
    this->LoadMesh("CTVolume");
    this->LoadMesh("SurfaceMesh");
    this->DisplayCoordinateAxes();
}

void VisualizationController::StartTracker()
{
    this->ren->InteractiveOff();
    this->ren2->InteractiveOff();
    this->foregroundRenderer->InteractiveOff();
    this->dataRepository->StartDataCollection();
}

void VisualizationController::UpdateTracker()
{
    this->dataRepository->GetTransforms();
    this->SetCamerasUsingWitMotionTracker();
}

double * VisualizationController::RotationMatrixToEulerAngles(vtkMatrix4x4 * R)
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

void VisualizationController::SetCamerasUsingWitMotionTracker()
{
    vtkNew<vtkTransform> tran;
    tran->Identity();
    tran->PostMultiply();

    // Extract Y angle from second acceleromter
    double *a = this->RotationMatrixToEulerAngles(this->dataRepository->accelerometer2ToTracker);
    a[1] = a[1] * 180 / M_PI;

    this->cameraTransform->Identity();
    this->cameraTransform->PostMultiply();
    this->camera2Transform->Identity();
    this->camera2Transform->PostMultiply();
    


    // Arbitrary distance to view Full 3D surface Mesh
    this->cameraTransform->Translate(0, 0, -800);
   
    // Radius of the C-ARM (Must figure out relation between real world CARM radius and VTK Coordinates)
    this->camera2Transform->Translate(0, 0, -800 + zoomFactor);

    // Rotate camera according to rotation matrix received from C-ARM accelerometer 
    this->cameraTransform->Concatenate(this->dataRepository->accelerometerToTracker);
    this->camera2Transform->Concatenate(this->dataRepository->accelerometerToTracker);

    // Transformation between accelerometer and CT coordinate systems
    this->cameraTransform->Concatenate(this->dataRepository->accelerometerToCT);
    this->camera2Transform->Concatenate(this->dataRepository->accelerometerToCT);

    this->cameraTransform->Translate(0, 0, a[1] - 349);
    this->cameraTransform->Update();

    //this->camera2Transform->Translate(0, 0, a[1] - 349);
    this->camera2Transform->Update();

    this->ren->GetActiveCamera()->SetPosition(cameraTransform->GetPosition());
    this->ren->GetActiveCamera()->SetFocalPoint(0, -100, a[1] - 175);
    this->cameraTransform->MultiplyPoint(up, out);
    this->ren->GetActiveCamera()->SetViewUp(out[0], out[1], out[2]);

    this->ren2->GetActiveCamera()->SetPosition(camera2Transform->GetPosition());
    this->ren2->GetActiveCamera()->SetFocalPoint(0, 0, 0);
    this->ren2->GetActiveCamera()->SetViewUp(out[0], out[1], out[2]);

    this->ren->ResetCameraClippingRange();
    this->ren2->ResetCameraClippingRange();
    

    delete[] a;
}
void VisualizationController::EditMeshColour(int r, int g, int b)
{
    this->surfaceMesh->GetProperty()->SetColor((double)r / 255.0, (double)g / 255.0, (double)b / 255.0);
}
void VisualizationController::LoadMesh(std::string id)
{
    

    // Get FileName from repository
    std::string fileName = this->dataRepository->GetVolumeFileNameFromId(id);


    vtkPolyData *data;

    QFileInfo info(QString::fromStdString(fileName));

    // parse the file extension and use the appropriate reader
    if (info.suffix() == QString("vtk"))
    {
        data = readAnPolyData<vtkPolyDataReader >(fileName.c_str());
    }
    else if (info.suffix() == QString("stl") || info.suffix() == QString("stlb"))
    {
        data = readAnPolyData<vtkSTLReader>(fileName.c_str());
    }
    else if (info.suffix() == QString("ply"))
    {
        data = readAnPolyData<vtkPLYReader>(fileName.c_str());
    }
    else if (info.suffix() == QString("obj"))
    {
        data = readAnPolyData<vtkOBJReader>(fileName.c_str());
    }
    else if (info.suffix() == QString("vtp"))
    {
        data = readAnPolyData<vtkXMLPolyDataReader>(fileName.c_str());
    }
    else if (info.suffix() == QString("mha"))
    {
        this->LoadVolume(fileName);
        return;
    }
    else
    {
        LOG_ERROR("Input file format not supported.");
        return;
    }

    // For surface mesh rendering
    vtkNew<vtkPolyDataMapper> mapper;

    mapper->SetInputData(data);
    this->surfaceMesh->SetMapper(mapper);
    this->ren->AddActor(this->surfaceMesh);

    this->surfaceMesh->SetPosition(0, 100, 0);

    this->ren->ResetCamera();
    this->ren->ResetCameraClippingRange();

}
void VisualizationController::LoadVolume(std::string fileName)
{
    vtkNew<vtkSmartVolumeMapper> volumeMapper;

    vtkAlgorithm *reader = nullptr;
    vtkImageData *input = nullptr;
    vtkSmartPointer<vtkMetaImageReader> metaReader = vtkSmartPointer<vtkMetaImageReader>::New();
    metaReader->SetFileName(fileName.c_str());
    metaReader->Update();
    input = metaReader->GetOutput();
    reader = metaReader;

    this->volumeMapper->SetInputConnection(reader->GetOutputPort());
    this->UpdateTransferFunction(VisualizationController::Fluoro);
    this->volume->SetMapper(this->volumeMapper);
    this->volume->SetProperty(this->volumeProperty);

    this->ren2->AddVolume(this->volume);

    // Spine is facing down by default; set to face up
    //this->volume->SetOrientation(0, 0, 180);
    this->volume->SetPosition(0, -100, 174);

    this->ren2->ResetCamera();
    this->ren2->ResetCameraClippingRange();

    // Layer the renderers accordingly
    ren2->SetLayer(0);
    foregroundRenderer->SetLayer(1);

    // Center the foreground renderer camera at an initial distance from the FieldOfView 
    foregroundRenderer->GetActiveCamera()->SetFocalPoint(fieldOfViewCenter, fieldOfViewCenter, 0);
    foregroundRenderer->GetActiveCamera()->SetPosition(fieldOfViewCenter, fieldOfViewCenter, 7500);
    foregroundRenderer->ResetCameraClippingRange();
    foregroundRenderer->InteractiveOff();


}
void VisualizationController::Zoom(int value)
{
    this->zoomFactor = 10*value;
}
void VisualizationController::ZoomFOV(int value)
{
    // Move the camera closer to the Field of View in order to increase the apparent size
    this->foregroundRenderer->GetActiveCamera()->SetPosition(fieldOfViewCenter, fieldOfViewCenter, 7500 - value*100);
    this->foregroundRenderer->ResetCameraClippingRange();
}

void VisualizationController::UpdateTransferFunction(int preset)
{
    colorFun->RemoveAllPoints();
    opacityFun->RemoveAllPoints();


    if (preset == VisualizationController::Fluoro)
    {
        this->SetToFluoro();
    }
    else if (preset == VisualizationController::Xray)
    {
        this->SetToXray();
    }
    else if (preset == VisualizationController::Bone)
    {
        this->SetToBone();
    }


}
void VisualizationController::SetToFluoro()
{
    colorFun->AddRGBPoint(-3010, 0, 0, 0, 0, 0);
    colorFun->AddRGBPoint(-1592.78540039063, 0.250980392156863, 0.250980392156863, 0.250980392156863);
    colorFun->AddRGBPoint(-124.556709289551, 0.501960784313725, 0.501960784313725, 0.501960784313725);
    colorFun->AddRGBPoint(998.206420898438, 0.752941176470588, 0.752941176470588, 0.752941176470588);
    colorFun->AddRGBPoint(2466.43505859375, 1, 1, 1);
    colorFun->AddRGBPoint(3071, 1, 1, 1);

    opacityFun->AddPoint(-3024, 0);
    opacityFun->AddPoint(-3024, 0);
    opacityFun->AddPoint(-1284.333984375, 0);
    opacityFun->AddPoint(171.556655883789, 0);
    opacityFun->AddPoint(702.093078613281, 0.0952381044626236);
    opacityFun->AddPoint(3071, 0);
    opacityFun->AddPoint(3071, 0);

    volumeMapper->SetBlendModeToComposite();
    volumeProperty->ShadeOff();

    this->ren2->SetBackground(.9, .9, .9);
}
void VisualizationController::SetToXray()
{
    colorFun->AddRGBPoint(-3010, 0, 0, 0, 0, 0);
    colorFun->AddRGBPoint(-1592.78540039063, 0.250980392156863, 0.250980392156863, 0.250980392156863);
    colorFun->AddRGBPoint(-124.556709289551, 0.501960784313725, 0.501960784313725, 0.501960784313725);
    colorFun->AddRGBPoint(998.206420898438, 0.752941176470588, 0.752941176470588, 0.752941176470588);
    colorFun->AddRGBPoint(2466.43505859375, 1, 1, 1);
    colorFun->AddRGBPoint(3071, 1, 1, 1);



    opacityFun->AddPoint(-3024, 0);
    opacityFun->AddPoint(-3024, 0);
    opacityFun->AddPoint(-1284.333984375, 0);
    opacityFun->AddPoint(171.556655883789, 0);
    opacityFun->AddPoint(702.093078613281, 0.0952381044626236);
    opacityFun->AddPoint(3071, 0);
    opacityFun->AddPoint(3071, 0);

    volumeMapper->SetBlendModeToComposite();
    volumeProperty->ShadeOff();
    this->ren2->SetBackground(0, 0, 0);
}
void VisualizationController::SetToBone()
{
    colorFun->AddRGBPoint(-16, 0.73, 0.25, 0.30, 0.49, .61);
    colorFun->AddRGBPoint(641, .90, .82, .56, .5, 0.0);
    colorFun->AddRGBPoint(3071, 1, 1, 1, .5, 0.0);

    opacityFun->AddPoint(-3024, 0, 0.5, 0.0);
    opacityFun->AddPoint(-16, 0, .49, .61);
    opacityFun->AddPoint(641, .72, .5, 0.0);
    opacityFun->AddPoint(3071, .71, 0.5, 0.0);

    volumeMapper->SetBlendModeToComposite();
    volumeProperty->ShadeOn();
    volumeProperty->SetAmbient(0.1);
    volumeProperty->SetDiffuse(0.9);
    volumeProperty->SetSpecular(0.2);
    volumeProperty->SetSpecularPower(10.0);
    volumeProperty->SetScalarOpacityUnitDistance(0.8919);

    this->ren2->SetBackground(0, 0, 0);
}
void VisualizationController::DisplayCoordinateAxes()
{
    vtkNew<vtkCylinderSource> cylinderx;
    vtkNew<vtkCylinderSource> cylindery;
    vtkNew<vtkCylinderSource> cylinderz;


    cylinderx->SetHeight(500);
    cylindery->SetHeight(500);
    cylinderz->SetHeight(500);

    vtkNew<vtkPolyDataMapper>  mapper2;
    vtkNew<vtkPolyDataMapper>  mapper3;
    vtkNew<vtkPolyDataMapper>  mapper4;
    vtkNew<vtkActor> cylinderActorx;
    vtkNew<vtkActor> cylinderActory;
    vtkNew<vtkActor> cylinderActorz;

    mapper2->SetInputConnection(cylinderx->GetOutputPort());
    mapper3->SetInputConnection(cylindery->GetOutputPort());
    mapper4->SetInputConnection(cylinderz->GetOutputPort());

    cylinderActorx->SetMapper(mapper2);
    cylinderActory->SetMapper(mapper3);
    cylinderActorz->SetMapper(mapper4);

    //this->ren->AddActor(cylinderActorx);
    //this->ren->AddActor(cylinderActory);
    //this->ren->AddActor(cylinderActorz);


    this->ren2->AddActor(cylinderActorx);
    this->ren2->AddActor(cylinderActory);
    this->ren2->AddActor(cylinderActorz);

    // Red for X
    cylinderActorx->GetProperty()->SetColor(1, 0, 0);
    cylinderActory->GetProperty()->SetColor(0, 1, 0);
    cylinderActorz->GetProperty()->SetColor(0, 0, 1);


    cylinderActorx->SetPosition(250, 0, 0);
    cylinderActory->SetPosition(0, 0, 250);
    cylinderActorz->SetPosition(0, 250, 0);
    cylinderActorx->SetOrientation(0, 0, 90);
    cylinderActory->SetOrientation(90, 0, 0);
    cylinderActorz->SetOrientation(0, 0, 0);

}