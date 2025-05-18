#include "main.h"


void Application::play_cb(Fl_Widget* w, void* data)
{
    Application* app = (Application*) data;
    app->label("Play button clicked");
    app->audio->play();
}

void Application::stop_cb(Fl_Widget* w, void* data)
{
    Application* app = (Application*) data;
    app->label("Stop button clicked");
    app->audio->stop();
}

void Application::slider_cb(Fl_Widget *w, void *data)
{
    Application* app = (Application*) data;

    double seconds = app->slider->value();
    int totalSeconds = (int)seconds;
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int secs = totalSeconds % 60;

    // Format slider value as string
    char buffer[60];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, secs);

    // Set new value in output box
    app->sliderOutput->value(buffer);
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

