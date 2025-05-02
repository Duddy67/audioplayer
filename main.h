#ifndef MAIN_H
#define MAIN_H
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <iostream>
#include <fstream>
#include "dialog_wnd.h"
#include "file_chooser.h"
#include "audio_settings.h"
#include "audio.h"
#include "../libraries/json.hpp"
#define HEIGHT_MENUBAR 18
#define MODAL_WND_POS 20
#define TEXT_SIZE 13
#define CONFIG_FILENAME "config.json"

using json = nlohmann::json;


class Application : public Fl_Double_Window 
{
        DialogWindow *dialogWnd = 0;
        AudioSettings *audioSettings = 0;
        FileChooser *fileChooser = 0;
        Audio *audio = 0;
        Fl_Menu_Bar *menu;
        Fl_Menu_Item *menuItem;
        std::string message;
        Fl_Button* playBtn;
        Fl_Button* stopBtn;

        struct AppConfig {
            std::string outputDevice;
            std::string inputDevice;
        };

    public:

        Application(int w, int h, const char *l, int argc, char *argv[]);

        void createMenu();
        // Function to save configuration to file
        void saveConfig(const AppConfig& config, const std::string& filename);
        // Function to load configuration from file
        AppConfig loadConfig(const std::string& filename);
        std::string getMessage() { return message; }
        void setMessage(std::string message);

        // Call back functions.
        static void quit_cb(Fl_Widget *w, void *data);
        static void noEscapeKey_cb(Fl_Widget *w, void *data);
        static void dialog_cb(Fl_Widget *w, void *data);
        static void audio_settings_cb(Fl_Widget *w, void *data);
        static void file_chooser_cb(Fl_Widget *w, void *data);
        static void ok_cb(Fl_Widget *w, void *data);
        static void cancel_cb(Fl_Widget *w, void *data);
        static void cancel_audio_settings_cb(Fl_Widget *w, void *data);
        static void save_cb(Fl_Widget *w, void *data);
        static void audio_cb(Fl_Widget *w, void *data);
        static void play_cb(Fl_Widget *w, void *data);
        static void stop_cb(Fl_Widget *w, void *data);
};

#endif
