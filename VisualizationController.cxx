#include "VisualizationController.h"

template< class PReader > vtkPolyData *readAnPolyData(const char *fname) {
    vtkSmartPointer< PReader > reader =
        vtkSmartPointer< PReader >::New();
    reader->SetFileName(fname);
    reader->Update();
    reader->GetOutput()->Register(reader);
    return(vtkPolyData::SafeDownCast(reader->GetOutput()));
}

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

    // Render field of view image in the foreground
    vtkSmartPointer<vtkPNGReader> reader1 = vtkSmartPointer<vtkPNGReader>::New();
    reader1->SetFileName("C:\\users\\danie\\Documents\\FieldOfViewSmall.png");
    imageViewer = vtkSmartPointer<vtkImageViewer2>::New();
    imageViewer->SetInputConnection(reader1->GetOutputPort());
    imageViewer->SetRenderer(this->foregroundRenderer);


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
}

void VisualizationController::StartTracker()
{
    this->ren->InteractiveOff();
    this->ren2->InteractiveOff();
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
    a[1] = a[1] * 180 / 3.14159;

    this->cameraTransform->PostMultiply();
    this->cameraTransform->Identity();

    // Radius of the C-ARM (Must figure out relation between real world CARM radius and VTK Coordinates)
    this->cameraTransform->Translate(0, 0, -zoomFactor*200);

    // Rotate camera according to rotation matrix received from C-ARM accelerometer 
    this->cameraTransform->Concatenate(this->dataRepository->accelerometerToTracker);

    // Transformation between accelerometer and CT coordinate systems
    this->cameraTransform->Concatenate(this->dataRepository->accelerometerToCT);

    this->cameraTransform->Translate(0, 0, -174);
    this->cameraTransform->Translate(0, 0, a[1] - 175);
    this->cameraTransform->Update();

    this->ren->GetActiveCamera()->SetPosition(cameraTransform->GetPosition());
    this->ren->GetActiveCamera()->SetFocalPoint(0, -100, a[1] - 175);
    this->cameraTransform->MultiplyPoint(up, out);
    this->ren->GetActiveCamera()->SetViewUp(out[0], out[1], out[2]);

    this->ren2->GetActiveCamera()->SetPosition(cameraTransform->GetPosition());
    this->ren2->GetActiveCamera()->SetFocalPoint(0, -100, a[1] - 175);
    this->ren2->GetActiveCamera()->SetViewUp(out[0], out[1], out[2]);

    this->ren->ResetCameraClippingRange();
    this->ren2->ResetCameraClippingRange();
    

    delete[] a;
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

    this->ren->ResetCamera();
    this->ren->ResetCameraClippingRange();

}
void VisualizationController::LoadVolume(std::string fileName)
{
    // For volume rendering
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
    this->volume->SetOrientation(0, 0, 180);

    this->ren2->ResetCamera();
    this->ren2->ResetCameraClippingRange();

    //this->foregroundRenderer->ResetCamera();
    //this->foregroundRenderer->ResetCameraClippingRange();

    ren2->SetLayer(0);
    //ren2->InteractiveOff();
    foregroundRenderer->SetLayer(1);

    //int *imageSize = imageViewer->GetSize();

    foregroundRenderer->GetActiveCamera()->SetFocalPoint(2600, 2600, 0);
    foregroundRenderer->GetActiveCamera()->SetPosition(2600, 2600, 7500);
    foregroundRenderer->ResetCameraClippingRange();
    foregroundRenderer->InteractiveOff();

    /*vtkSmartPointer<vtkConeSource> cone = vtkSmartPointer<vtkConeSource>::New();
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    mapper->SetInputConnection(cone->GetOutputPort());
    actor->SetMapper(mapper);
    this->foregroundRenderer->AddActor(actor);
    foregroundRenderer->AddActor(actor);*/

}
void VisualizationController::Zoom(int value)
{
    this->zoomFactor = value;
}
void VisualizationController::ZoomFOV(int value, int lastZoomValue)
{
    int difference = value - lastZoomValue;

    // Positive zoom
    /*if (difference >= 0)
    {
        this->foregroundRenderer->GetActiveCamera()->Zoom(-1/difference);
    }
    else
    {
        this->foregroundRenderer->GetActiveCamera()->Zoom(1/(-difference));
    }*/
    this->foregroundRenderer->GetActiveCamera()->SetPosition(2600, 2600, 7500 - value*100);
    double *a = this->foregroundRenderer->GetActiveCamera()->GetPosition();
    int *b = this->imageViewer->GetPosition();

    this->foregroundRenderer->ResetCameraClippingRange();
    LOG_INFO("Camera position: " << a[0] << " " << a[1] << " " << a[2]);
    LOG_INFO("Field Of View Position: " <<b[0] << " " << b[1] << " " << b[2]);
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