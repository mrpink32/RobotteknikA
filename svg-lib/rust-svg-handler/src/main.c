#include "svg-lib.h"

int main(int argc, char *argv[]) {
    const char *path = "/mnt/DATA/Repositories/RobotteknikA/svg-lib/test1.svg";
    char *content = open_file(path);
    if (content == NULL) {
        perror("open_file");
        return 1;
    }
    printf("%s\n", content);
    free(content);
    return 0;
}
