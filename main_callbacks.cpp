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

void Application::cursor_cb(Fl_Widget *w, void *data)
{
    Application* app = (Application*) data;

    // Format slider value as string
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%.0f", app->cursor->value());

    // Set new value in output box
    app->cursorOutput->value(buffer);
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

