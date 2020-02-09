#include "FElement.h"
#include "FWindow.h"
#include <vector>
#include <cstring>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <iostream>

#define INITIAL_FILE_X_POSITION 20
#define INITIAL_FILE_Y_POSITION 70
#define FILE_BITMAP "../img/file.xbm"
#define FOLDER_BITMAP "../img/folder.xbm"

std::vector<XRectangle> files;
std::vector<BitmapData> file_icons;
std::vector<XRectangle> ui_elems;
std::vector<BitmapData> bitmaps;
std::vector<FileData> files_info;

uint32_t file_X_pos = INITIAL_FILE_X_POSITION;
uint32_t file_Y_pos = INITIAL_FILE_Y_POSITION;

std::string WINDOW_TITLE = "Sistemas Operativos - File Explorer";
std::string current_path = "/";

FWindow::FWindow()
{
    init_server();
    init_window();
    open_window();
    get_current_dir_files();
}

void FWindow::init_server()
{
    d = XOpenDisplay(nullptr);
    if (d == nullptr)
    {
        printf("Cannot open display\n");
        exit(1);
    }
    s = DefaultScreen(d);
}

/* open connection with the server */
void FWindow::init_window()
{
    /* create window */
    w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, 800, 600, 1,
                            BlackPixel(d, s), WhitePixel(d, s));

    // Process Window Close Event through event handler so XNextEvent does Not fail
    Atom delWindow = XInternAtom(d, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(d, w, &delWindow, 1);

    /* select kind of events we are interested in */
    XSelectInput(d, w, ExposureMask | KeyPressMask);

    set_window_title(&WINDOW_TITLE[0]);
    ui_elems.push_back(create_rectangle(0, 45, 800, 5));

    bitmaps.push_back(createBitmap("../img/open.xbm"));
    bitmaps.push_back(createBitmap("../img/up-arrow.xbm"));
    bitmaps.push_back(createBitmap("../img/trash.xbm"));
    bitmaps.push_back(createBitmap("../img/create_folder.xbm"));
    bitmaps.push_back(createBitmap("../img/create_file.xbm"));

    /* map (show) the window */
    XMapWindow(d, w);
}

void FWindow::open_window()
{
    get_current_dir_files();
    while (1)
    {
        XNextEvent(d, &e);
        /* draw or redraw the window */
        if (e.type == Expose)
        {
            BitmapData BMD;
            // Draw elements on screen
            XDrawString(d, w, DefaultGC(d, s), 300, 28, &current_path[0], current_path.size());
            XFillRectangles(d, w, DefaultGC(d, s), &ui_elems[0], ui_elems.size());
            /* XFillRectangles(d, w, DefaultGC(d, s), &files[0], files.size()); */
            for(auto finfo : files_info)
            {
                if (finfo.file_type == DT_DIR)
                {
                    BMD = createBitmap(FOLDER_BITMAP);
                    XCopyPlane(d, BMD.P, w, DefaultGC(d, s), 0, 0, BMD.width, BMD.height, file_X_pos, file_Y_pos, 1);
                    XDrawString(d, w, DefaultGC(d, s), file_X_pos + 20, file_Y_pos + 97, &finfo.filename[0], finfo.filename.size());
                }
                else/*  if (finfo.file_type == DT_REG) */
                {
                    BMD = createBitmap(FILE_BITMAP);
                    XCopyPlane(d, BMD.P, w, DefaultGC(d, s), 0, 0, BMD.width, BMD.height, file_X_pos, file_Y_pos, 1);
                    XDrawString(d, w, DefaultGC(d, s), file_X_pos + 20, file_Y_pos + 110, &finfo.filename[0], finfo.filename.size());
                }
                // std::cout << finfo.filename << std::endl;
                if (file_X_pos == 620)
                {
                    file_X_pos = 20;
                    file_Y_pos += 110;
                }
                else 
                    file_X_pos += 150;
            }
            print_icons();
        }
        /* exit on key press */
        if (e.type == KeyPress)
            break;
        
        if (e.type == ButtonPress)
            break;

        // Handle Windows Close Event
        if (e.type == ClientMessage)
            break;
    }

    /* destroy our window */
    XDestroyWindow(d, w);

    /* close connection to server */
    XCloseDisplay(d);
}

void FWindow::set_window_title(char * title)
{
    XStoreName(d, w, title);
}

XRectangle FWindow::create_rectangle(int16_t x, int16_t y, uint16_t width, uint16_t height)
{
    XRectangle XR;
    XR.x = x;
    XR.y = y;
    XR.width = width;
    XR.height = height;
    return XR;
}


BitmapData FWindow::createBitmap(std::string filename)
{
    /* this variable will contain the ID of the newly created pixmap.    */
    Pixmap bitmap;
    BitmapData BD;

    /* these variables will contain the dimensions of the loaded bitmap. */
    unsigned int bitmap_width, bitmap_height;

    /* these variables will contain the location of the hot-spot of the   */
    /* loaded bitmap.                                                    */
    int hotspot_x, hotspot_y;

    /* this variable will contain the ID of the root window of the screen */
    /* for which we want the pixmap to be created.                        */
    Window root_win = DefaultRootWindow(d);

    /* load the bitmap found in the file "icon.bmp", create a pixmap      */
    /* containing its data in the server, and put its ID in the 'bitmap'  */
    /* variable.                                                          */
    int rc = XReadBitmapFile(d, root_win,
                            filename.c_str(),
                            &bitmap_width, &bitmap_height,
                            &bitmap,
                            &hotspot_x, &hotspot_y);
    /* check for failure or success. */
    switch (rc) {
        case BitmapOpenFailed:
            fprintf(stderr, "XReadBitmapFile - could not open file %s.\n", &filename[0]);
            break;
        case BitmapFileInvalid:
            fprintf(stderr,
                    "XReadBitmapFile - file '%s' doesn't contain a valid bitmap.\n",
                    &filename[0]);
            break;
        case BitmapNoMemory:
            fprintf(stderr, "XReadBitmapFile - not enough memory.\n");
            break;
        case BitmapSuccess:
            /* bitmap loaded successfully - do something with it... */
            BD.P = bitmap;
            BD.height = bitmap_height;
            BD.width = bitmap_width;
            return BD;
            break;
    }
    return BD;
}

void FWindow::get_current_dir_files()
{
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(current_path.c_str())) != NULL)
        {
            /* print all the files and directories within directory */
            files_info.clear();
            while ((ent = readdir(dir)) != NULL)
            {
                FileData FD;
                FD.filename = ent->d_name;
                FD.file_abs_path = current_path + ent->d_name;
                FD.file_type = ent->d_type;
                // printf("%s\n", ent->d_name);
                files_info.push_back(FD);
            }
            closedir(dir);
        }
        else
        {
            /* could not open directory */
            perror("Couldn't open the directory.\n");
            return;
        }
}

void FWindow::open_file(dirent *entry)
{
    if (entry->d_type == DT_UNKNOWN)
        std::cout << "Unkown file type.\n";
    else if(entry->d_type == DT_REG)
        // Open the file
        std::exit(1);
    else if(entry->d_type == DT_DIR)
        change_dir(entry->d_name);
}

void FWindow::change_dir(std::string dir_string)
{
    if (dir_string == "..")
    {
        // Go up 1 directory
    }
    else
    {
        current_path += (dir_string + "/");
        get_current_dir_files();
    }
}

void FWindow::print_icons()
{
    uint32_t initial_x_icons = 15;
    for(auto a : bitmaps)
    {
        XCopyPlane(d, a.P, w, DefaultGC(d, s), 0, 0, a.width, a.height, initial_x_icons, 5, 1);
        initial_x_icons += 40;
    }
}

XTextItem makeXTextItem(std::string text, uint32_t X, uint32_t Y)
{
    XTextItem XTxt;
    XTxt.chars = &text[0];
    XTxt.nchars = text.size();
    XTxt.delta = 1;
    return XTxt;
}