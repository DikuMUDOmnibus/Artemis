/**************************************************************************
*   File: class.c                                       Part of CircleMUD *
*  Usage: Source file for class-specific code                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
 * This file attempts to concentrate most of the code which must be changed
 * in order for new classes to be added.  If you're adding a new class,
 * you should go through this entire file from beginning to end and add
 * the appropriate new special cases for your new class.
 */



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "structs.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "interpreter.h"


/* Names first */

const char *class_abbrevs[] = {
  "Mu",
  "Cl",
  "Th",
  "Wa",
  "\n"
};


const char *pc_class_types[] = {
  "Magic User",
  "Cleric",
  "Thief",
  "Warrior",
  "\n"
};


/* The menu for choosing a class in interpreter.c: */
const char *class_menu =
"\r\n"
"Select a class:\r\n"
"  [C]leric\r\n"
"  [T]hief\r\n"
"  [W]arrior\r\n"
"  [M]agic-user\r\n";



/*
 * The code to interpret a class letter -- used in interpreter.c when a
 * new character is selecting a class and by 'set class' in act.wizard.c.
 */

int parse_class(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
  case 'm':
    return CLASS_MAGIC_USER;
    break;
  case 'c':
    return CLASS_CLERIC;
    break;
  case 'w':
    return CLASS_WARRIOR;
    break;
  case 't':
    return CLASS_THIEF;
    break;
  default:
    return CLASS_UNDEFINED;
    break;
  }
}

/*
 * bitvectors (i.e., powers of two) for each class, mainly for use in
 * do_who and do_users.  Add new classes at the end so that all classes
 * use sequential powers of two (1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4,
 * 1 << 5, etc.
 */

long find_class_bitvector(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
    case 'm':
      return (1 << 0);
      break;
    case 'c':
      return (1 << 1);
      break;
    case 't':
      return (1 << 2);
      break;
    case 'w':
      return (1 << 3);
      break;
    default:
      return 0;
      break;
  }
}


/*
 * These are definitions which control the guildmasters for each class.
 *
 * The first field (top line) controls the highest percentage skill level
 * a character of the class is allowed to attain in any skill.  (After
 * this level, attempts to practice will say "You are already learned in
 * this area."
 * 
 * The second line controls the maximum percent gain in learnedness a
 * character is allowed per practice -- in other words, if the random
 * die throw comes out higher than this number, the gain will only be
 * this number instead.
 *
 * The third line controls the minimu percent gain in learnedness a
 * character is allowed per practice -- in other words, if the random
 * die throw comes out below this number, the gain will be set up to
 * this number.
 * 
 * The fourth line simply sets whether the character knows 'spells'
 * or 'skills'.  This does not affect anything except the message given
 * to the character when trying to practice (i.e. "You know of the
 * following spells" vs. "You know of the following skills"
 */

#define SPELL   0
#define SKILL   1

/* #define LEARNED_LEVEL        0  % known which is considered "learned" */
/* #define MAX_PER_PRAC         1  max percent gain in skill per practice */
/* #define MIN_PER_PRAC         2  min percent gain in skill per practice */
/* #define PRAC_TYPE            3  should it say 'spell' or 'skill'?    */

int prac_params[4][NUM_CLASSES] = {
  /* MAG        CLE     THE     WAR */
  {95,          95,     90,     85},            /* learned level */
  {50,         50,    10,     10},            /* max per prac */
  {15,          15,     0,      0,},            /* min per pac */
  {SPELL,       SPELL,  SKILL,  SKILL}          /* prac name */
};


/*
 * ...And the appropriate rooms for each guildmaster/guildguard; controls
 * which types of people the various guildguards let through.  i.e., the
 * first line shows that from room 3017, only MAGIC_USERS are allowed
 * to go south.
 */
int guild_info[][3] = {

/* Midgaard */
  {CLASS_MAGIC_USER,    3017,   SCMD_SOUTH},
  {CLASS_CLERIC,        3004,   SCMD_NORTH},
  {CLASS_THIEF,         3027,   SCMD_EAST},
  {CLASS_WARRIOR,       3021,   SCMD_EAST},

/* Brass Dragon */
  {-999 /* all */ ,     5065,   SCMD_WEST},
  {-999, 20116, SCMD_NORTH},
  {-999, 20114, SCMD_SOUTH},

/* New Sparta */
  {CLASS_MAGIC_USER,    21075,  SCMD_NORTH},
  {CLASS_CLERIC,        21019,  SCMD_WEST},
  {CLASS_THIEF,         21014,  SCMD_SOUTH},
  {CLASS_WARRIOR,       21023,  SCMD_SOUTH},

/* this must go last -- add new guards above! */
{-1, -1, -1}};




/* THAC0 for classes and levels.  (To Hit Armor Class 0) */

 /* [class], [level] (all)  */
 const int thaco[NUM_CLASSES][350 + 1] = {
/* MAGE */
{ 100, 20, 20, 20, 20, 20, 20, 19, 19, 19,          19, 19, 19, 19, 19, 19, 18, 18, 18, 18,
18, 18, 18, 18, 17, 17, 17, 17, 17, 17,             17, 17, 17, 16, 16, 16, 16, 16, 16, 16,
16, 16, 15, 15, 15, 15, 15, 14, 14, 14,             13, 13, 13, 12, 12, 12, 12, 12, 12, 11, 
11, 11, 11, 11, 11, 11, 11, 10, 10, 10,             10, 10, 10, 9, 8, 7, 6, 5, 4, 3,
2, 1, 0, -1, -2, -3, -5, -9, -13, -16,           -25, -36, -45, -46, -47, -47, -47, -47, -47, -47,
-47, -47, -47, -47, -47, -48, -48, -49, -50, -50, 
-50, -51, -51, -51, -52, -52, -53, -53, -54, -54,
-54, -54, -54, -55, -56, -57, -58, -58, -58, -59, 
-59, -60, -61, -62, -63, -64, -65, -66, -67, -68,
-69, -70, -70, -70, -70, -71, -72, -73, -74, -75, 
-76, -77, -78, -79, -80, -81, -82, -83, -83, -83,
-83, -83, -84, -84, -85, -86, -87, -88, -89, -90, 
-91, -92, -93, -94, -95, -96, -97, -98, -99, -100, 
-101, -102, -103, -104, -105, -106, -107, -108, -109, -110, 
-111, -112, -113, -114, -115, -116, -117, -118, -119, -120,
-120, -120, -120, -120, -120, -120, -120, -120, -120, -120, 
-120, -120, -120, -120, -120, -120, -120, -120, -120, -120, 
-120, -120, -120, -120, -120, -120, -120, -120, -120, -120, 
-120, -120, -120, -120, -120, -120, -120, -120, -120, -120, 
-120, -120, -120, -120, -120, -120, -120, -120, -120, -120, 
-120, -120, -120, -120, -120, -120, -120, -120, -120, -120, 
-120, -120, -120, -120, -120, -120, -120, -120, -120, -120, 
-120, -120, -120, -120, -120, -120, -120, -120, -120, -120, 
-120, -120, -120, -120, -120, -120, -120, -120, -120, -120, 
-120, -120, -120, -120, -120, -120, -120, -120, -120, -120 
},

/* CLERIC */
{ 100, 20, 20, 20, 20, 19, 19, 19, 19, 19,          19, 19, 19, 18, 18, 18, 18, 17, 17, 17,
17, 16, 16, 16, 16, 16, 15, 15, 14, 14,             14, 14, 14, 14, 13, 13, 12, 12, 11, 11,
11, 10, 10, 9, 9, 9, 9, 9, 9, 9,                    8, 8, 8, 8, 8, 8, 8, 7, 7, 
6, 6, 5, 5, 4, 4, 4, 4, 4, 4,                       4, 3, 0, 0, -5, -7, -9, -11, -13, -14, 
-16, -18, -21, -23, -24, -25, -26, -28, -29, -32, 
-33, -34, -35, -36, -36, -37, -37, -38, -38, -39, 
-39, -40, -40, -40, -41, -42, -43, -44, -45, -46,
-48, -50, -52, -54, -56, -58, -60, -62, -64, -66,
-68, -70, -72, -74, -76, -78, -80, -82, -84, -86,
-88, -90, -92, -94, -96, -98, -100, -101, -102, -103, 
-104, -105, -106, -107, -108, -109, -110, -111, -112, -113,
-113, -113, -113, -113, -113, -113, -113, -113, -113, -113,
-113, -113, -113, -113, -113, -113, -113, -113, -113, -113,
-113, -113, -113, -113, -113, -113, -113, -113, -113, -113,
-113, -113, -113, -113, -113, -113, -113, -113, -113, -113,
-113, -113, -113, -113, -113, -113, -113, -113, -113, -113,
-113, -113, -113, -113, -113, -113, -113, -113, -113, -113,
-113, -113, -113, -113, -113, -113, -113, -113, -113, -113,
-113, -113, -113, -113, -113, -113, -113, -113, -113, -113,
-113, -113, -113, -113, -113, -113, -113, -113, -113, -113,
-113, -113, -113, -113, -113, -113, -113, -113, -113, -113,
-113, -113, -113, -113, -113, -113, -113, -113, -113, -113,
-113, -113, -113, -113, -113, -113, -113, -113, -113, -113,
-113, -113, -113, -113, -113, -113, -113, -113, -113, -113,
-113, -113, -113, -113, -113, -113, -113, -113, -113, -113,
-113, -113, -113, -113, -113, -113, -113, -113, -113, -113
},

/* THIEF */
{ 100, 20, 20, 20, 20, 20, 20, 18, 18, 18,       18, 18, 18, 18, 18, 17, 17, 16, 16, 16, 
16, 16, 16, 16, 16, 14, 14, 14, 14, 14,          14, 14, 14, 12, 12, 12, 12, 12, 12, 12, 
12, 10, 10, 10, 10, 10, 9, 9, 8, 8,              8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 
4, 4, 4, 4, 4, 2, 2, 2, 2, 2,                    1, 1, 1, 1, 0, 0, 0, -2, -5, -10, 
-15, -20, -25, -30, -40, -45, -50, -55, -60, -65, 
-70, -75, -76, -77, -78, -80, -83, -85, -89, -90, 
-92, -93, -94, -96, -99, -100, -101, -102, -103, -104,
-105, -106, -107, -108, -109, -110, -111, -112, -113, -114,
-115, -116, -117, -118, -119, -119, -119, -119, -119, -119,
-115, -116, -117, -118, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119,
-119, -119, -119, -119, -119, -119, -119, -119, -119, -119
},

/* WARRIOR */
{ 100, 20, 19, 18, 17, 16, 15, 14, 13, 12,        11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 
1, 0, -1, -3, -5, -7, -9, -10, -13, -15,          
-17, -19, -19, -19, -19, -19, -19, -21, -22, -24, 
-27, -27, -27, -27, -27, -27, -29, -33, -37, -39, 
-42, -42, -42, -42, -42, -42, -43, -45, -47, -49, 
-51, -53, -55, -57, -58, -60, -61, -63, -65, -67, 
-69, -73, -75, -78, -79, -82, -84, -86, -88, -93, 
-94, -95, -97, -98, -99, -99, -99, -99, -99, -99, 
-99, -99, -99, -100, -101, -102, -103, -104, -105, -106, 
-107, -108, -109, -109, -109, -110, -111, -111, -112, -113,
-113, -114, -115, -115, -116, -117, -118, -119, -120, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121,
-121, -121, -121, -121, -121, -121, -121, -121, -121, -121
 }
};



/*
 * Roll the 6 stats for a character... each stat is made of the sum of
 * the best 3 out of 4 rolls of a 6-sided die.  Each class then decides
 * which priority will be given for the best to worst stats.
 *
 * Also ensures that the character gets the appropriate racial stat shifts.
 */
void roll_real_abils(struct char_data * ch)
{
  int i, j, k, temp;
  ubyte table[6];
  ubyte rolls[4];

  void do_racial_shifts(struct char_data *ch);

  for (i = 0; i < 6; i++)
    table[i] = 0;

  for (i = 0; i < 6; i++) {

    for (j = 0; j < 4; j++)
      rolls[j] = number(1, 6);

    temp = rolls[0] + rolls[1] + rolls[2] + rolls[3] -
      MIN(rolls[0], MIN(rolls[1], MIN(rolls[2], rolls[3])));

    for (k = 0; k < 6; k++)
      if (table[k] < temp) {
	temp ^= table[k];

	table[k] ^= temp;
	temp ^= table[k];
      }
  }

  ch->real_abils.str_add = 0;

  switch (GET_CLASS(ch)) {
  case CLASS_MAGIC_USER:
    ch->real_abils.intel = table[0];
    ch->real_abils.wis = table[1];
    ch->real_abils.str = table[2];
    ch->real_abils.con = table[3];
    ch->real_abils.dex = table[4];
    ch->real_abils.cha = table[5];
    break;
  case CLASS_CLERIC:
    ch->real_abils.wis = table[0];
    ch->real_abils.intel = table[1];
    ch->real_abils.str = table[2];
    ch->real_abils.con = table[3];
    ch->real_abils.dex = table[4];
    ch->real_abils.cha = table[5];
    break;
  case CLASS_THIEF:
    ch->real_abils.dex = table[0];
    ch->real_abils.str = table[1];
    ch->real_abils.con = table[2];
    ch->real_abils.intel = table[3];
    ch->real_abils.wis = table[4];
    ch->real_abils.cha = table[5];
     if (ch->real_abils.str == 18)
      ch->real_abils.str_add = number(0, 75);

    break;
  case CLASS_WARRIOR:
    ch->real_abils.str = table[0];
    ch->real_abils.con = table[1];
    ch->real_abils.dex = table[2];
    ch->real_abils.wis = table[3];
    ch->real_abils.intel = table[4];
    ch->real_abils.cha = table[5];
    if (ch->real_abils.str == 18)
      ch->real_abils.str_add = number(0, 100);
    break;
  }
  do_racial_shifts(ch);
  ch->aff_abils = ch->real_abils;
}


/* Some initializations for characters, including initial skills */
void do_start(struct char_data * ch)
{
  void advance_level(struct char_data * ch);

  GET_LEVEL(ch) = 1;
  GET_EXP(ch) = 1;

  set_title(ch, NULL);
/*
  roll_real_abils(ch);
*/
  ch->points.max_hit = 10;

  switch (GET_CLASS(ch)) {

  case CLASS_MAGIC_USER:
    break;

  case CLASS_CLERIC:
    break;

  case CLASS_THIEF:
    SET_SKILL(ch, SKILL_SNEAK, 10);
    SET_SKILL(ch, SKILL_HIDE, 5);
    SET_SKILL(ch, SKILL_STEAL, 15);
    SET_SKILL(ch, SKILL_BACKSTAB, 10);
    SET_SKILL(ch, SKILL_PICK_LOCK, 10);
    SET_SKILL(ch, SKILL_TRACK, 10);
    break;

  case CLASS_WARRIOR:
    break;
  }

  advance_level(ch);

  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);

  GET_COND(ch, THIRST) = 24;
  GET_COND(ch, FULL) = 24;
  GET_COND(ch, DRUNK) = 0;

  ch->player.time.played = 0;
  ch->player.time.logson = time(0);
}



/*
 * This function controls the change to maxmove, maxmana, and maxhp for
 * each class every time they gain a level.
 */
void advance_level(struct char_data * ch)
{
  int add_hp = 0, add_mana = 0, add_move = 0, i;

  extern struct wis_app_type wis_app[];
  extern struct con_app_type con_app[];

  add_hp = con_app[GET_CON(ch)].hitp;

  switch (GET_CLASS(ch)) {

  case CLASS_MAGIC_USER:
    add_hp += number(3, 8);
    add_mana = (sqrt(GET_LEVEL(ch))*(GET_INT(ch) / 2));
    add_mana = number(add_mana - (add_mana / 10), add_mana + (add_mana / 10));
    add_move = number(2, (int) (GET_DEX(ch))/3);
    break;

  case CLASS_CLERIC:
    add_hp += number(5, 10);
    add_mana = (sqrt(GET_LEVEL(ch))*(GET_WIS(ch))) / 2.5;
    add_mana = number(add_mana - (add_mana / 10), add_mana + (add_mana / 10));
    add_move = number(2, (int) (GET_DEX(ch))/3);
    break;

  case CLASS_THIEF:
    add_hp = (sqrt(GET_LEVEL(ch))*(GET_CON(ch) / 2.5));
    add_hp = number(add_hp - (add_hp / 10), add_hp + (add_hp / 10));
    add_mana = number(1, 5);
    add_move = number(4, (int) (GET_DEX(ch))/2);
    break;

  case CLASS_WARRIOR:
    add_hp = (sqrt(GET_LEVEL(ch))*(GET_CON(ch) / 2));
    add_hp = number(add_hp - (add_hp / 10), add_hp + (add_hp / 10));
    add_mana = number(1,5);
    add_move = number(2, (int) (GET_DEX(ch))/2);
    break;
  }

  ch->points.max_hit += MAX(1, add_hp);
  ch->points.max_move += MAX(1, add_move);

  if (GET_LEVEL(ch) > 1)
    ch->points.max_mana += add_mana;
/* BLECH! */
  if (GET_LEVEL(ch) >= 101)
    GET_EXP(ch) = 0;   

  if (GET_CLASS(ch) == CLASS_MAGIC_USER || GET_CLASS(ch) == CLASS_CLERIC)
    GET_PRACTICES(ch) += MAX(2, wis_app[GET_WIS(ch)].bonus);
  else
    GET_PRACTICES(ch) += MIN(2, MAX(1, wis_app[GET_WIS(ch)].bonus));

  if (GET_LEVEL(ch) >= LVL_IMMORT) {
    for (i = 0; i < 3; i++)
      GET_COND(ch, i) = (char) -1;
    SET_BIT(PRF_FLAGS(ch), PRF_HOLYLIGHT);
  } else
    REMOVE_BIT(PRF_FLAGS(ch), PRF_HOLYLIGHT);

  save_char(ch, NOWHERE);

  sprintf(buf, "%s advanced to level %d", GET_NAME(ch), GET_LEVEL(ch));
  mudlogs(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
}



/*
 * invalid_class is used by handler.c to determine if a piece of equipment is
 * usable by a particular class, based on the ITEM_ANTI_{class} bitvectors.
 */

int invalid_class(struct char_data *ch, struct obj_data *obj) {
  if ((IS_OBJ_STAT(obj, ITEM_ANTI_MAGIC_USER) && IS_MAGIC_USER(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC) && IS_CLERIC(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR) && IS_WARRIOR(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_THIEF) && IS_THIEF(ch)))
	return 1;
  else
	return 0;
}



/* Names of class/levels and exp required for each level */

const struct title_type titles[NUM_CLASSES][104 + 1] = {
{ {"the Man", "the Woman", 0 },
      { "the Apprentice of Magic", "the Apprentice of Magic", 1 },
      { "the Spell Student", "the Spell Student", 2000 },
      { "the Scholar of Magic", "the Scholar of Magic", 4000 },
      { "the Delver in Spells", "the Delveress in Spells", 8000 },
      { "the Medium of Magic", "the Medium of Magic", 16000 },
      { "the Scribe of Magic", "the Scribess of Magic", 32000 },
      { "the Seer", "the Seeress", 64000 },
      { "the Sage", "the Sage", 125000 },
      { "the Illusionist", "the Illusionist", 250000 },
      { "the Abjurer", "the Abjuress", 500000 },
      { "the Invoker", "the Invoker", 750000 },
      { "the Enchanter", "the Enchantress", 1000000 },
      { "the Conjurer", "the Conjuress", 1250000 },
      { "the Magician", "the Witch", 1500000 },
      { "the Creator", "the Creator", 1800000 },
      { "the Savant", "the Savant", 2100000 },
      { "the Magus", "the Craftess", 2400000 },
      { "the Wizard", "the Wizard", 2750000 },
      { "the Warlock", "the War Witch", 3100000 },
      { "the Sorcerer", "the Sorceress", 3500000 },
      { "the Necromancer", "the Necromancress", 3900000 },
      { "the Thaumaturge", "the Thaumaturgess", 4300000 },
 { "the Student of the Occult", "the Student of the Occult", 4800000 },
      { "the Disciple of the Uncanny", "the Disciple of the Uncanny", 5300000 },
      { "the Minor Elemental", "the Minor Elementress", 5900000 },
      { "the Greater Elemental", "the Greater Elementress", 6600000 },
      { "the Crafter of Magics", "the Crafter of Magics", 7300000 },
      { "the Shaman", "Shaman", 8100000 },
      { "the Keeper of Talismans", "the Keeper of Talismans", 9000000 },
      { "the Archmage", "Archwitch", 10000000 },
      { "the Grand Warlock", "the Grand Enchantress", 11000000 },
      { "the Vizer of Magic", "the Empress of Magic", 12000000 },
      { "the Sage of Magic", "the Lady of Magic", 13000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 14000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 15500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 17000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 18500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 20500000 },
    { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 22500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 25000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 27500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 30000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 33000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 36000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 40000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 44000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 49000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 55000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 65000000 },
   { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 75000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 85000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 95000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 105000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 115000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 125000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 135000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 145000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 155000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 165000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 175000000 }, 
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 190000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 205000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 220000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 235000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 250000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 265000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 280000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 295000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 310000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 325000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 342500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 360000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 377500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 395000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 412500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 430000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 447500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 465000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 482500000 },
    { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 500000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 522500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 545000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 567500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 590000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 612500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 635000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 657500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 680000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 702500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 725000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 752500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 780000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 807500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 835000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 862500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 890000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 917500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 945000000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 972500000 },
      { "the Guildmaster Of Magic", "the Guildmistress Of Magic", 1000000000 },
      { "the Immortal", "the Immortal", 1100000000      },
      { "the Avatar", "the Avatar", 1200000000      },
      { "the Lessor God", "the Lessor Goddess", 1300000000      },
      { "the Greater God", "the Greater Goddess", 1400000000      }
    },
   { { "the Man", "the Woman", 0 },
      { "the Believer", "the Believer", 1 },
      { "the Attendant", "the Attendant", 2000 },
      { "the Acolyte", "the Acolyte", 4000 },
      { "the Novice", "the Novice", 8000 },
      { "the Missionary", "the Missionary", 16000 },
      { "the Adept", "the Adept", 32000 },
      { "the Deacon", "the Deaconess", 64000 },
      { "the Vicar", "the Vicaress", 125000 },
      { "the Priest", "the Priestess", 250000 },
      { "the Minister", "the Lady Minister", 500000 },
      { "the Canon", "the Canon", 750000 },
      { "the Levite", "the Levitess", 1000000 },
      { "the Curate", "the Curess", 1250000 },
      { "the Monk", "the Nunne", 1500000 },
      { "the Healer", "the Healess", 1800000 },
      { "the Chaplain", "the Chaplain", 2100000 },
      { "the Expositor", "the Expositress", 2400000 },
      { "the Bishop", "the Bishop", 2750000 },
      { "the Arch Bishop", "the Arch Lady of the Church", 3100000 },
      { "the Patriarch", "the Matriarch", 3500000 },
      { "the Patriarch", "the Matriarch", 3900000 },
      { "the Patriarch", "the Matriarch", 4300000 },
      { "the Patriarch", "the Matriarch", 4800000 },
      { "the Patriarch", "the Matriarch", 5300000 },
      { "the Patriarch", "the Matriarch", 5900000 },
      { "the Patriarch", "the Matriarch", 6600000 },
      { "the Patriarch", "the Matriarch", 7300000 },
      { "the Patriarch", "the Matriarch", 8100000 },
      { "the Patriarch", "the Matriarch", 9000000 },
      { "the Patriarch", "the Matriarch", 10000000 },
      { "the Greater Patriarch", "the Greater Matriarch", 11000000      },
      { "the Greater Patriarch", "the Greater Matriarch", 12000000      },
      { "the Greater Patriarch", "the Greater Matriarch", 13000000      },
      { "the Greater Patriarch", "the Greater Matriarch", 14000000      },
      { "the Greater Patriarch", "the Greater Matriarch", 15500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 17000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 18500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 20500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 22500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 25000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 27500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 30000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 33000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 36000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 40000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 44000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 49000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 55000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 65000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 75000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 85000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 95000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 105000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 115000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 125000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 135000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 145000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 155000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 165000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 175000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 190000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 205000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 220000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 235000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 250000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 265000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 280000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 295000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 310000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 325000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 342500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 360000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 377500000     },   
      { "the Greater Patriarch", "the Greater Matriarch", 395000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 412500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 430000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 447500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 465000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 482500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 500000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 522500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 545000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 567500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 590000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 612500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 635000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 657500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 680000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 702500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 725000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 752500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 780000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 807500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 835000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 862500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 890000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 917500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 945000000     },
      { "the Greater Patriarch", "the Greater Matriarch", 972500000     },
      { "the Greater Patriarch", "the Greater Matriarch", 1000000000     },
      { "the Immortal", "the Immortal", 1100000000      },
      { "the Avatar", "the Avatar", 1200000000      },
      { "the Lessor God", "the Lessor Goddess", 1300000000      },
      { "the Greater God", "the Greater Goddess", 1400000000      }
    },
   { { "the Man", "the Woman", 0 },
      { "the Pilferer", "the Pilferess", 1 },
      { "the Footpad", "the Footpad", 2000 },
      { "the Filcher", "the Filcheress", 4000 },
      { "the Pick-Pocket", "the Pick-Pocket", 8000 },
      { "the Sneak", "the Sneak", 16000 },
      { "the Pincher", "the Pincheress", 32000 },
      { "the Cut-Purse", "the Cut-Purse", 64000 },
      { "the Snatcher", "the Snatcheress", 125000 },
      { "the Sharper", "the Sharpress", 250000 },
      { "the Rogue", "the Rogue", 500000 },
      { "the Robber", "the Robber", 750000 },
      { "the Magsman", "the Magswoman", 1000000 },
      { "the Highwayman", "the Highwaywoman", 1250000 },
      { "the Burglar", "the Burglaress", 1500000 },
      { "the Thief", "the Thief", 1800000 },
      { "the Knifer", "the Knifer", 2100000 },
      { "the Quick-Blade", "the Quick-Blade", 2400000 },
      { "the Killer", "the Murderess", 2750000 },
      { "the Brigand", "the Brigand", 3100000 },
      { "the Cut-Throat", "the Cut-Throat", 3500000 },
      { "the Cut-Throat", "the Cut-Throat", 3900000 },
      { "the Cut-Throat", "the Cut-Throat", 4300000 },
      { "the Cut-Throat", "the Cut-Throat", 4800000 },
      { "the Cut-Throat", "the Cut-Throat", 5300000 },
      { "the Cut-Throat", "the Cut-Throat", 5900000 },
      { "the Cut-Throat", "the Cut-Throat", 6600000 },
      { "the Cut-Throat", "the Cut-Throat", 7300000 },
      { "the Cut-Throat", "the Cut-Throat", 8100000 },
      { "the Cut-Throat", "the Cut-Throat", 9000000 },
      { "the Cut-Throat", "the Cut-Throat", 10000000 },
      { "the Grand Assasin", "the Angel Of Death", 11000000      },
      { "the Grand Assasin", "the Angel Of Death", 12000000      },
     { "the Grand Assasin", "the Angel Of Death", 13000000      },
     { "the Grand Assasin", "the Angel Of Death", 14000000      },
     { "the Grand Assasin", "the Angel Of Death", 15500000      },
     { "the Grand Assasin", "the Angel Of Death", 17000000      },
     { "the Grand Assasin", "the Angel Of Death", 18500000      },
     { "the Grand Assasin", "the Angel Of Death", 20500000      },
     { "the Grand Assasin", "the Angel Of Death", 22500000      },
     { "the Grand Assasin", "the Angel Of Death", 25000000      },
     { "the Grand Assasin", "the Angel Of Death", 27500000      },
     { "the Grand Assasin", "the Angel Of Death", 30000000      },
     { "the Grand Assasin", "the Angel Of Death", 33000000      },
     { "the Grand Assasin", "the Angel Of Death", 36000000      },
     { "the Grand Assasin", "the Angel Of Death", 40000000      },
     { "the Grand Assasin", "the Angel Of Death", 44000000      },
     { "the Grand Assasin", "the Angel Of Death", 49000000      },
     { "the Grand Assasin", "the Angel Of Death", 55000000      },
     { "the Grand Assasin", "the Angel Of Death", 65000000      },
     { "the Grand Assasin", "the Angel Of Death", 75000000      },
     { "the Grand Assasin", "the Angel Of Death", 85000000      },
     { "the Grand Assasin", "the Angel Of Death", 95000000      },
     { "the Grand Assasin", "the Angel Of Death", 105000000      },
     { "the Grand Assasin", "the Angel Of Death", 115000000      },
     { "the Grand Assasin", "the Angel Of Death", 125000000      },
     { "the Grand Assasin", "the Angel Of Death", 135000000      },
     { "the Grand Assasin", "the Angel Of Death", 145000000      },
     { "the Grand Assasin", "the Angel Of Death", 155000000      },
     { "the Grand Assasin", "the Angel Of Death", 165000000      },
     { "the Grand Assasin", "the Angel Of Death", 175000000      },
     { "the Grand Assasin", "the Angel Of Death", 190000000      },
     { "the Grand Assasin", "the Angel Of Death", 205000000      },
     { "the Grand Assasin", "the Angel Of Death", 220000000      },
     { "the Grand Assasin", "the Angel Of Death", 235000000      },
     { "the Grand Assasin", "the Angel Of Death", 250000000      },
     { "the Grand Assasin", "the Angel Of Death", 265000000      },
     { "the Grand Assasin", "the Angel Of Death", 280000000      },
     { "the Grand Assasin", "the Angel Of Death", 295000000      },
     { "the Grand Assasin", "the Angel Of Death", 310000000      },
     { "the Grand Assasin", "the Angel Of Death", 325000000      },
     { "the Grand Assasin", "the Angel Of Death", 342500000      },
     { "the Grand Assasin", "the Angel Of Death", 360000000      },
     { "the Grand Assasin", "the Angel Of Death", 377500000      },
     { "the Grand Assasin", "the Angel Of Death", 395000000      },
     { "the Grand Assasin", "the Angel Of Death", 412500000      },
     { "the Grand Assasin", "the Angel Of Death", 430000000      },
     { "the Grand Assasin", "the Angel Of Death", 447500000      },
     { "the Grand Assasin", "the Angel Of Death", 465000000      },
     { "the Grand Assasin", "the Angel Of Death", 482500000      },
     { "the Grand Assasin", "the Angel Of Death", 500000000      },
     { "the Grand Assasin", "the Angel Of Death", 522500000      },
     { "the Grand Assasin", "the Angel Of Death", 545000000      },
     { "the Grand Assasin", "the Angel Of Death", 567500000      },
     { "the Grand Assasin", "the Angel Of Death", 590000000      },
     { "the Grand Assasin", "the Angel Of Death", 612500000      },
     { "the Grand Assasin", "the Angel Of Death", 635000000      },
     { "the Grand Assasin", "the Angel Of Death", 657500000      },
     { "the Grand Assasin", "the Angel Of Death", 680000000      },
     { "the Grand Assasin", "the Angel Of Death", 702500000      },
     { "the Grand Assasin", "the Angel Of Death", 725000000      },
     { "the Grand Assasin", "the Angel Of Death", 752500000      },
     { "the Grand Assasin", "the Angel Of Death", 780000000      },
     { "the Grand Assasin", "the Angel Of Death", 807500000      },
     { "the Grand Assasin", "the Angel Of Death", 835000000      },
     { "the Grand Assasin", "the Angel Of Death", 862500000      },
     { "the Grand Assasin", "the Angel Of Death", 890000000      },
     { "the Grand Assasin", "the Angel Of Death", 917500000      },
     { "the Grand Assasin", "the Angel Of Death", 945000000      },
     { "the Grand Assasin", "the Angel Of Death", 972500000      },
     { "the Grand Assasin", "the Angel Of Death", 1000000000      },
      { "the Immortal", "the Immortal", 1100000000      },
      { "the Avatar", "the Avatar", 1200000000      },
      { "the Lessor God", "the Lessor Goddess", 1300000000      },
     { "the Greater God", "the Greater Goddess", 1400000000      }
    },
   { { "the Man", "the Woman", 0 },
      { "the Swordpupil", "the Swordpupil", 1 },
      { "the Recruit", "the Recruit", 2000 },
      { "the Sentry", "the Sentress", 4000 },
      { "the Fighter", "the Fighter", 8000 },
      { "the Soldier", "the Soldier", 16000 },
      { "the Warrior", "the Warrior", 32000 },
      { "the Veteran", "the Veteran", 64000 },
      { "the Swordsman", "the Swordswoman", 125000 },
      { "the Fencer", "the Fenceress", 250000 },
      { "the Combatant", "the Combatess", 500000 },
      { "the Hero", "the Heroine", 750000 },
      { "the Myrmidon", "the Myrmidon", 1000000 },
      { "the Swashbuckler", "the Swashbuckleress", 1250000 },
      { "the Mercenary", "the Mercenaress", 1500000 },
      { "the Swordmaster", "the Swordmistress", 1800000 },
      { "the Lieutenant", "the Lieutenant", 2100000 },
      { "the Champion", "the Lady Champion", 2400000 },
      { "the Dragoon", "the Lady Dragoon", 2750000 },
      { "the Cavalier", "the Cavalier", 3100000 },
      { "the Knight", "the Lady Knight", 3500000 },
     { "the Knight", "the Lady Knight", 3900000 },
      { "the Knight", "the Lady Knight", 4300000 },
      { "the Knight", "the Lady Knight", 4800000 },
      { "the Knight", "the Lady Knight", 5300000 },
      { "the Knight", "the Lady Knight", 5900000 },
      { "the Knight", "the Lady Knight", 6600000 },
      { "the Knight", "the Lady Knight", 7300000 },
      { "the Knight", "the Lady Knight", 8100000 },
      { "the Knight", "the Lady Knight", 9000000 },
      { "the Knight", "the Lady Knight", 10000000 },
      { "the Knight Prince", "the Knight Princess", 11000000      },
      { "the Knight Prince", "the Knight Princess", 12000000      },
      { "the Knight Prince", "the Knight Princess", 13000000      },
      { "the Knight Prince", "the Knight Princess", 14000000      },
      { "the Knight Prince", "the Knight Princess", 15500000      },
      { "the Knight Prince", "the Knight Princess", 17000000      },
      { "the Knight Prince", "the Knight Princess", 18500000      },
      { "the Knight Prince", "the Knight Princess", 20500000      },
      { "the Knight Prince", "the Knight Princess", 22500000      },
      { "the Knight Prince", "the Knight Princess", 25000000      },
      { "the Knight Prince", "the Knight Princess", 27500000      },
      { "the Knight Prince", "the Knight Princess", 30000000      },
      { "the Knight Prince", "the Knight Princess", 33000000      },
      { "the Knight Prince", "the Knight Princess", 36000000      },
      { "the Knight Prince", "the Knight Princess", 40000000      },
      { "the Knight Prince", "the Knight Princess", 44000000      },
      { "the Knight Prince", "the Knight Princess", 49000000      },
      { "the Knight Prince", "the Knight Princess", 55000000      },
      { "the Knight Prince", "the Knight Princess", 65000000      },
      { "the Knight Prince", "the Knight Princess", 75000000      },
      { "the Knight Prince", "the Knight Princess", 85000000      },
      { "the Knight Prince", "the Knight Princess", 95000000      },
      { "the Knight Prince", "the Knight Princess", 105000000      },
      { "the Knight Prince", "the Knight Princess", 115000000      },
      { "the Knight Prince", "the Knight Princess", 125000000      },
      { "the Knight Prince", "the Knight Princess", 135000000      },
      { "the Knight Prince", "the Knight Princess", 145000000      },
      { "the Knight Prince", "the Knight Princess", 155000000      },
      { "the Knight Prince", "the Knight Princess", 165000000      },
      { "the Knight Prince", "the Knight Princess", 175000000      },
      { "the Knight Prince", "the Knight Princess", 190000000      },
      { "the Knight Prince", "the Knight Princess", 205000000      },
      { "the Knight Prince", "the Knight Princess", 220000000      },
      { "the Knight Prince", "the Knight Princess", 235000000      },
      { "the Knight Prince", "the Knight Princess", 250000000      },
      { "the Knight Prince", "the Knight Princess", 265000000      },
      { "the Knight Prince", "the Knight Princess", 280000000      },
      { "the Knight Prince", "the Knight Princess", 295000000      },
      { "the Knight Prince", "the Knight Princess", 310000000      },
      { "the Knight Prince", "the Knight Princess", 325000000      },
      { "the Knight Prince", "the Knight Princess", 342500000      },
      { "the Knight Prince", "the Knight Princess", 360000000      },
      { "the Knight Prince", "the Knight Princess", 377500000      },
      { "the Knight Prince", "the Knight Princess", 395000000      },
      { "the Knight Prince", "the Knight Princess", 412500000      },
      { "the Knight Prince", "the Knight Princess", 430000000      },
      { "the Knight Prince", "the Knight Princess", 447500000      },
      { "the Knight Prince", "the Knight Princess", 465000000      },
      { "the Knight Prince", "the Knight Princess", 482500000      },
      { "the Knight Prince", "the Knight Princess", 500000000      },
      { "the Knight Prince", "the Knight Princess", 522500000      },
      { "the Knight Prince", "the Knight Princess", 545000000      },
      { "the Knight Prince", "the Knight Princess", 567500000      },
      { "the Knight Prince", "the Knight Princess", 590000000      },
      { "the Knight Prince", "the Knight Princess", 612500000      },
      { "the Knight Prince", "the Knight Princess", 635000000      },
      { "the Knight Prince", "the Knight Princess", 657500000      },
      { "the Knight Prince", "the Knight Princess", 680000000      },
      { "the Knight Prince", "the Knight Princess", 702500000      },
      { "the Knight Prince", "the Knight Princess", 725000000      },
      { "the Knight Prince", "the Knight Princess", 752500000      },
      { "the Knight Prince", "the Knight Princess", 780000000      },
      { "the Knight Prince", "the Knight Princess", 807500000      },
      { "the Knight Prince", "the Knight Princess", 835000000      },
      { "the Knight Prince", "the Knight Princess", 862500000      },
      { "the Knight Prince", "the Knight Princess", 890000000      },
      { "the Knight Prince", "the Knight Princess", 917500000      },
      { "the Knight Prince", "the Knight Princess", 945000000      },
      { "the Knight Prince", "the Knight Princess", 972500000      },
      { "the Knight Prince", "the Knight Princess", 1000000000      },
      { "the Immortal", "the Immortal", 1100000000      },
      { "the Avatar", "the Avatar", 1200000000      },
      { "the Lessor God", "the Lessor Goddess", 1300000000      },
      { "the Greater God", "the Greater Goddess", 1400000000      }
},
};
