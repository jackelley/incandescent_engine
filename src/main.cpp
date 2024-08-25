#include "incandescent_engine.h"

int main() {
    IncandescentEngine engine;

    engine.initialize();
    engine.run();
    engine.cleanup();

    return 0;
}
