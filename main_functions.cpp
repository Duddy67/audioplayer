#include "main.h"
#include <cstdlib>


void Application::saveConfig(const AppConfig& config, const std::string& filename)
{
    json j;
    j["outputDevice"] = config.outputDevice;
    j["inputDevice"] = config.inputDevice;
    j["volume"] = config.volume;

    std::ofstream file(filename);
    file << j.dump(4); // Pretty print with 4 spaces indentation
    std::cout << "Configuration saved to " << filename << std::endl;

}

Application::AppConfig Application::loadConfig(const std::string& filename)
{
    AppConfig config;
    std::ifstream file(filename);

    // If no config file is found, create it.
    if (!file.is_open()) {
        config.outputDevice = "none";
        config.inputDevice = "none";
        config.volume = "0";
        this->saveConfig(config, filename);
        return config;
    }

    try {
        json j;
        file >> j;

        config.outputDevice = j.value("outputDevice", "none");
        config.inputDevice = j.value("inputDevice", "none");
        config.volume = j.value("volume", "0");
    }
    catch (const json::exception& e) {
        setMessage("Error parsing config: " + std::string(e.what()));
    }

    return config;
}

void Application::setMessage(std::string message)
{
    this->message = message;
}

Fl_Slider* Application::getSlider(const char *type)
{
    if (strcmp(type, "time") == 0) {
        return time;
    }

    if (strcmp(type, "volume") == 0) {
        return volume;
    }

    return nullptr;
}

void Application::setDuration(double seconds)
{
    std::map time = getTimeFromSeconds(seconds);

    // Format slider value as string
    char buffer[60];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", time["hours"], time["minutes"], time["seconds"]);

    // Set new value in output box
    duration->value(buffer);
}

std::map<std::string, int> Application::getTimeFromSeconds(double seconds) 
{
    int totalSeconds = (int)seconds;
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int secs = totalSeconds % 60;

    std::map<std::string, int> time{{"hours", hours}, {"minutes", minutes}, {"seconds", secs}};

    return time;
}

void Application::saveVolume()
{
    float vol = (float)volume->value();
    std::string volume = std::to_string(vol);
    AppConfig config = loadConfig(CONFIG_FILENAME);
    config.volume = volume;
    saveConfig(config, CONFIG_FILENAME);
}

