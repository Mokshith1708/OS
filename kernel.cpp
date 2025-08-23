#include "process.h"

extern "C" void kmain() {
    static Process p1(1);
    static Process p2(2);

    add_process(&p1);
    add_process(&p2);

    while (true) {
        schedule();
        p1.run();
        p2.run();
    }
}
