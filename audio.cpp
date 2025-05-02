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
Audio::Audio() : initialization(false) {
    ma_context_config config = ma_context_config_init();

    // Initialize the audio context.
    if (ma_context_init(NULL, 0, &config, &context) == MA_SUCCESS) {
        initialization = true;
        std::cerr << "Audio context initialized." << std::endl;
    }

    pEngine = new ma_engine;
    pSound = new ma_sound;
}

/*
 * Destructor
 */
Audio::~Audio() {
    if (initialization) {
        ma_context_uninit(&context);
        std::cerr << "Audio context uninitialized." << std::endl;
    }

    ma_sound_uninit(pSound);
    ma_engine_uninit(pEngine);
}

bool Audio::isInitialized() {
    return initialization;
}

std::vector<Audio::DeviceInfo> Audio::getDevices(ma_device_type deviceType) {
    // Create a device array.
    std::vector<DeviceInfo> devices;
    
    if (!initialization) {
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

void Audio::playFile(const char *filename)
{
    printf("Open audio file '%s'\n", filename);
    ma_result result;
    result = ma_engine_init(NULL, pEngine);
    result = ma_sound_init_from_file(pEngine, filename, 0, NULL, NULL, pSound);

    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio engine.");
        return;
    }

    ma_sound_start(pSound);
}

/*
 * Displays both the input and output audio devices in the console.
 */
void Audio::printAllDevices()
{
    if (!initialization) {
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

