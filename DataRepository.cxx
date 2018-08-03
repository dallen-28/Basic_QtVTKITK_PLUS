#include "DataRepository.h"

DataRepository::DataRepository(std::string configFile)
{
    this->dataCollector = vtkSmartPointer<vtkPlusDataCollector>::New();
    this->transformRepository = vtkSmartPointer<vtkPlusTransformRepository>::New();
    this->accelerometerToTracker = vtkSmartPointer<vtkMatrix4x4>::New();
    this->accelerometer2ToTracker = vtkSmartPointer<vtkMatrix4x4>::New();

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
}

DataRepository::~DataRepository()
{

}

std::string DataRepository::LoadVolume(std::string id)
{
    std::string filePath;
    vtkXMLDataElement* volumeElement = this->configRootElement->FindNestedElementWithNameAndId("Volume", id.c_str);
 

    return volumeElement->GetAttributeValue(id.c_str);
}

void DataRepository::StartDataCollection()
{
    vtkPlusDevice* trackerDevice;
    vtkPlusDevice* trackerDevice2;
    vtkPlusDevice* mixerDevice;

    // Get Accelerometer1
    if (dataCollector->GetDevice(trackerDevice, "TrackerDevice") != PLUS_SUCCESS)
    {
        LOG_ERROR("Unable to locate the device with ID = \"TrackerDevice\". Check config file.");
        exit;
    }
    this->myAccelerometer = dynamic_cast<vtkPlusWitMotionTracker*>(trackerDevice);

    // Get Accelerometer2
    if (dataCollector->GetDevice(trackerDevice2, "TrackerDevice2") != PLUS_SUCCESS)
    {
        LOG_ERROR("Unable to locate the device with ID = \"TrackerDevice2\". Check config file.");
        exit;
    }

    this->myAccelerometer2 = dynamic_cast<vtkPlusWitMotionTracker*>(trackerDevice2);

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
}