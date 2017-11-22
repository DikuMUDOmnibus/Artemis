/*
 * race.c: racial support functions. Mainly used by interpreter.c (nanny).
 *
 * 28/1/96 by Stuart Lamble (Danae@Artemis mud)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "structs.h"
#include "utils.h"

const char *race_abbrevs[] = {
  "Hum",
  "Elf",
  "Gnm",
  "Hlg"
};

const char *race_types[] = {
  "Human",
  "Elf",
  "Gnome",
  "Halfling",
};

const char *race_menu =
"\r\n"
"Select a race:\r\n"
"  [H]uman\r\n"
"  [E]lf\r\n"
"  [G]nome\r\n"
"  h[A]lfling\r\n"
"? for help\r\n";

const char *race_help =
"\r\n"
"Humans : are a general race, with no special attributes. They have no\r\n"
"         no restrictions with regards to their class.\r\n"
"Elves  : Pre-eminent magical creatures, they suffer from losses to their\r\n"
"         constitution, and can only be magic users or clerics.\r\n"
"Gnomes : As small, but highly agile, creatures, these creatures can only\r\n"
"         be thieves or clerics.\r\n"
"You figure the rest out\r\n";

int parse_race(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
    case 'h' : return RACE_HUMAN;
    case 'e' : return RACE_ELF;
    case 'g' : return RACE_GNOME;
    case 'a' : return RACE_HALFLING;
    case '?' : return RACE_HELP;
    default  : return RACE_UNDEFINED;
  }
}

/* bitvectors...dunno where they're used atm, but classes use them, so... :) */

long find_race_bitvector(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
    case 'h' : return (1 << 0);
    case 'e' : return (1 << 1);
    case 'g' : return (1 << 2);
    case 'a' : return (1 << 3);
    default  : return 0;
  }
}

/* Only *ever* call this once - when the character is being created! */
void do_racial_shifts(struct char_data *ch)
{
  switch (GET_RACE(ch))
  {
/* stick in the racial attrib shifts (eg, int +3, cha -2, etc) here */
    case RACE_HUMAN : break;
    case RACE_ELF   : ch->real_abils.con   -= 3;
		      ch->real_abils.intel += 2;
		      ch->real_abils.wis   += 1;
                      ch->real_abils.str   -= 2;
		      break;
    case RACE_GNOME : ch->real_abils.intel -= 1;
		      ch->real_abils.str   -= 3;
		      ch->real_abils.dex   += 2;
		      ch->real_abils.cha   -= 2;
		      break;
    case RACE_HALFLING : ch ->real_abils.con += 2;
                         ch ->real_abils.dex += 1;
                         ch ->real_abils.str -= 1;
    default         :
	logs("SYSERR: Invalid race in char passed to do_racial_shifts()");
  }
/* Make sure they don't have too low/high stats. */
  ch->real_abils.con   = MAX(3, MIN(19, ch->real_abils.con  ));
  ch->real_abils.intel = MAX(3, MIN(19, ch->real_abils.intel));
  ch->real_abils.dex   = MAX(3, MIN(19, ch->real_abils.dex  ));
  ch->real_abils.str   = MAX(3, MIN(19, ch->real_abils.str  ));
  ch->real_abils.wis   = MAX(3, MIN(19, ch->real_abils.wis  ));
  ch->real_abils.cha   = MAX(3, MIN(19, ch->real_abils.cha  ));
}

int check_valid_combo(int race, int class)
{
/* check for race/class combos that are ok: if ok, return 1; otherwise, 0 */
bool combos[NUM_RACES][NUM_CLASSES] = {
 /*            Mu Cl Th Wa  */
 /* Human */ { 1, 1, 1, 1 },
 /* Elf   */ { 1, 1, 0, 0 },
 /* Gnome */ { 0, 1, 1, 0 },	
 /* Halfing*/ { 0, 0, 1, 1 }
   };
   if ((class < 0) || (class >= NUM_CLASSES))
     return 1; /* assume ok */
   if ((race < 0) || (race >= NUM_RACES))
    {
  logs("SYSERR: Unknown race passed to check_valid_combo()");
  return 1; /* assume ok */
    }
    return combos[race][class];
}
