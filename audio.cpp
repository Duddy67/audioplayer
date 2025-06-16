#include "main.h"
#define MINIAUDIO_IMPLEMENTATION
#include "../libraries/miniaudio.h"

/*
 * Constructor
 */
Audio::Audio(Application* app) : pApplication(app), contextInit(false) {
    ma_context_config config = ma_context_config_init();

    // Initialize the audio context.
    if (ma_context_init(NULL, 0, &config, &context) == MA_SUCCESS) {
        contextInit = true;
        std::cerr << "Audio context initialized." << std::endl;
    }
}

/*
 * Destructor: Uninitializes all of the audio parameters before closing the app.
 */
Audio::~Audio() {
    if (contextInit) {
        ma_context_uninit(&context);
        std::cerr << "Audio context uninitialized." << std::endl;
    }
}

void Audio::uninit()
{
    if (outputDeviceInit) {
        ma_device_uninit(&outputDevice);
    }

    if (decoderInit) {
        ma_decoder_uninit(&decoder);
    }
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

/*
 * Callback used by MiniAudio to feed audio data to the device.
 */
static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    AudioCallbackData* pCallbackData = (AudioCallbackData*)pDevice->pUserData;

    if (pCallbackData == nullptr || pCallbackData->pDecoder == nullptr) {
        return;
    }

    float volume = pCallbackData->pInstance->getVolume();

    if (pCallbackData->pInstance->isPlaying()) {
        // Read audio data from the decoder.
        ma_decoder_read_pcm_frames(pCallbackData->pDecoder, pOutput, frameCount, NULL);
        // Update cursor.
        pCallbackData->pCursor->fetch_add(frameCount, std::memory_order_relaxed);
    }
    // paused or stopped
    else {
        // Fill the audio output buffer with silence (ie: zero) when playback is paused or stopped.
        memset(pOutput, 0, frameCount * pDevice->playback.channels * ma_get_bytes_per_sample(pDevice->playback.format));
        //std::cout << "Not playing (frames): " << pCallbackData->pCursor->load(std::memory_order_relaxed) << std::endl;
        //std::cout << "is_playing" << *pCallbackData->pIsPlaying << std::endl;
        // The cursor has reached the end of the file
        if (pCallbackData->pInstance->isEndOfFile()) {
            pCallbackData->pInstance->autoStop();
        }
    }

    if (volume != 1.0f && pCallbackData->pDecoder->outputFormat == ma_format_f32) {
        printf("Volume: %.2f \n", volume);
        float* samples = (float*)pOutput;
        size_t sampleCount = frameCount * pCallbackData->pDecoder->outputChannels;

        for (size_t i = 0; i < sampleCount; ++i) {
            samples[i] *= volume;
        }
    }
}

void Audio::setOutputDevice(const char *deviceName)
{
    bool found = false;
    auto outputDevices = getOutputDevices();

    for (ma_uint32 i = 0; i < (ma_uint32) outputDevices.size(); ++i) {
        if (strcmp(outputDevices[i].name.c_str(), deviceName) == 0) {
            std::cout << "Found target device: " << outputDevices[i].name << std::endl;
            memcpy(&outputDeviceID, &outputDevices[i].id, sizeof(ma_device_id));
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

    // Check first for a possible file previously loaded.
    if (decoderInit) {
        // Ensure no more callbacks are running.
        ma_device_stop(&outputDevice);  
        uninit();
        decoderInit = false;
    }

    // First store the original data file format.
    if (!storeOriginalFileFormat(filename)) {
        std::cerr << "Failed to load audio file." << std::endl;
        ma_context_uninit(&context);
        return;
    }

    // Then initialize decoder with format conversion.
    ma_decoder_config decoderConfig = ma_decoder_config_init(defaultOutputFormat, defaultOutputChannels, defaultOutputSampleRate);

    if (ma_decoder_init_file(filename, &decoderConfig, &decoder) != MA_SUCCESS) {
        std::cerr << "Failed to initialize decoder with conversion." << std::endl;
        ma_context_uninit(&context);
        return;
    }

    decoderInit = true;

    // Reset the cursor.
    cursor = 0;
    // Set the callbackData parameters used in the MiniAudio callback function.
    callbackData.pDecoder = &decoder;
    callbackData.pCursor = &cursor;
    callbackData.pInstance = this;

    // Configure device parameters.
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.pDeviceID = &outputDeviceID;
    deviceConfig.playback.format = decoder.outputFormat;
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate = decoder.outputSampleRate;
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = &callbackData;

    // Initialize and start device
    if (ma_device_init(&context, &deviceConfig, &outputDevice) != MA_SUCCESS) {
        std::cerr << "Failed to initialize playback device." << std::endl;
        ma_decoder_uninit(&decoder);
        ma_context_uninit(&context);
        return;
    }

    ma_result result = ma_device_start(&outputDevice);
    totalFrames = 0;
    ma_decoder_get_length_in_pcm_frames(&decoder, &totalFrames);
    // Reset the time slider flag.
    pApplication->hasSliderMoved = false;

    if (result != MA_SUCCESS) {
        printf("Failed to get sound length.\n");
    } else {
        // Compute the file length in seconds
        double totalSeconds = (double)totalFrames / decoder.outputSampleRate;
        printf("Sound duration: %.2f seconds\n", totalSeconds);
        // Set the time slider new bounds.
        pApplication->getSlider("time")->bounds(0, totalSeconds);
        // Reset the slider's cursor value (in case of new file loading).
        pApplication->getSlider("time")->value(0);
        // Inform the application about the sound duration.
        pApplication->setDuration(totalSeconds);
        // Reset the time counter.
        Application::time_cb(pApplication->getNullWidget(), pApplication);
    }
}

void Audio::setVolume(float value)
{
    // Make sure first a file is loaded.
    if (decoderInit) {
        volume.store(std::clamp(value, 0.0f, 1.0f), std::memory_order_relaxed);
    }
}

/*
 * Play or stop the sound according to its current state (ie: started or stopped).
 */
void Audio::toggle()
{
    // Make sure first a file is loaded.
    if (decoderInit) {
        // Toggle play/pause.
        is_playing = !is_playing; 

        if (is_playing) {
            // Launch the run function as a thread.
            std::thread t(&Audio::run, this);
            t.detach();
        }
        // The sound is played.
        else {
            //is_playing = false;
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
    while (isPlaying()) {
        // First get the cursor current position.
        ma_uint64 framePosition = cursor.load(std::memory_order_relaxed);
    //std::cout << "framePosition (frames): " << framePosition << std::endl;

        // Check for the time slider.
        if (pApplication->hasSliderMoved) {
            // Convert the time slider position from seconds to PCM frames
            //framePosition = (ma_uint64)(pApplication->getSlider("time")->value() * sampleRate);
            framePosition = (ma_uint64)(pApplication->getSlider("time")->value() * decoder.outputSampleRate);

            // Synchronize the sound cursor position to the time slider's.
            ma_result result = ma_decoder_seek_to_pcm_frame(&decoder, framePosition);

            if (result != MA_SUCCESS) {
                printf("\nFailed to seek to new position.\n");
            }
            else {
                // Synchronize the current position.
                seconds = pApplication->getSlider("time")->value();  
                cursor.store(framePosition, std::memory_order_relaxed);
            }
        }
        // The time slider is running along with the sound stream.
        else {
            //seconds = (double)framePosition / sampleRate;
            seconds = (double)cursor.load(std::memory_order_relaxed) / decoder.outputSampleRate;
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

void Audio::setCursor(double seconds)
{
    if (decoderInit) {
        ma_uint64 framePosition = (ma_uint64)(seconds * decoder.outputSampleRate);
        cursor.store(framePosition, std::memory_order_relaxed);
    }
}

/*
 * Checks whether the sound is playing or not. 
 */
bool Audio::isPlaying()
{
    if (decoderInit) {
        return is_playing.load(std::memory_order_relaxed) && !isEndOfFile();
    }

    return false;
}

/*
 * Function called when the end of the file is reached.
 */
void Audio::autoStop() 
{
    // Check first the end of the file is reached.
    if (is_playing && isEndOfFile()) {
        // Switch the start/stop button to stop.
        Application::toggle_cb(pApplication->getNullWidget(), pApplication);
        // Set the slider's cursor position at the very end of the stroke.
        double totalSeconds = (double)totalFrames / decoder.outputSampleRate;
        pApplication->getSlider("time")->value(totalSeconds);
    }
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
 * Probes the original file format and store its data.
 */
bool Audio::storeOriginalFileFormat(const char* filename)
{
    // Initialize a temporary decoder without any config data (ie: NULL).
    ma_decoder decoderProbe;

    if (ma_decoder_init_file(filename, NULL, &decoderProbe) != MA_SUCCESS) {
        return false;
    }

    // Retrieve data about the original file format.
    std::cout << "File channels: " << decoderProbe.outputChannels << std::endl;
    std::cout << "File sample rate: " << decoderProbe.outputSampleRate << std::endl;
    std::cout << "File format: " << decoderProbe.outputFormat << std::endl;

    // Done probing
    ma_decoder_uninit(&decoderProbe);  

    return true;
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

