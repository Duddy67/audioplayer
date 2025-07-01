#ifndef AUDIO_H
#define AUDIO_H

#include <string>
#include <iostream>
#include <filesystem>
#include <atomic>
#include <vector>
#include <thread>
#include <time.h>
#include "../libraries/miniaudio.h"

// Forward declaration.
class Application;

// Structure used to manipulate some Audio class members through the data_callback function.  
struct AudioCallbackData {
    ma_decoder *pDecoder;
    std::atomic<ma_uint64> *pCursor;
    std::atomic<bool> *pIsPlaying;
    // Pointer to the owning class.
    class Audio* pInstance;  
    Application* pApplication;
};

/*
 * The Audio class is a kind of interface allowing the application and the MiniAudio
 * library to communicate with each other.
 */
class Audio {
    private:
        // Structure that holds the device data.
        struct DeviceInfo {
            std::string name;
            ma_device_id id;
            bool isDefault;
        };
        struct OriginalFileFormat {
            std::string fileName;
            ma_uint32 outputChannels;
            ma_uint32 outputSampleRate;
            ma_format outputFormat;
        };
        ma_context context;
        ma_decoder decoder;
        Application* pApplication;
        AudioCallbackData callbackData;
        bool contextInit = false;
        bool decoderInit = false;
        bool outputDeviceInit = false;
        ma_uint64 totalFrames;
        std::atomic<ma_uint64> cursor;
        const ma_format defaultOutputFormat = ma_format_f32;
        const ma_uint32 defaultOutputChannels = 2;
        const ma_uint32 defaultOutputSampleRate = 44100;
        std::atomic<bool> is_playing = false;
        std::atomic<float> volume = 1.0f;
        double seconds;
        ma_device outputDevice;
        ma_device_id outputDeviceID = {0};
        OriginalFileFormat originalFileFormat;
        std::vector<DeviceInfo> getDevices(ma_device_type deviceType);
        std::vector<std::string> supportedFormats = {".wav", ".WAV",".mp3", ".MP3", ".flac", ".FLAC", ".ogg", ".OGG"};
        bool storeOriginalFileFormat(const char* filename);
        void uninit();
        bool initializeOutputDevice();
        void preparePlayer();

    public:
        Audio(Application *app);
        ~Audio();

        std::vector<DeviceInfo> getOutputDevices();
        std::vector<DeviceInfo> getInputDevices();
        void printAllDevices();
        void loadFile(const char *fileName);
        void setVolume(float value);
        void setOutputDevice(const char *deviceName);
        void setCursor(double seconds);
        void toggle();
        void run();
        void printDuration(double seconds);

        // Getters.
        ma_decoder getDecoder() { return decoder; }
        double getSeconds() { return seconds; }
        double getTotalSeconds();
        std::map<std::string, std::string> getOriginalFileFormat();
        std::vector<std::string> getSupportedFormats() { return supportedFormats; }
        float getVolume() { return volume.load(std::memory_order_relaxed); }
        bool isContextInit() { return contextInit; }
        bool isPlaying();
        bool isDecoderInit() { return decoderInit; }
        bool isEndOfFile() { return cursor.load(std::memory_order_relaxed) >= totalFrames; }
        void restart();
};

#endif // AUDIO_H
