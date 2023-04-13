#include <math.h>


typedef struct Rect {
    double_t x;
    double_t y;
    double_t width;
    double_t height;
    double_t rx;
    double_t ry;
    const char *style;
} rect;


rect create_rect(double_t x, double_t y, double_t width, double_t height, double_t rx, double_t ry, const char *style);

