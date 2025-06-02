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
 * Destructor: Uninitializes all of the audio parameters before closing the app.
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

/*
 * Gathers all the capture and playback device info into an array.
 */
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
        info.id = pDeviceInfos[i].id;
        info.isDefault = pDeviceInfos[i].isDefault;
        devices.push_back(info);
    }

    return devices;
}

/* Device getters. */

std::vector<Audio::DeviceInfo> Audio::getOutputDevices() {
    return getDevices(ma_device_type_playback);
}

std::vector<Audio::DeviceInfo> Audio::getInputDevices() {
    return getDevices(ma_device_type_capture);
}

void Audio::setOutputDevice(const char *deviceName)
{
    ma_device_id targetDeviceID = {0};
    bool found = false;
    auto outputDevices = getOutputDevices();

    for (ma_uint32 i = 0; i < (ma_uint32) outputDevices.size(); ++i) {
        if (strcmp(outputDevices[i].name.c_str(), deviceName) == 0) {
            std::cout << "Found target device: " << outputDevices[i].name << std::endl;
            memcpy(&targetDeviceID, &outputDevices[i].id, sizeof(ma_device_id));
            found = true;
            break;
        }
    }

    if (!found) {
        std::cerr << "Target device not found!" << std::endl;
        ma_context_uninit(&context);
        //return -1;
    }

}

/*
 * Loads a given audio file.
 */
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
        printf("Failed to initialize audio sound.");
        return;
    }

    soundInit = true;

    ma_uint64 totalFrames = 0;
    result = ma_sound_get_length_in_pcm_frames(pSound, &totalFrames);
    // Reset the time slider flag.
    pApplication->hasSliderMoved = false;

    if (result != MA_SUCCESS) {
        printf("Failed to get sound length.\n");
    } else {
        // Compute the sound length in seconds
        ma_uint64 sampleRate = ma_engine_get_sample_rate(pEngine);
        double totalSeconds = (double)totalFrames / sampleRate;
        printf("Sound duration: %.2f seconds\n", totalSeconds);
        // Set the time slider new bounds.
        pApplication->getSlider("time")->bounds(0, totalSeconds);
        // Inform the application about the sound duration.
        pApplication->setDuration(totalSeconds);
    }
}

void Audio::setVolume(float value)
{
    // Make sure first a file is loaded.
    if (soundInit) {
        ma_sound_set_volume (pSound, value);
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
            // Launch the run function as a thread.
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

/*
 * Computes the playback time (in seconds) while the sound is playing.
 * Informs the time slider whenever a second is elapsed. 
 * Note: This function is run in a thread (ie: asynchronously) to not block 
  *      the application while a sound is played. 
 */
void Audio::run()
{
    while (ma_sound_is_playing(pSound)) {
        // First get the cursor current position.
        ma_uint64 framePosition = 0;
        ma_result result = ma_sound_get_cursor_in_pcm_frames(pSound, &framePosition);

        if (result != MA_SUCCESS) {
            printf("Failed to get cursor position.");
            return;
        }

        ma_uint64 sampleRate = ma_engine_get_sample_rate(pEngine);

        // Check for the time slider.
        if (pApplication->hasSliderMoved) {
            // Convert the time slider position in seconds to PCM frames
            framePosition = (ma_uint64)(pApplication->getSlider("time")->value() * sampleRate);

            // Synchronize the sound cursor position to the time slider's.
            result = ma_sound_seek_to_pcm_frame(pSound, framePosition);

            if (result != MA_SUCCESS) {
                printf("\nFailed to seek to new position.\n");
            }
            else {
                // Synchronize the current position.
                seconds = pApplication->getSlider("time")->value();  
            }
        }
        // The time slider is running along with the sound stream.
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
        // Redraw the time slider output (ie: the time counter).
        Fl::check();

        // Delays the loop to avoid flooding the time slider output. 
        struct timespec ts = {.tv_sec = 0, .tv_nsec = 100 * 1000000}; // 100ms
        nanosleep(&ts, NULL);
    }

    return;
}

/*
 * Checks whether the sound is playing or not. 
 */
bool Audio::isPlaying()
{
    if (soundInit) {
        return ma_sound_is_playing(pSound);
    }

    return false;
}

/*
 * Prints the sound playing duration in h m s formated time. 
 */
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
 * Function used for debugging purpose.
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

