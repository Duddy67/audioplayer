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
    playBtn = new Fl_Button(10, 150, 80, 40, "Play");
    playBtn->callback(play_cb, this);
    stopBtn = new Fl_Button(110, 150, 80, 40, "Stop");
    stopBtn->callback(stop_cb, this);

    cursor = new Fl_Slider(10, 100, w - 20, 25);
    cursor->type(FL_HORIZONTAL);
    cursor->bounds(0, 10);
    cursor->step(1);
    cursor->value(0);


    cursorOutput = new Fl_Output(130, 60, 40, 30);
    cursorOutput->value("0");
    cursorOutput->textfont(FL_BOLD);
    cursorOutput->textsize(16);
    cursorOutput->align(FL_ALIGN_CENTER);
    cursor->callback(cursor_cb, this);

    group->end();

    // Stop adding children to this window.
    end();

    this->callback(noEscapeKey_cb, this);
    resizable(group);
    show();

    loadConfig(CONFIG_FILENAME);

    // Create and initialize the Audio object.
    this->audio = new Audio;

        setMessage("Failed to initialize audio system.");
    if (!this->audio->isContextInit()) {
        setMessage("Failed to initialize audio system.");
        this->dialog_cb(this->dialogWnd, this);
    }
}

int main(int argc, char *argv[])
{
    Application app(600, 400, "Player", argc, argv);
    //Application *app = new Application(300,200,"My Application", argc, argv);
    //app->show();

    return Fl::run();
}
