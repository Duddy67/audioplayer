#ifndef AUDIO_H
#define AUDIO_H

#include <string>
#include <iostream>
#include <atomic>
#include <vector>
#include <thread>
#include <time.h>
#include "../libraries/miniaudio.h"

// Forward declaration.
class Application;

/*
 * The Audio class is a kind of interface allowing the application and the MiniAudio
 * library to communicate with each other.
 */
class Audio {
    public:
        // Structure that holds the device data.
        struct DeviceInfo {
            std::string name;
            ma_device_id id;
            bool isDefault;
        };

        Audio(Application *app);
        ~Audio();

        std::vector<DeviceInfo> getOutputDevices();
        std::vector<DeviceInfo> getInputDevices();
        std::atomic<ma_uint64> cursor;
        void printAllDevices();
        void loadFile(const char *fileName);
        void setVolume(float value);
        void setOutputDevice(const char *deviceName);
        void toggle();
        void run();
        void printDuration(double seconds);
        //void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

        // Getters.
        ma_engine* getEngine() { return pEngine; }
        ma_sound* getSound() { return pSound; }
        double getSeconds() { return seconds; }
        bool isContextInit() { return contextInit; }
        bool isPlaying();

    private:
        ma_context context;
        ma_decoder decoder;
        Application* pApplication;
        bool contextInit;
        bool engineInit;
        bool soundInit;
        bool decoderInit;
        std::atomic<bool> is_playing = false;
        double seconds;
        ma_engine* pEngine;
        ma_sound* pSound;
        ma_timer* pTimer;
        ma_device outputDevice;
        ma_device_id outputDeviceID = {0};
        std::vector<DeviceInfo> getDevices(ma_device_type deviceType);

};

#endif // AUDIO_H
