#include "keyboard.h"
#include "vga.h"
#include <stdint.h>

#define KBD_DATA_PORT 0x60
#define KBD_STATUS_PORT 0x64

static char key_map[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',  
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,     
    'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\','z','x', 
    'c','v','b','n','m',',','.','/',0,'*',0,' ',0,0,0,0,0,          
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                           
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                                   
};




static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

char keyboard_getkey(void) {
    uint8_t status = inb(KBD_STATUS_PORT);
    if (!(status & 0x01)) return 0;     
    uint8_t code = inb(KBD_DATA_PORT);
    if (code & 0x80) return 0;           
    if (code >= 128) return 0;          
    return key_map[code];
}


void keyboard_init(void) {
   
}
