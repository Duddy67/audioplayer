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

void Application::time_cb(Fl_Widget *w, void *data)
{
    Application* app = (Application*) data;
    app->hasSliderMoved = false;

    if (dynamic_cast<Fl_Slider*>(w)) {
        std::cout << "Caller is an Fl_Slider.\n";
        app->hasSliderMoved = true;
    }

    double seconds = app->getSlider("time")->value();
    std::map time = app->getTimeFromSeconds(seconds);

    // Format slider value as string
    char buffer[60];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", time["hours"], time["minutes"], time["seconds"]);

    // Set new value in output box
    app->timeOutput->value(buffer);
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

    // Close the application when the "close" button is clicked.
    exit(0);
}

void Application::quit_cb(Fl_Widget *w, void *data)
{
    exit(0);
}

