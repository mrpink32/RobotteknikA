#include "svglib.h"


typedef struct Point {
    int32_t x;
    int32_t y;
} point;


typedef struct Rectangle {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    int32_t rx;
    int32_t ry;
    const char *style;
} rect;

rect create_rect(int32_t x, int32_t y, int32_t width, int32_t height, int32_t rx, int32_t ry, const char *style) {
    rect element;
    element.x = x;
    element.y = y;
    element.width = width;
    element.height = height;
    element.rx = rx;
    element.ry = ry;
    element.style = style;
    return element;
}


typedef struct Circle {
    int32_t cx;
    int32_t cy;
    int32_t r;
    const char *style;
} circle;

circle create_circle(int32_t cx, int32_t cy, int32_t r, const char *style) {
    circle element;
    element.cx = cx;
    element.cy = cy;
    element.r = r;
    element.style = style;
    return element;
}


typedef struct Ellipse {
    int32_t cx;
    int32_t cy;
    int32_t rx;
    int32_t ry;
    const char *style;
} ellipse;

ellipse create_ellipse(int32_t cx, int32_t cy, int32_t rx, int32_t ry, const char *style) {
    ellipse element;
    element.cx = cx;
    element.cy = cy;
    element.rx = rx;
    element.ry = ry;
    element.style = style;
    return element;
}


typedef struct Line {
    int32_t x1;
    int32_t y1;
    int32_t x2;
    int32_t y2;
    const char *style;
} line;

line create_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const char *style) {
    line element;
    element.x1 = x1;
    element.y1 = y1;
    element.x2 = x2;
    element.y2 = y2;
    element.style = style;
    return element;
}


typedef struct Polygon {
    std::vector<point> points;
    const char *style;
} polygon;

polygon create_polygon(std::vector<point> points, const char *style) {
    polygon element;
    element.points = points;
    element.style = style;
    return element;
}


typedef struct Polyline {
    std::vector<point> points;
    const char *style;
} polyline;

polyline create_polyline(std::vector<point> points, const char *style) {
    polyline element;
    element.points = points;
    element.style = style;
    return element;
}


void interpret_svg(char *path /*char *buf, int len*/) {
    // open file
    printf("path: %s\n", path);
    printf("opening file...\n");
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        perror("fopen");
        return;
    }
    printf("file opened\n");
    
}
