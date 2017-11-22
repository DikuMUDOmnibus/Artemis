%{
/*
 * Lexical scanner: Reads in from hosts.deny the IP addresses,
 * time (opt), days (opt), max level (opt), and excuse (opt),
 * storing them for denying connections.
 *
 * 960419-960422 Stuart Lamble <lamble@yoyo.cc.monash.edu.au>
 */
/*
 * Beware: this is only tested insofar as the connections are
 * actually denied properly. Whether or not it is stable, or
 * leaks memory, or other nasty things remains to be seen...
 * YOU HAVE BEEN WARNED.
 */

int state;

#include "states.h"
#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"

extern struct ban_list_element *ban_list;

/*
 * I'd prefer not to make this global, but it's the only way
 * I can think of under lex - and it's less convenient to write
 * this without lex.
 */
struct ban_list_element *current = NULL;

void store_current(void);
void deal_with_day(char *text);
void deal_with_excuse(char *text);
void deal_with_number(char *text);
void deal_with_range(char *text);
void deal_with_asterix(char *text);

%}
%%
^ip[ \t]*=[ \t]*	{ store_current(); state = IP;	}
^time[ \t]*=[ \t]*	{ state = TIME;			}
^days[ \t]*=[ \t]*	{ state = DAY;			}
^level[ \t]*=[ \t]*	{ state = LEVEL;		}
^excuse[ \t]*=[ \t]*	{ state = EXCUSE;		}

[0-9]+				{ deal_with_number(yytext);	}
\[[0-9]+[ \t]*-[0-9]+[ \t]*\]	{ deal_with_range(yytext);	}
[0-9]+[ \t]*-[0-9]+[ \t]*	{ deal_with_range(yytext);	}

\*			{ deal_with_asterix(yytext);	}

mon |
tue |
wed |
thu |
fri |
sat |
sun			{ deal_with_day(yytext);	}

#.*\n			; /* ignore comments */

[ \t]*\n		; /* and whitespace */

[ \t]*			;

.			{ deal_with_excuse(yytext);	}
%%
/* support code */
int current_ip_no;
int have_stored_day = 0;
int have_stored_excuse = 0;

void parse_excuse(char *text);

int yywrap() {
  if (current) {
    parse_excuse(NULL);
    current->next = ban_list;
    ban_list = current;
    current = NULL;
    have_stored_day = 0;
    have_stored_excuse = 0;
  }
  return 1;
}

void store_current(void) {
  current_ip_no = 0;
  have_stored_day = 0;
  have_stored_excuse = 0;
  if (current) {
    parse_excuse(NULL);
    current->next = ban_list;
    ban_list = current;
  }

  current = malloc(sizeof(struct ban_list_element));
  memset(current->ipfrom, 0, 4);
  memset(current->ipto, 0, 4);
  current->starttime = 0;
  current->endtime = 86399;
  current->days = 1 + 2 + 4 + 8 + 16 + 32 + 64; /* Mon-Sun */
  current->level = LVL_IMPL; /* Default: everybody. */
  current->reason = malloc(11);
  strcpy(current->reason, "Go away.\r\n");
}

void parse_day(char *text) {
  int val;
  char days[7][4] = {
	"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
  };
  static char from[4];
  int first_day = -1, second_day = -1;
  int loop;

  while ((val = input()) == ' ' || val == '\t');
  switch (val) {
    case ',' : break;
    case '-' :
	if (state == DAY2) {
	  fprintf(stderr, "Invalid input to days: two dashes?\n");
	  return;
	}
	memcpy(from, text, 3);
	from[4] = 0;
	state = DAY2;
	return;
    case '\n': unput(val); break;
    case '#' : unput(val); break;
    default  : fprintf(stderr, "Invalid input to days: %s %c\n", text, val);
		unput(val);
		return;
  }
  for (loop = 0; loop < 7; loop++) {
    if (!strncasecmp(days[loop], text, 3))
      first_day = loop;
    if ((state == DAY2) && (!strncasecmp(days[loop], from, 3)))
      second_day = loop;
  }
  if (first_day == -1) {
    fprintf(stderr, "SYSERR: Invalid day %s?\n", text);
    return;
  }
  if ((state == DAY2) && (second_day == -1)) {
    fprintf(stderr, "SYSERR: Invalid day %s?\n", from);
    return;
  }
  if (!have_stored_day)
    current->days = 0;
  have_stored_day = 1;
  for (loop = ((state == DAY2)?second_day:first_day);
       loop != first_day;
       loop = (loop + 1) % 7)
    current->days |= (1 << loop);
  current->days |= (1 << first_day); /* just in case */
}

void parse_excuse(char *text) {
  static int slack;
  int newsize;
  int loop;
  int val = 0;
  char the_line[256]; /* unlikely to get longer than that. */

  if (!text) { /* free all the slack space. */
    current->reason = realloc(current->reason, strlen(current->reason) + 1);
    slack = 0;
    return;
  }
  for (loop = 0; loop < strlen(text); loop++)
    the_line[loop] = text[loop];
  for (; (loop < 80) && ((val = input()) != '\n'); loop++)
    the_line[loop] = val;
  if (val == '\n') {
    the_line[loop++] = '\r';
    the_line[loop++] = '\n';
    unput('\n');
  }
  the_line[loop] = 0;
  if (!have_stored_excuse) {
    newsize = strlen(the_line) + 1;
    slack = 80 - (newsize % 80);
    current->reason = realloc(current->reason, newsize + slack);
    *current->reason = 0;
    have_stored_excuse = 1;
  } else {
    if (slack < strlen(the_line)) {
      slack += 80;
      current->reason = realloc(current->reason, strlen(current->reason) + slack + 1);
    }
    slack -= strlen(the_line);
  }
  strcat(current->reason, the_line);
}

void set_level(int level) {
  if ((level < 0) || (level > LVL_IMPL)) {
    fprintf(stderr, "SYSERR: Level %d too high in hosts.deny: ignoring.\n", level);
    return;
  }
  current->level = level;
}

void parse_ip_number(int num) {
  int val;

  if (current_ip_no < 3) {
    if ((val = input()) != '.') { /* MUST be a dot. */
      fprintf(stderr, "SYSERR: Invalid IP address in hosts.deny: trying to recover.\n");
      unput(val);
    }
  }
  if (current_ip_no > 3) {
    fprintf(stderr, "SYSERR: Invalid IP address in hosts.deny, ignoring extra field(s).\n");
    return;
  }
  if ((num < 0) || (num > 255)) {
    fprintf(stderr, "SYSERR: IP field %d (value %d) too large in hosts.deny. Ignoring.\n",
	current_ip_no, num);
    return;
  }
  current->ipfrom[current_ip_no] = num;
  current->ipto[current_ip_no] = num;
  current_ip_no++;
}

void parse_ip_range(char *text) {
/* [num-num] */
  int from, to;

  if (current_ip_no < 3) {
    if (input() != '.') { /* MUST be a dot. */
      fprintf(stderr, "Not a dot in ip!?\n");
    }
  }
  if (current_ip_no > 3) {
    fprintf(stderr, "SYSERR: Invalid IP address in hosts.deny, ignoring extra field(s).\n");
    return;
  }
  if (sscanf(text, "[%d-%d]", &from, &to) != 2)
    if (*text == '*') {
      from = 0;
      to = 255;
    }
    else {
      fprintf(stderr, "SYSERR: Invalid IP field %s in hosts.deny, ignoring.\n", text);
      return;
    }

  if ((from > to) || (from < 0) || (from > 255) || (to < 0) || (to > 255)) {
    fprintf(stderr, "SYSERR: Invalid IP field %s in hosts.deny, ignoring.\n", text);
    return;
  }

  current->ipfrom[current_ip_no] = from;
  current->ipto[current_ip_no] = to;
  current_ip_no++;
}

void set_time(char *text) {
  int hour1, hour2, minute1, minute2;
  int val1, val2;

  sscanf(text, "%d-%d", &val1, &val2);
  hour1 = val1 / 100;
  minute1 = val1 % 100;
  hour2 = val2 / 100;
  minute2 = val2 % 100;

  if ((hour1 < 0) || (hour1 > 23) || (hour2 < 0) || (hour2 > 23) ||
      (minute1 < 0) || (minute1 > 59) || (minute2 < 0) || (minute2 > 59)) {
    fprintf(stderr, "SYSERR: Time range %s invalid in hosts.deny: ignoring.\n", text);
    return;
  }
  if ((hour1 > hour2) || ((hour1 == hour2) && (minute1 > minute2))) {
    fprintf(stderr, "SYSERR: Time range %s invalid in hosts.deny: ignoring.\n", text);
    return;
  }
  current->starttime = (hour1 * 60 + minute1) * 60;
  current->endtime = (hour2 * 60 + minute2 + 1) * 60 - 1;
}

void deal_with_day(char *text) {
  if ((state != DAY) && (state != EXCUSE) && (state != DAY2)) {
    fprintf(stderr, "Invalid state for day in hosts.deny\n");
  } else {
    if (state == EXCUSE)
      parse_excuse(text);
    else
      parse_day(text);
  }
}

void deal_with_excuse(char *text) {
  if (state != EXCUSE) {
    fprintf(stderr, "Invalid text %s in hosts.deny?\n", text);
  } else {
    parse_excuse(text);
  }
}

void deal_with_asterix(char *text) {
  if (state == EXCUSE)
    return parse_excuse(text);
  if (state == IP)
    return parse_ip_range(text);
  fprintf(stderr, "Invalid asterix in hosts.deny: %s\n", text);
}

void deal_with_number(char *text) {
  int val = atoi(text);

  switch (state) {
    case DAY    :
    case DAY2	:
      fprintf(stderr, "Got number, expected day: %s.\n", text);
      break;
    case EXCUSE :
      parse_excuse(text);
      break;
    case TIME   : fprintf(stderr, "Got number, expected a time range: %s\n", text); break;
    case LEVEL  :
      set_level(val);
      break;
    case IP     :
      parse_ip_number(val);
      break;
  }
}

void deal_with_range(char *text) {
  switch (state) {
    case TIME :
      set_time(yytext);
      break;
    case IP   :
      parse_ip_range(yytext);
      break;
    case EXCUSE :
      parse_excuse(yytext);
      break;
    default :
      fprintf(stderr, "Got range in invalid field: %s\n", text);
      break;
  }
}

/* ====== Pretty printing routines, to print out the banned list ====== */

char *sec_to_pr(int time) {
  int hours;
  int minutes;
  static char result[6];

  time /= 60;
  hours = (time / 60);
  minutes = (time % 60);
  sprintf(result, "%2d:%02d", hours, minutes);
  return result;
}

char *print_days(char days) {
  int loop;
  char d[7][4] = {
	"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
  };
  static char result[28];
  char *tmp = result;
  int have_printed = 0;

  for (loop = 0; loop < 7; loop++)
    if (days & (1 << loop)) {
      if (have_printed) {
	*tmp = ',';
	tmp++;
      }
      have_printed = 1;
      sprintf(tmp, "%s", d[loop]);
      tmp += strlen(tmp);
    }
  return result;
}

void print_node(struct ban_list_element *node, struct char_data *ch) {
  int loop;
  char buf[8];
  char result[80]; /* heaps */
  char *tmp = result;
  char line[80];

  for (loop = 0; loop < 4; loop++) {
    if (node->ipfrom[loop] != node->ipto[loop]) {
      if ((node->ipfrom[loop] == 0) && (node->ipto[loop] == 255))
        sprintf(buf, "*");
      else
        sprintf(buf, "%s", "[%d-%d]");
    } else
      sprintf(buf, "%s", "%d");
    sprintf(tmp, buf, node->ipfrom[loop], node->ipto[loop]);
    tmp += strlen(tmp);
    if (loop < 3) {
      *tmp = '.';
      tmp++;
    }
  }
  /* The 27 seems to be a reasonable worst-case, and lets it all format
   * nicely in 80 columns.
   */
  tmp = line;
  sprintf(tmp, "%-27s %s ", result, sec_to_pr(node->starttime));
  tmp += strlen(tmp);
  sprintf(tmp, "%s %5d %s\r\n", sec_to_pr(node->endtime), node->level,
	print_days(node->days));
  send_to_char(line, ch);
}

void print_list(struct char_data *ch) {
  struct ban_list_element *curr;

  send_to_char("\
ip                          start end   level days\r\n\
                            time  time\r\n\
--------------------------- ----- ----- ----- ---------------------------\r\n", ch);

  for (curr = ban_list; curr; curr = curr->next)
    print_node(curr, ch);
}
