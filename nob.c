#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#define NOB_EXPERIMENTAL_DELETE_OLD
#include "thirdparty/nob.h"

#ifdef _WIN32
    #define PLATFORM "win"
    #define RAYLIB_DIR "thirdparty/raylib/raylib-5.5_win64_mingw-w64/"
    #define CC "gcc"
#else
    #define PLATFORM "linux"
    #define RAYLIB_DIR "thirdparty/raylib/raylib-5.5_linux_amd64/"
    #define CC "gcc"
#endif

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    Cmd cmd = {0};

#ifdef _WIN32
    cmd_append(&cmd, "windres", "-o", "src/resource.o", "src/resource.rc");
    if (!cmd_run(&cmd)) return 1;
#endif

    cmd_append(&cmd, CC);
    cmd_append(&cmd, "-Wall", "-Wextra");
    cmd_append(&cmd, "-ggdb");
    cmd_append(&cmd, "-I"RAYLIB_DIR"include", "-Isrc");
    cmd_append(&cmd, "-o", "paint", "src/main.c", "src/canvas.c", "src/tool.c", "src/ui.c");

    cmd_append(&cmd, RAYLIB_DIR"lib/libraylib.a");
#ifdef _WIN32
    cmd_append(&cmd, "src/resource.o");
    cmd_append(&cmd, "-lgdi32", "-lwinmm", "-lmsvcrt", "-lucrtbase", "-mwindows");
#else
    cmd_append(&cmd, "-lm");
#endif

    if (!cmd_run(&cmd)) return 1;

    return 0;
}
