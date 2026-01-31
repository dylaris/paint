#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#define NOB_EXPERIMENTAL_DELETE_OLD
#include "nob.h"

#ifdef _WIN32
    #define PLATFORM "win"
    #define RAYLIB_DIR "raylib/raylib-5.5_win64_mingw-w64/"
    #define CC "gcc"
#else
    #define PLATFORM "linux"
    #define RAYLIB_DIR "raylib/raylib-5.5_linux_amd64/"
    #define CC "gcc"
#endif

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    Cmd cmd = {0};

    cmd_append(&cmd, CC);
    cmd_append(&cmd, "-Wall", "-Wextra");
    cmd_append(&cmd, "-ggdb");
    cmd_append(&cmd, "-I"RAYLIB_DIR"include");
    cmd_append(&cmd, "-o", "paint", "paint.c");

    cmd_append(&cmd, RAYLIB_DIR"lib/libraylib.a");
#ifdef _WIN32
    cmd_append(&cmd, "-lgdi32", "-lwinmm", "-lmsvcrt", "-lucrtbase");
#else
    cmd_append(&cmd, "-lm");
#endif

    if (!cmd_run(&cmd)) return 1;
    return 0;
}
