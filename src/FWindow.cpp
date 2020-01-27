#include "FElement.h"
#include "FWindow.h"

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
            XFillRectangle(d, w, DefaultGC(d, s), 20, 20, 10, 10);
        }
        /* exit on key press */
        if (e.type == KeyPress)
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