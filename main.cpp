#include "main.h"


// Create the application.
Application::Application(int w, int h, const char *l, int argc, char *argv[]) : Fl_Double_Window(w, h, l)
{
    color(FL_WHITE);
    // Create and build the menu.
    menu = new Fl_Menu_Bar(0, 0, w, HEIGHT_MENUBAR);
    menu->box(FL_NO_BOX);
    createMenu();
    menu->textsize(TEXT_SIZE);
    menuItem = (Fl_Menu_Item *)menu->find_item("Edit/&Toolbar");
    menuItem->clear();

    // Container for other widgets.
    Fl_Group *group = new Fl_Group(0, HEIGHT_MENUBAR, w, h - HEIGHT_MENUBAR);   
    // Other widgets go here...

    toggleBtn = new Fl_Button(SPACE, HEIGHT - BUTTON_HEIGHT - SPACE, BUTTON_WIDTH, BUTTON_HEIGHT, "@>");
    toggleBtn->callback(toggle_cb, this);

    time = new Fl_Slider(SPACE, HEIGHT - SPACE - (BUTTON_HEIGHT * 2), w - (SPACE * 2), (SPACE * 2));
    time->type(FL_HORIZONTAL);
    time->step(1);
    time->value(0);
    time->callback(time_cb, this);
    // Trigger callback whenever the value changes, including dragging
    time->when(FL_WHEN_CHANGED);

    timeOutput = new Fl_Output(SPACE, HEIGHT - SPACE - (BUTTON_HEIGHT * 3), BUTTON_WIDTH, 30);
    timeOutput->value("00:00:00");
    timeOutput->textsize(16);
    timeOutput->label("Counter");
    timeOutput->align(FL_ALIGN_TOP);

    duration = new Fl_Output(w - BUTTON_WIDTH - SPACE, HEIGHT - SPACE - (BUTTON_HEIGHT * 3), BUTTON_WIDTH, 30);
    duration->value("00:00:00");
    duration->textsize(16);
    duration->label("Duration");
    duration->align(FL_ALIGN_TOP);

    volume = new Fl_Slider(w - (BUTTON_WIDTH * 2) - SPACE, HEIGHT - SPACE - BUTTON_HEIGHT, (BUTTON_WIDTH * 2), BUTTON_HEIGHT);
    volume->type(FL_HORIZONTAL);
    volume->step(0.01);
    volume->bounds(0, 1);
    volume->value(0);
    volume->label("Volume");
    volume->align(FL_ALIGN_TOP);
    volume->callback(volume_cb, this);

    volumeOutput = new Fl_Output(w - (BUTTON_WIDTH * 3), HEIGHT - SPACE - BUTTON_HEIGHT, BUTTON_WIDTH / 1.5, 30);
    volumeOutput->value("0%");
    volumeOutput->textsize(16);

    group->end();

    // Stop adding children to this window.
    end();

    this->callback(noEscapeKey_cb, this);
    resizable(group);
    show();

    AppConfig config = loadConfig(CONFIG_FILENAME);

    // Create and initialize the Audio object.
    this->audio = new Audio(this);

    if (!this->audio->isContextInit()) {
        setMessage("Failed to initialize audio system.");
        this->dialog_cb(this->dialogWnd, this);
    }

    audio->setOutputDevice(config.outputDevice.c_str());
    //audio->printAllDevices();

    // Get and set the last volume value since the app was closed.
    double volumeValue = std::stod(config.volume);
    volume->value(volumeValue);
    volume_cb(volume, this);
}

int main(int argc, char *argv[])
{
    Application app(WIDTH, HEIGHT, "Player", argc, argv);
    //Application *app = new Application(300,200,"My Application", argc, argv);
    //app->show();

    return Fl::run();
}
