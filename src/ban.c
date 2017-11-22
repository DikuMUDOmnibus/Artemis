/* ************************************************************************
*   File: ban.c                                         Part of CircleMUD *
*  Usage: banning/unbanning/checking sites and player names               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <netdb.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"

struct ban_list_element *ban_list = NULL;
struct ban_list_element *temp_bans = NULL;

extern FILE *yyin;
void yylex(void);

void load_banned(void)
{
  FILE *fl;

  ban_list = 0;

  if (!(fl = fopen(BAN_FILE, "r"))) {
    perror("Unable to open banfile");
    return;
  }
  yyin = fl;
  yyrestart(fl);
  yylex();

  fclose(fl);
  temp_bans = ban_list;
}

void reload_banned(void)
{
  struct ban_list_element *curr = ban_list;

  for (curr = temp_bans; curr; curr = temp_bans) {
    temp_bans = temp_bans->next;
    free(curr->reason);
    free(curr);
  }
  load_banned();
}

char *isbanned(unsigned char *ip, int level)
{
  struct ban_list_element *banned_node;
  struct tm *local_time;
  time_t current_time;
  char *asct;
  char days[7][4] = {
	"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
  };
  int daymask = 0;
  int secs;

  current_time = time(NULL);
  local_time = localtime(&current_time);
  asct = asctime(local_time);
  asct[3] = 0;

  for (secs = 0; secs < 7; secs++)
    if (!strcmp(asct, days[secs]))
      daymask = (1 << secs);

  secs = local_time->tm_sec + (local_time->tm_min +
	(local_time->tm_hour) * 60) * 60;

  for (banned_node = temp_bans; banned_node; banned_node = banned_node->next) {
    /* Is it in hours? */
    if ((secs < banned_node->starttime) || (secs > banned_node->endtime))
      continue; /* Nope. */
    /* Is it the right day? */
    if (!(banned_node->days & daymask))
      continue; /* Nope. */
    /* Is he coming from a banned IP? */
    if ((ip[0] < banned_node->ipfrom[0]) || (ip[0] > banned_node->ipto[0]) ||
	(ip[1] < banned_node->ipfrom[1]) || (ip[1] > banned_node->ipto[1]) ||
	(ip[2] < banned_node->ipfrom[2]) || (ip[2] > banned_node->ipto[2]) ||
	(ip[3] < banned_node->ipfrom[3]) || (ip[3] > banned_node->ipto[3]))
      continue; /* Nope. */
    /* Is his level <= the banned level? */
    if (level <= banned_node->level)
      return banned_node->reason; /* Yup. */
  }

  return NULL;
}

void print_list(struct char_data *ch);

int get_ip_addr(char *what, int *storeto, struct char_data *ch)
{
  int a, b, c, d;
  struct hostent *result = NULL;
  char buff[80];

  if (sscanf(what, "%d.%d.%d.%d", &a, &b, &c, &c) != 4) {
    result = gethostbyname(what);
    switch (h_errno) {
      case HOST_NOT_FOUND:
	send_to_char("No such host.\r\n", ch);
	return 1;
	break;
      case NO_ADDRESS:
	send_to_char("That host does not have an IP address.\r\n", ch);
	return 1;
	break;
      case 0 : break;
      default:
	send_to_char("Unknown error. :-(\r\n", ch);
	return 1;
	break;
    }
    a = (unsigned char) result->h_addr_list[0][0];
    b = (unsigned char) result->h_addr_list[0][1];
    c = (unsigned char) result->h_addr_list[0][2];
    d = (unsigned char) result->h_addr_list[0][3];
  }
  if ((a < 0) || (a > 255) || (b < 0) || (b > 255) ||
      (c < 0) || (c > 255) || (d < 0) || (d > 255)) {
    sprintf(buff, "%d.%d.%d.%d is not a valid IP address.\r\n",
	a, b, c, d);
    send_to_char(buff, ch);
    return 1;
  }
  storeto[0] = a;
  storeto[1] = b;
  storeto[2] = c;
  storeto[3] = d;
  return 0;
}

ACMD(do_ban)
{
  struct ban_list_element *new_ban = NULL;
  int ip[4];

  skip_spaces(&argument);
  if (!*argument) {
    if (!ban_list) {
      send_to_char("No sites are banned.\r\n", ch);
      return;
    }
    print_list(ch);
    return;
  }
  half_chop(argument, arg, buf);
  if (!*arg) {
    send_to_char("Usage: ban <ip address|hostname> <reason>\r\n\
24 hours, level 300, every day is assumed.\r\n", ch);
    return;
  }

  if (get_ip_addr(arg, ip, ch))
    return;
  
  new_ban = malloc(sizeof(struct ban_list_element));
  new_ban->starttime = 0;
  new_ban->endtime = 86400;
  new_ban->level = 300;
  new_ban->days = 1 + 2 + 4 + 8 + 16 + 32 + 64;
  new_ban->reason = malloc(strlen(buf) + 3);
  strcpy(new_ban->reason, buf);
  strcat(new_ban->reason, "\r\n");
  new_ban->next = temp_bans;
  new_ban->ipfrom[0] = new_ban->ipto[0] = ip[0] & 255;
  new_ban->ipfrom[1] = new_ban->ipto[1] = ip[1] & 255;
  new_ban->ipfrom[2] = new_ban->ipto[2] = ip[2] & 255;
  new_ban->ipfrom[3] = new_ban->ipto[3] = ip[3] & 255;
  temp_bans = new_ban;
  send_to_char("Done.\r\n", ch);
}


ACMD(do_unban)
{
  send_to_char("Sorry, but you can't unban sites interactively. Edit hosts.deny instead.\r\n", ch);
}


/**************************************************************************
 *  Code to check for invalid names (i.e., profanity, etc.)		  *
 *  Written by Sharon P. Goza						  *
 **************************************************************************/

typedef char namestring[MAX_NAME_LENGTH];

namestring *invalid_list = NULL;
int num_invalid = 0;

int Valid_Name(char *newname)
{
  int i;

  char tempname[MAX_NAME_LENGTH];

  /* return valid if list doesn't exist */
  if (!invalid_list || num_invalid < 1)
    return 1;

  /* change to lowercase */
  strcpy(tempname, newname);
  for (i = 0; tempname[i]; i++)
    tempname[i] = LOWER(tempname[i]);

  /* Does the desired name contain a string in the invalid list? */
  for (i = 0; i < num_invalid; i++)
    if (strstr(tempname, invalid_list[i]))
      return 0;

  return 1;
}


void Read_Invalid_List(void)
{
  FILE *fp;
  int i = 0;
  char string[80];

  if (!(fp = fopen(XNAME_FILE, "r"))) {
    perror("Unable to open invalid name file");
    return;
  }
  /* count how many records */
  while (fgets(string, 80, fp) != NULL && strlen(string) > 1)
    num_invalid++;

  rewind(fp);

  CREATE(invalid_list, namestring, num_invalid);

  for (i = 0; i < num_invalid; i++) {
    fgets(invalid_list[i], 80, fp);	/* read word */
    invalid_list[i][strlen(invalid_list[i]) - 1] = '\0'; /* cleave off \n */
  }

  fclose(fp);
}
