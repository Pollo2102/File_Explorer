#include "FElement.h"
#include "FWindow.h"
#include <vector>

std::vector<XRectangle> files;

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

    set_window_title("Sistemas Operativos - File Explorer");

    /* Rectangles for testing */
    files.push_back(create_rectangle(20, 20, 100, 100));
    files.push_back(create_rectangle(170, 20, 100, 100));
    files.push_back(create_rectangle(320, 20, 100, 100));
    files.push_back(create_rectangle(470, 20, 100, 100));
    files.push_back(create_rectangle(620, 20, 100, 100));
    files.push_back(create_rectangle(20, 170, 100, 100));
    files.push_back(create_rectangle(170, 170, 100, 100));
    files.push_back(create_rectangle(320, 170, 100, 100));
    files.push_back(create_rectangle(470, 170, 100, 100));
    files.push_back(create_rectangle(620, 170, 100, 100));

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
            // Rectangle gets drawn on screen
            /* XFillRectangle(d, w, DefaultGC(d, s), 20, 20, 10, 10); */
            XFillRectangles(d, w, DefaultGC(d, s), &files[0], files.size());
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