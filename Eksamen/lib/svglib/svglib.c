#include "svglib.h"


rect create_rect(double_t x, double_t y, double_t width, double_t height, double_t rx, double_t ry, const char *style) {
    rect rect;
    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;
    rect.rx = rx;
    rect.ry = ry;
    rect.style = style;
    return rect;
}


typedef struct Circle {
    double_t cx;
    double_t cy;
    double_t r;
    const char *style;
} circle;

circle create_circle(double_t cx, double_t cy, double_t r, const char *style) {
    circle circle;
    circle.cx = cx;
    circle.cy = cy;
    circle.r = r;
    circle.style = style;
    return circle;
}


typedef struct Line {
    double_t x1;
    double_t y1;
    double_t x2;
    double_t y2;
    const char *style;
} line;

line create_line(double_t x1, double_t y1, double_t x2, double_t y2, const char *style) {
    line line;
    line.x1 = x1;
    line.y1 = y1;
    line.x2 = x2;
    line.y2 = y2;
    line.style = style;
    return line;
}
