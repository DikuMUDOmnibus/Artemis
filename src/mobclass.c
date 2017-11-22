/* ************************************************************************
*  File: mobclass.c                                     Part of CircleMUD *
*  Usage: Mob class system                                                *
*                                                                         *
*  Written by Chris Voutsis  (hopefully)				  *
*	Dodgy Modifications by Andrew Bell				  *
*									  *
*  Usage:								  *
*		This file allows generic spec procs to be created in the  *
*	form of mob classes. The mobclass is then set in the mob file,    *
*	allowing a nifty set of idling, combat and initiation actions to  *
*	occur. Note that the abilities and spells used are all defined    *
*	by the level of the mob. For this reason, care should be taken    *
*	with the levels used in the mob file to ensure mobs are not too   *
*	overtly strong or weak. Note that the only procedures this module *
*	exports are the mob_combat, mob_init and mob_idle functions for   *
*	fighting, idling, and starting a fight.  Dont call the other	  *
*	procedures unless you know exactly what you are doing.		  *
*			(Tomas: hmmm a little poke here... uhhh ohhhh!)   *
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
/* External structures */
extern struct room_data *world;

/* To save editing structs.h... */

#define CLASS_FIGHTER CLASS_WARRIOR
#define CLASS_SPELLCASTER CLASS_MAGIC_USER
#define CLASS_HEALER CLASS_CLERIC
#define CLASS_ASSASSIN CLASS_THIEF

void combat_fighter(struct char_data *ch, struct char_data *vict) {

/* NOTE:  This is entirely useless if the mob skills are 0%	*/

  ACMD(do_rescue);

  switch (number(0, 25)) {
    case 0:
    case 1:
    case 3:
    case 4:
    case 5:
      if (GET_LEVEL(ch) >= 15)
        if (FIGHTING(vict))
          if ((FIGHTING(vict) != ch))
            if (IS_EVIL(ch)) { 
	      act("$n howls a defiant cry and throws $mself at you!",
	      FALSE, ch, 0, 0, TO_ROOM);	   
	      do_rescue(ch, FIGHTING(vict)->player.name, 0, 0);
	    } else  {
	      act("$n heroically leaps to the fore!",
	      FALSE, ch, 0, 0, TO_ROOM);
	      do_rescue(ch, FIGHTING(vict)->player.name, 0, 0);
	    }
      break;
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
      if (GET_LEVEL(ch) >= 10)
        command_interpreter(ch, "bash");
      break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
      if (GET_LEVEL(ch) >= 5)
        command_interpreter(ch, "kick");
      break;
    default:
      break;
  }
}

int offensive_spellcast_for_level(struct char_data *ch) {

/* a copy of the magic_user spec proc... except more vicious :) *
*	ADDENDUM:						*
* now recoded... *grin* even more vicious. First checks will    *
* see if it casts a non harming attack spell, sleep blind,      *
* and curse. Then it goes through the spell list by level of    *
* the mob. Once it reaches a spell which the mob is high nuff   *
* level to cast it rolls and has a 75% chance of doing it. If   *
* it doesn't succeed it drops to then next spell.. ad infinitum */

  if ((GET_LEVEL(ch) > 30) && (number(0, 10) == 0))
    return SPELL_SLEEP;

  if ((GET_LEVEL(ch) > 25) && (number(0, 8) == 0))
    return SPELL_BLINDNESS;

  if ((GET_LEVEL(ch) > 20) && (number(0, 10) == 0)) 
    return SPELL_CURSE;

  if ((GET_LEVEL(ch) > 250) && (number(0, 3) > 0)) 
    return SPELL_STARBURST;

  if ((GET_LEVEL(ch) > 100) && (number(0, 3) > 0)) 
    return SPELL_FIREBALL;

  if ((GET_LEVEL(ch) > 40) && (number(0, 3) > 0)) 
    return SPELL_COLOR_SPRAY;

  if ((GET_LEVEL(ch) > 30) && (number(0, 3) > 0)) 
    return SPELL_LIGHTNING_BOLT;

  if ((GET_LEVEL(ch) > 20) && (number(0, 3) > 0)) 
    return SPELL_SHOCKING_GRASP;

  if ((GET_LEVEL(ch) > 10) && (number(0, 3) > 0)) 
    return SPELL_BURNING_HANDS;

  if ((GET_LEVEL(ch) > 5) && (number(0, 3) > 0)) 
    return SPELL_CHILL_TOUCH;

  return SPELL_MAGIC_MISSILE;
}

void combat_spellcaster(struct char_data *ch, struct char_data *vict) {

  switch (number(0, 10)) {
    case 0:
    case 1:
    case 3:
    case 4:
    case 5:
      cast_spell(ch, vict, NULL, offensive_spellcast_for_level(ch));
      break;
    default:
      break;
  }
}

int offensive_healcast_for_level(struct char_data *ch) {

/* clerical combat spells. Similair call up as for spellcast */

  if ((GET_LEVEL(ch) > 15) && (number(0, 10) == 0)) 
    return SPELL_BLINDNESS;

  if ((GET_LEVEL(ch) > 200) && (number(0, 7) > 6)) 
    return SPELL_EARTHQUAKE;

  if ((GET_LEVEL(ch) > 150) && (number(0, 3) > 0)) 
    return SPELL_POWER_WORD_STUN;

  if ((GET_LEVEL(ch) > 50) && (number(0, 3) > 0)) 
    return SPELL_HARM;

  if ((GET_LEVEL(ch) > 30) && (number(0, 3) > 0)) 
    return SPELL_CALL_LIGHTNING;

  if ((GET_LEVEL(ch) > 10) && (number(0, 3) > 0)) 
    if (IS_EVIL(ch))
      return SPELL_DISPEL_GOOD;
    else if (IS_GOOD(ch))
      return SPELL_DISPEL_EVIL;

  return SPELL_POISON;
}

int defensive_healcast_for_level(struct char_data *ch) {

  if ((GET_LEVEL(ch) > 150) && (number(0, 3) > 0)) 
    return SPELL_SUPER_HEAL;

  if ((GET_LEVEL(ch) > 50) && (number(0, 3) > 0)) 
    return SPELL_HEAL;

  if ((GET_LEVEL(ch) > 20) && (number(0, 3) > 0)) 
    return SPELL_CURE_CRITIC;

  return SPELL_CURE_LIGHT;
}

void combat_healer(struct char_data *ch, struct char_data *vict) {

  switch (number(0, 18)) {
    case 0:
    case 1:
    case 2:
    case 3:
      if (IS_EVIL(ch) && number(0, 3) == 0)
        act("$n laughs diabolically and hurls a spell!", FALSE, ch, 0, 0, TO_ROOM);
      cast_spell(ch, vict, NULL, offensive_healcast_for_level(ch));
      break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
      if (FIGHTING(vict))
        if ((GET_HIT(FIGHTING(vict)) < (int) (GET_MAX_HIT(FIGHTING(vict)) / 2))
		&& (number(0, 1) == 0))
          cast_spell(ch, FIGHTING(vict), NULL, defensive_healcast_for_level(ch));
      break;
    default:
      break;
  }
}

void combat_assassin(struct char_data *ch, struct char_data *vict) {

  int perc;

  if (number(0, 5) == 0)
    if (GET_EQ(ch, WEAR_WIELD))
      if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) == (TYPE_PIERCE - TYPE_HIT))
        if (!MOB_FLAGGED(vict, MOB_AWARE)) {
          perc = number(1, 101);
          if (AWAKE(vict) && (perc > GET_SKILL(ch, SKILL_BACKSTAB)))
            damage(ch, vict, 0, TYPE_STAB);
          else
            hit(ch, vict, SKILL_BACKSTAB);
        }
}

void mob_combat(struct char_data *ch, struct char_data *victim) {

  if (!IS_NPC(ch) || (!victim) || ch->desc)
    return;

  if (ch->player.class == CLASS_FIGHTER)
    combat_fighter(ch, victim);
  if (ch->player.class == CLASS_SPELLCASTER)
    combat_spellcaster(ch, victim);
  if (ch->player.class == CLASS_HEALER)
    combat_healer(ch, victim);
  if (ch->player.class == CLASS_ASSASSIN)
    combat_assassin(ch, victim);
}

void idle_fighter(struct char_data *ch) {

  switch (number(0, 10)) {
  case 2:
    if (GET_EQ(ch, WEAR_WIELD))
      if (GET_LEVEL(ch) < 5) {
        if (IS_EVIL(ch)) {
	  act("$n clumsily swings $p and nearly hits himself!",
		FALSE, ch, GET_EQ(ch, WEAR_WIELD), 0, TO_ROOM);
	  act("$n growls angrily.", FALSE, ch, 0, 0, TO_ROOM);
	} else {
	  act("$n swings $p, a move that ends with $m fumbling $p!",
		FALSE, ch, GET_EQ(ch, WEAR_WIELD), 0, TO_ROOM);
          act("$n blushes.", FALSE, ch, 0, 0, TO_ROOM);
        }
      } else if (GET_LEVEL(ch) < 15) {
	 if (IS_EVIL(ch)) {
	  act("$n slowly wipes dried blood from $p, an evil look on $s face.",
		FALSE, ch, GET_EQ(ch, WEAR_WIELD), 0, TO_ROOM);
	} else {
	     act("$n examines $p for nicks and scratches.",
		FALSE, ch, GET_EQ(ch, WEAR_WIELD), 0, TO_ROOM);
	}
      } else {
	 if (IS_EVIL(ch)) {
	   act("$n performs an elaborate manouver, swinging $p about $s head.",
	   FALSE, ch, GET_EQ(ch, WEAR_WIELD), 0, TO_ROOM);
	 } else {	
	     act("$n practices a move ending perfectly with $p extended.",
	     FALSE, ch, GET_EQ(ch, WEAR_WIELD), 0, TO_ROOM);
	}
      }
    break;
  case 3:
  case 4:
  case 5:
  case 6:
    if (GET_HIT(ch) < (int) (GET_MAX_HIT(ch) / 2)) {
      if (GET_LEVEL(ch) < 5) {
	if (IS_EVIL(ch)) {
	  act("$n idly wipes blood off $s face.", FALSE, ch, 0, 0, TO_ROOM);
        } else {
	  act("$n picks at a few scabs, wincing slightly.",
	  FALSE, ch, 0, 0, TO_ROOM);
      	}
      } else if (GET_LEVEL(ch) < 15) {
	if (IS_EVIL(ch)) {
          act("$n clumsily bandages $s wounds with a dirty rag.", 
	  FALSE, ch, 0, 0, TO_ROOM);
        } else {
	  act("$n roughly bandages $s wounds.", FALSE, ch, 0, 0, TO_ROOM);
        }
        GET_HIT(ch) = MIN(GET_HIT(ch) + 5, GET_MAX_HIT(ch));
      } else if (GET_LEVEL(ch) < 25) {
	if (IS_EVIL(ch)) {
	  act("$n winces as $e sews up $s oozing wounds back together.",
	  FALSE, ch, 0, 0, TO_ROOM);
        } else {
          act("$n expertly bandages $s wounds.", FALSE, ch, 0, 0, TO_ROOM);
        }
        GET_HIT(ch) = MIN(GET_HIT(ch) + 10, GET_MAX_HIT(ch));
      } else if (GET_LEVEL(ch) >= 25) {
        if (IS_EVIL(ch)) {
    	  act("$n howls as $e painfully tends to a oozing wound.", FALSE, ch, 0, 0, TO_ROOM);
        } else {
          act("$n expertly bandages $s wounds.", FALSE, ch, 0, 0, TO_ROOM);
        }
        GET_HIT(ch) = MIN(GET_HIT(ch) + 25, GET_MAX_HIT(ch));
      }
    }
    break;
  case 7:
    if (number(0, 1) == 0)
      command_interpreter(ch, "grin");
    else
      act("$n looks at you.", FALSE, ch, 0, 0, TO_ROOM);
    break;
  case 8:
    if (GET_LEVEL(ch) < 10) {
      if (IS_EVIL(ch)) {
	act("$n looks as if he might just bite your ankles!",
           FALSE, ch, 0, 0, TO_ROOM);
      } else {
        act("$n tries to look mean and dangerous.", FALSE, ch, 0, 0, TO_ROOM);
      }
    } else if (GET_LEVEL(ch) < 30) {
      if (IS_EVIL(ch)) {
	act("$n peers about, looking for things to kill.",
         FALSE, ch, 0, 0, TO_ROOM);
      } else {
        act("$n looks grimly around, surveying the area.",
	 FALSE, ch, 0, 0, TO_ROOM);
      }
    } else if (GET_LEVEL(ch) < 100) {
      if (GET_EQ(ch, WEAR_WIELD)) {
        if (IS_EVIL(ch)) {
    	  act("$n suddenly raises $p to the ready, the smell of blood in $s nostrils.",
	  FALSE, ch, GET_EQ(ch, WEAR_WIELD), 0, TO_ROOM);
        } else {
          act("$n surveys the area, $s hand resting lightly on $p.",
	  FALSE, ch, GET_EQ(ch, WEAR_WIELD), 0, TO_ROOM);
        }
      } else {
        if (IS_EVIL(ch)) {
	  act("$n looks at you and laughs quietly, confident in $s ability.",
	  FALSE, ch, 0, 0, TO_ROOM);
        } else {
          act("$n surveys the area, quietly competant.",
	  FALSE, ch, 0, 0, TO_ROOM);
        }
      }
    } else {
      if (GET_EQ(ch, WEAR_WIELD)) {
        if (IS_EVIL(ch)) {
	  act("$n wipes some blood from $p onto the corpse of $s last victim",
	  FALSE, ch, GET_EQ(ch, WEAR_WIELD), 0, TO_ROOM);
        } else {
          act("$n hefts $p thoughtfully, ever watchful and alert.",
	  FALSE, ch, GET_EQ(ch, WEAR_WIELD), 0, TO_ROOM);
	}
      } else {
	if (IS_EVIL(ch)) {
	  act("$n watches you coldly through a set of bloodshot eyes.",
	  FALSE, ch, 0, 0, TO_ROOM);
        } else {
          act("$n looks around thoughtfully, ever watchful and alert.",
	  FALSE, ch, 0, 0, TO_ROOM);
	}
      }
    }
    break;
  default:
    break;
  }
}

void idle_spellcaster(struct char_data *ch) {

  switch(number(0, 15)) {
  case 0:
    if (IS_EVIL(ch)) {
	act("$n calls upon the powers of darkness for protection!",
	FALSE, ch, 0, 0, TO_ROOM);
    }   else  {
	act("$n calls upon the powers of light for protection!",
	FALSE, ch, 0, 0, TO_ROOM);
    }
    cast_spell(ch, ch, NULL, SPELL_ARMOR);
    break;
  case 1:
    if (!IS_AFFECTED(ch, AFF_DETECT_INVIS)) {
	act("$n reads a short passage off a scroll, which crumbles to dust.",
	FALSE, ch, 0, 0, TO_ROOM);
      cast_spell(ch, ch, NULL, SPELL_DETECT_INVIS);
    }
    break;
  case 2:
    if (!IS_AFFECTED(ch, AFF_INFRAVISION)) {
	act("$n opens $s spellbook and recites a short passage.",
	FALSE, ch, 0, 0, TO_ROOM);
      cast_spell(ch, ch, NULL, SPELL_INFRAVISION);
    }
    break;
  case 3:
    if (!IS_AFFECTED(ch, AFF_INVISIBLE)) {
	if (IS_EVIL(ch)) {
	act("$n calls upon the powers of darkness to cloak $m.",
	FALSE, ch, 0, 0, TO_ROOM);
      } else {
	act("$n calls upon the powers of light to cloak $m.",
	FALSE, ch, 0, 0, TO_ROOM);
      }
      cast_spell(ch, ch, NULL, SPELL_INVISIBLE);
    }
    break;
  case 4:
    if (IS_EVIL(ch)) {
      act("$n closes $s eyes and utters the words, 'grasklbur'.",
	FALSE, ch, 0, 0, TO_ROOM);
      if (GET_LEVEL(ch) < 25) {
	act("A strange ripple appears in the air for a brief moment.",
	FALSE, ch, 0, 0, TO_ROOM);
	command_interpreter(ch, "sigh");
      } else if (GET_LEVEL(ch) < 50) {
	act("A warped rift starts to open, the sounds of pain and torment echo from within.",
          FALSE, ch, 0, 0, TO_ROOM);
	command_interpreter(ch, "cackle");
	act("As abruptly as it appeared the rift closes.",
	  FALSE, ch, 0, 0, TO_ROOM);
	command_interpreter(ch, "growl");
      } else {
	act("An evil shudder passes through you as a warped rift appears in the air before you!",
	  FALSE, ch, 0, 0, TO_ROOM);
	act("$n speaks to a being within the rift in a strange gutteral tongue.",
	  FALSE, ch, 0, 0, TO_ROOM);
	act("$n releases the soul of $s last victim into the rift, which abruptly closes with a pop.",
	  FALSE, ch, 0, 0, TO_ROOM);
	command_interpreter(ch, "grin");
      }
    } else {   
      act("$n closes $s eyes and utters the words, 'xsbfeghm'.", 
        FALSE, ch, 0, 0, TO_ROOM);
      if (GET_LEVEL(ch) < 5) {
        act("Nothing seems to happen.", FALSE, ch, 0, 0, TO_ROOM);
        command_interpreter(ch, "sigh");
      } else if (GET_LEVEL(ch) < 15) {
        act("A small flame sputters into existance... then dies.", 
          FALSE, ch, 0, 0, TO_ROOM);
        command_interpreter(ch, "frown");
      } else {
        act("A bright flame erupts from $n's index finger.", 
        FALSE, ch, 0, 0, TO_ROOM);
        command_interpreter(ch, "grin");
      } 
    }
    break;
  case 5:
    act("$n murmurs an incantation under $s breath.", 
	FALSE, ch, 0, 0, TO_ROOM);
    break;
  case 6:
  case 7:
    act("$n studies a spell book intently.", FALSE, ch, 0, 0, TO_ROOM);
    break;
  default:
    break;
  }
}

void idle_healer(struct char_data *ch) {

  struct char_data *cons;
  int selected = 0;

  for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room) {
    if (!number(0, 4) && !selected && IS_NPC(cons) &&
	((IS_GOOD(ch) && IS_GOOD(cons)) || (IS_EVIL(ch) && IS_EVIL(cons)) ||
	(IS_NEUTRAL(ch) && IS_NEUTRAL(cons)))) {

/* so they will only heal MOBs of a SIMILAR alignment... */

      selected = 1;
      switch(number(0, 15)) {
      case 0:
      cast_spell(ch, cons, NULL, SPELL_ARMOR);
        break;
      case 1:
        if (!IS_AFFECTED(cons, AFF_DETECT_INVIS))
          cast_spell(ch, cons, NULL, SPELL_DETECT_INVIS);
        break;
      case 2:
        if (!IS_AFFECTED(cons, AFF_INFRAVISION))
          cast_spell(ch, cons, NULL, SPELL_INFRAVISION);
        break;
      case 3:
        cast_spell(ch, cons, NULL, SPELL_BLESS);
        break;
      case 4:
        cast_spell(ch, ch, NULL, SPELL_CREATE_FOOD);
        command_interpreter(ch, "eat waybread");
        break;
      case 5:
        if (IS_AFFECTED(cons, AFF_CURSE))
          cast_spell(ch, cons, NULL, SPELL_REMOVE_CURSE);
        break;
      case 6:
        if (IS_AFFECTED(cons, AFF_POISON))
          cast_spell(ch, cons, NULL, SPELL_REMOVE_POISON);
        break;
      case 7:
        if (!IS_AFFECTED(cons, AFF_SANCTUARY))
          cast_spell(ch, cons, NULL, SPELL_SANCTUARY);
        break;
      case 8:
	if (IS_EVIL(ch))
	  act("$n stares at you intently and mutters something about a sacrifice...",
	  FALSE, ch, 0, 0, TO_ROOM);
        else  
          act("$n murmurs an incantation under $s breath.", 
	  FALSE, ch, 0, 0, TO_ROOM);
        break;
      case 9:
	if (IS_EVIL(ch))
	  act("$n closes $s eyes and offers the soul of $s last victim to the powers of darkness!",
	  FALSE, ch, 0, 0, TO_ROOM);
        else  
          act("$n closes $s eyes in a prayer to $s god.", 
	  FALSE, ch, 0, 0, TO_ROOM);
	break;
      case 10:
      case 11:
      case 12:
        if (GET_HIT(cons) < (int) (GET_MAX_HIT(cons) / 2)) {
          act("$n studies $N intently.", FALSE, ch, 0, cons, TO_NOTVICT);
          act("$n studies you intently.", FALSE, ch, 0, cons, TO_VICT);
          cast_spell(ch, cons, NULL, defensive_healcast_for_level(ch));
        }
      default:
        selected = 0;
        break;
      }
    }
  }
  if (!selected)
    if (GET_HIT(ch) < (int) (GET_MAX_HIT(ch) / 2))
      cast_spell(ch, ch, NULL, defensive_healcast_for_level(ch));
}

void idle_assassin(struct char_data *ch) {

  void npc_steal(struct char_data *ch, struct char_data *victim);

  struct char_data *cons;

  if (!IS_AFFECTED(ch, AFF_SNEAK) && !IS_AFFECTED(ch, AFF_HIDE) &&
	!number(0, 4)) {
    act("$n winks at you.", FALSE, ch, 0, 0, TO_ROOM);
  }
  if (GET_POS(ch) == POS_STANDING) {
    for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room)
      if (!IS_NPC(cons) && (GET_LEVEL(cons) < LVL_IMMORT) && (!number(0, 4))) {
        npc_steal(ch, cons);
      }
  }

  if (!IS_AFFECTED(ch, AFF_SNEAK))
    command_interpreter(ch, "sneak");
  if (!IS_AFFECTED(ch, AFF_HIDE))
    command_interpreter(ch, "hide");
}

void mob_idle(struct char_data *ch) {

  if (!IS_NPC(ch) || ch->desc)
    return;

  if (ch->player.class == CLASS_FIGHTER)
    idle_fighter(ch);
  if (ch->player.class == CLASS_SPELLCASTER)
    idle_spellcaster(ch);
  if (ch->player.class == CLASS_HEALER)
    idle_healer(ch);
  if (ch->player.class == CLASS_ASSASSIN)
    idle_assassin(ch);
}

void init_fighter(struct char_data *ch, struct char_data *vict) {

  ACMD(do_bash);
  ACMD(do_kick);

/* Under here is where to add any combat screams for initial combat */

  act("$n roars and launches an attack!.", FALSE, ch, 0, 0, TO_ROOM);

  switch(number(0, 2)) {
  case 0:
    if (GET_LEVEL(ch) >= 10)
      if (GET_EQ(ch, WEAR_WIELD))
        do_bash(ch, vict->player.name, 0, 0);
      else
        do_kick(ch, vict->player.name, 0, 0);
    else
      hit(ch, vict, TYPE_UNDEFINED);
    break;
  case 1:
    if (GET_LEVEL(ch) >= 5)
      do_kick(ch, vict->player.name, 0, 0);
    else
      hit(ch, vict, TYPE_UNDEFINED);
    break;
  default:
    hit(ch, vict, TYPE_UNDEFINED);
    break;
  }
}

void init_spellcaster(struct char_data *ch, struct char_data *vict) {

/* Add shouts here */
   act("$n gestures rapidly and summons $s magical powers!", FALSE, ch, 0, 0, TO_ROOM);

  if (!number(0, 1))
    cast_spell(ch, vict, NULL, offensive_spellcast_for_level(ch));
  else
    hit(ch, vict, TYPE_UNDEFINED);
}

void init_healer(struct char_data *ch, struct char_data *vict) {

/* Add shouts here */
   act("$n prepares to call down the wrath of $s god!", FALSE, ch, 0, 0, TO_ROOM);
  if (!number(0, 1))
    cast_spell(ch, vict, NULL, offensive_healcast_for_level(ch));
  else
    hit(ch, vict, TYPE_UNDEFINED);
}

void init_assassin(struct char_data *ch, struct char_data *vict) {

/* lots of else.. hit() calls here... sigh, there's prob'ly a neater way */

  int perc;

/* After this line add initial actions */
  if (!number(0, 1)) {
    if (GET_EQ(ch, WEAR_WIELD)) {
      if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) == (TYPE_PIERCE - TYPE_HIT)) {
        if (!MOB_FLAGGED(vict, MOB_AWARE)) {
          perc = number(1, 101);
          if (AWAKE(vict) && (perc > GET_SKILL(ch, SKILL_BACKSTAB)))
            damage(ch, vict, 0, TYPE_STAB);
          else
            hit(ch, vict, SKILL_BACKSTAB);
        } else
          hit(ch, vict, TYPE_UNDEFINED);
      } else
        hit(ch, vict, TYPE_UNDEFINED);
    } else
      hit(ch, vict, TYPE_UNDEFINED);
  } else
    hit(ch, vict, TYPE_UNDEFINED);
}

void mob_initiate(struct char_data *ch, struct char_data *victim) {

  if (!victim)
    return;

  if (!IS_NPC(ch) || ch->desc || ch->player.class < 0) {
    hit(ch, victim, TYPE_UNDEFINED);
    return;
  }

  if (ch->player.class == CLASS_FIGHTER)
    init_fighter(ch, victim);
  if (ch->player.class == CLASS_SPELLCASTER)
    init_spellcaster(ch, victim);
  if (ch->player.class == CLASS_HEALER)
    init_healer(ch, victim);
  if (ch->player.class == CLASS_ASSASSIN)
    init_assassin(ch, victim);
}
