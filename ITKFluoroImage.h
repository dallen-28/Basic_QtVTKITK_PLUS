#pragma once

// ITK includes
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkResampleImageFilter.h"
#include "itkCenteredEuler3DTransform.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkRayCastInterpolateImageFunction.h"
#include "itkVTKImageExport.h"
#include "itkVTKImageImport.h"
#include "itkFlipImageFilter.h"

// VTK includes
#include "vtkImageImport.h"
#include "vtkImageExport.h"
#include "vtkImageActor.h"
#include "vtkImageData.h"
#include "vtkImageProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleImage.h" 
#include "vtkSmartVolumeMapper.h"
#include "vtkMetaImageReader.h"
#include "vtkColorTransferFunction.h"
#include "vtkPiecewiseFunction.h"
#include "vtkVolumeProperty.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkConeSource.h"
#include "vtkPolyDataMapper.h"

const unsigned int   Dimension = 3;

class ITKFluoroImage
{

public:
    // constructor/destructor
    ITKFluoroImage(std::string, double, double, double);
    ~ITKFluoroImage() {};

    // Update DRR based on Euler Angle's
    void UpdateDRR(double, double, double);
    void InitializeTransform(double, double, double);
    void InitializeInputImageParameters();
    void InitializeOutputImageParameters();
    void InitializeInterpolator();
    void InitializeITKExport();
    void ReadImage();
    vtkImageActor* ExportToVTK();

private:

    typedef short InputPixelType;
    typedef unsigned char OutputPixelType;
    typedef itk::Image< InputPixelType, Dimension >   InputImageType;
    typedef itk::Image< OutputPixelType, Dimension >   OutputImageType;
    typedef itk::CenteredEuler3DTransform< double >  TransformType;
    typedef itk::ImageFileReader< InputImageType >  ReaderType;
    typedef itk::ResampleImageFilter<InputImageType, InputImageType > FilterType;
    typedef InputImageType::RegionType     InputImageRegionType;
    typedef InputImageRegionType::SizeType InputImageSizeType;
    typedef itk::RayCastInterpolateImageFunction<InputImageType, double> InterpolatorType;
    typedef itk::RescaleIntensityImageFilter<InputImageType, OutputImageType > RescaleFilterType;
    typedef itk::ImageFileWriter< OutputImageType >  WriterType;
    typedef itk::VTKImageExport< InputImageType > ExportFilterType;
    typedef itk::VTKImageImport< InputImageType > ImportFilterType;

    // ITK Image Reader
    ReaderType::Pointer reader;

    // Input Image
    InputImageType::Pointer image;

    // DRR Filter
    FilterType::Pointer filter;

    // Euler Transform
    TransformType::Pointer transform;

    // Translation parameter
    TransformType::OutputVectorType translation;

    // Rotation Parameters (Euler Angles - Radians)
    double rx;
    double ry;
    double rz;

    // Image Origin
    InputImageType::PointType   imOrigin;

    // Image Resolution
    InputImageType::SpacingType imRes;

    // Image Region
    InputImageRegionType imRegion;

    // Input Image Size
    InputImageSizeType   imSize;

    // Image center (center of rotation)
    TransformType::InputPointType center;

    // RayCast Interpolator
    InterpolatorType::Pointer interpolator;

    // The Image FocalPoint
    InterpolatorType::InputPointType focalpoint;
    
    // Output Image Size
    InputImageType::SizeType size;

    // Output Image Spacing
    InputImageType::SpacingType spacing;

    // Output Image Origin
    double origin[Dimension];

    // Output Image Rescaler
    RescaleFilterType::Pointer rescaler;

    // Output Writer
    WriterType::Pointer writer;

    // Threshold
    double threshold;

    // Source to Image distance
    double sid;

    // Itk Exporter
    ExportFilterType::Pointer itkExporter;

    // Itk Importer
    ImportFilterType::Pointer itkImporter;

    // Vtk Importer
    vtkImageImport* vtkImporter;

    // Vtk Exporter
    vtkImageExport* vtkExporter;

    // For Exporting to VTK
    vtkImageActor* actor;

    // To set the window level
    vtkImageProperty *prop;
};
