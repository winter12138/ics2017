#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <string.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NEQ, TK_AND, TK_OR, TK_NOT, TK_DEC, TK_HEX, TK_NEG, TK_DEREF, TK_REG

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\$e([abcd]x|[sd]i|[sb]p|ip)", TK_REG},   // hexadecimal
  {"0[xX][0-9a-fA-F]+", TK_HEX},   // hexadecimal
  {"[0-9]+", TK_DEC},   // decimal
  {"\\(", '('},         // left bracket
  {"\\)", ')'},         // right bracket
  {"\\+", '+'},         // plus
  {"\\-", '-'},         // minus
  {"\\*", '*'},         // multiply
  {"/", '/'},           // divide
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},       // not equal
  {"&&", TK_AND},       // and
  {"\\|\\|", TK_OR},        // or
  {"!", TK_NOT},        // not
};
#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  union {
    int priority;
    char str[32];
  };
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        if(TK_NOTYPE == rules[i].token_type) {
          continue;
        }
        if (nr_token >= 32) {
          printf("too many tokens\n");
          return false;
        }
        switch (rules[i].token_type) {
          case TK_NOT: case TK_NEG: case TK_DEREF: {
            tokens[nr_token].priority = 1;
            break;
          }
          case '*': case '/': {
            tokens[nr_token].priority = 2;
            break;
          }
          case '+': case '-': {
            tokens[nr_token].priority = 3;
            break;
          }
          case TK_EQ: case TK_NEQ: {
            tokens[nr_token].priority = 4;
            break;
          }
          case TK_AND: {
            tokens[nr_token].priority = 5;
            break;
          }
          case TK_OR: {
            tokens[nr_token].priority = 6;
            break;
          }
          case TK_DEC: case TK_HEX: case TK_REG: {
            if (substr_len >= 32) {
              printf("number too long(>=32)\n");
              return false;
            }
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            break;
          }
        }
        tokens[nr_token].type = rules[i].token_type;
        ++nr_token;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

enum { BADEXPR = -3, NOTSURD, NOTMATCH};

int check_parentheses(int p, int q) {
  int i, cnt = 0;
  int notmatch = 0, notsurd = 0;
  if('(' != tokens[p].type || ')' != tokens[q].type) {
    notsurd = 1;
  }
  for (i = p; i <= q; ++i) {
    if('(' == tokens[i].type) {
      ++cnt;
    } else if(')' == tokens[i].type) {
      --cnt;
      if(cnt < 0) {
        return BADEXPR;
      } else if(0 == cnt && i < q) {
        notmatch = 1;
      }
    }
  }
  if(0 != cnt) {
    return BADEXPR;
  }
  if(notsurd) {
    return NOTSURD;
  }
  if(notmatch) {
    return NOTMATCH;
  }
  return true;
}

static int eval_flag  = 0;
int eval(int p, int q) {
  int i, a, check_result;
  int cnt, op, val1, val2;
  if(BADEXPR == eval_flag) {
    return 0;
  }
  if(p > q) {

    eval_flag = BADEXPR;
    return 0;
  } else if (p == q) {

    sscanf(tokens[p].str, "%i", &a);
    return a;
  } else {

    check_result = check_parentheses(p, q);
    if(true == check_result) {

      return eval(p + 1, q - 1);
    } else if(BADEXPR == check_result) {

      eval_flag = BADEXPR;
      return 0;
    } else {

      op = -1;
      cnt = 0;

      for (i = p; i <= q; ++i)
      {
        switch(tokens[i].type) {
          case '(': {
            ++cnt;
            break;
          }
          case ')': {
            --cnt;
            break;
          }
          case TK_NOT: case TK_NEG: case TK_DEREF: {
            if(0 == cnt && -1 == op) {
              op = i;
            }
            break;
          }
          case '*': case '/': {
            if(0 == cnt && (-1 == op 
              || tokens[i].priority >= tokens[op].type
              )) {
              op = i;
            }
            break;
          }
          case '+': case '-':{
            if(0 == cnt && (-1 == op 
              || tokens[i].priority >= tokens[op].type
              )) {
              op = i;
            }
            break;
          }
          case TK_EQ: case TK_NEQ: {
            if(0 == cnt && (-1 == op 
              || tokens[i].priority >= tokens[op].type
              )) {
              op = i;
            }
            break;
          }
          case TK_AND: {
            if(0 == cnt && (-1 == op 
              || tokens[i].priority >= tokens[op].type
              )) {
              op = i;
            }
            break;
          }
          case TK_OR: {
            if(0 == cnt && (-1 == op 
              || tokens[i].priority >= tokens[op].type
              )) {
              op = i;
            }
            break;
          }
        }
      }

      if(TK_NOT == tokens[op].type
        || TK_NEG == tokens[op].type
        || TK_DEREF == tokens[op].type
        ) {
        val1 = 0;
        val2 = eval(op + 1, q);
      } else {
        val1 = eval(p, op - 1);
        val2 = eval(op + 1, q);
      }

      switch(tokens[op].type) {
        case TK_NOT: return !val2;
        case TK_NEG: return -val2;
        case TK_DEREF: {
          val2 = vaddr_read(val2, 4);
          return val2;
        }
        case '+': return val1 + val2;
        case '-': return val1 - val2;
        case '*': return val1 * val2;
        case '/': return val1 / val2;
        case TK_EQ:   return val1 == val2;
        case TK_NEQ:  return val1 != val2;
        case TK_AND:  return val1 && val2;
        case TK_OR:   return val1 || val2;
        default: {
          eval_flag = BADEXPR;
          return 0;
        }
      }

    }
  }
}
uint32_t expr(char *e, bool *success) {
  int i, ans;
  if (!e || !make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
  for (i = 0; i < nr_token; ++i) {
    if (tokens[i].type == '-' 
        && (i == 0 
        || (tokens[i-1].type != TK_DEC && tokens[i-1].type != ')')
        )) {
      tokens[i].type = TK_NEG;
    } else if (tokens[i].type == '*' 
        && (i == 0 
        || (tokens[i-1].type != TK_DEC && tokens[i-1].type != ')')
        )) {
      tokens[i].type = TK_DEREF;
    }
  }

  eval_flag = 0;
  ans = eval(0, nr_token-1);
  if(BADEXPR == eval_flag) {
    *success = false;
    return 0;
  } else {
    *success = true;
    return ans;
  }
}
