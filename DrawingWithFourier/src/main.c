#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>

const char *filepath = "square.txt";
const int32_t N = 1600;
const int32_t M = 1200;
const int32_t q = 20000;
const int32_t speed = 1;

typedef struct point
{
	int32_t x;
	int32_t y;
} point;

int someFuncOne(const int32_t *a, int32_t t)
{
	auto count = sizeof(a) / sizeof(a[0]);
	int32_t result = 0;
	for (size_t i = 0; i < count; i++)
	{
		result += a[i] * cos(t);
	}
	return result;
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
	char buff[size];
	int32_t things_read = fscanf(file, "%s", buff);
	printf("things read: %d\n", things_read);
	for (size_t i = 0; i < size; i++)
	{
		printf("%s\n", buff[i]);
	}
	// printf("%d", someFuncOne());

	fclose(file);
	return 0;
}