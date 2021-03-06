#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_help(char *args);

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_si(char *args) {
  int step = 1;
  if(args){
    step = atoi(args);
  }
  if(0 == step){
    step = 1;
  }
  cpu_exec(step);
  return 0;
}

static int cmd_info(char *args) {
  if(NULL == args) {
    return 0;
  }
  if(strstr(args, "r")) {
    printf("%-10s %-#10x %-10d\n", "eax", cpu.eax, cpu.eax);
    printf("%-10s %-#10x %-10d\n", "ebx", cpu.ebx, cpu.ebx);
    printf("%-10s %-#10x %-10d\n", "ecx", cpu.ecx, cpu.ecx);
    printf("%-10s %-#10x %-10d\n", "edx", cpu.edx, cpu.edx);
    printf("%-10s %-#10x %-10d\n", "esi", cpu.esi, cpu.esi);
    printf("%-10s %-#10x %-10d\n", "edi", cpu.edi, cpu.edi);
    printf("%-10s %-#10x %-10d\n", "ebp", cpu.ebp, cpu.ebp);
    printf("%-10s %-#10x %-10d\n", "esp", cpu.esp, cpu.esp);
    printf("%-10s %-#10x %-10d\n", "eip", cpu.eip, cpu.eip);
    printf("%-10s %-#10x\n", "eflags", cpu.eflags);
    printf("%-10s %-10d\n", "CF", cpu.CF);
    printf("%-10s %-10d\n", "ZF", cpu.ZF);
    printf("%-10s %-10d\n", "SF", cpu.SF);
    printf("%-10s %-10d\n", "IF", cpu.IF);
    printf("%-10s %-10d\n", "OF", cpu.OF);
  } else if(strstr(args, "w")) {
    list_wps();
  }
  return 0;
}

static int cmd_p(char *args) {
  bool success;
  uint32_t value;

  value = expr(args, &success);
  if(true == success) {
    printf("%-#10x %-10d\n", value, value);
  } else {
    printf("Bad expression\n");
  }
  return 0;
}

static int cmd_x(char *args) {
  char *arg;
  int i, N;
  bool success;
  uint32_t value, addr;

  arg = strtok(NULL, " ");
  if(NULL == arg) {
    return 0;
  }
  N = atoi(arg);
  if(0 == N) {
    return 0;
  }

  arg = strtok(NULL, " ");
  if(NULL == arg) {
    return 0;
  }
  addr = expr(arg, &success);
  if(false == success) {
    printf("Bad expression\n");
    return 0;
  }
  for(i = 0; i < N; ++i, addr += 4) {
    value = vaddr_read(addr, 4);
    if(0 == i%4) {
      printf("%-#10x: ", addr);
    }
    printf("%0#10x ", value);
    if(0 == (i+1)%4) {
      printf("\n");
    }
  }
  printf("\n");

  return 0;
}

static int cmd_w(char *args) {
  bool success;
  uint32_t value;
  WP *wp;

  value = expr(args, &success);
  if(false == success) {
    printf("Bad expression\n");
    return 0;
  }

  wp = new_wp();
  if(!wp) {
    return 0;
  }
  strcpy(wp->str, args);
  wp->value = value;
  printf("Watchpoint %d: %s\n", wp->NO, wp->str);

  return 0;
}

static int cmd_d(char *args) {
  int no;
  if(!args){
    return 0;
  }
  no = atoi(args);
  if(!no){
    return 0;
  }
  if(!delete_wp(no)) {
    printf("fail to delete watchpoint %d\n", no);
  }
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "si", "\"si [N]\"\n\t Execute N instructions of the program and then suspend it(default N = 1)", cmd_si },
  { "info", "\"info SUBCMD\"\n\t Info r: print register status; info w:print watchpoints' informations", cmd_info },
  { "p", "\"p EXPR\"\n\t Find the value of the expression EXPR", cmd_p },
  { "x", "\"x N EXPR\"\n\t Find the value of the expression EXPR, the result as the starting memory address, in the form of hexadecimal output of the four N bytes", cmd_x },
  { "w", "\"w EXPR\"\n\t When the value of the expression EXPR changes, the program is suspended", cmd_w },
  { "d", "\"d N\"\n\t Delete the watchpoint with the serial number N", cmd_d },
  { "q", "Exit NEMU", cmd_q }

  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
