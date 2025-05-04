#include "main.h"

void Application::audio_settings_cb(Fl_Widget *w, void *data)
{
    Application* app = (Application*) data;

    // Build the modal window.
    if (app->audioSettings == 0) {
        app->audioSettings = new AudioSettings(app->x() + MODAL_WND_POS, app->y() + MODAL_WND_POS, 400, 200, "Audio Settings");
        app->audioSettings->getSaveButton()->callback(save_cb, app);
        app->audioSettings->getCancelButton()->callback(cancel_audio_settings_cb, app);
    }

    if (!app->audio->isContextInit()) {
        std::cerr << "Failed to initialize audio system." << std::endl;
    }
    else {
        // Get the required variables.
        auto outputDevices = app->audio->getOutputDevices();
        auto inputDevices = app->audio->getInputDevices();
        AppConfig config = app->loadConfig(CONFIG_FILENAME);

        // Create the very first option (ie: zero index).
        app->audioSettings->output->add("None");
        int defaultSelec = 0, selection = 0;

        for (size_t i = 0; i < outputDevices.size(); ++i) {
            // Create an option for the device.
            app->audioSettings->output->add(outputDevices[i].name.c_str());

            // Check for selection.
            if (outputDevices[i].isDefault) {
                defaultSelec = i + 1;
            }
            else if (config.outputDevice.compare(outputDevices[i].name.c_str()) == 0) {
                selection = i + 1;
            }
        }

        // Set the device selection.
        selection = (selection > 0) ? selection : defaultSelec;
        app->audioSettings->output->value(selection);

        // Reset selection variables.
        defaultSelec = 0, selection = 0;

        // Create the very first option (ie: zero index).
        app->audioSettings->input->add("None");

        for (size_t i = 0; i < inputDevices.size(); ++i) {
            // Create an option for the device.
            app->audioSettings->input->add(inputDevices[i].name.c_str());

            // Check for selection.
            if (inputDevices[i].isDefault) {
                defaultSelec = i + 1;
            }
            else if (config.inputDevice.compare(inputDevices[i].name.c_str()) == 0) {
                selection = i + 1;
            }
        }

        // Set the device selection.
        selection = (selection > 0) ? selection : defaultSelec;
        app->audioSettings->input->value(selection);
    }

    app->audioSettings->show();
}


void Application::save_cb(Fl_Widget* w, void* data)
{
    Application* app = (Application*) data;
    app->label(app->audioSettings->output->text());

    AppConfig config = app->loadConfig(CONFIG_FILENAME);
    config.outputDevice = app->audioSettings->output->text();
    config.inputDevice = app->audioSettings->input->text();
    app->saveConfig(config, CONFIG_FILENAME);

    app->audioSettings->hide();
}

void Application::cancel_audio_settings_cb(Fl_Widget* w, void* data)
{
    Application* app = (Application*) data;
    app->label("Cancel button clicked");
    app->audioSettings->hide();
}
