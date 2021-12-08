#include <iostream>
#include "lib/sensel.h"
#include "lib/sensel_device.h"
#include <array>
#include <atomic>
#include <vector>

using namespace std;

struct Contact
{
    bool state = false;
    float force = 0.0;
    float x = 0.0;
    float y = 0.0;
    float delta_x = 0.0;
    float delta_y = 0.0;
    int fingerID = -1;
};

class Sensel
{
  public:
    Sensel()
    {
        senselGetDeviceList(&list);

        // Get a list of avaialble Sensel devices
        //cout << "Devices available: " << list.num_devices << "\n";
        //cout << "name of device: " << list.devices << "\n";
        //cout << "Device 0: " << list.devices[0].idx << "\n";
        //cout << "Device 1: " << list.devices[1].idx << "\n";

        if (list.num_devices == 0)
        {
            fprintf(stdout, "No Sensel device found.\n");
            return;
        }
        else
        {
            fprintf(stdout, "Sensel device detected.\n");
            senselDetected = true;
        }

        //Open a Sensel device by the id in the SenselDeviceList, handle initialized
        senselOpenDeviceByID(&handle, list.devices[senselIndex].idx);
        //Set the frame content to scan contact data
        senselSetFrameContent(handle, FRAME_CONTENT_CONTACTS_MASK);
        //Allocate a frame of data, must be done before reading frame data
        senselAllocateFrameData(handle, &frame);

        senselSetContactsMask(handle, CONTACT_MASK_DELTAS);

        //Start scanning the Sensel device
        senselStartScanning(handle);
    }

    // ID contructor for when you want more sensels
    Sensel(unsigned int senselID)
    {
        senselIndex = senselID;

        senselGetDeviceList(&list);

        // Get a list of avaialble Sensel devices
        //cout << "Devices available: " << list.num_devices << "\n";
        //cout << "name of device: " << list.devices << "\n";
        //cout << "Device 0: " << list.devices[0].idx << "\n";
        //cout << "Device 1: " << list.devices[1].idx << "\n";

        if (list.num_devices == 0)
        {
            fprintf(stdout, "No Sensel device found.\n");
            return;
        }
        else
        {
            fprintf(stdout, "Sensel device detected.\n");
            senselDetected = true;
        }

        //Open a Sensel device by the id in the SenselDeviceList, handle initialized
        senselOpenDeviceByID(&handle, list.devices[senselIndex].idx);
        //Set the frame content to scan contact data
        senselSetFrameContent(handle, FRAME_CONTENT_CONTACTS_MASK);
        //Allocate a frame of data, must be done before reading frame data
        senselAllocateFrameData(handle, &frame);

        senselSetContactsMask(handle, CONTACT_MASK_DELTAS);

        //Start scanning the Sensel device
        senselStartScanning(handle);
    }

    void shutDown()
    {
        senselClose(handle);
    }

    void check()
    {
        //for (int i = 0; i < fingers.size(); i++)
        //    fingers[i].state.store(false);

        if (senselDetected)
        {
            unsigned int num_frames = 0;
            //Read all available data from the Sensel device
            senselReadSensor(handle);
            //Get number of frames available in the data read from the sensor
            senselGetNumAvailableFrames(handle, &num_frames);

            for (int f = 0; f < num_frames; f++)
            {
                //Read one frame of data
                senselGetFrame(handle, frame);

                contactAmount = frame->n_contacts;
                // Get contact data
                if (contactAmount > 0)
                {

                    for (int c = 0; c < contactAmount; c++)
                    {
                        // mapping
                        unsigned int state = frame->contacts[c].state;
                        float force = frame->contacts[c].total_force / 8192.0f;
                        float x = frame->contacts[c].x_pos / 240.0f;
                        float y = frame->contacts[c].y_pos / 139.0f;
                        float delta_x = frame->contacts[c].delta_x;
                        float delta_y = frame->contacts[c].delta_y;

                        int LED = static_cast<int>(x * 24);
                        int prevLED = static_cast<int>(fingers[c].x * 24);
                        int brightness = static_cast<int>(force * 200);

                        bool setLED = true;

                        for (int pos = 0; pos < LEDPositions.size(); pos++)
                            if (LEDPositions[pos] == LED || LEDPositions[pos] == prevLED)
                                setLED = false;

                        if (state == CONTACT_START && c < fingers.size())
                        {

                            fingers[c].state = true;
                            fingers[c].force = force;
                            fingers[c].x = x;
                            fingers[c].y = y;
                            fingers[c].delta_x = delta_x;
                            fingers[c].delta_y = delta_y;
                            fingers[c].fingerID = ++idx; //frame->contacts[c].id;

                            if (setLED)
                                senselSetLEDBrightness(handle, LED, brightness);

                            //cout << "Finger[" << c << "] ID: " << fingers[c].fingerID << "\n";
                        }
                        else if (state == CONTACT_MOVE && c < fingers.size())
                        {

                            if (setLED && prevLED != LED)
                            {
                                senselSetLEDBrightness(handle, LED, 0);
                                senselSetLEDBrightness(handle, prevLED, 0);
                            }

                            fingers[c].force = force;
                            fingers[c].x = x;
                            fingers[c].y = y;
                            fingers[c].delta_x = delta_x;
                            fingers[c].delta_y = delta_y;

                            if (setLED)
                                senselSetLEDBrightness(handle, LED, brightness);
                        }
                        else if (state == CONTACT_END && c < fingers.size())
                        {

                            if (setLED)
                                senselSetLEDBrightness(handle, LED, 0);

                            fingers[c].state = false;
                            fingers[c].force = force;
                            fingers[c].x = x;
                            fingers[c].y = y;
                            fingers[c].delta_x = delta_x;
                            fingers[c].delta_y = delta_y;
                            fingers[c].fingerID = -1;
                            --idx;
                        }
                    }
                }
            }
        }
    }

    void addLEDBrightness(float position, float brightness)
    {
        if (senselDetected)
        {
            int LED = position * 24;
            int light = brightness * 100;

            senselSetLEDBrightness(handle, LED, light);
            LEDPositions.push_back(LED);
        }
    }

    array<Contact, 20> fingers;

    unsigned int senselIndex = 0;
    int idx = -1;
    bool senselDetected = false;
    unsigned int contactAmount = 0;
    vector<int> LEDPositions;

  private:
    SENSEL_HANDLE handle = NULL;
    //List of all available Sensel devices
    SenselDeviceList list;
    //SenselFrame data that will hold the contacts
    SenselFrameData *frame = NULL;
};
