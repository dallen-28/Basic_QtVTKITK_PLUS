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
    this->cameraTransform = vtkSmartPointer<vtkTransform>::New();
    this->camera2Transform = vtkSmartPointer<vtkTransform>::New();

    // Set camera up direction to +x
    this->up[0] = 1.0;
    this->up[0] = 0.0;
    this->up[0] = 0.0;
    this->up[0] = 0.0;

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

    // 
    this->cameraTransform->Translate(0, 0, -800);
    this->cameraTransform->Concatenate(this->dataRepository->accelerometerToTracker);
    this->cameraTransform->Concatenate(this->dataRepository->accelerometerToCT);
    this->cameraTransform->Translate(0, 0, -174);
    this->cameraTransform->Translate(0, 0, a[1] - 175);
    this->cameraTransform->Update();

    this->ren->GetActiveCamera()->SetPosition(cameraTransform->GetPosition());
    this->ren->GetActiveCamera()->SetFocalPoint(0, -100, a[1] - 175);
    cameraTransform->MultiplyPoint(up, out);
    this->ren->GetActiveCamera()->SetViewUp(out[0], out[1], out[2]);

    this->ren2->GetActiveCamera()->SetPosition(cameraTransform->GetPosition());
    this->ren2->GetActiveCamera()->SetFocalPoint(0, -100, a[1] - 175);
    this->ren2->GetActiveCamera()->SetViewUp(out[0], out[1], out[2]);

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
    this->UpdateTransferFunction(Preset::Fluoro);
    this->volume->SetMapper(this->volumeMapper);
    this->volume->SetProperty(this->volumeProperty);

    this->ren2->AddVolume(this->volume);

    this->ren2->ResetCamera();
    this->ren2->ResetCameraClippingRange();
}

void VisualizationController::UpdateTransferFunction(int preset)
{
    colorFun->RemoveAllPoints();
    opacityFun->RemoveAllPoints();


    if (preset == Preset::Fluoro)
    {
        this->SetToFluoro();
    }
    else if (preset == Preset::Xray)
    {
        this->SetToXray();
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