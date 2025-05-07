#include "main.h"
#define MINIAUDIO_IMPLEMENTATION
#include "../libraries/miniaudio.h"

void Application::audio_cb(Fl_Widget *w, void *data)
{
    Application* app = (Application*) data;
    std::cerr << "audio_cb" << std::endl;

    if (app->audio == 0) {
        app->audio = new Audio;
    }
}

/*
 * Constructor
 */
Audio::Audio() : contextInit(false), engineInit(false), soundInit(false) {
    ma_context_config config = ma_context_config_init();

    // Initialize the audio context.
    if (ma_context_init(NULL, 0, &config, &context) == MA_SUCCESS) {
        contextInit = true;
        std::cerr << "Audio context initialized." << std::endl;
    }

    pEngine = new ma_engine;

    if (ma_engine_init(NULL, pEngine) == MA_SUCCESS) {
        engineInit = true;
        std::cerr << "Audio engine initialized." << std::endl;
    }

    pSound = new ma_sound;
}

/*
 * Destructor
 */
Audio::~Audio() {
    if (contextInit) {
        ma_context_uninit(&context);
        std::cerr << "Audio context uninitialized." << std::endl;
    }

    if (soundInit) {
        ma_sound_uninit(pSound);
        delete pSound;
    }

    ma_engine_uninit(pEngine);
    delete pEngine;
}

std::vector<Audio::DeviceInfo> Audio::getDevices(ma_device_type deviceType) {
    // Create a device array.
    std::vector<DeviceInfo> devices;
    
    if (!contextInit) {
        return devices;
    }

    // initialize some MiniAudio variables.
    ma_device_info* pDeviceInfos = nullptr;
    ma_uint32 deviceCount = 0;
    ma_result result;

    // Get playback or capture devices.
    if (deviceType == ma_device_type_playback || deviceType == ma_device_type_duplex) {
        result = ma_context_get_devices(&context, &pDeviceInfos, &deviceCount, nullptr, nullptr);
    }
    else {
        result = ma_context_get_devices(&context, nullptr, nullptr, &pDeviceInfos, &deviceCount);
    }

    if (result != MA_SUCCESS) {
        std::cerr << "Failed to retrieve devices." << std::endl;
        return devices;
    }

    // Store the device data in the array.
    for (ma_uint32 i = 0; i < deviceCount; ++i) {
        DeviceInfo info;
        info.name = pDeviceInfos[i].name;
        info.isDefault = pDeviceInfos[i].isDefault;
        devices.push_back(info);
    }

    return devices;
}

std::vector<Audio::DeviceInfo> Audio::getOutputDevices() {
    return getDevices(ma_device_type_playback);
}

std::vector<Audio::DeviceInfo> Audio::getInputDevices() {
    return getDevices(ma_device_type_capture);
}

void Audio::loadFile(const char *filename)
{
    printf("Load audio file '%s'\n", filename);

    if (soundInit) {
        ma_sound_uninit(pSound);
        soundInit = false;
    }

    ma_result result;
    result = ma_sound_init_from_file(pEngine, filename, 0, NULL, NULL, pSound);

    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio engine.");
        return;
    }

    soundInit = true;
}

void Audio::play()
{
    // Make sure first a file is loaded and not played before playing.
    if (soundInit && !ma_sound_is_playing(pSound)) {
        ma_sound_start(pSound);
        // Run counter function as a thread.
        std::thread t(&Audio::counter, this);
        t.detach();
    }

    return;
}

void Audio::stop()
{
    // Make sure first a file is loaded and played before stopping.
    if (soundInit && ma_sound_is_playing(pSound)) {
        ma_sound_stop(pSound);
    }
}

void Audio::counter()
{
    printf("Start counter thread.\n");
    // Display playback time while sound is playing
    while (ma_sound_is_playing(pSound)) {
        ma_result result = ma_sound_get_cursor_in_pcm_frames(pSound, &framePosition);

        if (result != MA_SUCCESS) {
            printf("Failed to get cursor position.");
            return;
        }

        ma_uint64 sampleRate = ma_engine_get_sample_rate(pEngine);

        double seconds = (double)framePosition / sampleRate;

        printf("\rPlayback Time: %.2f seconds", seconds);
        fflush(stdout);  // Ensure the output updates in place
        struct timespec ts = {.tv_sec = 0, .tv_nsec = 100 * 1000000}; // 100ms
        nanosleep(&ts, NULL);
        //ma_sleep(100); // Sleep for 100 milliseconds
    }

    printf("Stop counter thread.\n");
    return;
}

/*
 * Displays both the input and output audio devices in the console.
 */
void Audio::printAllDevices()
{
    if (!contextInit) {
        std::cerr << "Audio context not initialized." << std::endl;
        return;
    }

    auto outputDevices = getOutputDevices();
    auto inputDevices = getInputDevices();

    std::cout << "=== Available Audio Devices ===" << std::endl;

    std::cout << "\nOutput Devices:" << std::endl;
    for (size_t i = 0; i < outputDevices.size(); ++i) {
        std::cout << "  " << i + 1 << ": " << outputDevices[i].name;
        if (outputDevices[i].isDefault) {
            std::cout << " (default)";
        }
        std::cout << std::endl;
    }

    std::cout << "\nInput Devices:" << std::endl;
    for (size_t i = 0; i < inputDevices.size(); ++i) {
        std::cout << "  " << i + 1 << ": " << inputDevices[i].name;
        if (inputDevices[i].isDefault) {
            std::cout << " (default)";
        }
        std::cout << std::endl;
    }
}

