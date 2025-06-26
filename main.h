#ifndef MAIN_H
#define MAIN_H
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Output.H>
#include <iostream>
#include <fstream>
#include <bits/stdc++.h> // std::map
#include "dialog_wnd.h"
#include "file_chooser.h"
#include "audio_settings.h"
#include "audio.h"
#include "../libraries/json.hpp"
#define WIDTH 600
#define HEIGHT 400
#define BUTTON_WIDTH 80
#define BUTTON_HEIGHT 40
#define SPACE 10
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
        std::string message;
        Fl_Menu_Bar *menu;
        Fl_Menu_Item *menuItem;
        Fl_Button *toggleBtn;
        Fl_Slider *time;
        Fl_Slider *volume;
        Fl_Output *timeOutput;
        Fl_Output *volumeOutput;
        Fl_Output *duration;
        // Null Fl_Widget pointer aimed to be passed as first argument of some callback functions
        Fl_Widget *nullWidget = nullptr;
        Fl_Multiline_Output *fileInfo;

        struct AppConfig {
            std::string outputDevice;
            std::string inputDevice;
            std::string volume;
        };

    public:

        Application(int w, int h, const char *l, int argc, char *argv[]);

        void createMenu();
        // Function to save configuration to file
        void saveConfig(const AppConfig& config, const std::string& filename);
        // Function to load configuration from file
        AppConfig loadConfig(const std::string& filename);
        std::string getMessage() { return message; }
        Fl_Slider* getSlider(const char *type);
        Fl_Button* getButton() { return toggleBtn; }
        Fl_Widget* getNullWidget() { return nullWidget; }
        void setMessage(std::string message);
        void setDuration(double seconds);
        void saveVolume();
        void updateToggleButton();
        std::map<std::string, int> getTimeFromSeconds(double seconds);
        bool hasSliderMoved = false;
        void dispayFileInfo(std::map<std::string, std::string> info);

        // Call back functions.
        static void quit_cb(Fl_Widget *w, void *data);
        static void noEscapeKey_cb(Fl_Widget *w, void *data);
        static void dialog_cb(Fl_Widget *w, void *data);
        static void audio_settings_cb(Fl_Widget *w, void *data);
        static void file_chooser_cb(Fl_Widget *w, void *data);
        static void ok_cb(Fl_Widget *w, void *data);
        static void cancel_cb(Fl_Widget *w, void *data);
        static void cancel_audio_settings_cb(Fl_Widget *w, void *data);
        static void save_audio_settings_cb(Fl_Widget *w, void *data);
        static void toggle_cb(Fl_Widget *w, void *data);
        static void time_cb(Fl_Widget *w, void *data);
        static void volume_cb(Fl_Widget *w, void *data);
};

#endif
