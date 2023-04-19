#pragma once
#include <stdint.h>
#include <vector>
#include <stdio.h>


// Struct declarations
typedef struct Point point;
typedef struct Rectangle rect;
typedef struct Circle circle;
typedef struct Ellipse ellipse;
typedef struct Line line;
typedef struct Polyline polyline;
typedef struct Polygon polygon;
typedef struct Path path;


// function declarations
rect create_rect(int32_t x, int32_t y, int32_t width, int32_t height, int32_t rx, int32_t ry, const char *style);

circle create_circle(int32_t x, int32_t y, int32_t r, const char *style);

ellipse create_ellipse(int32_t cx, int32_t cy, int32_t rx, int32_t ry, const char *style);

line create_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const char *style);

polygon create_polygon(std::vector<point> points, const char *style);

polyline create_polyline(std::vector<point> points, const char *style);

void interpret_svg(char *buf, int len);