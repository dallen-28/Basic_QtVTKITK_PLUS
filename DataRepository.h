#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <vtkPlusWitMotionTracker.h>
#include <vtkPlusVirtualMixer.h>
#include <vtkPlusNDITracker.h>
#include <vtkPlusDevice.h>
#include <vtkPlusTransformRepository.h>
#include <vtkPlusDataCollector.h>


class DataRepository
{

public:

    // The only variables we need to expose to the visualization controller 
    // are the transforms 
    vtkSmartPointer<vtkMatrix4x4>                       accelerometerToTracker;
    vtkSmartPointer<vtkMatrix4x4>                       accelerometer2ToTracker;

    DataRepository(std::string);
    ~DataRepository();

    // Return the full path name of the volume specified by ID
    std::string LoadVolume(std::string);
    void StartDataCollection();
    void GetTransforms();


private:


    /* Config File Name */
    std::string                                         configFile;

    /* Config root element */
    vtkSmartPointer<vtkXMLDataElement>                  configRootElement;

    /* Tracker Devices */

    // Track C-ARM orientation
    vtkPlusWitMotionTracker                             *myAccelerometer;

    // Track table Z Translation
    vtkPlusWitMotionTracker                             *myAccelerometer2;

    // MixerDevice to combine tracker frames
    // from multiple devices into one channel;
    vtkPlusVirtualMixer                                 *myMixer;

    /* Tracker Related Objects */
    vtkSmartPointer<vtkPlusDataCollector>               dataCollector;
    vtkSmartPointer<vtkPlusTransformRepository>         transformRepository;
    vtkPlusChannel                                      *trackerChannel;
    PlusTrackedFrame                                    trackerFrame;

    // Transform Names
    PlusTransformName                                   accelerometerToTrackerName;
    PlusTransformName                                   accelerometer2ToTrackerName;
       
};