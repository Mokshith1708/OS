#include "hal/hal_console.h"
#include "shell.h"

void kmain(void) {
  hal_console_init();                     // your UART init
  hal_console_puts("[OS] Boot OK (M0-safe)\r\n");
  hal_console_puts("hi\r\n");
  shell_run();                            // blocking shell
  for(;;) { /* idle */ }
}



// #include "hal/hal_console.h"
// #include "pmm.h"
// int kmain(void) {
//     hal_console_puts("Project Monolith Kernel Initialized.\n");
//     hal_console_puts("--- Module 2 Test ---\n");
//     pmm_init();
//     uintptr_t user_base = pmm_get_user_space_base();
//     size_t user_size = pmm_get_user_space_size();
//     size_t user_size_kb = user_size / 1024;
//     hal_console_puts("User space available: ");
//     hal_console_put_int((int)user_size_kb);
//     hal_console_puts("KB at ");
//     hal_console_put_hex((uint32_t)user_base);
//     hal_console_puts("\n");
//     while (1) {}
//     return 0;
// }
