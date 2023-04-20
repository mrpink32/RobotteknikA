#include "svg_to_vec_func.h"

enum
{
    RECTANGLE,
    CIRCLE,
    ELLIPSE,
    LINE,
    POLYLINE,
    POLYGON,
    PATH,
    SQUIRCLE,
    POLYNOMIAL,
};

typedef struct
{
    const char *name;
    int32_t value;
} key_value_pair;

static key_value_pair elements[] = {
    {typename(rect), RECTANGLE},
    {typename(circle), CIRCLE},
    {typename(ellipse), ELLIPSE},
    {typename(line), LINE},
    {typename(polyline), POLYLINE},
    {typename(polygon), POLYGON},
    {typename(path), PATH},
    {typename(squircle), SQUIRCLE},
    {typename(polynomial), POLYNOMIAL},
};