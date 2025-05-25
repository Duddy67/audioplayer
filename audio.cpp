#include "main.h"
#define MINIAUDIO_IMPLEMENTATION
#include "../libraries/miniaudio.h"

/*
 * Constructor
 */
Audio::Audio(Application* app) : pApplication(app), contextInit(false), engineInit(false), soundInit(false) {
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

    // Uninitialize the previous loaded sound if any.
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

    ma_uint64 totalFrames = 0;
    result = ma_sound_get_length_in_pcm_frames(pSound, &totalFrames);
    pApplication->hasSliderMoved = false;

    if (result != MA_SUCCESS) {
        printf("Failed to get sound length.\n");
    } else {
        ma_uint64 sampleRate = ma_engine_get_sample_rate(pEngine);
        double totalSeconds = (double)totalFrames / sampleRate;
        printf("Sound duration: %.2f seconds\n", totalSeconds);
        pApplication->getSlider("time")->bounds(0, totalSeconds);
        pApplication->setDuration(totalSeconds);
    }
}

/*
 * Play or stop the sound according to its current state (ie: started or stopped).
 */
void Audio::toggle()
{
    // Make sure first a file is loaded.
    if (soundInit) {
        // The sound is not played.
        if (!ma_sound_is_playing(pSound)) {
            ma_sound_start(pSound);
            // Run the run function as a thread.
            std::thread t(&Audio::run, this);
            t.detach();
        }
        // The sound is played.
        else {
            ma_sound_stop(pSound);
        }
    }

    return;
}

void Audio::run()
{
    // Computes playback time (in seconds) while the sound is playing.
    while (ma_sound_is_playing(pSound)) {
        ma_uint64 framePosition = 0;
        ma_result result = ma_sound_get_cursor_in_pcm_frames(pSound, &framePosition);

        if (result != MA_SUCCESS) {
            printf("Failed to get cursor position.");
            return;
        }

        ma_uint64 sampleRate = ma_engine_get_sample_rate(pEngine);

        if (pApplication->hasSliderMoved) {
            // Convert desired position in seconds to PCM frames
            framePosition = (ma_uint64)(pApplication->getSlider("time")->value() * sampleRate);

            result = ma_sound_seek_to_pcm_frame(pSound, framePosition);

            if (result != MA_SUCCESS) {
                printf("\nFailed to seek to new position.\n");
            }
            else {
                // Synchronize the current position.
                seconds = pApplication->getSlider("time")->value();  
            }
        }
        else {
            seconds = (double)framePosition / sampleRate;
        }

        printf("\rPlayback Time: ");
        printDuration(seconds);
        // Ensure the output updates in place
        fflush(stdout);  
        // Update the time slider value.
        pApplication->getSlider("time")->value(seconds);
        // Don't pass the time slider widget as first argument as it is used to detect
        // which widget type is calling the callback function.
        // As the Audio class is not a widget, any widget type but Fl_Slider can be passed
        // instead. The first argument is not used by the callback function anyway.
        Application::time_cb(pApplication->getNullWidget(), pApplication);
        Fl::check();

        struct timespec ts = {.tv_sec = 0, .tv_nsec = 100 * 1000000}; // 100ms
        nanosleep(&ts, NULL);
    }

    return;
}

bool Audio::isPlaying()
{
    if (soundInit) {
        return ma_sound_is_playing(pSound);
    }

    return false;
}

void Audio::printDuration(double seconds)
{
    int totalSeconds = (int)seconds;
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int secs = totalSeconds % 60;

    printf("%02d:%02d:%02d", hours, minutes, secs);
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

