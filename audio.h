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

        bool isInitialized();
        std::vector<DeviceInfo> getOutputDevices();
        std::vector<DeviceInfo> getInputDevices();
        void printAllDevices();
        void playFile(const char *fileName);
        // Getters.
        ma_engine* getEngine() { return pEngine; }
        ma_sound* getSound() { return pSound; }


    private:
        ma_context context;
        bool initialization;
        ma_engine* pEngine;
        ma_sound* pSound;
        ma_timer* pTimer;

        std::vector<DeviceInfo> getDevices(ma_device_type deviceType);
};

#endif // AUDIO_H
