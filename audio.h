#ifndef AUDIO_H
#define AUDIO_H

#include <string>
#include <vector>
#include <thread>
#include <time.h>
#include "../libraries/miniaudio.h"

// Forward declaration.
class Application;

class Audio {
    public:
        // Structure that holds the device data.
        struct DeviceInfo {
            std::string name;
            bool isDefault;
        };

        Audio(Application *app);
        ~Audio();

        std::vector<DeviceInfo> getOutputDevices();
        std::vector<DeviceInfo> getInputDevices();
        void printAllDevices();
        void loadFile(const char *fileName);
        void play();
        void stop();
        void counter();
        void printDuration(double seconds);
        // Getters.
        ma_engine* getEngine() { return pEngine; }
        ma_sound* getSound() { return pSound; }
        double getSeconds() { return seconds; }
        bool isContextInit() { return contextInit; }


    private:
        ma_context context;
        Application* pApplication;
        bool contextInit;
        bool engineInit;
        bool soundInit;
        double seconds;
        ma_engine* pEngine;
        ma_sound* pSound;
        ma_timer* pTimer;

        std::vector<DeviceInfo> getDevices(ma_device_type deviceType);
};

#endif // AUDIO_H
