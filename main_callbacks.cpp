#include "main.h"


void Application::toggle_cb(Fl_Widget* w, void* data)
{
    Application* app = (Application*) data;
    app->audio->toggle();

    // Display the next action icon (play or stop) according to the sound current state.
    if (app->audio->isPlaying()) {
        // Stop icon.
        app->getButton()->label("@||");
    }
    // The sound is not played
    else {
        // Play icon.
        app->getButton()->label("@>");
    }

    Fl::check();
}

/*
 *
 */
void Application::time_cb(Fl_Widget *w, void *data)
{
    Application* app = (Application*) data;
    // By default the slider goes forward while the sound is playing.
    app->hasSliderMoved = false;

    // Check if the slider has been moved by the user.
    // Note: The time slider widget is passed as w parameter whenever the slider is moved.
    if (dynamic_cast<Fl_Slider*>(w)) {
        // Set the flag to true to tell the audio class to synchronize the sound cursor
        // with the time slider's new value (in seconds).
        app->hasSliderMoved = true;
    }

    // Get the seconds elapsed (from the time value slider).
    double seconds = app->getSlider("time")->value();
    // Convert the seconds in hours minutes seconds time format.
    std::map time = app->getTimeFromSeconds(seconds);

    // Format slider value as string.
    char buffer[60];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", time["hours"], time["minutes"], time["seconds"]);

    // Set new value in output box.
    app->timeOutput->value(buffer);
}

void Application::volume_cb(Fl_Widget *w, void *data)
{
    Application* app = (Application*) data;
    // Set the sound volume from the current volume slider value.
    app->audio->setVolume((float)app->volume->value());

    // Convert the volume value to percentage then update the volume output box.
    int percentage = (int)((float)app->volume->value() * 100.0f);

    // Format slider value as string.
    char buffer[60];
    snprintf(buffer, sizeof(buffer), "%d%%", percentage);

    // Set new value in output box.
    app->volumeOutput->value(buffer);
}

/*
 * Prevents the escape key to close the application. 
 */
void Application::noEscapeKey_cb(Fl_Widget *w, void *data)
{
    // If the escape key is pressed it's ignored.
    if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape) {
        return;
    }

    Application* app = (Application*) data;
    app->saveVolume();

    // Close the application when the "close" button is clicked.
    exit(0);
}

void Application::quit_cb(Fl_Widget *w, void *data)
{
    Application* app = (Application*) data;
    app->saveVolume();

    exit(0);
}

