
/* ************************************************************************
*   File: spec_procs.c                                  Part of CircleMUD *
*  Usage: implementation of special procedures for mobiles/objects/rooms  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"


/*   external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct command_info cmd_info[];
extern int top_of_world;

/* extern functions */
void add_follower(struct char_data * ch, struct char_data * leader);
int defaulthelper = 3060;
void mob_initiate(struct char_data *ch, struct char_data *victim);

struct social_type {
  char *cmd;
  int next_line;
};


/* ********************************************************************
*  Special procedures for mobiles                                     *
******************************************************************** */

int spell_sort_info[MAX_SKILLS+1];

extern char *spells[];

void mob_teleport(struct char_data *ch, int to_room)
{
  if(!to_room){
    do {
       to_room = number(0,top_of_world-1000);
       } while (IS_SET(world[to_room].room_flags, ROOM_PRIVATE));
  } else {
    if(IS_SET(world[to_room].room_flags ,ROOM_PRIVATE | ROOM_GODROOM | ROOM_DEATH | ROOM_NOMOB))
      return;
    }
    act("$n creates an astral gate and steps into it.",FALSE,ch,0,0,TO_ROOM);
    char_from_room(ch);
    char_to_room(ch,to_room);
    act("An astral gate suddenly appears and $n steps out of it.",FALSE,ch,0,0,TO_ROOM);
}

void damage_equips(struct char_data *ch, struct char_data *vict)
{
        struct obj_data *o;
        char buf[MAX_STRING_LENGTH];

        if (ch->in_room != vict->in_room)
                return;
        o=vict->equipment[number(1,WEAR_WIELD-1)];
        if (o)
        {
                if(o->obj_flags.value[0] > 1)
                {
                        if(GET_OBJ_TYPE(o) == ITEM_ARMOR)
                        {
                                if(IS_SET(o->obj_flags.extra_flags,ITEM_HUM))
                                {
                                       REMOVE_BIT(o->obj_flags.extra_flags,ITEM_HUM);
                                        sprintf(buf,"$n kicks %s, denting it rem
oving its enchantment, CARCHINK!",o->short_description);
                                        act(buf,TRUE,ch,0,vict,TO_ROOM);
                                } else
                                {
                                       /*--GET_AC(vict);*/
                                        --o->obj_flags.value[0];
                                        sprintf(buf,"$n kicks %s, damaging it slightly, THUMP!",o->short_description);
                                        act(buf,TRUE,ch,0,vict,TO_ROOM);
                                }
                        }
                }
        }
}
SPECIAL(kickbasher)/*(struct char_data *ch, int cmd, char *arg)*/
/* PROCEDURE TO MAKE MOBS KICK PLAYERS */
{
        char buf[128]; /*name[128];*/
        struct char_data *vict, *v2;
        int i,n,dir,vno;
        struct obj_data *o,*oo,*nexto,*unequip_char(struct char_data *ch, int pos);
        ACMD(do_look);
        if(cmd)
         return(FALSE);
        if(vict==FIGHTING(ch)){
          if(GET_POS(ch) < POS_FIGHTING){
            ACMD(do_stand);
            if(number(1,100) < 25)
                return(FALSE);
          }
          if((GET_LEVEL(ch) >= LVL_GOD)&&(GET_LEVEL(vict) < LVL_GOD))
          {
                for(v2= world[ch->in_room].people; v2;v2=v2->next_in_room){
                        if(FIGHTING(v2)==ch)
                        {
                                damage(ch,v2,GET_MAX_HIT(v2)>>4,SKILL_BASH);
                                GET_POS(v2)=POS_SITTING;

                           WAIT_STATE(v2,number(0,2)*PULSE_VIOLENCE);
                        }
                }
          }
          if(!IS_NPC(vict)){
            n=number(0,4);
            if(n==0){
              if(o==vict->equipment[WEAR_WIELD]) {
                obj_to_room(o=unequip_char(vict,WEAR_WIELD),vict->in_room);
                act("$n kicks the weapon from $N's hands.",TRUE,ch,0,vict,TO_NOTVICT);
                act("$n kicks your weapon from your hands.",TRUE,ch,0,vict,TO_VICT);
                if(MOB_FLAGGED(ch,MOB_AWARE)){
                  for(oo=ch->carrying;oo;oo=nexto) {
                     nexto=oo->next_content;
                     act("$n drops $p.",1,ch,oo,0,TO_ROOM);
                     obj_from_char(oo);
                     obj_to_room(oo,ch->in_room);
                  }
                  sprintf(buf,"%s",o->name);
                  for(i=0;buf[i] && (buf[i]!=' ');++i);
                  buf[i]=0;

        if(o==get_obj_in_list_vis(ch,buf,ch->carrying)){
                    vno=number(1,2);
                    if(o->obj_flags.value[vno] > 1)
                       o->obj_flags.value[vno]--;
                  }
                }
            }
        } else if (n!=1) {
          damage_equips(ch,vict);
        } else {
          damage(ch,vict,GET_LEVEL(ch)<<2,SKILL_KICK);
          if(!(vict==FIGHTING(ch))) return(FALSE);
          if(number(1,7) < 2){
            dir = number(0,5);
            if(CAN_GO(vict,dir)){
              if(do_simple_move(vict,dir,FALSE)==1){
                act("$n's massive kick, sends $N sprawling into the next room!",
 TRUE,ch,0,vict,TO_NOTVICT);
                act("$n comes carriering in from the next room!", TRUE,vict,0,0,
TO_NOTVICT);
                act("$n kicks you flying, sending you sprawling into the next ro
om!", TRUE,ch,0,vict,TO_VICT);
                if(FIGHTING(vict))

              stop_fighting(FIGHTING(vict));
                     stop_fighting(vict);
                     GET_POS(vict)=POS_SITTING;
                     WAIT_STATE(vict,2*PULSE_VIOLENCE);
		     do_look(vict,"",15,0);
          }
        }
      }
    }
   }
  return(TRUE);
  }
  return(FALSE);
}

void sort_spells(void)
{
  int a, b, tmp;

  /* initialize array */
  for (a = 1; a < MAX_SKILLS; a++)
    spell_sort_info[a] = a;

  /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
  for (a = 1; a < MAX_SKILLS - 1; a++)
    for (b = a + 1; b < MAX_SKILLS; b++)
      if (strcmp(spells[spell_sort_info[a]], spells[spell_sort_info[b]]) > 0) {
	tmp = spell_sort_info[a];
	spell_sort_info[a] = spell_sort_info[b];
	spell_sort_info[b] = tmp;
      }
}


char *how_good(int percent)
{
  static char buf[256];

  if (percent == 0)
    strcpy(buf, " (not learned)");
  else if (percent <= 10)
    strcpy(buf, " (awful)");
  else if (percent <= 20)
    strcpy(buf, " (bad)");
  else if (percent <= 40)
    strcpy(buf, " (poor)");
  else if (percent <= 55)
    strcpy(buf, " (average)");
  else if (percent <= 70)
    strcpy(buf, " (fair)");
  else if (percent <= 80)
    strcpy(buf, " (good)");
  else if (percent <= 85)
    strcpy(buf, " (very good)");
  else
    strcpy(buf, " (superb)");

  return (buf);
}

char *prac_types[] = {
  "spell",
  "skill"
};

#define LEARNED_LEVEL	0	/* % known which is considered "learned" */
#define MAX_PER_PRAC	1	/* max percent gain in skill per practice */
#define MIN_PER_PRAC	2	/* min percent gain in skill per practice */
#define PRAC_TYPE	3	/* should it say 'spell' or 'skill'?	 */

/* actual prac_params are in class.c */
extern int prac_params[4][NUM_CLASSES];

#define LEARNED(ch) (prac_params[LEARNED_LEVEL][(int)GET_CLASS(ch)])
#define MINGAIN(ch) (prac_params[MIN_PER_PRAC][(int)GET_CLASS(ch)])
#define MAXGAIN(ch) (prac_params[MAX_PER_PRAC][(int)GET_CLASS(ch)])
#define SPLSKL(ch) (prac_types[prac_params[PRAC_TYPE][(int)GET_CLASS(ch)]])

void list_skills(struct char_data * ch)
{
  extern char *spells[];
  extern struct spell_info_type spell_info[];
  int i, sortpos, class;

  if (!GET_PRACTICES(ch))
    strcpy(buf, "You have no practice sessions remaining.\r\n");
  else
    sprintf(buf, "You have %d practice session%s remaining.\r\n",
	    GET_PRACTICES(ch), (GET_PRACTICES(ch) == 1 ? "" : "s"));

  sprintf(buf, "%sYou know of the following %ss:\r\n", buf, SPLSKL(ch));

  strcpy(buf2, buf);

  class = GET_CLASS(ch);
  if (class == -1)
    class = CLASS_WARRIOR;

  for (sortpos = 1; sortpos < MAX_SKILLS; sortpos++) {
    i = spell_sort_info[sortpos];
    if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
      strcat(buf2, "**OVERFLOW**\r\n");
      break;
    }
    if (GET_LEVEL(ch) >= spell_info[i].min_level[(int) class]) {
         sprintf(buf, "%-20s %s\r\n", spells[i], how_good(GET_SKILL(ch, i)));
         strcat(buf2, buf);
    }
  }

  page_string(ch->desc, buf2, 1);
}


SPECIAL(guild)
{
  int skill_num, percent;

  extern struct spell_info_type spell_info[];
  extern struct int_app_type int_app[26];

  if (IS_NPC(ch) || !CMD_IS("practice"))
    return 0;

  skip_spaces(&argument);

  if (!*argument) {
    list_skills(ch);
    return 1;
  }
  if (GET_PRACTICES(ch) <= 0) {
    send_to_char("You do not seem to be able to practice now.\r\n", ch);
    return 1;
  }

  skill_num = find_skill_num(argument);

  if (skill_num < 1 ||
      GET_LEVEL(ch) < spell_info[skill_num].min_level[(int) GET_CLASS(ch)]) {
    sprintf(buf, "You do not know of that %s.\r\n", SPLSKL(ch));
    send_to_char(buf, ch);
    return 1;
  }
  if (GET_SKILL(ch, skill_num) >= LEARNED(ch)) {
    send_to_char("You are already learned in that area.\r\n", ch);
    return 1;
  }
  send_to_char("You practice for a while...\r\n", ch);
  GET_PRACTICES(ch)--;

  percent = GET_SKILL(ch, skill_num);
  percent += MIN(MAXGAIN(ch), MAX(MINGAIN(ch), int_app[GET_INT(ch)].learn));

  SET_SKILL(ch, skill_num, MIN(LEARNED(ch), percent));

  if (GET_SKILL(ch, skill_num) >= LEARNED(ch))
    send_to_char("You are now learned in that area.\r\n", ch);

  return 1;
}



SPECIAL(dump)
{
  struct obj_data *k;
  int value = 0;

  ACMD(do_drop);
  char *fname(char *namelist);

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    extract_obj(k);
  }

  if (!CMD_IS("drop"))
    return 0;

  do_drop(ch, argument, cmd, 0);

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    value += MAX(1, MIN(50, GET_OBJ_COST(k) / 10));
    extract_obj(k);
  }

  if (value) {
    act("You are awarded for outstanding performance.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n has been awarded for being a good citizen.", TRUE, ch, 0, 0, TO_ROOM);

    if (GET_LEVEL(ch) < 3)
      gain_exp(ch, value);
    else
      GET_GOLD(ch) += value;
  }
  return 1;
}



SPECIAL(mayor)
{
  ACMD(do_gen_door);

  static char open_path[] =
  "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

  static char close_path[] =
  "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static char *path;
  static int index;
  static bool move = FALSE;

  if (!move) {
    if (time_info.hours == 6) {
      move = TRUE;
      path = open_path;
      index = 0;
    } else if (time_info.hours == 20) {
      move = TRUE;
      path = close_path;
      index = 0;
    }
  }
  if (cmd || !move || (GET_POS(ch) < POS_SLEEPING) ||
      (GET_POS(ch) == POS_FIGHTING))
    return FALSE;

  switch (path[index]) {
  case '0':
  case '1':
  case '2':
  case '3':
    perform_move(ch, path[index] - '0', 1);
    break;

  case 'W':
    GET_POS(ch) = POS_STANDING;
    act("$n awakens and groans loudly.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'S':
    GET_POS(ch) = POS_SLEEPING;
    act("$n lies down and instantly falls asleep.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'a':
    act("$n says 'Hello Honey!'", FALSE, ch, 0, 0, TO_ROOM);
    act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'b':
    act("$n says 'What a view!  I must get something done about that dump!'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'c':
    act("$n says 'Vandals!  Youngsters nowadays have no respect for anything!'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'd':
    act("$n says 'Good day, citizens!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'e':
    act("$n says 'I hereby declare the bazaar open!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'E':
    act("$n says 'I hereby declare Midgaard closed!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'O':
    do_gen_door(ch, "gate", 0, SCMD_UNLOCK);
    do_gen_door(ch, "gate", 0, SCMD_OPEN);
    break;

  case 'C':
    do_gen_door(ch, "gate", 0, SCMD_CLOSE);
    do_gen_door(ch, "gate", 0, SCMD_LOCK);
    break;

  case '.':
    move = FALSE;
    break;

  }

  index++;
  return FALSE;
}


/* ********************************************************************
*  General special procedures for mobiles                             *
******************************************************************** */


void npc_steal(struct char_data *ch, struct char_data *victim)
{
  int gold;

  if (IS_NPC(victim))
    return;
  if (GET_LEVEL(victim) >= LVL_IMMORT)
    return;

  if (AWAKE(victim) && (number(0, GET_LEVEL(ch)) == 0)) {
    act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
  } else {
    /* Steal some gold coins */
    gold = (int) ((GET_GOLD(victim) * number(1, 10)) / 100);
    if (gold > 0) {
      GET_GOLD(ch) += gold;
      GET_GOLD(victim) -= gold;
    }
  }
}


SPECIAL(snake)
{
  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room) &&
      (number(0, 42 - GET_LEVEL(ch)) == 0)) {
    act("$n bites $N!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
    act("$n bites you!", 1, ch, 0, FIGHTING(ch), TO_VICT);
    call_magic(ch, FIGHTING(ch), 0, SPELL_POISON, GET_LEVEL(ch), CAST_SPELL);
    return TRUE;
  }
  return FALSE;
}

/* The new spec procs to allow dual classing of mobclasses.. dont fiddle */

SPECIAL(assassin)
{
  void combat_assassin(struct char_data *ch, struct char_data *vict);

  if (FIGHTING(ch))
    combat_assassin(ch, FIGHTING(ch));

  return TRUE;
}

SPECIAL(spellcaster)
{
  void combat_spellcaster(struct char_data *ch, struct char_data *vict);

  if (FIGHTING(ch))
    combat_spellcaster(ch, FIGHTING(ch));

  return TRUE;
}

SPECIAL(fighter)
{
  void combat_fighter(struct char_data *ch, struct char_data *vict);

  if (FIGHTING(ch))
    combat_fighter(ch, FIGHTING(ch));

  return TRUE;
}

SPECIAL(healer)
{
  void combat_healer(struct char_data *ch, struct char_data *vict);

  if (FIGHTING(ch))
    combat_healer(ch, FIGHTING(ch));

  return TRUE;
}

SPECIAL(thief)
{
  struct char_data *cons;

  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POS_STANDING)
    return FALSE;

  for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room)
    if (!IS_NPC(cons) && (GET_LEVEL(cons) < LVL_IMMORT) && (!number(0, 4))) {
      npc_steal(ch, cons);
      return TRUE;
    }
  return FALSE;
}


SPECIAL(magic_user)
{
  struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
    if (FIGHTING(vict) == ch && !number(0, 4))
      break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL)
    vict = FIGHTING(ch);

  if ((GET_LEVEL(ch) > 13) && (number(0, 10) == 0))
   cast_spell(ch, vict, NULL, SPELL_SLEEP);
  if ((GET_LEVEL(ch) > 7) && (number(0, 8) == 0))
   cast_spell(ch, vict, NULL, SPELL_BLINDNESS);
  if ((GET_LEVEL(ch) > 12) && (number(0, 12) == 0)) {
    if (IS_EVIL(ch))
    cast_spell(ch, vict, NULL, SPELL_ENERGY_DRAIN);
    else if (IS_GOOD(ch))
      cast_spell(ch, vict, NULL, SPELL_DISPEL_EVIL);
  }
  if (number(0, 4))
    return TRUE;

  switch (GET_LEVEL(ch)) {
  case 4:
  case 5:
    cast_spell(ch, vict, NULL, SPELL_MAGIC_MISSILE);
    break;
  case 6:
  case 7:
    cast_spell(ch, vict, NULL, SPELL_CHILL_TOUCH);
    break;
  case 8:
  case 9:
    cast_spell(ch, vict, NULL, SPELL_BURNING_HANDS);
    break;
  case 10:
  case 11:
    cast_spell(ch, vict, NULL, SPELL_SHOCKING_GRASP);
    break;
  case 12:
  case 13:
    cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BOLT);
    break;
  case 14:
  case 15:
  case 16:
  case 17:
    cast_spell(ch, vict, NULL, SPELL_COLOR_SPRAY);
    break;
  default:
    cast_spell(ch, vict, NULL, SPELL_FIREBALL);
    break;
  }
  return TRUE;

}


/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */

SPECIAL(guild_guard)
{
  int i;
  extern int guild_info[][3];
  struct char_data *guard = (struct char_data *) me;
  char *buf = "The guard humiliates you, and blocks your way.\r\n";
  char *buf2 = "The guard humiliates $n, and blocks $s way.";

  if (!IS_MOVE(cmd) || IS_AFFECTED(guard, AFF_BLIND))
    return FALSE;

  for (i = 0; guild_info[i][0] != -1; i++) {
    if ((IS_NPC(ch) || GET_CLASS(ch) != guild_info[i][0]) &&
	world[ch->in_room].number == guild_info[i][1] &&
	cmd == guild_info[i][2]) {
      send_to_char(buf, ch);
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      return TRUE;
    }
  }

  return FALSE;
}



SPECIAL(puff)
{
  ACMD(do_say);

  if (cmd)
    return (0);

  switch (number(0, 60)) {
  case 0:
    do_say(ch, "My god!  It's full of stars!", 0, 0);
    return (1);
  case 1:
    do_say(ch, "How'd all those fish get up here?", 0, 0);
    return (1);
  case 2:
    do_say(ch, "I'm a very female dragon.", 0, 0);
    return (1);
  case 3:
    do_say(ch, "I've got a peaceful, easy feeling.", 0, 0);
    return (1);
  default:
    return (0);
  }
}



SPECIAL(fido)
{

  struct obj_data *i, *temp, *next_obj;

  if (cmd || !AWAKE(ch))
    return (FALSE);


switch(number(0, 3)) 
	{
       	case 0:  /* fido action */
      	  act("$n sniffs the ground.", FALSE, ch, 0, 0,TO_ROOM);
      	  return(1);
       	case 1:  /* fido action */
      	  act("$n looks at you and wags its tail.", FALSE, ch, 0, 0,TO_ROOM);
      	  return(1);
       	case 2:  /* fido action */
  	  for (i = world[ch->in_room].contents; i; i = i->next_content) 
		{
    		if (GET_OBJ_TYPE(i) == ITEM_CONTAINER && GET_OBJ_VAL(i, 3)) 
			{
      			act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
      			for (temp = i->contains; temp; temp = next_obj) 
				{
				next_obj = temp->next_content;
				obj_from_obj(temp);
				obj_to_room(temp, ch->in_room);
      				}
      			extract_obj(i);
      			return (TRUE);
    			}
  		}
  		return (FALSE);
    		break;
	}	
return (FALSE);
}

 

SPECIAL(cat)
{

  struct obj_data *i, *temp, *next_obj;

  if (cmd || !AWAKE(ch))
    return (FALSE);


switch(number(0, 3)) 
	{
       	case 0:  /* cat action */
      	  act("$n rubs up against your leg, trying to get a pat.", FALSE, ch, 0, 0,TO_ROOM);
      	  return(1);
       	case 1:  /* cat action */
      	  act("$n looks at you and MEEOW'S in the cutest of ways.", FALSE, ch, 0, 0,TO_ROOM);
      	  return(1);
       	case 2:  /* cat action */
  	  for (i = world[ch->in_room].contents; i; i = i->next_content) 
		{
    		if (GET_OBJ_TYPE(i) == ITEM_CONTAINER && GET_OBJ_VAL(i, 3)) 
			{
      			act("$n starts playing with a corpse.", FALSE, ch, 0, 0, TO_ROOM);
      			act("Somehow $n manages to bat the corpse in the air..... Hey it did not come back down.", FALSE, ch, 0, 0, TO_ROOM);
      			for (temp = i->contains; temp; temp = next_obj) 
				{
				next_obj = temp->next_content;
				obj_from_obj(temp);
				obj_to_room(temp, ch->in_room);
      				}
      			extract_obj(i);
      			return (TRUE);
    			}
  		}
  		return (FALSE);
    		break;
	}	
return (FALSE);
}



SPECIAL(rat)
{

  struct obj_data *i, *temp, *next_obj;

  if (cmd || !AWAKE(ch))
    return (FALSE);


switch(number(0, 3)) 
	{
       	case 0:  /* rat action */
      	  act("$n lifts it's nose in the air and sniffs twitching.", FALSE, ch, 0, 0,TO_ROOM);
      	  return(1);
       	case 1:  /* rat action */
      	  act("$n looks at you and skitters to the nearest wall.", FALSE, ch, 0, 0,TO_ROOM);
      	  return(1);
       	case 2:  /* rat action */
  	  for (i = world[ch->in_room].contents; i; i = i->next_content) 
		{
    		if (GET_OBJ_TYPE(i) == ITEM_CONTAINER && GET_OBJ_VAL(i, 3)) 
			{
      			act("$n leaps onto a corpse and eats it all from the inside.", FALSE, ch, 0, 0, TO_ROOM);
      			for (temp = i->contains; temp; temp = next_obj) 
				{
				next_obj = temp->next_content;
				obj_from_obj(temp);
				obj_to_room(temp, ch->in_room);
      				}
      			extract_obj(i);
      			return (TRUE);
    			}
  		}
  		return (FALSE);
    		break;
	}	
return (FALSE);
}



SPECIAL(janitor)
{
  struct obj_data *i;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content) {
    if (!CAN_WEAR(i, ITEM_WEAR_TAKE))
      continue;
    if (GET_OBJ_TYPE(i) != ITEM_DRINKCON && GET_OBJ_COST(i) >= 15)
      continue;
    act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
    obj_from_room(i);
    obj_to_char(i, ch);
    return TRUE;
  }

  return FALSE;
}


SPECIAL(cityguard)
{
  struct char_data *tch, *evil;
  int max_evil;

  if (cmd || !AWAKE(ch) || FIGHTING(ch))
    return FALSE;

  max_evil = 1000;
  evil = 0;

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (!IS_NPC(tch) && CAN_SEE(ch, tch) && IS_SET(PLR_FLAGS(tch), PLR_KILLER)) {
      act("$n calls down the power of the gods to strike $N!'", 1, ch, 0, tch, TO_ROOM);
      GET_HIT(tch) = (int) (GET_HIT(tch) / 2);
      GET_MANA(tch) = (int) (GET_MANA(tch) / 2);
      GET_MOVE(tch) = (int) (GET_MANA(tch) / 2);
      act("$n strikes at $N's very soul and screams, 'BEGONE SCUM PLAYER KILLER!!!!!!'", 1, ch, 0, tch, TO_NOTVICT);
      act("$n strikes you to your very soul, screaming, 'BEGONE SCUM PLAYER KILLER!!!!!!'", 1, ch, 0, tch, TO_VICT);
      act("You feel your soul being ripped and torn before the awesome power of the gods!", 1, ch, 0, tch, TO_VICT);
      /* hit(ch, tch, TYPE_UNDEFINED); */
      return (TRUE);
    }
  }

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (!IS_NPC(tch) && CAN_SEE(ch, tch) && IS_SET(PLR_FLAGS(tch), PLR_THIEF)){
      act("$n says 'OOOOOOOOO Look.. one of those PLAYER THIEVES!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
      /* hit(ch, tch, TYPE_UNDEFINED); */
      return (TRUE);
    }
  }

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) 
  {
    if (CAN_SEE(ch, tch) && FIGHTING(tch)) 
    {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
	  (IS_NPC(tch) || IS_NPC(FIGHTING(tch)))) 
      {
	max_evil = GET_ALIGNMENT(tch);
	evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(FIGHTING(evil)) >= 0)) {
    act("$n screams 'PROTECT THE INNOCENT!  BANZAI!  CHARGE!  ARARARAGGGHH!'", FALSE, ch, 0, 0, TO_ROOM);
    mob_initiate(ch, evil);
    return (TRUE);
  }
  return (FALSE);
}


SPECIAL(pet_shops)
{
  char buf[MAX_STRING_LENGTH], pet_name[256];
  int pet_room;
  struct char_data *pet;

  pet_room = ch->in_room + 1;

  if (CMD_IS("list")) {
    send_to_char("Available pets are:\r\n", ch);
    for (pet = world[pet_room].people; pet; pet = pet->next_in_room) {
      sprintf(buf, "%8d - %s\r\n", 3 * GET_EXP(pet), GET_NAME(pet));
      send_to_char(buf, ch);
    }
    return (TRUE);
  } else if (CMD_IS("buy")) {

    argument = one_argument(argument, buf);
    argument = one_argument(argument, pet_name);

    if (!(pet = get_char_room(buf, pet_room))) {
      send_to_char("There is no such pet!\r\n", ch);
      return (TRUE);
    }
    if (GET_GOLD(ch) < (GET_EXP(pet) * 3)) {
      send_to_char("You don't have enough gold!\r\n", ch);
      return (TRUE);
    }
    GET_GOLD(ch) -= GET_EXP(pet) * 3;

    pet = read_mobile(GET_MOB_RNUM(pet), REAL);
    GET_EXP(pet) = 0;
    SET_BIT(AFF_FLAGS(pet), AFF_CHARM);

    if (*pet_name) {
      sprintf(buf, "%s %s", pet->player.name, pet_name);
      /* free(pet->player.name); don't free the prototype! */
      pet->player.name = str_dup(buf);

      sprintf(buf, "%sA small sign on a chain around the neck says 'My name is %s'\r\n",
	      pet->player.description, pet_name);
      /* free(pet->player.description); don't free the prototype! */
      pet->player.description = str_dup(buf);
    }
    char_to_room(pet, ch->in_room);
    add_follower(pet, ch);

    /* Be certain that pets can't get/carry/use/wield/wear items */
    IS_CARRYING_W(pet) = 1000;
    IS_CARRYING_N(pet) = 100;

    send_to_char("May you enjoy your pet.\r\n", ch);
    act("$n buys $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

    return 1;
  }
  /* All commands except list and buy */
  return 0;
}

/* 
 *  File: sund_procs.c                          Part of Exile MUD
 *  
 *  Special procedures for the mobs and objects of Sundhaven.
 *
 *  Exile MUD is based on CircleMUD, Copyright (C) 1993, 1994.
 *  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.       
 *
 */


/*  Mercy's Procs for the Town of Sundhaven  */


SPECIAL(silktrader)
{
  ACMD(do_say);
  if (cmd)
    return 0;

  if (world[ch->in_room].sector_type == SECT_CITY)
  switch (number(0, 30)) {
   case 0:
      act("$n eyes a passing woman.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "Come, m'lady, and have a look at this precious silk!", 0, 0);
      return(1);
   case 1:
      act("$n says to you, 'Wouldn't you look lovely in this!'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n shows you a gown of indigo silk.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 2:
      act("$n holds a pair of silk gloves up for you to inspect.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 3:
      act("$n cries out, 'Have at this fine silk from exotic corners of the world you will likely never see!", FALSE, ch, 0, 0,TO_ROOM);
      act("$n smirks.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 4:
      do_say(ch, "Step forward, my pretty locals!", 0, 0);
      return(1);
   case 5:
      act("$n shades his eyes with his hand.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 6:
      do_say(ch, "Have you ever seen an ogre in a silken gown?", 0, 0);
      do_say(ch, "I didn't *think* so!", 0, 0);
      act("$n throws his head back and cackles with insane glee!", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 7:
      act("$n hands you a glass of wine.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "Come, have a seat and view my wares.", 0, 0);
      return(1);
   case 8:
      act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n shakes his head sadly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 9:
      act("$n fiddles with some maps.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 10:
      do_say(ch, "Here here! Beggars and nobles alike come forward and make your bids!", 0, 0);
      return(1);
   case 11:
      do_say(ch, "I am in this bourgeois hamlet for a limited time only!", 0, 0);
      act("$n swirls some wine in a glass.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
  }

  if (world[ch->in_room].sector_type != SECT_CITY)
  switch (number(0, 20)) {
   case 0:
      do_say(ch, "Ah! Fellow travellers! Come have a look at the finest 
silk this side of the infamous Ched Razimtheth!", 0, 0);
      return(1);
   case 1:
      act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "You are feebly attired for the danger that lies 
ahead.", 0, 0);
      do_say(ch, "Silk is the way to go.", 0, 0);
      act("$n smiles warmly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 2:
      do_say(ch, "Worthy adventurers, hear my call!", 0, 0);
      return(1);
   case 3:
      act("$n adjusts his cloak.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 4:
      act("$n says to you, 'Certain doom awaits you, therefore shall you die in silk.'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n bows respectfully.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 5:
      do_say(ch, "Can you direct me to the nearest tavern?", 0, 0);
      return(1);
   case 6:
      do_say(ch, "Heard the latest ogre joke?", 0, 0);
      act("$n snickers to himself.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 7:
      do_say(ch, "What ho, traveller! Rest your legs here for a spell and peruse the latest in fashion!", 0, 0);
      return(1);
   case 8:
      do_say(ch, "Beware ye, traveller, lest ye come to live in Exile!", 0, 0);
      act("$n grins evilly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 9:
      act("$n touches your shoulder.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "A word of advice. Beware of any ale labled 'mushroom' or 'pumpkin'.", 0, 0);
      act("$n shivers uncomfortably.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
  
  }
  return(0);
}


SPECIAL(athos)
{
    ACMD(do_say);
  if(cmd)
   return 0;
    switch (number(0, 20)) {
    case 0:
      act("$n gazes into his wine gloomily.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
    case 1:
      act("$n grimaces.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
    case 2:
      act("$n asks you, 'Have you seen the lady, pale and fair, with a heart of stone?'", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "That monster will be the death of us all.", 0, 0);
      return(1);
    case 3:
      do_say(ch, "God save the King!", 0, 0);
      return(1);
    case 4:
      do_say(ch, "All for one and .. one for...", 0, 0);
      act("$n drowns himself in a swig of wine.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
    case 5:
      act("$n looks up with a philosophical air.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "Women - God's eternal punishment on man.", 0, 0);
      return(1);
    case 6:
      act("$n downs his glass and leans heavily on the oaken table.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "You know, we would best band together and wrestle the monstrous woman from her lair and home!", 0, 0);
      return(1);
  default: return(FALSE);
                break; }
    return(0);
}



SPECIAL(hangman)
{
  ACMD(do_say);
if(cmd) return 0;
  switch (number(0, 15)) {
  case 0:
    act("$n whirls his noose like a lasso and it lands neatly around your 
neck.", FALSE, ch, 0, 0,TO_ROOM);
    do_say(ch, "You're next, you ugly rogue!", 0, 0);
    do_say(ch, "Just kidding.", 0, 0);
    act("$n pats you on your head.", FALSE, ch, 0, 0,TO_ROOM);
    return(1);
  case 1:
    do_say(ch, "I was conceived in Exile and have been integrated into 
society!", 0, 0);
    do_say(ch, "Muahaha!", 0, 0);
    return(1);
  case 2:
    do_say(ch, "Anyone have a butterknife I can borrow?", 0, 0);
    return(1);
  case 3:
    act("$n suddenly pulls a lever.", FALSE, ch, 0, 0,TO_ROOM);
    act("With the flash of light on metal a giant guillotine comes 
crashing down!", FALSE, ch, 0, 0,TO_ROOM);
    act("A head drops to the ground from the platform.", FALSE, ch, 0, 0,TO_ROOM);
    act("$n looks up and shouts wildly.", FALSE, ch, 0, 0,TO_ROOM);
    act("$n shouts, 'Next!'", FALSE, ch, 0, 0, TO_ROOM); 
    return(1);
  case 4:
   act("$n whistles a local tune.", FALSE, ch, 0, 0,TO_ROOM);
   return(1);
   default:
     return(FALSE);
     break;
  }
  return(0);
}   



SPECIAL(butcher)
{
  ACMD(do_say);
if(cmd) return 0;
  switch (number(0, 40)) {
   case 0:
      do_say(ch, "I need a Union.", 0, 0);
      act("$n glares angrily.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n rummages about for an axe.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 1:
      act("$n gnaws on a toothpick.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 2:
      act("$n runs a finger along the edge of a giant meat cleaver.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n grins evilly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 3:
      do_say(ch, "Pork for sale!", 0, 0);
      return(1);
   case 4:
      act("$n whispers to you, 'I've got some great damage eq in the back room. Wanna see?'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n throws back his head and cackles with insane glee!", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 5:
      act("$n yawns.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 6:
      act("$n throws an arm around the headless body of an ogre and asks to have his picture taken.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 7:
      act("$n listlessly grabs a cleaver and hurls it into the wall behind your head.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 8:
      act("$n juggles some fingers.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 9:
      act("$n eyes your limbs.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n chuckles.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 10:
      do_say(ch, "Hi, Alice.", 0, 0);
      return(1);
   case 11:
      do_say(ch, "Everyone looks like food to me these days.", 0, 0);
      act("$n sighs loudly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 12:
      act("$n throws up his head and shouts wildly.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n shouts, 'Bring out your dead!'", FALSE, ch, 0, 0, TO_ROOM);
      return(1);
   case 13:
      do_say(ch, "The worms crawl in, the worms crawl out..", 0, 0);
      return(1);
   case 14:
      act("$n sings 'Brave, brave Sir Patton...'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n whistles a tune.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n smirks.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 15:
      do_say(ch, "Get Lurch to bring me over a case and I'll sport you a year's supply of grilled ogre.", 0, 0);
      return(1);
    default: return(FALSE);
                break; }
    return(0);
}



SPECIAL(stu)
{
  ACMD(do_say);
  ACMD(do_flee);
  if(cmd)
    return 0;
  switch (number(0, 60)) {
    case 0:
      do_say(ch, "I'm so damn cool, I'm too cool to hang out with myself!", 0, 0);
      break;
    case 1:
      do_say(ch, "I'm really the NICEST guy you ever MEET!", 0, 0);
      break;
    case 2:
      do_say(ch, "Follow me for exp, gold and lessons in ADVANCED C!", 0, 0);
      break;
    case 3:
      do_say(ch, "Mind if I upload 200 megs of pregnant XXX gifs with no descriptions to your bbs?", 0, 0);
      break;
    case 4:
      do_say(ch, "Sex? No way! I'd rather jog 20 miles!", 0, 0);
      break;
    case 5:
      do_say(ch, "I'll take you OUT!!   ...tomorrow", 0, 0);
      break;
    case 6:
      do_say(ch, "I invented Mud you know...", 0, 0);
      break;
    case 7:
      do_say(ch, "Can I have a cup of water?", 0, 0);
      break;
    case 8:
      do_say(ch, "I'll be jogging down ventnor ave in 10 minutes if you want some!", 0, 0);
      break;
    case 9:
      do_say(ch, "Just let me pull a few strings and I'll get ya a site, they love me! - doesnt everyone?", 0, 0);
      break;
    case 10:
      do_say(ch, "Pssst! Someone tell Mercy to sport me some levels.", 0, 0);
      act("$n nudges you with his elbow.", FALSE, ch, 0, 0,TO_ROOM);
      break;
    case 11:
      do_say(ch, "Edgar! Buddy! Let's group and hack some ogres to tiny quivering bits!", 0, 0);
      break;
    case 12:
      act("$n tells you, 'Skylar has bad taste in women!'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n screams in terror!", FALSE, ch, 0, 0,TO_ROOM);
      do_flee(ch, 0, 0, 0);
      break;
    case 13:
      if (number(0, 32767)<10){
      act("$n whispers to you, 'Dude! If you fucking say 'argle bargle' to the glowing fido he'll raise you a level!'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n flexes.", FALSE, ch, 0, 0,TO_ROOM);}
      return(1);
    default:
      return(FALSE);
      break;
   return(1);
  }
  return 0;
}


SPECIAL(sund_earl)
{
  ACMD(do_say);
  if (cmd)
    return(FALSE);
  switch (number(0, 20)) {
   case 0:
      do_say(ch, "Lovely weather today.", 0, 0);
      return(1);
   case 1:
    act("$n practices a lunge with an imaginary foe.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 2:
      do_say(ch, "Hot performance at the gallows tonight.", 0, 0);
     act("$n winks suggestively.", FALSE, ch, 0, 0,TO_ROOM); 
     return(1);
   case 3:
      do_say(ch, "Must remember to up the taxes at my convenience.", 0, 0);
      return(1);
   case 4:
      do_say(ch, "Sundhaven is impermeable to the enemy!", 0, 0);
      act("$n growls menacingly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
 case 5:
      do_say(ch, "Decadence is the credence of the abominable.", 0, 0);
      return(1);
 case 6:
      do_say(ch, "I look at you and get a wonderful sense of impending doom.", 0, 0);
      act("$n chortles merrily.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
 case 7:
      act("$n touches his goatee ponderously.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
 case 8:
      do_say(ch, "It's Mexican Madness night at Maynards!", 0, 0);
      act("$n bounces around.", FALSE, ch, 0, 0, TO_ROOM);
      return(1); 
    default: return(FALSE);
              break;
    return(0);  
 }
}


SPECIAL(blinder)
{
  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room) &&
      (number(0, 100)+GET_LEVEL(ch) >= 50)) {
    act("$n whispers, 'So, $N! You wouldst share my affliction!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
    act("$n whispers, 'So, $N! You wouldst share my affliction!", 1, ch, 0, FIGHTING(ch), TO_VICT);
    act("$n's frayed cloak blows as he points at $N.", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
    act("$n's frayed cloak blows as he aims a bony finger at you.", 1, ch, 0, FIGHTING(ch), TO_VICT);
    act("A flash of pale fire explodes in $N's face!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
    act("A flash of pale fire explodes in your face!", 1, ch, 0, FIGHTING(ch), TO_VICT);
    call_magic(ch, FIGHTING(ch), 0, SPELL_BLINDNESS, GET_LEVEL(ch), CAST_SPELL);
    return TRUE;
  }
  return FALSE;
}


SPECIAL(idiot)
{
  ACMD(do_say);
if(cmd) return FALSE;
  switch (number(0, 40)) {
   case 0:
      do_say(ch, "even if idiot = god", 0, 0);
      do_say(ch, "and Stu = idiot", 0, 0);
      do_say(ch, "Stu could still not = god.", 0, 0);
      act("$n smiles.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 1:
      act("$n balances a newbie sword on his head.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 2:
      act("$n doesn't think you could stand up to him in a duel.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 3:
      do_say(ch, "Rome really was built in a day.", 0, 0);
      act("$n snickers.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 4:
      act("$n flips over and walks around on his hands.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 5:
      act("$n cartwheels around the room.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 6:
      do_say(ch, "How many ogres does it take to screw in a light bulb?", 0, 0);
      act("$n stops and whaps himself upside the head.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 7:
      do_say(ch, "Uh huh. Uh huh huh.", 0, 0);
      return TRUE;
   case 8:
      act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n whistles quietly.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 9:
      act("$n taps out a tune on your forehead.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 10:
      act("$n has a battle of wits with himself and comes out unharmed.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 11:
      do_say(ch, "All this and I am just a number.", 0, 0);
      act("$n cries on your shoulder.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 12:
      do_say(ch, "A certain hunchback I know dresses very similar to you, very similar...", 0, 0);
      return TRUE;
   default: 
      return FALSE;
  }
 return FALSE;
}
     

SPECIAL(marbles)
{
  struct obj_data *tobj = me;

  if (tobj->in_room == NOWHERE)
    return 0;

  if (CMD_IS("north") || CMD_IS("south") || CMD_IS("east") || CMD_IS("west") ||
      CMD_IS("up") || CMD_IS("down")) {
    if (number(1, 100) + GET_DEX(ch) > 50) {
      act("You slip on $p and fall.", FALSE, ch, tobj, 0, TO_CHAR);
      act("$n slips on $p and falls.", FALSE, ch, tobj, 0, TO_ROOM);
      GET_POS(ch) = POS_SITTING;
      return 1;
    }
    else {
      act("You slip on $p, but manage to retain your balance.", FALSE, ch, tobj, 0, TO_CHAR);
      act("$n slips on $p, but manages to retain $s balance.", FALSE, ch, tobj, 0, TO_ROOM);
    }
  }
  return 0;
}

SPECIAL(icewizard){

  struct char_data *tch, *vict;
  int low_on_hits= 10000;

  if (cmd)
    return (FALSE);

  /* Find out who has the lowest hitpoints and burn his ass off */
  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room){
    if ((!IS_NPC(tch)) && (GET_LEVEL(tch) < LVL_GOD))
      if (tch->points.hit < low_on_hits){
	low_on_hits = tch->points.hit;
	vict = tch;
      }
   if((!IS_NPC(tch)) && (GET_LEVEL(tch) < LVL_GOD)){
  act("$n screams 'Bonjour! you tiny, little looser!!'", FALSE, ch, 0, 0, TO_ROOM);
  act("$n looks at $N", 1, ch, 0, vict, TO_NOTVICT);
  act("$n looks at YOU!", 1, ch, 0, vict, TO_VICT);
  cast_spell(ch, vict, 0, SPELL_FIREBALL);
  return TRUE;
   }
  else {
   act("$n begs a god for forgiveness", 1, ch, 0, vict, TO_ROOM);
  }
  }  
  
  return FALSE;
}

SPECIAL(big_dragon)
{
  int mh;
  int dam,level;
  struct obj_data *burn, *frozen;
  struct char_data *tch, *victim;
  extern int mag_savingthrow(struct char_data * ch, int type);


  level=GET_LEVEL(ch);
  if((CMD_IS("steal"))&&(GET_LEVEL(ch) < LVL_IMMORT)){
    WAIT_STATE(ch,10*PULSE_VIOLENCE);
    return(TRUE);
  }
  if(CMD_IS("backstab"))  /* backstab */
    for(tch=world[ch->in_room].people ; tch ; tch=tch->next_in_room)
      if(MOB_FLAGGED(ch, MOB_SPEC))
        if(mob_index[ch->nr].func==big_dragon){
          act("$n utters the words 'qassir plaffa'.", 1,ch, 0, 0, TO_ROOM);
	 dam = (level*level)>>1;
        victim=FIGHTING(ch);
  	if(mag_savingthrow(victim, SAVING_BREATH) )
    	dam >>= 1;
  	damage(ch, victim, dam, SPELL_FIRE_BREATH);
/* And now for the damage on inventory */
 
  if (!mag_savingthrow(victim, SAVING_BREATH) ) {
    for(burn=victim->carrying ; burn ; burn=burn->next_content){
      if(number(1,7) == 3){
        if((burn->obj_flags.type_flag==ITEM_CONTAINER)&&
	  (IS_SET(burn->obj_flags.extra_flags,ITEM_SFX)))
          return FALSE;
        else {
          act("$o burns to charred remains",0,victim,burn,0,TO_CHAR);
          extract_obj(burn);
          return TRUE;
	     }
        }
     }
  }
         return FALSE;
        }
  if(cmd) return FALSE;
  mh=0x7fff;
  for(tch=world[ch->in_room].people; tch; tch = tch->next_in_room )
  if (FIGHTING(ch)!=NULL){
  /*damage_equips(ch,FIGHTING(ch));*/
  act("$n utters the words 'qassir exhalies'.", 1, ch, 0, 0, TO_ROOM);
 dam = level*level;
        victim=FIGHTING(ch);
  if ( mag_savingthrow(victim, SAVING_BREATH) )
    dam >>= 1;
  damage(ch, victim, dam, SPELL_GAS_BREATH);
	}

  if (FIGHTING(ch) != NULL){
  act("$n utters the words 'qassir iceisca'.", 1, ch, 0, 0, TO_ROOM);
dam = (level*level)>>2;
        victim=FIGHTING(ch);
  if ( mag_savingthrow(victim, SAVING_BREATH) )
    dam >>= 1;
damage(ch, victim, dam, SPELL_FROST_BREATH);
 
  if (!mag_savingthrow(victim, SAVING_BREATH)){
    for(frozen=victim->carrying ; 
      frozen && (frozen->obj_flags.type_flag!=ITEM_DRINKCON) && 
      (frozen->obj_flags.type_flag!=ITEM_POTION);
       frozen=frozen->next_content); 
    if(frozen) {
      act("$o freezes then shatters.",0,victim,frozen,0,TO_CHAR);
      extract_obj(frozen);
      if(IS_SET(world[victim->in_room].room_flags,ROOM_DARK))
      if(frozen==victim->equipment[WEAR_LIGHT]){
        if(frozen->obj_flags.value[2]){
          world[victim->in_room].light--;
          frozen->obj_flags.value[2]=0;
	  act("$o is extinguished.",0,victim,frozen,0,TO_CHAR);
	}
      }
    }
  }
}

  if (FIGHTING(ch)!=NULL){
  act("$n utters the words 'qassir boltata'.", 1, ch, 0, 0, TO_ROOM);
  dam = (level*level)>>2;
        victim=FIGHTING(ch);
  if ( mag_savingthrow(victim, SAVING_BREATH) )
    dam >>= 1;
  damage(ch, victim, dam, SPELL_LIGHTNING_BREATH);
	}

  if (FIGHTING(ch)!=NULL){
  act("$n utters the words 'qassir fireata'.", 1, ch, 0, 0, TO_ROOM);
 dam = (level*level)>>1;
        victim=FIGHTING(ch);
  if(mag_savingthrow(victim, SAVING_BREATH) )
    dam >>= 1;
  damage(ch, victim, dam, SPELL_FIRE_BREATH);
 
  /* And now for the damage on inventory */
 
  if (!mag_savingthrow(victim, SAVING_BREATH) ) {
    for(burn=victim->carrying ; burn ; burn=burn->next_content){
      if(number(1,7) == 3){
        if((burn->obj_flags.type_flag==ITEM_CONTAINER)&&
           (IS_SET(burn->obj_flags.extra_flags,ITEM_SFX)))
          return FALSE;
    act("$o burns to charred remains",0,victim,burn,0,TO_CHAR);
          extract_obj(burn);
          return TRUE;
        }
      }
    }
}

    if(FIGHTING(ch)!=NULL){
  act("$n utters the words 'qassir acidica'.", 1, ch, 0, 0, TO_ROOM);
  dam = (level*level)>>1;
        victim=FIGHTING(ch);
  if(mag_savingthrow(victim, SAVING_BREATH) )
    dam >>= 1;
  damage(ch, victim, dam, SPELL_ACID_BREATH);
 
  /* And now for the damage on inventory */
 
  if (!mag_savingthrow(victim, SAVING_BREATH) ) {
    for(burn=victim->carrying ; burn ; burn=burn->next_content){
      if(number(1,7) == 3){
        if((burn->obj_flags.type_flag==ITEM_CONTAINER)&&
           (IS_SET(burn->obj_flags.extra_flags,ITEM_SFX)))
          return FALSE;
        else {
          act("$o melts away to nothing",0,victim,burn,0,TO_CHAR);
          extract_obj(burn);
          return TRUE;
    }
      }
    }
  }
}
  return TRUE;
}

SPECIAL(little_dragon)
{
 int mh;
  int dam,level;
  struct obj_data *burn;
  struct char_data *tch, *victim;
  extern int mag_savingthrow(struct char_data * ch, int type);
 

if(CMD_IS("backstab"))  /* backstab */
    for(tch=world[ch->in_room].people ; tch ; tch=tch->next_in_room)
      if(MOB_FLAGGED(ch, MOB_SPEC))
        if(mob_index[ch->nr].func==big_dragon){
          act("$n utters the words 'qassir plaffa'.", 1,ch, 0, 0, TO_ROOM);
         dam = (level*level)>>1;
        victim=FIGHTING(ch);
        if(mag_savingthrow(victim, SAVING_BREATH) )
        dam >>= 1;
        damage(ch, victim, dam, SPELL_FIRE_BREATH);
/* And now for the damage on inventory */
 
  if (!mag_savingthrow(victim, SAVING_BREATH) ) {
    for(burn=victim->carrying ; burn ; burn=burn->next_content){
      if(number(1,7) == 3){
        if((burn->obj_flags.type_flag==ITEM_CONTAINER)&&
          (IS_SET(burn->obj_flags.extra_flags,ITEM_SFX)))
          return FALSE;
        else {
          act("$o burns to charred remains",0,victim,burn,0,TO_CHAR);
          extract_obj(burn);
          return TRUE;
             }
     }
     }
  }
         return FALSE;
        }
 if(cmd) return FALSE;
  mh=0x7fff;
  for(tch=world[ch->in_room].people; tch; tch = tch->next_in_room )
  if (FIGHTING(ch)!=NULL){
  act("$n utters the words 'qassir fireata'.", 1, ch, 0, 0, TO_ROOM);
 dam = (level*level)>>1;
        victim=FIGHTING(ch);
  if(mag_savingthrow(victim, SAVING_BREATH) )
    dam >>= 1;
  damage(ch, victim, dam, SPELL_FIRE_BREATH);
 
  /* And now for the damage on inventory */
 
  if (!mag_savingthrow(victim, SAVING_BREATH) ) {
    for(burn=victim->carrying ; burn ; burn=burn->next_content){
      if(number(1,7) == 3){
  if((burn->obj_flags.type_flag==ITEM_CONTAINER)&&
           (IS_SET(burn->obj_flags.extra_flags,ITEM_SFX)))
          return FALSE;
    act("$o burns to charred remains",0,victim,burn,0,TO_CHAR);
          extract_obj(burn);
          return TRUE;
        }
      }
    }
  }
	return(FALSE);
}


/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */


SPECIAL(bank)
{
  int amount;

  if (CMD_IS("balance")) {
    if (GET_BANK_GOLD(ch) > 0)
      sprintf(buf, "Your current balance is %d coins.\r\n",
	      GET_BANK_GOLD(ch));
    else
      sprintf(buf, "You currently have no money deposited.\r\n");
    send_to_char(buf, ch);
    return 1;
  } else if (CMD_IS("deposit")) {
    if ((amount = atoi(argument)) <= 0) {
      send_to_char("How much do you want to deposit?\r\n", ch);
      return 1;
    }
    if (GET_GOLD(ch) < amount) {
      send_to_char("You don't have that many coins!\r\n", ch);
      return 1;
    }
    GET_GOLD(ch) -= amount;
    GET_BANK_GOLD(ch) += amount;
    sprintf(buf, "You deposit %d coins.\r\n", amount);
    send_to_char(buf, ch);
    act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
    return 1;
  } else if (CMD_IS("withdraw")) {
    if ((amount = atoi(argument)) <= 0) {
      send_to_char("How much do you want to withdraw?\r\n", ch);
      return 1;
    }
    if (GET_BANK_GOLD(ch) < amount) {
      send_to_char("You don't have that many coins deposited!\r\n", ch);
      return 1;
    }
    GET_GOLD(ch) += amount;
    GET_BANK_GOLD(ch) -= amount;
    sprintf(buf, "You withdraw %d coins.\r\n", amount);
    send_to_char(buf, ch);
    act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
    return 1;
  } else
    return 0;
}

/* #####################################################################
####  New Zone Special Procedures that are added  Graeme Mc donough  ###
####################################################################### */

 
/* Mob for Canibal Haven */
SPECIAL(headhunter)
{
  struct char_data *tch;
 
     if (cmd || !AWAKE(ch) || (GET_POS(ch) == POS_FIGHTING))
      return (FALSE);
 
     for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) 
     {
         if (!IS_NPC(tch) && (GET_LEVEL(tch) < LVL_GOD)) 
         {
              act("$n says 'UMMBA WALA WALA. YUM YUM. OSSHA AAAAARRRRGGGHH'", FALSE, ch, 0, 0, TO_ROOM);
	      mob_initiate(ch, tch);
              return(TRUE);
         }
         else {
              act("$n grovels in the dirt. He has seen a GOD ", FALSE, ch, 0, 0, TO_ROOM);
              return(TRUE);
         }
 
      }
  return FALSE;
}
 
 
/* Mob for Canibal Haven */
SPECIAL(warrior)
{
  struct char_data *tch;
 
     if (cmd || !AWAKE(ch) || (GET_POS(ch) == POS_FIGHTING))
      return (FALSE);
 
     for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room)
     {
         if (!IS_NPC(tch) && (GET_LEVEL(tch) < LVL_GOD))
         {
              act("$n says 'NIGG NIGG OOLA JA OOOKK OSSHA YYYAAAAARRRRGGGHH'", FALSE, ch, 0, 0, TO_ROOM);
              mob_initiate(ch, tch);
              return(TRUE);
         }
         else {
              act("$n grovels in the dirt. He has seen a GOD ", FALSE, ch, 0, 0, TO_ROOM);
              return(TRUE);
         }
      }
  return FALSE;
}


/* Fountain Menic mob */
SPECIAL(pink)
{
   ACMD(do_say);
 
  if(cmd)
   return (0);
 
  switch (number(0, 60)) 
    {
       case 0:
          act("The $n Squirts water at you.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 1:
          do_say(ch, "oooooo  look at him", 0, 0);
          act("The $n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 2:
          act("The $n fills your container with water.", FALSE, ch, 0,0,TO_ROOM);
          return (1);
       case 3:
          act("The $n looks in your ear.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 4:
          act("The $n hops up and down.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 5:
          act("EEEEwwww  Yellow water .", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 6:
          act("The $n shows you his big trunk.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 7:
          act("The $n shows you his big trunk.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 8:
          return (FALSE);
      }
      return FALSE;
}
 
 
/* Canibal Haven mob */
SPECIAL(bodyguard)
{
 struct char_data *tch;
   ACMD(do_say);
 
  if(cmd)
   return (0);
 
  switch (number(0, 60)) 
    {
       case 0:
          do_say(ch, "UMMBAHH", 0, 0);
          act("$n snarls at you.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 1:
          act("$n turns its head and sends a piercing glare at you.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 2:
          act("$n wavers  his fist menacingly at you.", FALSE, ch, 0,0,TO_ROOM);
          return (1);
       case 3:
          do_say(ch, "MAGAMMWW.", 0, 0);
          act("$n grunts.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 4:
          act("$n Moves towards you and is stopped by the chief.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 5:
          act("$n snarls at you.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 6:
          act("$n shows you his Sharp Teeth.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 7:
          do_say(ch, "GOOLAA MMENN UUUUU", 0, 0);
          return (1);
       case 8:
         if (cmd || !AWAKE(ch) || (GET_POS(ch) == POS_FIGHTING))
          return (FALSE);
 
         for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room)
         {
             if (!IS_NPC(tch) && (GET_LEVEL(tch) < LVL_GOD))
             {
                  act("$n says 'UMMBA WALA WALA. YUM YUM. OSSHA AAAAARRRRGGGHH'", FALSE, ch, 0, 0, TO_ROOM);
                  mob_initiate(ch, tch);
                  return(TRUE);
             }
             else 
                 {
                  act("$n grovels in the dirt. He has seen a GOD ", FALSE, ch, 0, 0, TO_ROOM);
                  return(TRUE);
                 }
         }
          return (1);
          default:
          return (0);
      }
      return FALSE;
}
 
/* Canibal Haven mob */
SPECIAL(witchdoctor)
{
   ACMD(do_say);
 
   if(cmd)
     return (0);
 
  switch (number(0, 60)) {
   case 0:
      do_say(ch, "Um Nooga Geega Ju!", 0, 0);
      act("$n pokes his finger hard into your chest.", FALSE, ch, 0, 0,TO_ROOM);
      return (1);
   case 1:
      act("$n defiantly throws a handful of beads at your feet.", FALSE, ch, 0, 0,TO_ROOM);
      return (1);
   case 2:
      act("$n waves  a sharp bone menacingly at you.", FALSE, ch, 0, 0,TO_ROOM);
      return (1);
   case 3:
      do_say(ch, "Ooh! Ah! Waala Waala bam ganj!.", 0, 0);
      act("$n cackles insanely.", FALSE, ch, 0, 0,TO_ROOM);
      return (1);
   case 4:
      act("$n mutters unintelligbly to himself.", FALSE, ch, 0, 0,TO_ROOM);
   do_say(ch, "I see all!!.", 0, 0);
   act("$n dances a silly jig around the room.", FALSE, ch, 0, 0,TO_ROOM);
      return (1);
  case 5:
      act("$n screams, YAHHAHAHOOOOEY!.", FALSE, ch, 0, 0,TO_ROOM);
      return (1);
   case 6:
      act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "Mmmm! Yum, you look tasty!!", 0, 0);
      act("$n slices some carrots and throws them into the pot.", FALSE, ch, 0, 0,TO_ROOM);
      return (1);
   case 7:
      do_say(ch, "Well! what are you looking at?.", 0, 0);
      act("$n gives you a shove.", FALSE, ch, 0, 0,TO_ROOM);
      return (1);
   case 8:
      act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "I'm having a friend over for dinner, care to join us.", 0, 0);
      act("$n snickers under his breath.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
      default:
      return(0);
  }
}

/* Canibal Haven mob */
SPECIAL(prophet)
{
   ACMD(do_say);
 
  if(cmd)
   return (0);
 
  switch (number(0, 60)) {
   case 0:
      do_say(ch, "your death rapidly approaches", 0, 0);
      act("$n smiles.", FALSE, ch, 0, 0,TO_ROOM);
      return (1);
   case 1:
      act("$n turns its head and sends a piercing glare at you.", FALSE, ch, 0, 0,TO_ROOM);
      return (1);
   case 2:
      act("$n wavers menacingly in front of you.", FALSE, ch, 0,0,TO_ROOM);
      return (1);
   case 3:
      do_say(ch, "He who hesitates is lost.", 0, 0);
      act("$n cackles insanely.", FALSE, ch, 0, 0,TO_ROOM);
      return (1);
   case 4:
      act("$n draws a breath from a huge green water bong.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "Bow, you that have come to receive the words of the prophet.", 0, 0);
      act("$n puffs a cloud of nauseus purple smoke in your face.", FALSE, ch, 0, 0,TO_ROOM);
      return (1);
  case 5:
      act("$n sighs.", FALSE, ch, 0, 0,TO_ROOM);
      return (1);
   case 6:
      do_say(ch, "receive the enlightenment of vision", 0, 0);
      act("$n blows sweet smelling smoke in your face.", FALSE, ch, 0, 0,TO_ROOM);
      return (1);
   case 7:
      do_say(ch, "Who are you that dares defile the holy cave?.", 0, 0);
      return (1);
   case 8:
      act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "And you deem yourself worthy of prophecy?.", 0, 0);
      act("$n grumbles under her breath.", FALSE, ch, 0, 0,TO_ROOM);
      return (1);
      default:
      return (0);
  }
  return FALSE;
}

 
/* Canibal Haven Mob */
SPECIAL(guardian)
{
   char buf[256], buf2[256];
 
   if (cmd > 6 || cmd < 1)
      return FALSE;
 
   strcpy(buf,  "The Guardian humiliates you, and blocks your way.\n\r");
   strcpy(buf2, "The Guardian humiliates $n, and blocks $s way.");
 
   if ((ch->in_room == real_room(31857)) && (cmd == 1))
   {
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(buf, ch);
      return TRUE;
   }
 
   return FALSE;
 
}

 
/* Elvin Forest Tree Guardian Mob */
SPECIAL(trguardian)
{
  struct char_data *tch;
  int min_evil;
  char buf[256], buf2[256];
  min_evil = -350;

 
  if (cmd > 6 || cmd < 1)
    return FALSE;

 strcpy(buf,  "The Guardian blocks your way.  .. EVIL SCUM! ..  and Snarls.\n\r");
 strcpy(buf2, "The Guardian humiliates $n, and blocks $s way. The Guardian Snarls.");

  if ((ch->in_room == real_room(19001)) && (cmd == 1) && IS_NPC(ch))
  {
    for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) 
    {
      if ((GET_ALIGNMENT(tch) <= min_evil))
      {
        act(buf2, FALSE, ch, 0, 0, TO_ROOM);
        send_to_char(buf, ch);
        mob_initiate(ch, tch);
        return (TRUE);
      }
    }
  }
  return (FALSE);
}




SPECIAL(extratempdam)
{
  char buf[MAX_STRING_LENGTH];
  int opt,lev,cost;
  lev = GET_LEVEL(ch);
  cost = 60 * lev * lev;

if((GET_CLASS(ch)==CLASS_MAGIC_USER) || (GET_CLASS(ch)==CLASS_CLERIC))
  {
      if (CMD_IS("list")) 
        {
        send_to_char("The receptionist says. GO AWAY!", ch);
        return(FALSE);
        }
      else if (CMD_IS("buy")) 
        {
        send_to_char("The receptionist says. GO AWAY!", ch);
        return(FALSE);
        }
      return(FALSE);
  }

if ((GET_CLASS(ch)==CLASS_WARRIOR) || (GET_CLASS(ch)==CLASS_THIEF))
  {
    if (GET_LEVEL(ch) <= 117)
      {	
      send_to_char(" You are not at a high enough level yet", ch);
      return(FALSE);
      }
    if (GET_LEVEL(ch) > 117)
    {
      if (CMD_IS("list")) 
        { /* List */
          send_to_char("Buy 1  --- Extra temp to-damage points\n\r", ch);
          send_to_char("Buy 2  --- Extra temp to-hit points\n\r", ch);
          send_to_char(" ", ch);
          send_to_char(" ", ch);
          send_to_char(" ", ch);
          sprintf(buf,"The receptionist says. The cost for any of these services is %d coins.\n\r",cost);
          send_to_char(buf,ch);
          return(TRUE);
        } 
        else if (CMD_IS("buy")) 
          { /* Buy */
            argument = one_argument(argument, buf);
            opt = atoi(buf);
            if(cost > GET_GOLD(ch))
            {
              send_to_char("The receptionist says. You do not have enough money. GET LOST!!!\n\r",ch);
              return(TRUE);
            }
            switch(opt)
            {
              case 1:
	        GET_DAMROLL(ch) = (GET_DAMROLL(ch) + 1);
                GET_GOLD(ch) -= cost;
                send_to_char("You feel that you can damage more!\n\r",ch);
                return TRUE;
              case 2:
                GET_HITROLL(ch) = (GET_HITROLL(ch) + 1);
                GET_GOLD(ch) -= cost;
                send_to_char("You feel that it is easyer to hit!\n\r",ch);
                return TRUE;
              default:
              send_to_char("Huh?\n\r",ch);
              return(TRUE);
            }
          }
        return(FALSE);
    }
  }
  return(FALSE);
}


 
 

SPECIAL(hospital)
{
  char buf[MAX_STRING_LENGTH];
  int opt,lev,cost;

  lev = GET_LEVEL(ch);
  cost = 300 * lev * lev;
  if (CMD_IS("list")) { /* List */
    send_to_char("1 - Hit points restoration\n\r", ch);
    send_to_char("2 - Mana restoration\n\r",ch);
    sprintf(buf,"Your cost for any of these services is %d coins.\n\r",cost);
    send_to_char(buf,ch);
    return(TRUE);
 } else if (CMD_IS("buy")) { /* Buy */
    argument = one_argument(argument, buf);
    opt = atoi(buf);
    if(cost > GET_GOLD(ch)){
       send_to_char("Get a few more coins and come on back.\n\r",ch);
       return(TRUE);
    }
    switch(opt){
      case 1:
        GET_HIT(ch) = GET_MAX_HIT(ch);
        GET_GOLD(ch) -= cost;
        send_to_char("You feel magnificent!\n\r",ch);
        return TRUE;
      case 2:
        GET_MANA(ch) = GET_MAX_MANA(ch);
        GET_GOLD(ch) -= cost;
        send_to_char("You feel marvelous!\n\r",ch);
        return TRUE;
      default:
        send_to_char("Huh?\n\r",ch);
        return(TRUE);
    }
  }
  return(FALSE);
}



#define COST 1000000
#define ANTI_HUNGER_COST 100000000
#define ANTI_THIRST_COST 50000000
char *metasign[]={
    "HIT+++++",
    "MANA++++",
    "MOVE++++",
    "You will never again be hungry!",
    "You won't be thirsty again!",
};


SPECIAL(metaphysician)
{
  struct char_data *meta;
  char buf[MAX_STRING_LENGTH],rem[MAX_STRING_LENGTH];
  int i,k,opt,lev,reps;

  if (IS_NPC(ch))
    return(FALSE);
  for(meta=world[ch->in_room].people;meta;meta=meta->next_in_room){
    if(!IS_NPC(meta))
      continue;
    if(mob_index[meta->nr].func==metaphysician)
      break;
  }
  if(!meta)
    return(FALSE);;
  if (CMD_IS("list")) { /* List */
   send_to_char("These will cost 1,000,000 experience points.\n\r",ch);
    send_to_char(" 1 - Hit points inflator\n\r",ch);
    send_to_char(" 2 - Mana increase\n\r",ch);
    send_to_char(" 3 - More mobility\n\r",ch);
   send_to_char("This will cost 100,000,000 experience points.\n\r",ch);
    send_to_char(" 4 - Freedom from hunger\n\r",ch);
   send_to_char("This will cost 50,000,000 experience points.\n\r",ch);
    send_to_char(" 5 - Freedom from thirst\n\r",ch);
    return(TRUE);
  } else if (CMD_IS("buy")) { /* Buy */
    half_chop(argument, buf, rem);
    opt = atoi(buf);
    if((opt < 1) || (opt > 5)){
      act("$n tells you 'What?'",TRUE,meta,0,ch,TO_VICT);
      return(TRUE);
    }
    reps = atoi(rem);
    if(reps <= 0)
      reps = 1;
    for(i=0;i<reps;i++){
      lev=GET_LEVEL(ch);
      if(lev > 100) lev=100;
      if(COST > GET_EXP(ch)){
         if(i==0){
           act("$n tells you 'Come back when you're more experienced.'",
             TRUE,meta,0,ch,TO_VICT);
         } else {
           sprintf(buf,"$n tells you 'You managed only %d meta(s).'",i);
           act(buf,TRUE,meta,0,ch,TO_VICT);
           break;
         }
         return(TRUE);
      }
      if(GET_EXP(ch) > COST)
        GET_EXP(ch)-=COST;
      else
        GET_EXP(ch)=0;
      switch(opt){
       case  1: if((GET_CLASS(ch)==CLASS_WARRIOR) || (GET_CLASS(ch)==CLASS_THIEF)){
		k=ch->points.max_hit;
                ch->points.max_hit+=1+
                  (k<300)+(k<1000)+(k<3000)+(k<10000)+(k<30000)+(k<100000);
               break;
               }
               else { 
                      sprintf(buf, "$n tells you 'Only Warrior's or Thief's can buy this.'");
                      act(buf,TRUE,meta,0,ch,TO_VICT);
                      return 0;
                    }
       case 2: if((GET_CLASS(ch)==CLASS_MAGIC_USER) || (GET_CLASS(ch)==CLASS_CLERIC)){
       		k=ch->points.max_mana;
                ch->points.max_mana+=1+
                  (k<300)+(k<1000)+(k<3000)+(k<10000)+(k<30000)+(k<100000);
                break;
               }
               else { 
                      sprintf(buf, "$n tells you 'Only Mages or Cleric's can buy this.'");
                      act(buf,TRUE,meta,0,ch,TO_VICT);
                      return 0;
                    }
       case  3: k=ch->points.max_move;
                ch->points.max_move+=1+
                  (k<300)+(k<1000)+(k<3000)+(k<10000)+(k<30000)+(k<100000);
                break;
       case  4: if(ANTI_HUNGER_COST > GET_EXP(ch))
		{
                  act("$n tells you 'Come back when you're more experienced.'",
                  TRUE,meta,0,ch,TO_VICT);
                  break;
		}
                else 
		{
                  GET_EXP(ch)-=ANTI_HUNGER_COST;
                  GET_COND(ch,FULL) = -1; 
                  break;
		}
       case  5: if(ANTI_THIRST_COST > GET_EXP(ch))
		{
                  act("$n tells you 'Come back when you're more experienced.'",
                  TRUE,meta,0,ch,TO_VICT);
                  break;
		}
                else 
		{
                  GET_EXP(ch)-=ANTI_THIRST_COST;
                  GET_COND(ch,THIRST) = -1; 
                  break;
		}
      }
    }
    sprintf(buf,"$n tells you '%s - %d times.'",metasign[opt-1],i);
    act(buf,TRUE,meta,0,ch,TO_VICT);
    return(TRUE);
  }
  return(FALSE);
}



/*SPECIAL(thin_ice)
{
   struct descriptor_data *i;
   extern struct descriptor_data *descriptor_list;
   struct char_data *victim;
   ACMD(do_look); 
   switch (number(0,10)) {
   case 0:
      send_to_room("A blistering wind blows from the north.\n\r", 
ch->in_room);      
      return(1);
   case 1:
      send_to_room("The ice crackles under your weight.\n\r", ch->in_room);     
      return(1);
   case 2:
      send_to_room("You shiver as you watch the ice crack just up ahead.\n\r", ch->in_room);      
      return(1);
   case 3:
      send_to_room("The ice below your feet breaks open, dropping you into the icy water.\n\r", ch->in_room); 
	    act("$n falls through the ice, to the water below.", FALSE, 
ch, 0, 0, TO_ROOM);
	    char_from_room(ch);
	    char_to_room(ch, 9006);
	    act("You plunge into the icy waters from above.", FALSE, ch, 0, victim, TO_VICT);
	    do_look(victim, "", 15, 0);
      return(1);
   default:
      return(0);
   }
} */


void loadhelper(struct char_data *ch, int room)
{
	int num,vn,loadroom;
	struct char_data *tmp, *mob;

	tmp=FIGHTING(ch);
	if(!tmp)
	  return;
	switch(mob_index[ch->nr].virtual){
       		case 12030:	vn=12038; break;
                default: 	vn=defaulthelper; break;
	}
	num=real_mobile(vn);
	loadroom = (room >= 0) ? room : ch->in_room;
	if(num > 0){
		mob=read_mobile(num,REAL);
		if(mob){
			char_to_room(mob,loadroom);
			act("$n appears from nowhere to assist.", TRUE, mob, 0, 0, TO_ROOM);
			mob_initiate(mob, tmp);
		}
	}
}


SPECIAL(loader)
{
	if(cmd)
          return(FALSE);
        if(GET_LEVEL(ch) < 30){
          if(number(1,13)==3)
            loadhelper(ch,-1);
        } else if(FIGHTING(ch)){
             if(number(1,13) < 4)
               loadhelper(ch,-1);
             else
               call_magic(ch, ch, 0, SPELL_HEAL, GET_LEVEL(ch), CAST_SPELL);
             return(TRUE);
	}
	return(FALSE);
}
		

/* ***************************************************************
**       Freshly-Picked Spec_Procs for Artemis by Raven         **
*************************************************************** */


/* Tundra Devil */
SPECIAL(ice_devil)
{
   ACMD(do_say);
 
  if(cmd)
   return (0);
 
  switch (number(0, 60)) 
    {
       case 0:
          act("$n pauses to read from an evil tome.", FALSE, ch, 0, 0,TO_ROOM);
          do_say(ch, "Xskarn Geshnakh, O Asmodeus!!", 0, 0);
          return (1);
       case 1:
          do_say(ch, "Lord Asmodeus calls for a sacrifice!!", 0, 0);
          act("$n turns its hideous head to look at you.", FALSE, ch, 0, 0,TO_ROOM);
          do_say(ch, "You will serve the purpose nicely.", 0, 0);
          return (1);
       case 2:
          act("The pentagram begins humming with an unholy magic...", FALSE, ch, 0,0,TO_ROOM);
          return (1);
       case 3:
          act("$n begins writing unholy symbols in the air.", FALSE, ch, 0, 0,TO_ROOM);
          act("The symbols in the air burn brightly before fading.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 4:
          act("$n flourishes its great iron spear.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 5:
          act("The hideous Ice Devil swings its great clawed hand out at you!!", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 6:
          act("$n lashes its tail about.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 7:
          act("$n fixes you with its great insectile eyes.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 8:
          return (FALSE);
      }
      return FALSE;
}


/* Tundra Guardian */
SPECIAL(ice_guard)
{
   ACMD(do_say);
 
  if(cmd)
   return (0);
 
  switch (number(0, 60)) 
    {
       case 0:
          act("$n lurches towards you with a shattering roar!!!", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 1:
          act("$n lurches towards you with a shattering roar!!!", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 2:
          act("$n swings its huge ice paw at you!!!", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 3:
          act("$n swings its huge ice paw at you!!!", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 4:
          act("$n sniffs around, trying to locate your hot blood...", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 5:
          act("$n sniffs around, trying to locate your hot blood...", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 6:
          return (FALSE);
      }
      return FALSE;
}


/* Tundra Giant */
SPECIAL(ice_giant)
{
   ACMD(do_say);
 
  if(cmd)
   return (0);
 
  switch (number(0, 60)) 
    {
       case 0:
          act("$n lets out a thunderous warcry!!!", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 1:
          act("$n raises a great fist of black ice...", FALSE, ch, 0, 0,TO_ROOM);
          act("$n slams its fist back into the icy ground, right next to you!!", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 2:
          act("$n roars, 'ME KILL HOT PERSON!!", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 3:
          act("$n tries its level best to stand on you!!!", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 4:
          act("$n looks at you icily.", FALSE, ch, 0, 0,TO_ROOM);
          act("$n booms, 'Vat you vant, hot person?", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 5:
          return (FALSE);
      }
      return FALSE;
}


/* Tundra Wyrm */
SPECIAL(ice_wyrm){

  struct char_data *tch, *vict;
  int low_on_hits = 10000;

  if (cmd)
    return (FALSE);

  /* Find out who has the lowest hitpoints and burn his ass off */
  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room){
    if ((!IS_NPC(tch)) && (GET_LEVEL(tch) < LVL_GOD))
      if (tch->points.hit < low_on_hits){
	low_on_hits = tch->points.hit;
	vict = tch;
      }
   if((!IS_NPC(tch)) && (GET_LEVEL(tch) < LVL_GOD)){
  act("$n roars, 'Adventurer for lunch!!!'", FALSE, ch, 0, 0, TO_ROOM);
  act("$n turns it huge head towards $N...", 1, ch, 0, vict, TO_NOTVICT);
  act("$n turns its monstrous scaly head towards YOU!", 1, ch, 0, vict, TO_VICT);
  cast_spell(ch, vict, 0, SPELL_FIREBALL);
  return TRUE;
   }
  else {
   act("$n thoughtfully warms a God up.", 1, ch, 0, vict, TO_ROOM);
  }
  }  
  
  return FALSE;
}


/* Tundra Mammoth */
SPECIAL(mammoth)
{
   ACMD(do_say);
 
  if(cmd)
   return (0);
 
  switch (number(0, 60)) 
    {
       case 0:
          act("$n lets out a mad trumpeting!!!", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 1:
          act("$n rolls its tiny, mad red eyes.", FALSE, ch, 0, 0,TO_ROOM);
          act("$n waves its trunk at you menacingly!!", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 2:
          act("$n stamps along through the snow.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 3:
          act("$n almost stands on you!!!", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 4:
          return (FALSE);
      }
      return FALSE;
}


/* Tundra Yeti */
SPECIAL(yeti)
{
   ACMD(do_say);
 
  if(cmd)
   return (0);
 
  switch (number(0, 60)) 
    {
       case 0:
          act("$n bares its yellow teeth and screams at you!!", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 1:
          act("$n snarls at you, like a maddened lion.", FALSE, ch, 0, 0,TO_ROOM);
          act("$n rushes through the trees and snow towards you!!!!!", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 2:
          act("$n swings at you with its black claws, missing by an inch!!", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 3:
          return (FALSE);
      }
      return FALSE;
}


/* Tundra Judge Penguin */
SPECIAL(judge_penguin)
{
   ACMD(do_say);
 
  if(cmd)
   return (0);
 
  switch (number(0, 60)) 
    {
       case 0:
          act("$n marks something down on his notepad.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 1:
          act("$n picks up a card near his feet.", FALSE, ch, 0, 0,TO_ROOM);
          act("$n holds up a '10'.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 2:
          act("$n picks up a card near his feet.", FALSE, ch, 0, 0,TO_ROOM);
          act("$n holds up a '9'.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 3:
          act("$n picks up a card near his feet.", FALSE, ch, 0, 0,TO_ROOM);
          act("$n holds up a '8'.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 4:
          act("$n picks up a card near his feet.", FALSE, ch, 0, 0,TO_ROOM);
          act("$n holds up a '7'.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 5:
          act("$n picks up a card near his feet.", FALSE, ch, 0, 0,TO_ROOM);
          act("$n holds up a '6'.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 6:
          act("$n writes something down on his notepad.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 7:
          return (FALSE);
      }
      return FALSE;
}


/* Tundra Outcast Figment */
SPECIAL(outcast_fig)
{
   ACMD(do_say);
 
  if(cmd)
   return (0);
 
  switch (number(0, 60)) 
    {
       case 0:
          act("$n skis along gracefully.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 1:
          act("$n munches on some freeze-dried fruit.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 2:
          act("$n skis around a tree.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 3:
          act("$n cries, 'Woe is me, I am outcast!!!", FALSE, ch, 0, 0,TO_ROOM);
          act("$n says, 'But I'm not the one who's freezing.'", FALSE, ch, 0, 0,TO_ROOM);
          act("$n grins evilly.", FALSE, ch, 0, 0,TO_ROOM);
          return (1);
       case 4:
          return (FALSE);
      }
      return FALSE;
}

/* Tomas's dodgy spec procs */

/* Damn it, Tomas I'm going to teach you how to indent if its the last thing
   I ever do !!!!!  You gotta make your code readable, awwright ? :) */

SPECIAL(fatlady)
{
  ACMD(do_say);

  if(cmd || FIGHTING(ch))
    return 0;

  switch (number(0, 30)) {
  case 0:

    return(1);
  case 1:
    act("$n mutters about shared accounts.", FALSE, ch, 0, 0,TO_ROOM);
    return(1);
  case 2:
    act("$n growls.", FALSE, ch, 0, 0,TO_ROOM);
    act("$n points a finger at you.", FALSE, ch, 0, 0,TO_ROOM);
    act("$n DEMANDS your id card!", FALSE, ch, 0, 0,TO_ROOM);
    return(1);
  case 3:
    act("$n sighs.", FALSE, ch, 0, 0,TO_ROOM);
    act("$n says, 'people to catch, mudders to kill'.", FALSE, ch, 0, 0,TO_ROOM);
    return(1);
  case 4:
    act("$n stares at you intently for a moment.", FALSE, ch, 0, 0,TO_ROOM);
    act("$n says, '11920037, you are charged with illegally mudding!'.", FALSE, ch, 0, 0,TO_ROOM);
    return(1);
  case 5:
    act("$n gets out a pen and paper.", FALSE, ch, 0, 0,TO_ROOM);
    return(1);
  case 6:
    act("$n says, 'HEY you! Mudding is not allowed! get into g43!", FALSE, ch, 0, 0,TO_ROOM);
    return(1);
  case 7:
    act("$n cackles evily.", FALSE, ch, 0, 0,TO_ROOM);
    act("$n writes down your student number.", FALSE, ch, 0, 0,TO_ROOM);
    act("$n says, 'thats a weeks suspension on your account!", FALSE, ch, 0, 0,TO_ROOM);
    return(1);
  default:
    return(FALSE);
    break;
  }
   return(0);
}



SPECIAL(oldman)
{
  ACMD(do_say);

  struct char_data *i;

  if(cmd || FIGHTING(ch))
    return 0;


  switch (number(0, 50)) 
    {
  	case 0:
    	  act("$n peers about gloomily.", FALSE, ch, 0, 0,TO_ROOM);
    	  return(1);
  	case 1:
    	  act("$n mutters something about the 'good old days'.", FALSE, ch, 0, 0,TO_ROOM);
    	  return(1);
  	case 2:
    	  act("$n asks you, 'Do you know the way to Ravenloft??'", FALSE, ch, 0, 0,TO_ROOM);
    	  act("$n points vaguely east then north.", FALSE, ch, 0, 0,TO_ROOM);
    	  return(1);
  	case 3:
    	  do_say(ch, "I was an evil player once.. but the God's caught me and made me as you see me today.", 0, 0);
    	  return(1);
  	case 4:
    	  do_say(ch, "Hey... what ever happened to those slaves in Arachnos?", 0, 0);
    	  act("$n stares at you as if there is something you should be doing.", FALSE, ch, 0, 0,TO_ROOM);
    	  return(1);
  	case 5:
    	  act("$n looks up with a philosophical air.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "Women - God's eternal punishment on man.", 0, 0);
    	  return(1);
  	case 6:
    	  act("$n looks at somone you cannot see.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "Yeah, i've heard about him too... he thought that he wouldn't get caught!", 0, 0);
    	  return(1);
  	case 7:
    	  do_say(ch, "I remember when pewter and sapphire rings were all the rage for newbies!", 0, 0);
    	  return(1);
  	case 8:
    	  do_say(ch, "Hey.. you ever killed the beholder? I hear he has this cool green cloak!", 0, 0);
    	  return(1);
  	case 9:
    	          act("$n is urged by the Gods to heal a worthy player.", FALSE, ch, 0, 0,TO_ROOM);
    for (i = world[ch->in_room].people; i; i = i->next_in_room)
      if ((i != ch) && !IS_NPC(i) && (!number(0, 3))) 
	{       /* healing */
          	if (GET_HIT(i) >= GET_MAX_HIT(i))
            	  return (1);
                  /* formulie to work extra hp's */
                  GET_HIT(i) = (GET_HIT(i) + (GET_MAX_HIT(i) / 13));
    	          act("$n chants and moves his hands in a strange way.", FALSE, ch, 0, 0,TO_ROOM);
                  send_to_char("You feel better.\r\n", i);
      	}
            	  return (1);
  	case 10:
    	  act("$n looks at the ground and digs at it with a worn old shoe.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "I managed to overhear a God say something about a Death Spiral and a Mazekeeper.", 0, 0);
    	  return(1);
  	case 11:
    	  act("$n looks at you with a stern look.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "Geeez ...  and the God's NOW say. there is 3 chessboards now.", 0, 0);
    	  return(1);
  	case 12:
    	  act("$n laughs at you while pointing towards the Elven Village.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "I see you have been trying to kill elfs. Perhaps you better stick to fidos.", 0, 0);
    	  return(1);
  	case 13:
    	  act("$n gazes towards the East.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "If you are tough and above level 20, go and climb some rock.", 0, 0);
    	return(1);
  	case 14:
    	  act("$n looks sternly around.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "To go exploring while on the sea, will mean death to those under level 20.", 0, 0);
    	  return(1);
  	case 15:
    	  act("$n sneezes and wipes his face with the cuff off his sleeve.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "There is a place.. a deadly place.. to the North there are Canibals.", 0, 0);
    	  return(1);
  	case 16:
    	  act("$n points a bony finger in the air.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "I hear Zedd Town is a cool place for new players to venture.  Just go East and East over the sea from here.", 0, 0);
    	  return(1);
  	case 17:
    	  act("$n looks you all over.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "The Sea can be a dangerous place if you venture around there.", 0, 0);
    	  return(1);
  	case 18:
    	  act("$n looks to the East.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "You will find Rome if you go that way.", 0, 0);
    	  return(1);
  	case 19:
    	  act("$n looks East and Up to the Mountains.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "You will find ice and snow up in the Mountains.", 0, 0);
    	  return(1);
  	case 20:
    	  do_say(ch, "You will find a chessboard if you go North and then West from here.", 0, 0);
    	  return(1);
  	case 21:
    	  act("$n looks off to the South.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "If you go all the way South from here you will find a Dump, and if you go down.. a Sewer Maze.", 0, 0);
    	  return(1);
  	case 22:
    	  do_say(ch, "Only a fast and quick level 20 can kill a Green Blob.", 0, 0);
    	  return(1);
  	case 23:
    	          act("$n is urged by the Gods to heal a worthy player.", FALSE, ch, 0, 0,TO_ROOM);
    for (i = world[ch->in_room].people; i; i = i->next_in_room)
      if ((i != ch) && !IS_NPC(i) && (!number(0, 3))) 
	{       /* healing */
          	if (GET_HIT(i) >= GET_MAX_HIT(i))
            	  return (1);
                  /* formulie to work extra hp's */
                  GET_HIT(i) = (GET_HIT(i) + (GET_MAX_HIT(i) / 13));
    	          act("$n chants and moves his hands in a strange way.", FALSE, ch, 0, 0,TO_ROOM);
                  send_to_char("You feel better.\r\n", i);
      	}
            	  return (1);
  	case 24:
    	          act("$n is urged by the Gods to spell a worthy player.", FALSE, ch, 0, 0,TO_ROOM);
    for (i = world[ch->in_room].people; i; i = i->next_in_room)
      if ((i != ch) && !IS_NPC(i) && (!number(0, 3))) 
	{       /* mana */
          	if (GET_MANA(i) >= GET_MAX_MANA(i))
            	  return (1);
                  /* formulie to work extra mana */
          	else if (GET_LEVEL(i) > 25)
                  GET_MANA(i) = (GET_MANA(i) + (GET_MAX_MANA(i) / 13));
    	          act("$n chants and moves his hands and feet in a strange way.", FALSE, ch, 0, 0,TO_ROOM);
                  send_to_char("Your magic seems better.\r\n", i);
      	}
            	  return (1);
  	case 25:
    	  act("$n Looks at you admiring your single good item.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "Baaaahhhh you need some better Equipment.  Zedd town is ful of good stuff for new players.", 0, 0);
    	  return(1);
  	case 26:
    	  do_say(ch, "Go south, South and then West to kill foxes and sparrows.", 0, 0);
    	  return(1);
  	case 27:
    	  do_say(ch, "The Mayor of Midguard has the key to the gates.", 0, 0);
    	  return(1);
  	case 28:
    	  do_say(ch, "You can find cool stuff in the donation room, just East in the Temple of Midgaard.", 0, 0);
    	  return(1);
  	case 29:
    	  do_say(ch, "You will find a combat arena, just West in the Temple of Midgaard. Be shure to practice your moves there... NOTHING ELSE!", 0, 0);
    	  return(1);
  	case 30:
    	  do_say(ch, "Killing a Fido will give you meat and a more good allignment.", 0, 0);
    	  return(1);
  	case 31:
    	  act("$n looks down at the ground.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "The sewers are a good place for XP and equipment.", 0, 0);
    	  return(1);
  	case 32:
    	  act("$n performs a pathetic display of his robe.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "It took me years of work to get a robe of this quality.", 0, 0);
    	  return(1);
  	case 33:
    	  do_say(ch, "There are dwarves somewhere from the eastern gate of Muidguaard.", 0, 0);
    	  return(1);
  	case 34:
    	  do_say(ch, "Don't do anything bad arond town. The town Guards are swift and deadly to those on the other side of the law.", 0, 0);
    	  return(1);
  	case 35:
    	          act("$n is urged by the Gods to heal a worthy player.", FALSE, ch, 0, 0,TO_ROOM);
    for (i = world[ch->in_room].people; i; i = i->next_in_room)
      if ((i != ch) && !IS_NPC(i) && (!number(0, 3))) 
	{       /* healing */
          	if (GET_HIT(i) >= GET_MAX_HIT(i))
            	  return (1);
                  /* formulie to work extra hp's */
                  GET_HIT(i) = (GET_HIT(i) + (GET_MAX_HIT(i) / 13));
    	          act("$n chants and moves his hands in a strange way.", FALSE, ch, 0, 0,TO_ROOM);
                  send_to_char("You feel better.\r\n", i);
      	}
    	  return(1);
        case 36:
    	          act("$n is urged by the Gods to spell a worthy player.", FALSE, ch, 0, 0,TO_ROOM);
    for (i = world[ch->in_room].people; i; i = i->next_in_room)
      if ((i != ch) && !IS_NPC(i) && (!number(0, 3))) 
	{       /* mana */
          	if (GET_MANA(i) >= GET_MAX_MANA(i))
            	  return (1);
                  /* formulie to work extra mana */
          	else if (GET_LEVEL(i) > 25)
                  GET_MANA(i) = (GET_MANA(i) + (GET_MAX_MANA(i) / 13));
                  act("$n chants and moves his hands and feet in a strange way.", FALSE, ch, 0, 0,TO_ROOM);
                  send_to_char("Your magic seems better.\r\n", i);
      	}
    	  return(1);
        case 37:
    	  do_say(ch, "Watch out for the fat lady!", 0, 0);
    	  return(1);
	case 38:
    	  do_say(ch, "Not many know how to get an Intangable Backpack.", 0, 0);
    	  return(1);
	case 39:
    	  do_say(ch, "Remember that you can explore the Great Piramid.", 0, 0);
    	  return(1);
	case 40:
    	  act("$n looks at a translucent form of one of the Gods.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "A figment?? But I thought the you Gods killed them all.", 0, 0);
    	  return(1);
	case 41:
    	  act("$n makes a serious face and says.", FALSE, ch, 0, 0,TO_ROOM);
    	  do_say(ch, "The Gods put me here to give Advice, Heal, give Mana and give Movement.", 0, 0);
    	  return(1);
        case 42:
    	          act("$n is urged by the Gods to heal a worthy player.", FALSE, ch, 0, 0,TO_ROOM);
    for (i = world[ch->in_room].people; i; i = i->next_in_room)
      if ((i != ch) && !IS_NPC(i) && (!number(0, 3)))
        {       /* healing */
                if (GET_HIT(i) >= GET_MAX_HIT(i))
                  return (1);
                  /* formulie to work extra hp's */
                  GET_HIT(i) = (GET_HIT(i) + (GET_MAX_HIT(i) / 13));
                  act("$n chants and moves his hands in a strange way.", FALSE, ch, 0, 0,TO_ROOM);
                  send_to_char("You feel better.\r\n", i);
        }
                  return (1);
        case 43:
    	          act("$n is urged by the Gods to spell a worthy player.", FALSE, ch, 0, 0,TO_ROOM);
    for (i = world[ch->in_room].people; i; i = i->next_in_room)
      if ((i != ch) && !IS_NPC(i) && (!number(0, 3)))
        {       /* mana */
                if (GET_MANA(i) >= GET_MAX_MANA(i))
                  return (1);
                  /* formulie to work extra mana */
                else if (GET_LEVEL(i) > 25)
                  GET_MANA(i) = (GET_MANA(i) + (GET_MAX_MANA(i) / 13));
                  act("$n chants and moves his hands and feet in a strange way.", FALSE, ch, 0, 0,TO_ROOM);
                  send_to_char("Your magic seems better.\r\n", i);
        }
                  return (1);
        case 44:
    	          act("$n is urged by the Gods to heal a worthy player.", FALSE, ch, 0, 0,TO_ROOM);
    for (i = world[ch->in_room].people; i; i = i->next_in_room)
      if ((i != ch) && !IS_NPC(i) && (!number(0, 3)))
        {       /* healing */
                if (GET_HIT(i) >= GET_MAX_HIT(i))
                  return (1);
                  /* formulie to work extra hp's */
                  GET_HIT(i) = (GET_HIT(i) + (GET_MAX_HIT(i) / 13));
                  act("$n chants and moves his hands in a strange way.", FALSE, ch, 0, 0,TO_ROOM);
                  send_to_char("You feel better.\r\n", i);
        }
                  return (1);
        case 45:
    	          act("$n is urged by the Gods to spell a worthy player.", FALSE, ch, 0, 0,TO_ROOM);
    for (i = world[ch->in_room].people; i; i = i->next_in_room)
      if ((i != ch) && !IS_NPC(i) && (!number(0, 3)))
        {       /* mana */
                if (GET_MANA(i) >= GET_MAX_MANA(i))
                  return (1);
                  /* formulie to work extra mana */
                else if (GET_LEVEL(i) > 25)
                  GET_MANA(i) = (GET_MANA(i) + (GET_MAX_MANA(i) / 13));
                  act("$n chants and moves his hands and feet in a strange way.", FALSE, ch, 0, 0,TO_ROOM);
                  send_to_char("Your magic seems better.\r\n", i);
        }
                  return (1);
        case 46:
    	          act("$n is urged by the Gods to heal a worthy player.", FALSE, ch, 0, 0,TO_ROOM);
    for (i = world[ch->in_room].people; i; i = i->next_in_room)
      if ((i != ch) && !IS_NPC(i) && (!number(0, 3)))
        {       /* healing */
                if (GET_HIT(i) >= GET_MAX_HIT(i))
                  return (1);
                  /* formulie to work extra hp's */
                  GET_HIT(i) = (GET_HIT(i) + (GET_MAX_HIT(i) / 13));
                  act("$n chants and moves his hands in a strange way.", FALSE, ch, 0, 0,TO_ROOM);
                  send_to_char("You feel better.\r\n", i);
        }
                  return (1);
        case 47:
    	          act("$n is urged by the Gods to spell a worthy player.", FALSE, ch, 0, 0,TO_ROOM);
    for (i = world[ch->in_room].people; i; i = i->next_in_room)
      if ((i != ch) && !IS_NPC(i) && (!number(0, 3)))
        {       /* mana */
                if (GET_MANA(i) >= GET_MAX_MANA(i))
                  return (1);
                  /* formulie to work extra mana */
                else if (GET_LEVEL(i) > 25)
                  GET_MANA(i) = (GET_MANA(i) + (GET_MAX_MANA(i) / 13));
                  act("$n chants and moves his hands and feet in a strange way.", FALSE, ch, 0, 0,TO_ROOM);
                  send_to_char("Your magic seems better.\r\n", i);
        }
                  return (1);
        case 48:
    	          act("$n is urged by the Gods to heal a worthy player.", FALSE, ch, 0, 0,TO_ROOM);
    for (i = world[ch->in_room].people; i; i = i->next_in_room)
      if ((i != ch) && !IS_NPC(i) && (!number(0, 3)))
        {       /* healing */
                if (GET_HIT(i) >= GET_MAX_HIT(i))
                  return (1);
                  /* formulie to work extra hp's */
                  GET_HIT(i) = (GET_HIT(i) + (GET_MAX_HIT(i) / 13));
                  act("$n chants and moves his hands in a strange way.", FALSE, ch, 0, 0,TO_ROOM);
                  send_to_char("You feel better.\r\n", i);
        }
                  return (1);
    	break;
  	default:
    	          act("$n is urged by the Gods to vigorise a worthy player.", FALSE, ch, 0, 0,TO_ROOM);
    		for (i = world[ch->in_room].people; i; i = i->next_in_room)
      		if ((i != ch) && !IS_NPC(i) && (!number(0, 3))) 
                    {       /* movement */
                if (GET_MOVE(i) >= GET_MAX_MOVE(i))
                  return (1);
                 /* formulie to work extra movement */
                 GET_MOVE(i) = (GET_MOVE(i) + (GET_MAX_MOVE(i) / 13));
              act("$n chants and moves his feet in a strange way.", FALSE, ch, 0, 0,TO_ROOM);
                  send_to_char("Your legs feel better.\r\n", i);
          	return (1);
		    }	

    	return(0);
      	}
  }




SPECIAL(flying_snake)
{
  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room) &&
      (number(0, 42 - GET_LEVEL(ch)) == 0)) {
        switch(number(0, 1)) {
        case 0: /* poison the player */
          act("$n bites $N! on the face.", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
          act("$n bites you on your nose.", 1, ch, 0, FIGHTING(ch), TO_VICT);
          call_magic(ch, FIGHTING(ch), 0, SPELL_POISON, GET_LEVEL(ch), CAST_SPELL);
          return TRUE;
        case 1: /* blind the player */
          act("$n spits $N! in the face.", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
          act("$n spits at your face.", 1, ch, 0, FIGHTING(ch), TO_VICT);
          call_magic(ch, FIGHTING(ch), 0, SPELL_BLINDNESS, GET_LEVEL(ch), CAST_SPELL);
          return TRUE;
        default: 
          return(FALSE);
          break;
        }
  }
  return FALSE;
}
