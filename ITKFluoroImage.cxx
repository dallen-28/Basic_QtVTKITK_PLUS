#include "ITKFluoroImage.h"

/**
* This function will connect the given itk::VTKImageExport filter to
* the given vtkImageImport filter.
*/
template <typename ITK_Exporter, typename VTK_Importer>
void ConnectPipelines(ITK_Exporter exporter, VTK_Importer* importer)
{
    importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());
    importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
    importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
    importer->SetSpacingCallback(exporter->GetSpacingCallback());
    importer->SetOriginCallback(exporter->GetOriginCallback());
    importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());
    importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());
    importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
    importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
    importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
    importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
    importer->SetCallbackUserData(exporter->GetCallbackUserData());
}

/**
* This function will connect the given vtkImageExport filter to
* the given itk::VTKImageImport filter.
*/
template <typename VTK_Exporter, typename ITK_Importer>
void ConnectPipelines(VTK_Exporter* exporter, ITK_Importer importer)
{
    importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());
    importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
    importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
    importer->SetSpacingCallback(exporter->GetSpacingCallback());
    importer->SetOriginCallback(exporter->GetOriginCallback());
    importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());
    importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());
    importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
    importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
    importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
    importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
    importer->SetCallbackUserData(exporter->GetCallbackUserData());
}


// Initialize FluoroImage
ITKFluoroImage::ITKFluoroImage(std::string inputFile, double rx1, double ry1, double rz1)
{
    // Flip Image
    typedef   unsigned char  PixelType;
    typedef itk::Image<PixelType, 2 >   ImageType2;
    typedef itk::FlipImageFilter< ImageType2 >  FilterType2;
    FilterType2::Pointer filter2 = FilterType2::New();
    typedef FilterType2::FlipAxesArrayType     FlipAxesArrayType;
    FlipAxesArrayType flipArray;
    flipArray[0] = 0;
    flipArray[1] = 1;
    filter2->SetFlipAxes(flipArray);
    
    typedef itk::ImageFileReader< ImageType2  >  ReaderType2;
    typedef itk::ImageFileWriter< ImageType2 >  WriterType2;

    ReaderType2::Pointer reader2 = ReaderType2::New();
    WriterType2::Pointer writer2 = WriterType2::New();

    reader2->SetFileName(inputFile);
    reader2->Update();
    filter2->SetInput(reader2->GetOutput());
    writer2->SetFileName("C:\\users\\danie\\documents\\CTChestDRR.mha");
    writer2->SetInput(filter2->GetOutput());
    writer2->Update();


    // Read Image
    this->reader = ReaderType::New();
    this->reader->SetFileName("C:\\users\\danie\\documents\\CTChestDRR.mha");
    //this->reader->SetFileName(inputFile);

    this->ReadImage();

    // Initialize Filter
    this->filter = FilterType::New();
    this->filter->SetInput(image);
    this->filter->SetDefaultPixelValue(0);

    // Initialize Transform
    this->InitializeTransform(rx1, ry1, rz1);

    // Initalize Input Image Parameters
    this->InitializeInputImageParameters();

    // Initialize Interpolator
    this->InitializeInterpolator();
    this->filter->SetInterpolator(interpolator);
    this->filter->SetTransform(transform);

    // Initialize Output Image Parameters
    this->InitializeOutputImageParameters();
    this->filter->SetSize(this->size);
    this->filter->SetOutputSpacing(this->spacing);
    this->filter->SetOutputOrigin(this->origin);    

    this->InitializeITKExport();
    
    //Initialize VTK Image Actor to represent DRR
    this->actor = vtkImageActor::New();
    this->prop = vtkImageProperty::New();
    this->prop->SetColorLevel(51.53);
    this->prop->SetColorWindow(53758.89);
    this->actor->SetProperty(this->prop);

    this->actor->SetInputData(this->vtkImporter->GetOutput());
}

// Read Input Image
void ITKFluoroImage::ReadImage()
{
    try
    {
        reader->Update();
    }
    catch (itk::ExceptionObject & err)
    {
        std::cerr << "ERROR: Invalid Input Image!" << std::endl;
        std::cerr << err << std::endl;
    }
    this->image = reader->GetOutput();
}
void ITKFluoroImage::InitializeITKExport()
{

    this->itkExporter = ExportFilterType::New();
    this->itkExporter->SetInput(filter->GetOutput());
    this->vtkImporter = vtkImageImport::New();

    ConnectPipelines(this->itkExporter, this->vtkImporter);

    this->itkImporter = ImportFilterType::New();
    this->vtkExporter = vtkImageExport::New();
    ConnectPipelines(this->vtkExporter, this->itkImporter);

    // Only going from Itk to Vtk for now
    this->vtkImporter->Update();
}


// An Euler transformation is defined to position the input volume.
// The \code{ResampleImageFilter} uses this transform to position the
// output DRR image for the desired view.
void ITKFluoroImage::InitializeTransform(double rx1, double ry1, double rz1)
{
    this->transform = TransformType::New();
    this->transform->SetComputeZYX(false);
    this->translation[0] = 0.0;
    this->translation[1] = 0.0;
    this->translation[2] = 0.0;
    this->rx = rx1;
    this->ry = ry1;
    this->rz = rz1;
    this->transform->SetTranslation(translation);
    this->transform->SetRotation(this->rx, this->ry, this->rz);
}

// Initialize the parameters for the input image
// based on the header information from the file.
// Used by the Euler transform for orienting the volume
// in 3D space
void ITKFluoroImage::InitializeInputImageParameters()
{
    this->imOrigin = image->GetOrigin();
    this->imRes = image->GetSpacing();
    this->imRegion = image->GetBufferedRegion();
    this->imSize = imRegion.GetSize();
    this->imOrigin[0] += imRes[0] * static_cast<double>(imSize[0]) / 2.0;
    this->imOrigin[1] += imRes[1] * static_cast<double>(imSize[1]) / 2.0;
    this->imOrigin[2] += imRes[2] * static_cast<double>(imSize[2]) / 2.0;

    this->center[0] = imOrigin[0];
    this->center[1] = imOrigin[1];
    this->center[2] = 0.0;
    this->transform->SetCenter(center);
}

// The \code{RayCastInterpolateImageFunction} is instantiated and passed the transform
// object. The \code{RayCastInterpolateImageFunction} uses this
// transform to reposition the x-ray source such that the DRR image
// and x-ray source move as one around the input volume. This coupling
// mimics the rigid geometry of the x-ray gantry.
void ITKFluoroImage::InitializeInterpolator()
{
    this->interpolator = InterpolatorType::New();
    this->threshold = 0.0;
    this->interpolator->SetThreshold(this->threshold);
    this->interpolator->SetTransform(this->transform);
    this->sid = 400.0;
    this->focalpoint[0] = imOrigin[0];
    this->focalpoint[1] = imOrigin[1];
    this->focalpoint[2] = imOrigin[2] - this->sid / 2.;
    this->interpolator->SetFocalPoint(focalpoint);
}
void ITKFluoroImage::InitializeOutputImageParameters()
{
    // Size of output image
    this->size[0] = 501;
    this->size[1] = 501;
    this->size[2] = 1;

    // Spacing
    this->spacing[0] = this->imRes[0];
    this->spacing[1] = this->imRes[1];
    this->spacing[2] = 1.0;

    // Origin
    this->origin[0] = this->imOrigin[0] - this->imRes[0] * ((double)this->size[0] - 1.) / 2.;
    this->origin[1] = this->imOrigin[1] - this->imRes[1] * ((double)this->size[0] - 1.) / 2.;
    this->origin[2] = this->imOrigin[2] + sid / 2.;
}
void ITKFluoroImage::UpdateDRR(double eulerX, double eulerY, double eulerZ)
{
    this->InitializeTransform(eulerX, eulerY, eulerZ);
    this->InitializeInterpolator();

    this->interpolator->SetTransform(this->transform);

    this->filter->SetInterpolator(this->interpolator);
    this->filter->SetTransform(this->transform);
    this->filter->Update();

}
vtkImageActor* ITKFluoroImage::ExportToVTK()
{
    return this->actor;
}