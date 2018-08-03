#include "VisualizationController.h"

template< class PReader > vtkPolyData *readAnPolyData(const char *fname) {
    vtkSmartPointer< PReader > reader =
        vtkSmartPointer< PReader >::New();
    reader->SetFileName(fname);
    reader->Update();
    reader->GetOutput()->Register(reader);
    return(vtkPolyData::SafeDownCast(reader->GetOutput()));
}

VisualizationController::VisualizationController()
{
    this->surfaceMesh = vtkSmartPointer<vtkActor>::New();
    this->volume = vtkSmartPointer<vtkVolume>::New();
}
void VisualizationController::LoadMesh(std::string id)
{
    this->dataRepository = new DataRepository()

    // Get FileName from repository
    std::string fileName = this->dataRepository->LoadVolume(id);

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
    }
    else
    {
        QErrorMessage *em = new QErrorMessage(this);
        em->showMessage("Input file format not supported");
        return;
    }

    // For surface mesh rendering
    vtkNew<vtkPolyDataMapper> mapper;

    // For volume rendering
    vtkNew<vtkSmartVolumeMapper> volumeMapper;

    mapper->SetInputData(data);
    surfaceMesh->SetMapper(mapper);
    ren->AddActor(surfaceMesh);

    ren->ResetCamera();
    ren->ResetCameraClippingRange();
    this->openGLWidget->GetRenderWindow()->Render();

}
void VisualizationController::LoadVolume(std::string fileName)
{
    vtkAlgorithm *reader = nullptr;
    vtkImageData *input = nullptr;
    vtkMetaImageReader *metaReader = vtkMetaImageReader::New();
    metaReader->SetFileName(fileName.c_str);
    metaReader->Update();
    input = metaReader->GetOutput();
    reader = metaReader;


}

void VisualizationController::UpdateTransferFunction()
{
    vtkNew<vtkVolumeProperty> property;
    vtkNew<vtkColorTransferFunction> colorFun;
    vtkNew<vtkPiecewiseFunction> opacityFun;


    property->SetColor(colorFun);
    property->SetScalarOpacity(opacityFun);
    property->SetInterpolationTypeToLinear();

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

    property->ShadeOff();
}