// #include "hal/hal_console.h"

// void kmain(void) {
//     hal_console_init();
//     hal_console_puts("[OS] Boot OK (MPS2-AN385)\n");

//     hal_console_puts("Counting: ");
//     for (int i = 0; i < 5; i++) {
//         hal_console_put_int(i);
//         hal_console_putc(' ');
//     }
//     hal_console_putc('\n');

//     hal_console_puts("Hex values: ");
//     for (int i = 0; i < 5; i++) {
//         hal_console_put_hex(0xDEADBEEF + i);
//         hal_console_putc(' ');
//     }
//     hal_console_putc('\n');

//     hal_console_puts("Entering infinite loop...\n");
//     while (1) {
//         /* optionally echo */
//         // int c = hal_console_getchar();
//         // hal_console_putc(c);
//     }
// }



#include "hal/hal_console.h"
#include "shell.h"

void kmain(void) {
  hal_console_init();                     // your UART init
  // debug_step();   
  hal_console_puts("[OS] Boot OK (M0-safe)\r\n");
  // debug_step();   
  hal_console_puts("hi\r\n");
  // debug_step();   

  hal_console_puts("Testing 123\r\n");

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
