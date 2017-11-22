/* ************************************************************************
*   File: fight.c                                       Part of CircleMUD *
*  Usage: Combat system                                                   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"
#include "screen.h"

/* Structures */
struct char_data *combat_list = NULL;	/* head of l-list of fighting chars */
struct char_data *next_combat_list = NULL;

/* External structures */
extern struct room_data *world;
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data *object_list;
extern int pk_allowed;		/* see config.c */
extern int auto_save;		/* see config.c */
extern int max_exp_gain;	/* see config.c */

/* External procedures */
char *fread_action(FILE * fl, int nr);
char *fread_string(FILE * fl, char *error);
void stop_follower(struct char_data * ch);
void do_rent(struct char_data *ch, int cmd, char *arg);
ACMD(do_flee);
void hit(struct char_data * ch, struct char_data * victim, int type);
void forget(struct char_data * ch, struct char_data * victim);
void remember(struct char_data * ch, struct char_data * victim);
int ok_damage_shopkeeper(struct char_data * ch, struct char_data * victim);
void mprog_hitprcnt_trigger(struct char_data * mob, struct char_data * ch);
void mprog_death_trigger(struct char_data * mob, struct char_data * killer);
void mprog_fight_trigger(struct char_data * mob, struct char_data * ch);
void mob_initiate(struct char_data *ch, struct char_data *victim);
void mob_combat(struct char_data *ch, struct char_data *victim);

/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
  {"hit", "hits"},		/* 0 */
  {"sting", "stings"},
  {"whip", "whips"},
  {"slash", "slashes"},
  {"bite", "bites"},
  {"bludgeon", "bludgeons"},	/* 5 */
  {"crush", "crushes"},
  {"pound", "pounds"},
  {"claw", "claws"},
  {"maul", "mauls"},
  {"thrash", "thrashes"},	/* 10 */
  {"pierce", "pierces"},
  {"blast", "blasts"},
  {"punch", "punches"},
  {"stab", "stabs"}
};

#define IS_WEAPON(type) (((type) >= TYPE_HIT) && ((type) < TYPE_SUFFERING))

/* The Fight related routines */

/* quickie hack routine */

extern struct title_type titles[NUM_CLASSES][104 + 1];

int cap_exp(struct char_data *ch, int max_gain) {
  int gain = max_gain;
  int tmp;

  tmp = titles[(int) GET_CLASS(ch)][GET_LEVEL(ch) + 2].exp;

  if (GET_LEVEL(ch) < 100) {
    gain = tmp - MIN(titles[(int) GET_CLASS(ch)][GET_LEVEL(ch)].exp,
		GET_EXP(ch));
    gain = MIN(gain, max_gain);
  } else
    gain *= 4; /* Quick hack. Danae. */
  return gain;
}

void weapon_effects(struct char_data *ch, struct char_data *vict,
                    struct obj_data *weapon);

void appear(struct char_data * ch)
{
  if (affected_by_spell(ch, SPELL_INVISIBLE))
    affect_from_char(ch, SPELL_INVISIBLE);

  REMOVE_BIT(AFF_FLAGS(ch), AFF_INVISIBLE | AFF_HIDE);

  if (GET_LEVEL(ch) < LVL_IMMORT)
    act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  else
    act("You feel a strange presence as $n appears, seemingly from nowhere.",
	FALSE, ch, 0, 0, TO_ROOM);
}



void load_messages(void)
{
  FILE *fl;
  int i, type;
  struct message_type *messages;
  char chk[128];

  if (!(fl = fopen(MESS_FILE, "r"))) {
    sprintf(buf2, "Error reading combat message file %s", MESS_FILE);
    perror(buf2);
    exit(1);
  }
  for (i = 0; i < MAX_MESSAGES; i++) {
    fight_messages[i].a_type = 0;
    fight_messages[i].number_of_attacks = 0;
    fight_messages[i].msg = 0;
  }


  fgets(chk, 128, fl);
  while (!feof(fl) && (*chk == '\n' || *chk == '*'))
    fgets(chk, 128, fl);

  while (*chk == 'M') {
    fgets(chk, 128, fl);
    sscanf(chk, " %d\n", &type);
    for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type != type) &&
	 (fight_messages[i].a_type); i++);
    if (i >= MAX_MESSAGES) {
      fprintf(stderr, "Too many combat messages.  Increase MAX_MESSAGES and recompile.");
      exit(1);
    }
    CREATE(messages, struct message_type, 1);
    fight_messages[i].number_of_attacks++;
    fight_messages[i].a_type = type;
    messages->next = fight_messages[i].msg;
    fight_messages[i].msg = messages;

    messages->die_msg.attacker_msg = fread_action(fl, i);
    messages->die_msg.victim_msg = fread_action(fl, i);
    messages->die_msg.room_msg = fread_action(fl, i);
    messages->miss_msg.attacker_msg = fread_action(fl, i);
    messages->miss_msg.victim_msg = fread_action(fl, i);
    messages->miss_msg.room_msg = fread_action(fl, i);
    messages->hit_msg.attacker_msg = fread_action(fl, i);
    messages->hit_msg.victim_msg = fread_action(fl, i);
    messages->hit_msg.room_msg = fread_action(fl, i);
    messages->god_msg.attacker_msg = fread_action(fl, i);
    messages->god_msg.victim_msg = fread_action(fl, i);
    messages->god_msg.room_msg = fread_action(fl, i);
    fgets(chk, 128, fl);
    while (!feof(fl) && (*chk == '\n' || *chk == '*'))
      fgets(chk, 128, fl);
  }

  fclose(fl);
}


void update_pos(struct char_data * victim)
{

  if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POS_STUNNED))
    return;
  else if (GET_HIT(victim) > 0)
    GET_POS(victim) = POS_STANDING;
  else if (GET_HIT(victim) <= -11)
    GET_POS(victim) = POS_DEAD;
  else if (GET_HIT(victim) <= -6)
    GET_POS(victim) = POS_MORTALLYW;
  else if (GET_HIT(victim) <= -3)
    GET_POS(victim) = POS_INCAP;
  else
    GET_POS(victim) = POS_STUNNED;
}


void check_killer(struct char_data * ch, struct char_data * vict)
{
  if (!PLR_FLAGGED(vict, PLR_KILLER) && !PLR_FLAGGED(vict, PLR_THIEF)
      && !PLR_FLAGGED(ch, PLR_KILLER) && !IS_NPC(ch) && !IS_NPC(vict) &&
      !IS_SET(ROOM_FLAGS(ch->in_room), ROOM_PK_ALLOW) &&
      !IS_SET(ROOM_FLAGS(vict->in_room), ROOM_PK_ALLOW) &&
      (ch != vict)) {
    char buf[256];

    SET_BIT(PLR_FLAGS(ch), PLR_KILLER);
    sprintf(buf, "PC Killer bit set on %s for initiating attack on %s at %s.",
	    GET_NAME(ch), GET_NAME(vict), world[vict->in_room].name);
    mudlogs(buf, BRF, LVL_IMMORT, TRUE);
    send_to_char("If you want to be a PLAYER KILLER, so be it...\r\n", ch);
  }
}


/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data * ch, struct char_data * vict)
{
  if (ch == vict)
    return;

  assert(!FIGHTING(ch));

  ch->next_fighting = combat_list;
  combat_list = ch;

  if (IS_AFFECTED(ch, AFF_SLEEP))
    affect_from_char(ch, SPELL_SLEEP);

  FIGHTING(ch) = vict;
  GET_POS(ch) = POS_FIGHTING;

  if (!pk_allowed)
    check_killer(ch, vict);
}



/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data * ch)
{
  struct char_data *temp;

  if (ch == next_combat_list)
    next_combat_list = ch->next_fighting;

  REMOVE_FROM_LIST(ch, combat_list, next_fighting);
  ch->next_fighting = NULL;
  FIGHTING(ch) = NULL;
  GET_POS(ch) = POS_STANDING;
  update_pos(ch);
}



void make_corpse(struct char_data * ch)
{
  struct obj_data *corpse, *o;
  struct obj_data *money;
  int i;
  extern int max_npc_corpse_time, max_pc_corpse_time;

  struct obj_data *create_money(int amount);

   struct extra_descr_data *new_descr;
   
   if(!IS_NPC(ch)){
     i=number(0,NUM_WEARS-1);
     if(ch->equipment[i]){
       o=unequip_char(ch,i);
       extract_obj(o);
     }
     CREATE(corpse, struct obj_data, 1);
     clear_object(corpse);
     corpse->obj_flags.wear_flags=1;
     corpse->obj_flags.weight=1000;
     corpse->name = strdup("gravestone");
     corpse->in_room = NOWHERE;
     corpse->item_number = NOWHERE;
     sprintf(buf,"The Gravestone of %s",GET_NAME(ch));
     corpse->short_description = strdup(buf);
     sprintf(buf,"The Gravestone With %s engraved on it lies here.",GET_NAME(ch));
     corpse->description = strdup(buf);
     CREATE(new_descr, struct extra_descr_data, 1);
     new_descr->keyword = strdup("gravestone");
     sprintf(buf,"The engraving reads: Here Lies %s, may they rest in peace. \n\r",GET_NAME(ch));
     new_descr->description = strdup(buf);
     new_descr->next = corpse->ex_description;
     corpse->ex_description = new_descr;
     corpse->next = object_list;
     object_list = corpse;
     obj_to_room(corpse,ch->in_room);
    return;
    } 
/* 
	Old Corpse System.
*/
  corpse = create_obj();

  corpse->item_number = NOTHING;
  corpse->in_room = NOWHERE;
  corpse->name = str_dup("corpse");

  sprintf(buf2, "The corpse of %s is lying here.", GET_NAME(ch));
  corpse->description = str_dup(buf2);

  sprintf(buf2, "the corpse of %s", GET_NAME(ch));
  corpse->short_description = str_dup(buf2);

  GET_OBJ_TYPE(corpse) = ITEM_CONTAINER;
  GET_OBJ_WEAR(corpse) = ITEM_WEAR_TAKE;
  GET_OBJ_EXTRA(corpse) = ITEM_NODONATE;
  GET_OBJ_VAL(corpse, 0) = 0;	/* You can't store stuff in a corpse */
  GET_OBJ_VAL(corpse, 3) = 1;	/* corpse identifier */
  GET_OBJ_WEIGHT(corpse) = GET_WEIGHT(ch) + IS_CARRYING_W(ch);
  GET_OBJ_RENT(corpse) = 100000;
  if (IS_NPC(ch))
    GET_OBJ_TIMER(corpse) = max_npc_corpse_time;
  else
    GET_OBJ_TIMER(corpse) = max_pc_corpse_time;

  /* transfer character's inventory to the corpse */
  corpse->contains = ch->carrying;
  for (o = corpse->contains; o != NULL; o = o->next_content)
    o->in_obj = corpse;
  object_list_new_owner(corpse, NULL);

  /* transfer character's equipment to the corpse */
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i))
      obj_to_obj(unequip_char(ch, i), corpse);

  /* transfer gold */
  if (GET_GOLD(ch) > 0) {
    /* following 'if' clause added to fix gold duplication loophole */
    if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc)) {
      money = create_money(GET_GOLD(ch));
      obj_to_obj(money, corpse);
    }
    GET_GOLD(ch) = 0;
  }
  ch->carrying = NULL;
  IS_CARRYING_N(ch) = 0;
  IS_CARRYING_W(ch) = 0;

  obj_to_room(corpse, ch->in_room);
}


/* When ch kills victim */
void change_alignment(struct char_data * ch, struct char_data * victim)
{
  /*
   * new alignment change algorithm: if you kill a monster with alignment A,
   * you move 1/16th of the way to having alignment -A.  Simple and fast.
   */
  GET_ALIGNMENT(ch) += (-GET_ALIGNMENT(victim) - GET_ALIGNMENT(ch)) >> 4;
}



void death_cry(struct char_data * ch)
{
  int door, was_in;

  act("Your blood freezes as you hear $n's death cry.", FALSE, ch, 0, 0, TO_ROOM);
  was_in = ch->in_room;

  for (door = 0; door < NUM_OF_DIRS; door++) {
    if (CAN_GO(ch, door)) {
      ch->in_room = world[was_in].dir_option[door]->to_room;
      act("Your blood freezes as you hear someone's death cry.", FALSE, ch, 0, 0, TO_ROOM);
      ch->in_room = was_in;
    }
  }
}

void fighting_cry(struct char_data * ch)
{
  int door, was_in;

  if (!number(0, 5)) {
    was_in = ch->in_room;

    for (door = 0; door < NUM_OF_DIRS; door++) {
      ch->in_room = world[was_in].dir_option[door]->to_room;
      act("You hear the ring of sword on sword, as if there was fighting nearby!", FALSE, ch, 0, 0, TO_ROOM);
      ch->in_room = was_in;
    }
  }
}
  
void raw_kill(struct char_data * ch, struct char_data *killer)
{
  if (FIGHTING(ch))
    stop_fighting(ch);

  while (ch->affected)
    affect_remove(ch, ch->affected);

/*  death_cry(ch); */

  if (killer)
     mprog_death_trigger(ch, killer);

  make_corpse(ch);
  if(IS_NPC(ch))
      extract_char(ch);
  else {
      ch->points.gold>>=1;
      do_rent(ch,0,0);
      }
}



 void die(struct char_data * ch, struct char_data *killer)
{
  int pen;

  gain_exp(ch, -(GET_EXP(ch) >> 1));
  if (!IS_NPC(ch)){
    REMOVE_BIT(PLR_FLAGS(ch), PLR_KILLER | PLR_THIEF);
      pen = ch->points.max_hit/100;
      if (pen < 0)
         return;
     ch->points.max_mana -=pen;
     pen = ch->points.max_mana/100;
     if (pen < 0)
        return;
     ch->points.max_mana -= pen;
  }
  raw_kill(ch,killer);
}



void perform_group_gain(struct char_data * ch, int base,
			     struct char_data * victim)
{
  int share;

  share = MIN(cap_exp(ch, max_exp_gain), MAX(1, base));

  if (share > 1) {
    sprintf(buf2, "You receive your share of experience -- %d points.\r\n", share);
    send_to_char(buf2, ch);
  } else
    send_to_char("You receive your share of experience -- one measly little point!\r\n", ch);

  gain_exp(ch, share);
  change_alignment(ch, victim);
}


void group_gain(struct char_data * ch, struct char_data * victim)
{
  int tot_members, base;
  struct char_data *k;
  struct follow_type *f;

  if (!(k = ch->master))
    k = ch;

  if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room))
    tot_members = 1;
  else
    tot_members = 0;

  for (f = k->followers; f; f = f->next)
    if (IS_AFFECTED(f->follower, AFF_GROUP) && f->follower->in_room == ch->in_room)
      tot_members++;

  /* round up to the next highest tot_members */
  base = (GET_EXP(victim) / 3) + tot_members - 1;

  if ((tot_members >= 1) && (IS_NPC(victim))) {
    if (MOB_FLAGGED(victim, MOB_NOXP))
       base = 0;
    else 
       base = MAX(1, GET_EXP(victim) / (3 * tot_members));
   } else
    base = 0;

  if (IS_AFFECTED(k, AFF_GROUP) && k->in_room == ch->in_room)
    perform_group_gain(k, base, victim);

  for (f = k->followers; f; f = f->next)
    if (IS_AFFECTED(f->follower, AFF_GROUP) && f->follower->in_room == ch->in_room)
      perform_group_gain(f->follower, base, victim);
}



char *replace_string(char *str, char *weapon_singular, char *weapon_plural)
{
  static char buf[256];
  char *cp;

  cp = buf;

  for (; *str; str++) {
    if (*str == '#') {
      switch (*(++str)) {
      case 'W':
	for (; *weapon_plural; *(cp++) = *(weapon_plural++));
	break;
      case 'w':
	for (; *weapon_singular; *(cp++) = *(weapon_singular++));
	break;
      default:
	*(cp++) = '#';
	break;
      }
    } else
      *(cp++) = *str;

    *cp = 0;
  }				/* For */

  return (buf);
}


/* message for doing damage with a weapon */
void dam_message(int dam, struct char_data * ch, struct char_data * victim,
		      int w_type)
{
  char *buf;
  int msgnum;

  static struct dam_weapon_type {
    char *to_room;
    char *to_char;
    char *to_victim;
  } dam_weapons[] = {

    /* use #w for singular (i.e. "slash") and #W for plural (i.e. "slashes") */

    {
      "$n tries to #w $N, but misses.",	/* 0: 0     */
      "You try to #w $N, but miss.",
      "$n tries to #w you, but misses."
    },

    {
      "$n tickles $N as $e #W $M.",	/* 1: 1..2  */
      "You tickle $N as you #w $M.",
      "$n tickles you as $e #W you."
    },

    {
      "$n barely #W $N.",		/* 2: 3..4  */
      "You barely #w $N.",
      "$n barely #W you."
    },

    {
      "$n #W $N.",			/* 3: 5..6  */
      "You #w $N.",
      "$n #W you."
    },

    {
      "$n #W $N hard.",			/* 4: 7..10  */
      "You #w $N hard.",
      "$n #W you hard."
    },

    {
      "$n #W $N very hard.",		/* 5: 11..14  */
      "You #w $N very hard.",
      "$n #W you very hard."
    },

    {
      "$n #W $N extremely hard.",	/* 6: 15..19  */
      "You #w $N extremely hard.",
      "$n #W you extremely hard."
    },

    {
      "$n massacres $N to small fragments with $s #w.",	/* 7: 19..23 */
      "You massacre $N to small fragments with your #w.",
      "$n massacres you to small fragments with $s #w."
    },

    {
      "$n Obliterates $N with $s deadly #w!!",	/* 8: > 23   */
      "You Obliterate $N with your deadly #w!!",
      "$n Obliterates you with $s deadly #w!!"
    },

    {
      "$n CHunks $N with $s deadly #w!!",	/* 9: > 28   */
      "You CHunk $N with your deadly #w!!",
      "$n CHunks you with $s deadly #w!!"
    },

    {
      "$n DISmembers $N with $s deadly #w!!",	/* 10: > 34   */
      "You DISmember $N with your deadly #w!!",
      "$n DISmembers you with $s deadly #w!!"
    },

    {
      "$n GUTS $N with $s deadly #w!!",	/* 11: > 41   */
      "You GUT $N with your deadly #w!!",
      "$n GUTS you with $s deadly #w!!"
    },

    {
      "$n DISEMbowels $N with $s deadly #w!!",	/* 12: > 50   */
      "You DISEMbowel $N with your deadly #w!!",
      "$n DISEMbowels you with $s deadly #w!!"
    },

    {
      "$n STAMPS $N with $s deadly #w!!",	/* 14: > 66   */
      "You STAMP $N with your deadly #w!!",
      "$n STAMPS you with $s deadly #w!!"
    },

    {
      "$n BONE CRunches $N with $s deadly #w!!",	/* 15: > 85   */
      "You BONE CRunch $N with your deadly #w!!",
      "$n BONE CRunches you with $s deadly #w!!"
    },

    {
      "$n FLAYS LImbs $N with $s deadly #w!!",	/* 16: > 120   */
      "You FLAY LImbs $N with your deadly #w!!",
      "$n FLAYS LImbs you with $s deadly #w!!"
    },

    {
      "$n FLAYS BONes $N with $s deadly #w!!",	/* 17: > 220   */
      "You FLAY BONes $N with your deadly #w!!",
      "$n FLAYS BONes you with $s deadly #w!!"
    },

    {
      "$n SPIFLICATEs $N with $s deadly #w!!",	/* 18: > 420   */
      "You SPIFLICATE $N with your deadly #w!!",
      "$n SPIFLICATEs you with $s deadly #w!!"
    },

    {
      "$n GRINDS BONES $N with $s deadly #w!!",	/* 19: > 720   */
      "You GRIND BONES $N with your deadly #w!!",
      "$n GRINDS BONES you with $s deadly #w!!"
    },

    {
      "$n REMOVES BODY parts of $N with $s deadly #w!!",	/* 20: > 1220   */
      "You REMOVE BODY parts  of $N with your deadly #w!!",
      "$n REMOVES BODY parts of you with $s deadly #w!!"
    },

    {
      "$n RIPS OFF BODY Parts of $N with $s deadly #w!!",	/* 21: > 1920   */
      "You RIP OFF BODY Parts of $N with your deadly #w!!",
      "$n RIPS OFF BODY Parts of you with $s deadly #w!!"
    },

    {
      "$n RIPS OFF AND DEStroys body parts $N with $s deadly #w!!",	/* 22: > 7720   */
      "You RIP OFF AND DEStroy body parts $N with your deadly #w!!",
      "$n RIPS OFF AND DEStroys body parts you with $s deadly #w!!"
    },

    {
      "$n LIQUIFIES AND DESTroys $N body parts with $s deadly #w!!",	/* 23: > 10720   */
      "You LIQUIFY AND DESTroy $N body parts with your deadly #w!!",
      "$n LIQUIFIES AND DESTroys your body parts with $s deadly #w!!"
    },

    {
      "$n LIQUIFIES AND DECIMAtes $N body parts with $s deadly #w!!",	/* 24: > 15720   */
      "You LIQUIFY AND DECIMAte $N body parts with your deadly #w!!",
      "$n LIQUIFIES AND DECIMAtes your body parts with $s deadly #w!!"
    },

    {
      "$n Rips off $N head and shits down $S neck!",	/* 25: > 90720   */
      "You Rip off $N head and shit down $S neck!",
      "$n Rips off your head and shits down your neck. Youre DEAD.. SO There!"
    }
  };


  w_type -= TYPE_HIT;		/* Change to base of table with text */

  if (dam == 0)		msgnum = 0;
  else if (dam <= 2)    msgnum = 1;
  else if (dam <= 4)    msgnum = 2;
  else if (dam <= 6)    msgnum = 3;
  else if (dam <= 10)   msgnum = 4;
  else if (dam <= 14)   msgnum = 5;
  else if (dam <= 19)   msgnum = 6;
  else if (dam <= 23)   msgnum = 7;
  else if (dam <= 28)   msgnum = 8;
  else if (dam <= 34)   msgnum = 9;
  else if (dam <= 41)   msgnum = 10;
  else if (dam <= 50)   msgnum = 11;
  else if (dam <= 66)   msgnum = 12;
  else if (dam <= 85)   msgnum = 13;
  else if (dam <= 120)   msgnum = 14;
  else if (dam <= 220)   msgnum = 15;
  else if (dam <= 420)   msgnum = 16;
  else if (dam <= 720)   msgnum = 17;
  else if (dam <= 1220)   msgnum = 18;
  else if (dam <= 1920)   msgnum = 19;
  else if (dam <= 2720)   msgnum = 20;
  else if (dam <= 4720)   msgnum = 21;
  else if (dam <= 7720)   msgnum = 22;
  else if (dam <= 10720)   msgnum = 23;
  else if (dam <= 15720)   msgnum = 24;
  else if (dam <= 95720)   msgnum = 25;
  else			msgnum = 26;

  /* damage message to onlookers */
  buf = replace_string(dam_weapons[msgnum].to_room,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  act(buf, FALSE, ch, NULL, victim, TO_NOTVICT);

  /* damage message to damager */
  send_to_char(CCYEL(ch, C_CMP), ch);
  buf = replace_string(dam_weapons[msgnum].to_char,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  act(buf, FALSE, ch, NULL, victim, TO_CHAR);
  send_to_char(CCNRM(ch, C_CMP), ch);

  /* damage message to damagee */
  send_to_char(CCRED(victim, C_CMP), victim);
  buf = replace_string(dam_weapons[msgnum].to_victim,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  act(buf, FALSE, ch, NULL, victim, TO_VICT | TO_SLEEP);
  send_to_char(CCNRM(victim, C_CMP), victim);
}


/*
 * message for doing damage with a spell or skill
 *  C3.0: Also used for weapon damage on miss and death blows
 */
int skill_message(int dam, struct char_data * ch, struct char_data * vict,
		      int attacktype)
{
  int i, j, nr;
  struct message_type *msg;

  struct obj_data *weap = GET_EQ(ch, WEAR_WIELD);

  for (i = 0; i < MAX_MESSAGES; i++) {
    if (fight_messages[i].a_type == attacktype) {
      nr = dice(1, fight_messages[i].number_of_attacks);
      for (j = 1, msg = fight_messages[i].msg; (j < nr) && msg; j++)
	msg = msg->next;

      if (!IS_NPC(vict) && (GET_LEVEL(vict) >= LVL_IMMORT)) {
	act(msg->god_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	act(msg->god_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT);
	act(msg->god_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
      } else if (dam != 0) {
	if (GET_POS(vict) == POS_DEAD) {
	  send_to_char(CCYEL(ch, C_CMP), ch);
	  act(msg->die_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	  send_to_char(CCNRM(ch, C_CMP), ch);

	  send_to_char(CCRED(vict, C_CMP), vict);
	  act(msg->die_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	  send_to_char(CCNRM(vict, C_CMP), vict);

	  act(msg->die_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
	} else {
	  send_to_char(CCYEL(ch, C_CMP), ch);
	  act(msg->hit_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	  send_to_char(CCNRM(ch, C_CMP), ch);

	  send_to_char(CCRED(vict, C_CMP), vict);
	  act(msg->hit_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	  send_to_char(CCNRM(vict, C_CMP), vict);

	  act(msg->hit_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
	}
      } else if (ch != vict) {	/* Dam == 0 */
	send_to_char(CCYEL(ch, C_CMP), ch);
	act(msg->miss_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	send_to_char(CCNRM(ch, C_CMP), ch);

	send_to_char(CCRED(vict, C_CMP), vict);
	act(msg->miss_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	send_to_char(CCNRM(vict, C_CMP), vict);

	act(msg->miss_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
      }
      return 1;
    }
  }
  return 0;
}


void damage(struct char_data * ch, struct char_data * victim, int dam,
	    int attacktype)
{
  int exp;

  if (GET_POS(victim) <= POS_DEAD) {
    logs("SYSERR: Attempt to damage a corpse.");
    return;			/* -je, 7/7/92 */
  }
  /* You can't damage an immortal! */
  if (!IS_NPC(victim) && (GET_LEVEL(victim) >= LVL_IMMORT))
    dam = 0;

  /* shopkeeper protection */
  if (!ok_damage_shopkeeper(ch, victim))
    return;

  if (victim != ch) {
    if (GET_POS(ch) > POS_STUNNED) {
      if (!(FIGHTING(ch)))
	set_fighting(ch, victim);

      if (IS_NPC(ch) && IS_NPC(victim) && victim->master &&
	  !number(0, 10) && IS_AFFECTED(victim, AFF_CHARM) &&
	  (victim->master->in_room == ch->in_room)) {
	if (FIGHTING(ch))
	  stop_fighting(ch);
	mob_initiate(ch, victim->master);
	return;
      }
    }
    if (GET_POS(victim) > POS_STUNNED && !FIGHTING(victim)) {
      set_fighting(victim, ch);
      if (MOB_FLAGGED(victim, MOB_MEMORY) && !IS_NPC(ch) &&
	  (GET_LEVEL(ch) < LVL_IMMORT))
	remember(victim, ch);
    }
  }
  if (victim->master == ch)
    stop_follower(victim);

  if (IS_AFFECTED(ch, AFF_INVISIBLE | AFF_HIDE))
    appear(ch);

  if (IS_AFFECTED(victim, AFF_SANCTUARY))
    dam >>= 1;		/* 1/2 damage when sanctuary */

  if (!pk_allowed) {
    check_killer(ch, victim);

    if (PLR_FLAGGED(ch, PLR_KILLER) && (ch != victim) && !IS_NPC(victim) && !PLR_FLAGGED(victim, PLR_KILLER))
      dam = 0;
     /* dam >>= 1; */
  }

  dam = MAX(MIN(dam, 100000), 0);
  GET_HIT(victim) -= dam;

  if ((ch != victim) && (IS_NPC(victim))) 
    if (!MOB_FLAGGED(victim, MOB_NOXP)) 
	gain_exp(ch, GET_LEVEL(victim) * dam);

  if ((ch != victim) && (!IS_NPC(victim)) && (IS_NPC(ch)))
	gain_exp(ch, GET_LEVEL(victim) * dam);

  update_pos(victim);

  /*
   * skill_message sends a message from the messages file in lib/misc.
   * dam_message just sends a generic "You hit $n extremely hard.".
   * skill_message is preferable to dam_message because it is more
   * descriptive.
   * 
   * If we are _not_ attacking with a weapon (i.e. a spell), always use
   * skill_message. If we are attacking with a weapon: If this is a miss or a
   * death blow, send a skill_message if one exists; if not, default to a
   * dam_message. Otherwise, always send a dam_message.
   */
  if (!IS_WEAPON(attacktype))
    skill_message(dam, ch, victim, attacktype);
  else {
    if (GET_POS(victim) == POS_DEAD || dam == 0) {
      if (!skill_message(dam, ch, victim, attacktype))
	dam_message(dam, ch, victim, attacktype);
    } else
      dam_message(dam, ch, victim, attacktype);
  }

  /* Use send_to_char -- act() doesn't send message if you are DEAD. */
  switch (GET_POS(victim)) {
  case POS_MORTALLYW:
    act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n", victim);
    break;
  case POS_INCAP:
    act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You are incapacitated an will slowly die, if not aided.\r\n", victim);
    break;
  case POS_STUNNED:
    act("$n is stunned, but will probably regain consciousness again.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You're stunned, but will probably regain consciousness again.\r\n", victim);
    break;
  case POS_DEAD:
    act("$n is dead!  R.I.P.", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("You are dead!  Sorry...\r\n", victim);
    break;

  default:			/* >= POSITION SLEEPING */
    if (dam > (GET_MAX_HIT(victim) >> 2))
      act("That really did HURT!", FALSE, victim, 0, 0, TO_CHAR);

    if (GET_HIT(victim) < (GET_MAX_HIT(victim) >> 2)) {
      sprintf(buf2, "%sYou wish that your wounds would stop BLEEDING so much!%s\r\n",
	      CCRED(victim, C_SPR), CCNRM(victim, C_SPR));
      send_to_char(buf2, victim);
      if (MOB_FLAGGED(victim, MOB_WIMPY) && (ch != victim))
	do_flee(victim, "", 0, 0);
    }
    if (!IS_NPC(victim) && GET_WIMP_LEV(victim) && (victim != ch) &&
	GET_HIT(victim) < GET_WIMP_LEV(victim)) {
      send_to_char("You wimp out, and attempt to flee!\r\n", victim);
      do_flee(victim, "", 0, 0);
    }
    break;
  }

  if (!IS_NPC(victim) && !(victim->desc)) {
    do_flee(victim, "", 0, 0);
    if (!FIGHTING(victim)) {
      act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
      GET_WAS_IN(victim) = victim->in_room;
      char_from_room(victim);
      char_to_room(victim, 0);
    }
  }
  if (!AWAKE(victim))
    if (FIGHTING(victim))
      stop_fighting(victim);

  if (GET_POS(victim) == POS_DEAD) {
    if (IS_NPC(victim) || victim->desc)
      if (IS_AFFECTED(ch, AFF_GROUP))
	group_gain(ch, victim);
      else {
	exp = MIN(cap_exp(ch, max_exp_gain), GET_EXP(victim) / 3);

	/* Calculate level-difference bonus */
	if (IS_NPC(ch))
	  exp += MAX(0, (exp * MIN(4, (GET_LEVEL(victim) - GET_LEVEL(ch)))) >> 3);
	else
	  exp += MAX(0, (exp * MIN(8, (GET_LEVEL(victim) - GET_LEVEL(ch)))) >> 3);
        if (!IS_NPC(victim))
		exp = 1;
        else if (MOB_FLAGGED(victim, MOB_NOXP))
		exp = 1;
	else	
        	exp = MAX(exp, 1);
	if (exp > 1) {
	  sprintf(buf2, "You receive %d experience points.\r\n", exp);
	  send_to_char(buf2, ch);
	} else
	  send_to_char("You receive one lousy experience point.\r\n", ch);
	gain_exp(ch, exp);
	change_alignment(ch, victim);
      }
    if (!IS_NPC(victim)) {
      sprintf(buf2, "%s killed by %s at %s", GET_NAME(victim), GET_NAME(ch),
	      world[victim->in_room].name);
      mudlogs(buf2, BRF, LVL_IMMORT, TRUE);
      if (MOB_FLAGGED(ch, MOB_MEMORY))
	forget(ch, victim);
    }
    die(victim, ch);
  }
}



void hit(struct char_data * ch, struct char_data * victim, int type)
{
  struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
  int w_type, victim_ac, calc_thaco, dam, diceroll, ff;

  extern int thaco[NUM_CLASSES][LVL_IMPL+1];
  /*extern byte backstab_mult[];*/
  extern struct str_app_type str_app[];
  extern struct dex_app_type dex_app[];

  if (ch->in_room != victim->in_room) {
    if (FIGHTING(ch) && FIGHTING(ch) == victim)
      stop_fighting(ch);
    return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }

  mprog_hitprcnt_trigger(ch, FIGHTING(ch));
  mprog_fight_trigger(ch, FIGHTING(ch));

  if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON)
    w_type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
  else {
    if (IS_NPC(ch) && (ch->mob_specials.attack_type != 0))
      w_type = ch->mob_specials.attack_type + TYPE_HIT;
    else
      w_type = TYPE_HIT;
  }

  /* Calculate the raw armor including magic armor.  Lower AC is better. */

  if (!IS_NPC(ch)){
    /*calc_thaco = thaco[(int) GET_CLASS(ch)][(int) GET_LEVEL(ch)];*/
     if (GET_CLASS(ch) == CLASS_MAGIC_USER){
        if (GET_LEVEL(ch) <= 4){
          calc_thaco = 20;
   	}
     	else {
         /* calc_thaco = (20-(GET_LEVEL(ch)/3)); */
         calc_thaco = thaco[(int) GET_CLASS(ch)][(int) GET_LEVEL(ch)];
        }
     }
     if (GET_CLASS(ch) == CLASS_WARRIOR){
        if (GET_LEVEL(ch) <= 4){
          calc_thaco = 15;
        }
     	else {
         /* calc_thaco = (15-(GET_LEVEL(ch))); */
         calc_thaco = thaco[(int) GET_CLASS(ch)][(int) GET_LEVEL(ch)];
        }
     }
     if (GET_CLASS(ch) == CLASS_THIEF){
        if (GET_LEVEL(ch) <= 4){
          calc_thaco = 18;
        }
     	else {
         /* calc_thaco = (18-(GET_LEVEL(ch)/2)); */
         calc_thaco = thaco[(int) GET_CLASS(ch)][(int) GET_LEVEL(ch)];
        }
     }
     if (GET_CLASS(ch) == CLASS_CLERIC){
        if (GET_LEVEL(ch) <= 5){
          calc_thaco = 20;
        }
     	else {
         /* calc_thaco = (20-(GET_LEVEL(ch))/3); */
         calc_thaco = thaco[(int) GET_CLASS(ch)][(int) GET_LEVEL(ch)];
        }
     }
     	/* THAC0 for monsters is set in the HitRoll */
     if ((GET_CLASS(ch) != CLASS_CLERIC) && (GET_CLASS(ch) != CLASS_WARRIOR) && (GET_CLASS(ch) != CLASS_THIEF) && (GET_CLASS(ch) != CLASS_MAGIC_USER))
     {
    	calc_thaco = 20;
     }
}
if ((GET_CLASS(ch) != CLASS_CLERIC) && (GET_CLASS(ch) != CLASS_WARRIOR))  /* exclude the cleric and warrior for int and wis bonus */
/* If the warrior is a dumb shit.. he will be worse and worse off the higher the warrior gets */
	{

  calc_thaco -= (int) ((GET_INT(ch) - 13) / 1.5); 
  calc_thaco -= (int) ((GET_WIS(ch) - 13) / 1.5);	/* So does wisdom */
  	}
if ((GET_CLASS(ch) != CLASS_CLERIC))
        {
        calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
        }
  calc_thaco -= GET_HITROLL(ch);
  diceroll = number(1, 20);

  victim_ac = GET_AC(victim) / 10;

  if (AWAKE(victim))
    victim_ac += dex_app[GET_DEX(victim)].defensive;

  victim_ac = MAX(-10, victim_ac);	/* -10 is lowest */

  /* decide whether this is a hit or a miss */
   if ((calc_thaco < victim_ac) && (diceroll != 1) && AWAKE(victim)) {
	dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
        dam += GET_DAMROLL(ch);
      if (wielded){
      dam += dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2));
      if(IS_SET(wielded->obj_flags.extra_flags,ITEM_SFX)){
        /* ff=(ch->char_specials.fighting != NULL);*/
         ff=(FIGHTING(ch) != NULL);
         weapon_effects(ch,victim,wielded);
         if(ch->char_specials.fighting){
          if(ch->char_specials.fighting->in_room != ch->in_room) return;
         } else {
            if(ff) return;
         }
       }
    }
    else {
      if (IS_NPC(ch)) {
	dam += dice(ch->mob_specials.damnodice, ch->mob_specials.damsizedice);
      } else
	dam += number(0, 2);	/* Max. 2 dam with bare hands */
    }
    if (GET_POS(victim) < POS_FIGHTING)
      dam *= 1 + (POS_FIGHTING - GET_POS(victim)) / 3;
    /* Position  sitting  x 1.33 */
    /* Position  resting  x 1.66 */
    /* Position  sleeping x 2.00 */
    /* Position  stunned  x 2.33 */
    /* Position  incap    x 2.66 */
    /* Position  mortally x 3.00 */

    dam = MAX(1, dam);		/* at least 1 hp damage min per hit */

    if (type == SKILL_BACKSTAB && (GET_LEVEL(ch) >= 8)) {
      /*dam *= backstab_mult[(int) GET_LEVEL(ch)];*/
      dam *= ((int) (GET_LEVEL(ch)/3)+1);
      damage(ch, victim, dam, SKILL_BACKSTAB);
    } else
      damage(ch, victim, dam, w_type);
  }
  else if ((((diceroll < 20) && AWAKE(victim)) &&
       ((diceroll == 1) || ((calc_thaco - diceroll) > victim_ac)))) {
    if (type == SKILL_BACKSTAB)
      damage(ch, victim, 0, SKILL_BACKSTAB);
    else
      damage(ch, victim, 0, w_type);
  } else {
    /* okay, we know the guy has been hit.  now calculate damage. */
    dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
    dam += GET_DAMROLL(ch);

    if (wielded)
      dam += dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2));
    else {
      if (IS_NPC(ch)) {
	dam += dice(ch->mob_specials.damnodice, ch->mob_specials.damsizedice);
      } else
	dam += number(0, 2);	/* Max. 2 dam with bare hands */
    }

    if (GET_POS(victim) < POS_FIGHTING)
      dam *= 1 + (POS_FIGHTING - GET_POS(victim)) / 3;
    /* Position  sitting  x 1.33 */
    /* Position  resting  x 1.66 */
    /* Position  sleeping x 2.00 */
    /* Position  stunned  x 2.33 */
    /* Position  incap    x 2.66 */
    /* Position  mortally x 3.00 */

    dam = MAX(1, dam);		/* at least 1 hp damage min per hit */

    if (type == SKILL_BACKSTAB && (GET_LEVEL(ch) >= 8)) {
      /*dam *= backstab_mult[(int) GET_LEVEL(ch)];*/
      dam *= ((int) (GET_LEVEL(ch)/2)+1);
      damage(ch, victim, dam, SKILL_BACKSTAB);
    } else
      damage(ch, victim, dam, w_type);
  }
}



/* control the fights going on.  Called every 2 seconds from comm.c. */
void perform_violence(void)
{
  struct char_data *ch;
  extern struct index_data *mob_index;

  for (ch = combat_list; ch; ch = next_combat_list) {
    next_combat_list = ch->next_fighting;

    if (FIGHTING(ch) == NULL || ch->in_room != FIGHTING(ch)->in_room) {
      stop_fighting(ch);
      continue;
    }

    if (IS_NPC(ch)) {
      if (GET_MOB_WAIT(ch) > 0) {
	if (number(0, 2) == 0) {
	  GET_MOB_WAIT(ch) -= PULSE_VIOLENCE;
	  continue;
	}
      }
      GET_MOB_WAIT(ch) = 0;
      if (GET_POS(ch) < POS_FIGHTING)
        if (number(0, 1) == 0) {
	  GET_POS(ch) = POS_FIGHTING;
	  act("$n scrambles to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
        }
    }

    if (GET_POS(ch) < POS_FIGHTING) {
      send_to_char("You can't fight while sitting!!\r\n", ch);
      continue;
    }
    /* For some reason, the code reaches here with one of these two
     * vars NULL. Leave this check in until I can figure out why this
     * is happening. (it should at least stop the crashes.)
     *
     * - Stuart. (Danae)
     *
    if (!FIGHTING(ch) || !FIGHTING(FIGHTING(ch)))
	return;
     */
    mob_combat(ch, FIGHTING(ch));
    if (FIGHTING(ch))
      hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
    if (MOB_FLAGGED(ch, MOB_SPEC) && mob_index[GET_MOB_RNUM(ch)].func != NULL)
      (mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, "");
  }
}
void weapon_effects(struct char_data *ch, struct char_data *vict,
 struct obj_data *weapon)
{
  int n;
  /*	extern struct index_data *obj_index;
  extern struct index_data *mob_index; */
  int magic_user(struct char_data *ch, int cmd, char *arg);

  n=weapon->obj_flags.value[0];
  switch(n){
    case 0:
      if(number(3,15) < GET_INT(ch))
        call_magic(ch, vict, 0, SPELL_MAGIC_MISSILE_WPN, 50, CAST_SPELL);
      return;
    case 1:
      if(number(6,19) < GET_INT(ch))
        call_magic(ch, vict, 0, SPELL_SHOCK_WPN, 50, CAST_SPELL);
      return;
    case 2:
      if(number(10,30) < GET_INT(ch))
        call_magic(ch, vict, 0, SPELL_FIREBALL_WPN, GET_LEVEL(ch), CAST_SPELL);
      return;
    case 3:
      if(number(10,30)<GET_INT(ch))
        call_magic(ch, vict, 0, SPELL_FIREBALL_WPN, 50, CAST_SPELL);
      return;
    case 4:
      if(IS_SET(world[ch->in_room].room_flags,ROOM_PEACEFUL)){
  send_to_char("Aiieee! You are punished for using your weapon here!\n\r",
          ch);
        call_magic(ch, ch, 0, SPELL_FIREBALL_WPN, 50, CAST_SPELL);
      } else if(number(5,30) < GET_INT(ch))
        call_magic(ch, vict, 0, SPELL_CURE_WPN, 50, CAST_SPELL);
      return;
    case 5:
      if(number(15,35)<GET_INT(ch))
        call_magic(ch, vict, 0, SPELL_DISPEL_EVIL_WPN, 50, CAST_SPELL);
      return;
    case 6:
      if(number(15,35)<GET_INT(ch))
        call_magic(ch, vict, 0, SPELL_COLOR_SPRAY_WPN, 50, CAST_SPELL);
      return;
    case 7:
      if(number(12,30) < GET_INT(ch))
        call_magic(ch, vict, 0, SPELL_BOLT_WPN, GET_LEVEL(ch), CAST_SPELL);
      return;
    case 8:
      if(number(4,16) < GET_INT(ch))
        call_magic(ch, vict, 0, SPELL_CHILL_WPN, 50, CAST_SPELL);
      return;
    case 9:
      if(number(40,160) < GET_INT(ch))
        call_magic(ch, vict, 0, SPELL_HARM_WPN, 50, CAST_SPELL);
      return;
  }
}




