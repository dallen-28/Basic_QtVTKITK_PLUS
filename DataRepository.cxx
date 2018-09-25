#include "DataRepository.h"

DataRepository::DataRepository(std::string configFile)
{
    this->dataCollector = vtkSmartPointer<vtkPlusDataCollector>::New();
    this->transformRepository = vtkSmartPointer<vtkPlusTransformRepository>::New();
    this->accelerometerToTracker = vtkSmartPointer<vtkMatrix4x4>::New();
    this->accelerometer2ToTracker = vtkSmartPointer<vtkMatrix4x4>::New();
    this->needleToWebcam = vtkSmartPointer<vtkMatrix4x4>::New();
    this->configRootElement = vtkSmartPointer<vtkXMLDataElement>::New();
    this->accelerometerToCT = vtkSmartPointer<vtkMatrix4x4>::New();
    double m[16] = { 0,1,0,0,
        0,0,1,0,
        1,0,0,0,
        0,0,0,1 };
    this->accelerometerToCT->DeepCopy(m);
    this->ReadConfiguration(configFile);
}


void DataRepository::ReadConfiguration(std::string configFile)
{
    this->configFile = configFile;
    LOG_INFO("Connect using configuration file: ", this->configFile);

    this->configRootElement = vtkSmartPointer<vtkXMLDataElement>::Take(vtkXMLUtilities::ReadElementFromFile(this->configFile.c_str()));

    // Read configuration
    if (PlusXmlUtils::ReadDeviceSetConfigurationFromFile(this->configRootElement, this->configFile.c_str()) == PLUS_FAIL)
    {
        LOG_ERROR("Unable to read configuration from file" << configFile.c_str());
        exit;
    }

    vtkPlusConfig::GetInstance()->SetDeviceSetConfigurationData(this->configRootElement);

    if (dataCollector->ReadConfiguration(this->configRootElement) != PLUS_SUCCESS)
    {
        LOG_ERROR("Configuration incorrect for vtkPlusDataCollector.");
        exit;
    }

    // Set transform names
    accelerometerToTrackerName.SetTransformName("AccelToTracker");
    accelerometer2ToTrackerName.SetTransformName("AccelToTracker2");
    needleToWebcamName.SetTransformName("Marker0ToWebcam");
}
  

DataRepository::~DataRepository()
{

}

std::string DataRepository::GetVolumeFileNameFromId(std::string id)
{
    vtkSmartPointer<vtkXMLDataElement> volumeElement = vtkSmartPointer<vtkXMLDataElement>::New();
 
    volumeElement = this->configRootElement->FindNestedElementWithNameAndAttribute("Volume", "Id", id.c_str());
    std::string name = volumeElement->GetAttribute("Path");
    return name; 
}
vtkMatrix4x4* DataRepository::GetMatrixFromId(std::string id)
{
    vtkSmartPointer<vtkXMLDataElement> volumeElement = vtkSmartPointer<vtkXMLDataElement>::New();
    vtkMatrix4x4 *matrix = vtkMatrix4x4::New();

    volumeElement = this->configRootElement->FindNestedElementWithNameAndAttribute("Volume", "Id", id.c_str());
    std::string matrixString = volumeElement->GetAttribute("Matrix");

    if (matrixString == "0")
    {
        matrix->Identity();
        return matrix;
    }
    
    std::istringstream iss(matrixString);
    std::vector<std::string> results(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());

    double *d = new double[16];
    double a = 0;

    for (int i = 0; i < 16; i++)
    {
        a = atof(results.at(i).c_str());
        d[i] = a;
 
    }

    matrix->DeepCopy(d);
    return matrix;
}

void DataRepository::StartDataCollection()
{
    vtkPlusDevice* accelerometerDevice;
    vtkPlusDevice* accelerometer2Device;
    vtkPlusDevice* webcamDevice;
    vtkPlusDevice* mixerDevice;

    // Get Accelerometer1
    if (dataCollector->GetDevice(accelerometerDevice, "TrackerDevice") != PLUS_SUCCESS)
    {
        LOG_ERROR("Unable to locate the device with ID = \"TrackerDevice\". Check config file.");
        exit;
    }
    this->myAccelerometer = dynamic_cast<vtkPlusWitMotionTracker*>(accelerometerDevice);

    // Get Accelerometer2
    if (dataCollector->GetDevice(accelerometer2Device, "TrackerDevice2") != PLUS_SUCCESS)
    {
        LOG_ERROR("Unable to locate the device with ID = \"TrackerDevice2\". Check config file.");
        exit;
    }

    this->myAccelerometer2 = dynamic_cast<vtkPlusWitMotionTracker*>(accelerometer2Device);

    if (dataCollector->GetDevice(webcamDevice, "WebcamDevice") != PLUS_SUCCESS)
    {
        LOG_ERROR("Unable to locate the device with ID = \"WebcamDevice\". Check config file.");
        exit;
    }

    this->myWebcam = dynamic_cast<vtkPlusOpticalMarkerTracker*>(webcamDevice);

    // Get Mixer
    if (dataCollector->GetDevice(mixerDevice, "TrackedVideoDevice") != PLUS_SUCCESS)
    {
        LOG_ERROR("Unable to locate the device with ID = \"TrackerVideoDevice\". Check config file.");
        exit;
    }

    this->myMixer = dynamic_cast<vtkPlusVirtualMixer*>(mixerDevice);

    // Connect to devices
    if (dataCollector->Connect() != PLUS_SUCCESS)
    {
        std::cout << ".................... [FAILED]" << std::endl;
        LOG_ERROR("Failed to connect to devices!");
        exit(EXIT_FAILURE);
    }
    if (dataCollector->Start() != PLUS_SUCCESS)
    {
        LOG_ERROR("Failed to start Data collection!");
        exit(EXIT_FAILURE);
    }
    if (transformRepository->ReadConfiguration(this->configRootElement) != PLUS_SUCCESS)
    {
        LOG_ERROR("Configuration incorrect for vtkPlusTransformRepository.");
        exit(EXIT_FAILURE);
    }
   
    if (myMixer->GetOutputChannelByName(trackerChannel, "MergedChannel") != PLUS_SUCCESS)
    {
        LOG_ERROR("Unable to locate the channel with Id=\"TrackerChannel\". Check config file.");
        exit(EXIT_FAILURE);
    }
}
void DataRepository::GetTransforms()
{
    // Get trackerFrame from trackerChannel
    this->trackerChannel->GetTrackedFrame(trackerFrame);

    // Set transforms in repository from frame
    this->transformRepository->SetTransforms(trackerFrame);

    bool isValid = false;

    // Get Transforms From Repository
    if (this->transformRepository->GetTransform(accelerometerToTrackerName, accelerometerToTracker, &isValid) != PLUS_SUCCESS)
    {
        LOG_ERROR("Failed to get accelerometerToTracker transform");
        exit;
    }
    if (this->transformRepository->GetTransform(accelerometer2ToTrackerName, accelerometer2ToTracker, &isValid) != PLUS_SUCCESS)
    {
        LOG_ERROR("Failed to get accelerometer2ToTracker transform");
        exit;
    }
    if (this->transformRepository->GetTransform(needleToWebcamName, needleToWebcam, &isValid) != PLUS_SUCCESS)
    {
        LOG_ERROR("Failed to get Marker0ToWebcam transform");
        exit;
    }
}