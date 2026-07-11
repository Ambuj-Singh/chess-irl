#include "core/App.hpp"

#include <cstdlib>

int main(
    int argc,
    char* argv[]
) {
    /*
     * SDL may use these parameters on some platforms.
     */
    (void)argc;
    (void)argv;

    App app;

    if (!app.initialize()) {
        return EXIT_FAILURE;
    }

    app.run();

    return EXIT_SUCCESS;
}