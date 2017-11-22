/* ************************************************************************
*   File: act.offensive.c                               Part of CircleMUD *
*  Usage: player-level commands of an offensive nature                    *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"

#define LEARNED(ch) (prac_params[LEARNED_LEVEL][(int)GET_CLASS(ch)]);
#define LEARNED_LEVEL    0;

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern int pk_allowed;

/* extern functions */
void raw_kill(struct char_data * ch, struct char_data * killer);
void mob_initiate(struct char_data *ch, struct char_data *victim);

ACMD(do_assist)
{
  struct char_data *helpee, *opponent;

  if (FIGHTING(ch)) {
    send_to_char("You're already fighting!  How can you assist someone else?\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Whom do you wish to assist?\r\n", ch);
  else if (!(helpee = get_char_room_vis(ch, arg)))
    send_to_char(NOPERSON, ch);
  else if (helpee == ch)
    send_to_char("You can't help yourself any more than this!\r\n", ch);
  else {
    for (opponent = world[ch->in_room].people;
	 opponent && (FIGHTING(opponent) != helpee);
	 opponent = opponent->next_in_room)
		;

    if (!opponent)
      act("But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!CAN_SEE(ch, opponent))
      act("You can't see who is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!pk_allowed && !IS_NPC(opponent))	/* prevent accidental pkill */
      act("Use 'murder' if you really want to attack $N.", FALSE,
	  ch, 0, opponent, TO_CHAR);
    else {
      send_to_char("You join the fight!\r\n", ch);
      act("$N assists you!", 0, helpee, 0, ch, TO_CHAR);
      act("$n assists $N.", FALSE, ch, 0, helpee, TO_NOTVICT);
      mob_initiate(ch, opponent);
    }
  }
}


ACMD(do_hit)
{
  struct char_data *vict;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Hit who?\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, arg)))
    send_to_char("They don't seem to be here.\r\n", ch);
  else if (vict == ch) {
    send_to_char("You hit yourself...OUCH!.\r\n", ch);
    act("$n hits $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
  } else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == vict))
    act("$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, vict, TO_CHAR);
  else {
    if (!pk_allowed) {
      if (!IS_NPC(vict) && !IS_NPC(ch) && (subcmd != SCMD_MURDER)) {
	send_to_char("Use 'murder' to hit another player.\r\n", ch);
	return;
      }
      if (IS_AFFECTED(ch, AFF_CHARM) && !IS_NPC(ch->master) && !IS_NPC(vict))
	return;			/* you can't order a charmed pet to attack a
				 * player */
    }
    if ((GET_POS(ch) == POS_STANDING) && (vict != FIGHTING(ch))) {
      mob_initiate(ch, vict);
      WAIT_STATE(ch, PULSE_VIOLENCE + 2);
    } else
      send_to_char("You do the best you can!\r\n", ch);
  }
}



ACMD(do_kill)
{
  struct char_data *vict;

  if ((GET_LEVEL(ch) < LVL_IMPL) || IS_NPC(ch)) {
    do_hit(ch, argument, cmd, subcmd);
    return;
  }
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Kill who?\r\n", ch);
  } else {
    if (!(vict = get_char_room_vis(ch, arg)))
      send_to_char("They aren't here.\r\n", ch);
    else if (ch == vict)
      send_to_char("Your mother would be so sad.. :(\r\n", ch);
    else {
      act("You chop $M to pieces!  Ah!  The blood!", FALSE, ch, 0, vict, TO_CHAR);
      act("$N chops you to pieces!", FALSE, vict, 0, ch, TO_CHAR);
      act("$n brutally slays $N!", FALSE, ch, 0, vict, TO_NOTVICT);
      raw_kill(vict, ch);
    }
  }
}



ACMD(do_backstab)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, buf);

  if (!(vict = get_char_room_vis(ch, buf))) {
    send_to_char("Backstab who?\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("How can you sneak up on yourself?\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT) {
    send_to_char("Only piercing weapons can be used for backstabbing.\r\n", ch);
    return;
  }
  if (FIGHTING(vict)) {
    send_to_char("You can't backstab a fighting person -- they're too alert!\r\n", ch);
    return;
  }

  if (MOB_FLAGGED(vict, MOB_AWARE)) {
    act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    mob_initiate(vict, ch);
    return;
  }

  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_BACKSTAB);

  if (AWAKE(vict) && (percent > prob))
    damage(ch, vict, 0, SKILL_BACKSTAB);
  else
    hit(ch, vict, SKILL_BACKSTAB);
 if(IS_NPC(vict)){
  SET_BIT(MOB_FLAGS(vict),MOB_AWARE);
 }
}



ACMD(do_order)
{
  char name[100], message[256];
  char buf[256];
  bool found = FALSE;
  int org_room;
  struct char_data *vict;
  struct follow_type *k;

  half_chop(argument, name, message);

  if (!*name || !*message)
    send_to_char("Order who to do what?\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, name)) && !is_abbrev(name, "followers"))
    send_to_char("That person isn't here.\r\n", ch);
  else if (ch == vict)
    send_to_char("You obviously suffer from skitzofrenia.\r\n", ch);

  else {
    if (IS_AFFECTED(ch, AFF_CHARM)) {
      send_to_char("Your superior would not aprove of you giving orders.\r\n", ch);
      return;
    }
    if (vict) {
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, vict, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, vict, TO_ROOM);

      if ((vict->master != ch) || !IS_AFFECTED(vict, AFF_CHARM))
	act("$n has an indifferent look.", FALSE, vict, 0, 0, TO_ROOM);
      else {
	send_to_char(OK, ch);
	command_interpreter(vict, message);
      }
    } else {			/* This is order "followers" */
      sprintf(buf, "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, vict, TO_ROOM);

      org_room = ch->in_room;

      for (k = ch->followers; k; k = k->next) {
	if (org_room == k->follower->in_room)
	  if (IS_AFFECTED(k->follower, AFF_CHARM)) {
	    found = TRUE;
	    command_interpreter(k->follower, message);
	  }
      }
      if (found)
	send_to_char(OK, ch);
      else
	send_to_char("Nobody here is a loyal subject of yours!\r\n", ch);
    }
  }
}



ACMD(do_flee)
{
  int i, attempt, loss;

  if (!FIGHTING(ch)) {
    send_to_char("You have already escaped combat!!\r\n", ch);
    return;
  }

  for (i = 0; i < 6; i++) {
    attempt = number(0, NUM_OF_DIRS - 1);	/* Select a random direction */
    if (CAN_GO(ch, attempt) &&
	!IS_SET(ROOM_FLAGS(EXIT(ch, attempt)->to_room), ROOM_DEATH)) {
      act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);
      if (do_simple_move(ch, attempt, TRUE)) {
	send_to_char("You flee head over heels.\r\n", ch);
	if (FIGHTING(ch)) {
	  if (!IS_NPC(ch)) {
	    loss = GET_MAX_HIT(FIGHTING(ch)) - GET_HIT(FIGHTING(ch));
	    loss *= GET_LEVEL(FIGHTING(ch));
	    gain_exp(ch, -loss);
	  }
	  if (FIGHTING(FIGHTING(ch)) == ch)
	    stop_fighting(FIGHTING(ch));
	  stop_fighting(ch);
	}
      } else {
	act("$n tries to flee, but can't!", TRUE, ch, 0, 0, TO_ROOM);
      }
      return;
    }
  }
  send_to_char("PANIC!  You couldn't escape!\r\n", ch);
}


ACMD(do_doorbash)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, arg);

  if (GET_SKILL(ch, SKILL_DOORBASH) == 0) {
    send_to_char("You'd better leave all the door smashing to the big tough trained fighters.\r\n", ch);
    return;
  }
 if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      send_to_char("DoorBash??? That is not a door!!!\r\n", ch);
    } else {
      send_to_char("DoorBash what?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("You imagine yourself as a door and try to break yourself down...\r\n", ch);
    return;
  }
  if (GET_EQ(ch, WEAR_WIELD) && !IS_NPC(ch) && IS_NPC(ch)) {
    send_to_char("You need to have no weapon to be able to bust a door down.\r\n", ch);
    return;
  }
  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_DOORBASH);

 /* if (DOOR_IS_OPEN(ch, obj, door)) {
    send_to_char("You admire the open door and wonder how that thought ever came to pass.\r\n", ch);
    return;
	}
*/
	

  if (IS_NPC(vict) && !IS_NPC(vict));
    send_to_char("Silly arn't we...... Monsters are NOT DOORS!.\r\n", ch);
    percent = 101;
    return;

  if (percent > prob) {
    damage(ch, vict, 4, SKILL_BASH);
    GET_POS(ch) = POS_SITTING;
    send_to_char("You SMASH into the door and crumple down to the floor in a heap, the door in one peace.\r\n", ch);
  } else {
    damage(ch, vict, 2, SKILL_DOORBASH);
    /* GET_POS(ch) = POS_SITTING; */
    /* WAIT_STATE(vict, PULSE_VIOLENCE); */
  }
  /* WAIT_STATE(ch, PULSE_VIOLENCE * 2); */
}



ACMD(do_bash)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, arg);

  if (GET_SKILL(ch, SKILL_BASH) == 0) {
    send_to_char("You'd better leave all the martial arts to the trained fighters.\r\n", ch);
    return;
  }
 if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Bash who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD) && !IS_NPC(ch)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_BASH);

  if (MOB_FLAGGED(vict, MOB_NOBASH))
    percent = 101;

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_BASH);
    GET_POS(ch) = POS_SITTING;
  } else {
    damage(ch, vict, 2, SKILL_BASH);
    GET_POS(vict) = POS_SITTING;
    WAIT_STATE(vict, PULSE_VIOLENCE);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

/*
ACMD(do_trip)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, arg);

  if (GET_SKILL(ch, SKILL_TRIP) == 0) {
    send_to_char("You'd better leave all the martial arts to the trained fighters.\r\n", ch);
    return;
  }
 if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Trip who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_FEET) && !IS_NPC(ch)) {
    send_to_char("You cant do this barefoot.\r\n", ch);
    return;
  }
  percent = number(1, 101);	
  prob = GET_SKILL(ch, SKILL_TRIP);

  if (MOB_FLAGGED(vict, MOB_NOBASH))
    percent = 101;

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_TRIP);
    GET_POS(ch) = POS_SITTING;
  } else {
    damage(ch, vict, 2, SKILL_TRIP);
    GET_POS(vict) = POS_SITTING;
    WAIT_STATE(vict, PULSE_VIOLENCE);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}
*/


ACMD(do_trip)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, arg);

  if (GET_SKILL(ch, SKILL_TRIP) == 0) {
    GET_POS(ch) = POS_SITTING;
    act("$N tries a fancy trip move on you. You trip over onto your bum.", FALSE, ch, 0, vict, TO_CHAR);
    send_to_char("You'd better leave all the triping to the trained thiefs.\r\n", ch);
    return;
  }
 if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Trip who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today... ha ha ha ... twit...\r\n", ch);
    return;
  }
  percent = number(1, 101);
  prob = GET_SKILL(ch, SKILL_TRIP);

  if (MOB_FLAGGED(vict, MOB_NOBASH))
    percent = 101;

  if (percent > prob) {
    GET_POS(ch) = POS_SITTING;
    act("$N tries a fancy trip move on you. You trip over onto your bum.", FALSE, ch, 0, vict, TO_CHAR);
    act("$n tries to Trip $N and fails.  A small cloud of dust rises from where $n sits.", FALSE, ch, 0, vict, TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE * 8);
  } else {
    GET_POS(vict) = POS_SITTING;
    act("You try a fancy trip move on $N, and smile as $N falls on there bum.", FALSE, ch, 0, vict, TO_CHAR);
    act("$n tries to Trip $N. You see $N fall over.", FALSE, ch, 0, vict, TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE * 4);
  }
}


ACMD(do_rescue)
{
  struct char_data *vict, *tmp_ch;
  int percent, prob;

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("Whom do you want to rescue?\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("What about fleeing instead?\r\n", ch);
    return;
  }
  if (FIGHTING(ch) == vict) {
    send_to_char("How can you rescue someone you are trying to kill?\r\n", ch);
    return;
  }
  for (tmp_ch = world[ch->in_room].people; tmp_ch &&
       (FIGHTING(tmp_ch) != vict); tmp_ch = tmp_ch->next_in_room);

  if (!tmp_ch) {
    act("But nobody is fighting $M!", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  if (GET_SKILL(ch, SKILL_RESCUE) == 0)
    send_to_char("But only trained warriors can do this!", ch);
  else {
    percent = number(1, 101);	/* 101% is a complete failure */
    prob = GET_SKILL(ch, SKILL_RESCUE);

    if (percent > prob) {
      send_to_char("You fail the rescue!\r\n", ch);
      return;
    }
    send_to_char("Banzai!  To the rescue...\r\n", ch);
    act("You are rescued by $N, you are confused!", FALSE, vict, 0, ch, TO_CHAR);
    act("$n heroically rescues $N!", FALSE, ch, 0, vict, TO_NOTVICT);

    if (FIGHTING(vict) == tmp_ch)
      stop_fighting(vict);
    if (FIGHTING(tmp_ch))
      stop_fighting(tmp_ch);
    if (FIGHTING(ch))
      stop_fighting(ch);

    set_fighting(ch, tmp_ch);
    set_fighting(tmp_ch, ch);

    WAIT_STATE(vict, 2 * PULSE_VIOLENCE);
  }

}

ACMD(do_kick)
{
  struct char_data *vict;
  int percent, prob;

  if (GET_SKILL(ch, SKILL_KICK) == 0) {
    send_to_char("You'd better leave all the martial arts to the trained fighters.\r\n", ch);
    return;
  }
 if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("You think about it but it is too peaceful. You remove that bad thought...\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Kick who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_FEET) && !IS_NPC(ch)) {
    send_to_char("You cant do this barefoot.\r\n", ch);
    return;
  }
  percent = ((10 - (GET_AC(vict) / 10)) << 1) + number(1, 101);	/* 101% is a complete
								 * failure */
  prob = GET_SKILL(ch, SKILL_KICK);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_KICK);
  } else
    damage(ch, vict, dice(GET_LEVEL(ch), 4), SKILL_KICK);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

ACMD(do_headbutt)
{
  struct char_data *vict;
  int percent, prob;

  if (GET_SKILL(ch, SKILL_HEADBUTT) == 0) {
    send_to_char("You'd better leave all the martial arts to the trained fighters.\r\n", ch);
    return;
  }
 if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("You think about it but it is too peaceful. You remove that bad thought...\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_HEAD) && !IS_NPC(ch)) {
    send_to_char("Are you nuts ?  Wear a helmet!\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Headbutt who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  percent = ((10 - (GET_AC(vict) / 10)) << 1) + number(1, 101);	/* 101% is a complete
								 * failure */
  prob = GET_SKILL(ch, SKILL_HEADBUTT);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_HEADBUTT);
  } else
    damage(ch, vict, dice(GET_LEVEL(ch), 10), SKILL_HEADBUTT);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}


ACMD(do_bodyslam)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, arg);

  if (GET_SKILL(ch, SKILL_BODYSLAM) == 0) {
    send_to_char("You'd better leave all the martial arts to the trained fighters.\r\n", ch);
    return;
  }
 if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Bodyslam who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_SHIELD) && !IS_NPC(ch)) {
    send_to_char("You need to have a shield to do this.\r\n", ch);
    return;
  }
  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_BODYSLAM);

  if (MOB_FLAGGED(vict, MOB_NOBASH))
    percent = 101;

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_BODYSLAM);
    GET_POS(ch) = POS_SITTING;
  } else {
    damage(ch, vict, dice(GET_LEVEL(ch), 10), SKILL_BODYSLAM);
    GET_POS(vict) = POS_SITTING;
    WAIT_STATE(vict, PULSE_VIOLENCE);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}


ACMD(do_groinkick)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, arg);

  if (GET_SKILL(ch, SKILL_GROIN_KICK) == 0) {
    send_to_char("You'd better leave all the kicking to the trained fighters.\r\n", ch);
    return;
  }
 if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Groin Kick who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_FEET) && !IS_NPC(ch)) {
    send_to_char("You need to have a something on your feet to do this.\r\n", ch);
    return;
  }
  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_GROIN_KICK);

  if (MOB_FLAGGED(vict, MOB_NOBASH))
    percent = 101;

  if (percent > prob) {
    damage(ch, ch, dice(GET_LEVEL(ch), 20), SKILL_GROIN_KICK);
    GET_POS(ch) = POS_SITTING;
    send_to_char("You miss your kick and flop to the ground, legs spread...OUCH!!...\r\n", ch);
    send_to_char("You see through a painfull face that your intended victum has not noticed you. ..\r\n", ch);
	return;
    }
   if (percent < prob) {
    damage(ch, vict, dice(GET_LEVEL(ch), 20), SKILL_GROIN_KICK);
    GET_POS(vict) = POS_SITTING;
    WAIT_STATE(vict, PULSE_VIOLENCE);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 5);
}

/*void do_shoot(struct char_data *ch, struct char_data *victim, int type)*/
ACMD(do_shoot)
{
  struct obj_data *obj;
  struct char_data *vict;
  int percent, prob;
  int dam;
 
  if(!(obj=ch->equipment[WEAR_HOLD]))
    return;
  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Shoot Who??..\r\n",ch);
      return;
      }
  }
  if(vict == ch){
    if(!IS_NPC(ch))
      send_to_char("Shoot yourself? Nah...\n\r", ch);
    return;
  }
  if(GET_OBJ_TYPE(obj)!=ITEM_FIREWEAPON) {
    if(!IS_NPC(ch))
      send_to_char("To shoot, you need to HOLD a firing weapon.\n\r",ch);
    return;
  }
  if(obj->obj_flags.value[0] <= 0){
    if(!IS_NPC(ch))
      send_to_char("Oops.  Nothing to shoot.\n\r",ch);
    act("Hmmm.  $n fires an empty $p.",
      FALSE, ch, obj,0,TO_ROOM);
    return;
  }
  if(IS_AFFECTED(ch,AFF_CHARM) && ch->master)
    vict = ch->master;
  if((GET_LEVEL(ch) < LVL_IMMORT)&&(!IS_NPC(ch)))
  obj->obj_flags.value[0]--;
  if((!IS_NPC(ch))){
    prob = GET_SKILL(ch,SKILL_SHOOT);
  percent = ((10 - (GET_AC(vict) / 10)) << 1) + number(1, 101);	
  }
  else {
    prob = number(1,101);
  percent = ((10 - (GET_AC(vict) / 10)) << 1) + number(1, 101);	
  }
    dam=dice(GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 2), GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 3));
  if(percent > prob){
    damage(ch, vict, 0, SKILL_SHOOT);
  } else {
    damage(ch, vict, dam, SKILL_SHOOT);
  }
  if(IS_NPC(ch))
    return;
  if(GET_LEVEL(ch) < (LVL_IMMORT+1)){
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    WAIT_STATE(vict, PULSE_VIOLENCE * 2);
  } else
     return;
}
