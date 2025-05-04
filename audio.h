#ifndef AUDIO_H
#define AUDIO_H

#include <string>
#include <vector>
#include "../libraries/miniaudio.h"

class Audio {
    public:
        // Structure that holds the device data.
        struct DeviceInfo {
            std::string name;
            bool isDefault;
        };

        Audio();
        ~Audio();

        std::vector<DeviceInfo> getOutputDevices();
        std::vector<DeviceInfo> getInputDevices();
        void printAllDevices();
        void loadFile(const char *fileName);
        void play();
        void stop();
        // Getters.
        ma_engine* getEngine() { return pEngine; }
        ma_sound* getSound() { return pSound; }
        bool isContextInit() { return contextInit; }


    private:
        ma_context context;
        bool contextInit;
        bool engineInit;
        bool soundInit;
        ma_engine* pEngine;
        ma_sound* pSound;
        ma_timer* pTimer;

        std::vector<DeviceInfo> getDevices(ma_device_type deviceType);
};

#endif // AUDIO_H
