#include "main.h"

void Application::file_chooser_cb(Fl_Widget *w, void *data)
{
    Application* app = (Application*) data;

    if (app->fileChooser == 0) {
        app->fileChooser = new FileChooser();
    }

    switch (app->fileChooser->show()) {
        case -1:   // Error
            break;
        case 1:    // Cancel
            break;
        default:   // Choice
            app->fileChooser->preset_file(app->fileChooser->filename());
            //app->fileChooser->open(app->fileChooser->filename());
            app->audio->loadFile(app->fileChooser->filename());
            break;
    }
}


