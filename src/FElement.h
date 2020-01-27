#ifndef FELEMENT_H
#define FELEMENT_H

#include <cstdint>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

class FElement
{
public:
    FElement();
    void setWidth(uint32_t w) { width = w; };
    void setHeight(uint32_t h) { height = h; };
    uint32_t getWidth() { return width; };
    uint32_t getHeight() { return height; };

    void setX(uint32_t XParam) { x = XParam; };
    void setY(uint32_t YParam) { y = YParam; };
    uint32_t getX() { return x; };
    uint32_t getY() { return y; };

protected:
    uint32_t x;
    uint32_t y;
    uint32_t height;
    uint32_t width;
    Display *d;
    int s;
    Window w;
    XEvent e;
};

#endif