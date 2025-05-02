#include "main.h"


void Application::createMenu()
{
    //Fl_Menu_Item item = {"Edit/&Toolbar2", 0,0, 0, FL_MENU_TOGGLE|FL_MENU_VALUE};
    menu->add("File", 0, 0, 0, FL_SUBMENU);
    menu->add("File/&New", FL_ALT + 'n', 0, 0);
    menu->add("File/_&Save");
    menu->add("File/_&Open", 0, file_chooser_cb, (void*) this);
    menu->add("File/&Quit", FL_CTRL + 'q',(Fl_Callback*) quit_cb, 0, 0);
    menu->add("Edit", 0, 0, 0, FL_SUBMENU);
    menu->add("Edit/&Copy", FL_CTRL + 'c',0, 0, 0);
    menu->add("Edit/&Past", FL_CTRL + 'v',0, 0, FL_MENU_INACTIVE);
    menu->add("Edit/&Cut", FL_CTRL + 'x',0, 0, 0);
    menu->add("Edit/&Toolbar", 0,0, 0, FL_MENU_TOGGLE|FL_MENU_VALUE);
    menu->add("Edit/_&Settings", 0, audio_settings_cb, (void*) this);
    menu->add("Help", 0, 0, 0, FL_SUBMENU);
    menu->add("Help/Index", 0, 0, 0, 0);
    menu->add("Help/About", 0, dialog_cb, (void*) this);
    // etc...

    return;
}
