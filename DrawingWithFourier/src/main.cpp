#define _USE_MATH_DEFINES

#include <cmath>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <iostream>
#include <vector>

// #include "simple_svg_1.0.0.hpp"

const char *filepath = "pringle.svg";
const int32_t N = 1600;
const int32_t M = 1200;
const int32_t q = 20000;
const int32_t speed = 1;

int32_t someFunc(std::vector<int32_t> a, std::vector<int32_t> b, int32_t t)
{
	if (a.size() == b.size())
	{
		int32_t result = 0;
		for (size_t i = 1; i <= a.size(); i++)
		{
			result += a[i] * cos((cos(M_PI * i) * floor(i / 2)) * 2 * M_PI * t + b[i]);
		}
		return result;
	}
	return NULL;
}

int main(int argc, char const *argv[])
{
	FILE *file = fopen(filepath, "r");
	if (fseek(file, 0, SEEK_END) != 0)
	{
		printf("Error seeking file\n");
		return 1;
	}
	int32_t size = ftell(file);
	printf("file size: %d\n", size);
	std::vector<int32_t> a = {13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
	std::vector<int32_t> b = {13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

	printf("%g\n", someFunc(a, b, 1));

	fclose(file);
	return 0;
}