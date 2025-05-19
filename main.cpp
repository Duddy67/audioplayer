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

    slider = new Fl_Slider(SPACE, HEIGHT - SPACE - (BUTTON_HEIGHT * 2), w - (SPACE * 2), (SPACE * 2));
    slider->type(FL_HORIZONTAL);
    slider->bounds(0, 10);
    slider->step(1);
    slider->value(0);

    sliderOutput = new Fl_Output(SPACE, HEIGHT - SPACE - (BUTTON_HEIGHT * 3), BUTTON_WIDTH, 30);
    sliderOutput->value("00:00:00");
    //sliderOutput->textfont(FL_BOLD);
    sliderOutput->textsize(16);
    sliderOutput->align(FL_ALIGN_CENTER);
    slider->callback(slider_cb, this);

    group->end();

    // Stop adding children to this window.
    end();

    this->callback(noEscapeKey_cb, this);
    resizable(group);
    show();

    loadConfig(CONFIG_FILENAME);

    // Create and initialize the Audio object.
    this->audio = new Audio(this);

    if (!this->audio->isContextInit()) {
        setMessage("Failed to initialize audio system.");
        this->dialog_cb(this->dialogWnd, this);
    }
}

int main(int argc, char *argv[])
{
    Application app(WIDTH, HEIGHT, "Player", argc, argv);
    //Application *app = new Application(300,200,"My Application", argc, argv);
    //app->show();

    return Fl::run();
}
