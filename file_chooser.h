#ifndef FILE_CHOOSER_H
#define FILE_CHOOSER_H
#include <stdlib.h>     // exit,malloc
#include <FL/Fl_Native_File_Chooser.H>

class FileChooser : public Fl_Native_File_Chooser
{
    // Return an 'untitled' default pathname
    const char* untitled_default() 
    {
        static char *filename = 0;

        if (!filename) {
            const char *home = fl_getenv("HOME") ? fl_getenv("HOME") :            // unix
                               fl_getenv("HOME_PATH") ? fl_getenv("HOME_PATH") :  // windows
                               ".";                                               // other

            filename = (char*) malloc(strlen(home) + 20);
            sprintf(filename, "%s/untitled.txt", home);
        }

        return (filename);
    }

    public:

    FileChooser() {
        // Initialize the file chooser
        filter("Wav\t*.wav\n");
        preset_file(this->untitled_default());
    }

    // 'Open' the file
    void open(const char *filename)
    {
        printf("Open '%s'\n", filename);
    }
};

#endif
