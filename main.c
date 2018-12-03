#include "engine.h"


int main() {
    initialise();

    int exit_code = main_loop();

    tear_down();

    return exit_code;
}
