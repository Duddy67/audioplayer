#include "main.h"
#include <cstdlib>

void Application::saveConfig(const AppConfig& config, const std::string& filename)
{
    json j;
    j["outputDevice"] = config.outputDevice;
    j["inputDevice"] = config.inputDevice;

    std::ofstream file(filename);
    file << j.dump(4); // Pretty print with 4 spaces indentation
    std::cout << "Configuration saved to " << filename << std::endl;

}

Application::AppConfig Application::loadConfig(const std::string& filename)
{
    AppConfig config;
    std::ifstream file(filename);

    // If no config file is found, create it.
    if (!file.is_open()) {
        config.outputDevice = "none";
        config.inputDevice = "none";
        this->saveConfig(config, filename);
        return config;
    }

    try {
        json j;
        file >> j;

        config.outputDevice = j.value("outputDevice", "none");
        config.inputDevice = j.value("inputDevice", "none");
    }
    catch (const json::exception& e) {
        setMessage("Error parsing config: " + std::string(e.what()));
    }

    return config;
}

void Application::setMessage(std::string message)
{
    this->message = message;
}

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

int main(int argc, char *argv[])
{
    Application app(600, 400, "Player", argc, argv);
    //Application *app = new Application(300,200,"My Application", argc, argv);
    //app->show();

    return Fl::run();
}
