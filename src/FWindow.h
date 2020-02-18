#ifndef FWINDOW_H
#define FWINDOW_H

#include <cstdint>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include "FElement.h"


struct BitmapData {
    std::string filename;
    Pixmap P;
    uint32_t width;
    uint32_t height;
    uint32_t x;
    uint32_t y;
};

struct FileData {
    std::string filename;
    std::string file_abs_path;
    unsigned char file_type;
    uint32_t x;
    uint32_t y;
    uint32_t height;
    uint32_t width;
};
class FWindow : public FElement
{
public:
    FWindow();

    void init_server();
    void init_window();
    void init_subwindow();
    void open_window();
    void open_file(dirent *entry);
    BitmapData createBitmap(std::string filename);
    void get_current_dir_files();
    void change_dir(std::string dir_string);
    void print_icons();
    void push_file();
    void handle_mouse_click();
    void check_mouse_coordinates(uint32_t x_coor, uint32_t y_coor);
    void print_screen();

    void set_window_title(char* title);
    XRectangle create_rectangle(int16_t x, int16_t y, uint16_t width, uint16_t height);
};

#endif