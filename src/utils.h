#ifndef UTILS_H
#define UTILS_H

#define ARRAY_LEN(arr) ((unsigned int) (sizeof(arr)/sizeof(arr[0])))

#define da_append(da, item) \
    do { \
        if ((da)->count + 1 > (da)->capacity) { \
            (da)->capacity = (da)->capacity < 32 ? 32 : 2*(da)->capacity; \
            (da)->items = realloc((da)->items, sizeof((da)->items[0])*(da)->capacity); \
            assert((da)->items && "run out of memory"); \
        } \
        (da)->items[(da)->count++] = (item); \
    } while (0)

#define da_reset(da) do { (da)->count = 0; } while (0)

#define da_free(da) \
    do { \
        if ((da)->items) free((da)->items); \
        (da)->items = NULL; \
        (da)->count = 0; \
        (da)->capacity = 0; \
    } while (0)

#define ABS(a) ((a) < 0 ? -(a) : (a))
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define CLAMP(n, min, max) \
    do { \
        if ((n) < (min)) (n) = (min); \
        if ((n) > (max)) (n) = (max); \
    } while (0)
#define DIST(v1, v2) sqrt(((v1).x-(v2).x)*((v1).x-(v2).x) + ((v1).y-(v2).y)*((v1).y-(v2).y))

#endif // UTILS_H
