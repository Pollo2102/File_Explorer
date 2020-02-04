#include "FElement.h"
#include "FWindow.h"
#include <vector>
#include <cstring>
#include <string>

#define INITIAL_FILE_Y_POSITION 70
#define INITIAL_FILE_X_POSITION 20

std::vector<XRectangle> files;
std::vector<XRectangle> ui_elems;
std::vector<BitmapData> bitmaps;

std::string WINDOW_TITLE = "Sistemas Operativos - File Explorer";
std::string current_path = "/path/to/current/dir";

FWindow::FWindow()
{
    init_server();
    init_window();
    open_window();
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
    /* bitmaps.push_back(createBitmap("/media/diego/Secondary_ext41/Universidad/Sistemas_Operativos_II/File_Explorer/img/folder.xbm")); */
    bitmaps.push_back(createBitmap("../img/folder.xbm"));


    /* Rectangles for testing */
    files.push_back(create_rectangle(20, 70, 100, 100));
    files.push_back(create_rectangle(170, 70, 100, 100));
    files.push_back(create_rectangle(320, 70, 100, 100));
    files.push_back(create_rectangle(470, 70, 100, 100));
    files.push_back(create_rectangle(620, 70, 100, 100));
    files.push_back(create_rectangle(20, 220, 100, 100));
    files.push_back(create_rectangle(170, 220, 100, 100));
    files.push_back(create_rectangle(320, 220, 100, 100));
    files.push_back(create_rectangle(470, 220, 100, 100));
    files.push_back(create_rectangle(620, 220, 100, 100));

    /* map (show) the window */
    XMapWindow(d, w);
}

void FWindow::open_window()
{
    while (1)
    {
        XNextEvent(d, &e);
        /* draw or redraw the window */
        if (e.type == Expose)
        {
            // Draw elements on screen
            XDrawString(d, w, DefaultGC(d, s), 300, 28, &current_path[0], current_path.size());
            XFillRectangles(d, w, DefaultGC(d, s), &ui_elems[0], ui_elems.size());
            XFillRectangles(d, w, DefaultGC(d, s), &files[0], files.size());
            for(auto bmps : bitmaps) XCopyPlane(d, bmps.P, w, DefaultGC(d, s), 0, 0, bmps.width, bmps.height, 100, 5, 1);
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

XTextItem makeXTextItem(std::string text, uint32_t X, uint32_t Y)
{
    XTextItem XTxt;
    XTxt.chars = &text[0];
    XTxt.nchars = text.size();
    XTxt.delta = 1;
    return XTxt;
}