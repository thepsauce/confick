#include <stddef.h>

typedef struct Line {
    char *buffer;
    size_t length;
    size_t capacity;
} *Line;

typedef struct Text {
    Line lines;

} *Text;
