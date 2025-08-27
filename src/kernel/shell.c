#include <stdint.h>
#include <stddef.h>
#include "vga.h"
#include "keyboard.h"

#define LINE_MAX 128
#define CMD_MAX  32

static char linebuf[LINE_MAX];



static int str_equal(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return (*a == 0 && *b == 0);
}

static int starts_with(const char *s, const char *p) {
    while (*p) {
        if (*s++ != *p++) return 0;
    }
    return 1;
}


static const char* split_command(const char *line, char *dest, size_t maxlen) {
    size_t i = 0;
    while (*line == ' ') line++;
    while (*line && *line != ' ' && i + 1 < maxlen) {
        dest[i++] = *line++;
    }
    dest[i] = 0;
    while (*line == ' ') line++;
    return line;
}


static int parse_uint8(const char *s) {
    if (!s || *s == 0) return -1;
    int val = 0;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
        for (; *s; ++s) {
            char c = *s;
            int d;
            if (c >= '0' && c <= '9') d = c - '0';
            else if (c >= 'a' && c <= 'f') d = 10 + (c - 'a');
            else if (c >= 'A' && c <= 'F') d = 10 + (c - 'A');
            else return -1;
            val = (val << 4) | d;
            if (val > 0xFF) return -1;
        }
        return val;
    } else {
        
        for (; *s; ++s) {
            if (*s < '0' || *s > '9') return -1;
            val = val * 10 + (*s - '0');
            if (val > 0xFF) return -1;
        }
        return val;
    }
}


typedef void (*cmd_fn)(const char *args);


static void cmd_help(const char *args);




static void cmd_cls(const char *args) {
    (void)args;
    clear_screen();
}

static void cmd_echo(const char *args) {
    if (!args) return;
    print(args);
    putchar('\n');
}

static void cmd_ver(const char *args) {
    (void)args;
    print("MervOS 1.0 - shell-based\n");
}

static void cmd_time(const char *args) {
    (void)args;
    print("time: (not implemented) RTC/clock missing\n");
}

static void cmd_color(const char *args) {
    if (!args || *args == 0) {
        print("Usage: color <hex|dec>\n");
        return;
    }
    int v = parse_uint8(args);
    if (v < 0) {
        print("Invalid color. Example: color 0x0A\n");
        return;
    }
    set_color((uint8_t)v);
}

static void cmd_info(const char *args) {
    (void)args;
    print("MervOS info: basic kernel, no paging, no modules\n");
}

static void cmd_restart(const char *args) {
    (void)args;
    print("Restarting ...\n");

   
    __asm__ volatile (
        "mov $0xFE, %al\n"
        "out %al, $0x64\n"
    );

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
static void cmd_exit(const char *args) {
    (void)args;  
    clear_screen();
    print("Shutting down MervOS...\n");

    
}
static void cmd_about(const char *args) {
    (void)args;
    print("MervOS - minimal educational OS\n");
    print("Core: VGA, PS/2 keyboard, simple shell\n");
    print("Author: you (plus helper)\n");
}

static void cmd_test(const char *args) {
    (void)args;
    print("running test... ok\n");
}

static void cmd_help(const char *args) {
    (void)args;
    print("Commands:\n");
    print("  help      - show this message\n");
    print("  cls/clear - clear screen\n");
    print("  echo ...  - print remaining text\n");
    print("  ver       - version\n");
    print("  time      - show time (stub)\n");
    print("  color x   - set default text color (hex or dec)\n");
    print("  info      - kernel info (stub)\n");
    print("  about     - about / credits\n");
    print("  exit      - noop in kernel (placeholder)\n");
    print("  restart      - restart the pc\n");
    print("  test      - run a quick test\n");
}


static struct {
    const char *name;
    cmd_fn      func;
} commands[] = {
    { "help",  cmd_help  },
    { "cls",   cmd_cls   },
    { "clear", cmd_cls   }, /* alias */
    { "echo",  cmd_echo  },
    { "ver",   cmd_ver   },
    { "time",  cmd_time  },
    { "color", cmd_color },
    { "info",  cmd_info  },
    { "about", cmd_about },
    { "exit",  cmd_exit  },
    { "restart",  cmd_restart  },
    { "test",  cmd_test  },


    { NULL,    NULL      }
};


static void read_line(char *out, size_t maxlen) {
    size_t idx = 0;
    for (;;) {
        char c = keyboard_getkey();
        if (c == 0) continue;

        if (c == '\n' || c == '\r') {
            putchar('\n');
            if (idx >= maxlen) idx = maxlen - 1;
            out[idx] = 0;
            return;
        }

        if (c == '\b') {
            if (idx > 0) {
                idx--;
                putchar('\b'); 
            }
            continue;
        }

        if (c == '\t') {
            const int tabsize = 4;
            for (int i = 0; i < tabsize; ++i) {
                if (idx + 1 < maxlen) {
                    out[idx++] = ' ';
                    putchar(' ');
                }
            }
            continue;
        }

        if ((unsigned char)c >= 32 && (unsigned char)c <= 126) {
            if (idx + 1 < maxlen) {
                out[idx++] = c;
                putchar(c);
            }
            continue;
        }
  
    }
}


static void process_command(const char *line) {
    char cmd[CMD_MAX];
    const char *args = split_command(line, cmd, sizeof(cmd));

    if (cmd[0] == 0) return;


    for (size_t i = 0; commands[i].name != NULL; ++i) {
        if (str_equal(cmd, commands[i].name)) {
            commands[i].func(args);
            return;
        }
    }

    print("Unknown command: ");
    print(cmd);
    print("\nType 'help' for commands.\n");
}



void shell_run(void) {
    keyboard_init();
    clear_screen();
    print("Welcome to MervOS shell\n");
    print("Type 'help' for commands\n\n");

    for (;;) {
        print("mervos> ");
        read_line(linebuf, LINE_MAX);
        process_command(linebuf);
    }
}
