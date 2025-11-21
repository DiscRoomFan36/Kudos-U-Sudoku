
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "thirdparty/nob.h"

#define SRC_FOLDER          "./src/"
#define BUILD_FOLDER        "./build/"
#define THIRDPARTY_FOLDER   "./thirdparty/"

#define RAYLIB_FOLDER       THIRDPARTY_FOLDER"raylib-5.5/"


bool check_if_file_exists(const char *filepath) {
    return access(filepath, F_OK) == 0;
}


Cmd cmd = {0};


void cmd_cc(void) {
    cmd_append(&cmd, "clang");
    cmd_append(&cmd, "-std=gnu11");
}

void cmd_c_flags(void) {
    cmd_append(&cmd, "-Wall", "-Wextra");
    // cmd_append(&cmd, "-Werror");
    cmd_append(&cmd, "-I"THIRDPARTY_FOLDER);
}

void cmd_debug(void) {
    cmd_append(&cmd, "-ggdb");
}

void cmd_link_with_raylib(void) {
    cmd_append(&cmd, "-I"RAYLIB_FOLDER"src");
    cmd_append(&cmd, "-L"RAYLIB_FOLDER"src");
    cmd_append(&cmd, "-l:libraylib.so", "-lm");
}

bool build_debug(void) {
    cmd_cc();
    cmd_c_flags();
    cmd_debug();
    // cmd_append(&cmd, "-O2");
    cmd_append(&cmd, "-o", BUILD_FOLDER"main_debug");
    cmd_append(&cmd, SRC_FOLDER"main.c");
    cmd_link_with_raylib();
    if (!cmd_run(&cmd)) return false;

    return true;
}

bool build_release(void) {
    cmd_cc();
    cmd_c_flags();
    cmd_append(&cmd, "-O2");
    cmd_append(&cmd, "-o", BUILD_FOLDER"main_release");
    cmd_append(&cmd, SRC_FOLDER"main.c");
    cmd_link_with_raylib();
    if (!cmd_run(&cmd)) return false;

    return true;
}

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);


    // TODO make this a command line arg
    #define BESTED_PATH "/home/fletcher/Programming/C-things/Bested.h/Bested.h"
    if (check_if_file_exists(BESTED_PATH)) {
        cmd_append(&cmd, "cp");
        cmd_append(&cmd, BESTED_PATH);
        cmd_append(&cmd, THIRDPARTY_FOLDER"Bested.h");
        if (!cmd_run(&cmd)) return 1;
    }

    mkdir_if_not_exists(BUILD_FOLDER);

    if (!build_debug())     return 1;
    // if (!build_release())   return 1;

    return 0;
}
