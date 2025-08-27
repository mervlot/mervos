#include "vga.h"
#include "keyboard.h"
#include "shell.h"   


void kernel_main(void) {
  clear_screen();
  print("Welcome to MervOS!\n");
  print("Type something:\n");
  print_color("This is in red text!\n", 0x04);
  print_color("This is in green text!\n", 0x02);
  print_color("This is in blue text!\n", 0x01);
  print_color("This is in yellow text!\n", 0x0E);
  print_color("This is in cyan text!\n", 0x03); 
  print_color("This is in magenta text!\n", 0x05);
  print_color("This is in white text!\n", 0x0F);
  print_color("This is in gray text!\n", 0x07);
  print_color("This is in light gray text!\n", 0x08);
  print_color("This is in light red text!\n", 0x0C);
  print_color("This is in light green text!\n", 0x0A);
  print_color("This is in light blue text!\n", 0x09);
  print_color("This is in light yellow text!\n", 0x0D);
  print_color("This is in light cyan text!\n", 0x0B);
  print_color("This is in light magenta text!\n", 0x0D);
  print("\n");
  
keyboard_init();
shell_run();  
    for (;;) {
        __asm__ volatile("hlt");
    }
}
