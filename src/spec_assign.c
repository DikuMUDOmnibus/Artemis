/* ************************************************************************
*   File: spec_assign.c                                 Part of CircleMUD *
*  Usage: Functions to assign function pointers to objs/mobs/rooms        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <stdio.h>
#include "structs.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"

extern struct room_data *world;
extern int top_of_world;
extern int mini_mud;
extern struct index_data *mob_index;
extern struct index_data *obj_index;

/* functions to perform assignments */

void ASSIGNMOB(int mob, SPECIAL(fname))
{
  if (real_mobile(mob) >= 0)
    mob_index[real_mobile(mob)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant mob #%d",
	    mob);
    logs(buf);
  }
}

void ASSIGNOBJ(int obj, SPECIAL(fname))
{
  if (real_object(obj) >= 0)
    obj_index[real_object(obj)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant obj #%d",
	    obj);
    logs(buf);
  }
}

void ASSIGNROOM(int room, SPECIAL(fname))
{
  if (real_room(room) >= 0)
    world[real_room(room)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant rm. #%d",
	    room);
    logs(buf);
  }
}


/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
  SPECIAL(postmaster);
  SPECIAL(cityguard);
  SPECIAL(receptionist);
  SPECIAL(cryogenicist);
  SPECIAL(guild_guard);
  SPECIAL(guild);
  SPECIAL(puff);
  SPECIAL(fido);
  SPECIAL(cat);
  SPECIAL(rat);
  SPECIAL(janitor);
  SPECIAL(mayor);
  SPECIAL(snake);
  SPECIAL(thief);
  SPECIAL(magic_user);
  void assign_kings_castle(void);
  SPECIAL(sund_earl);
  SPECIAL(hangman);  
  SPECIAL(blinder);
  SPECIAL(silktrader);
  SPECIAL(butcher);   
  SPECIAL(idiot);  
  SPECIAL(athos);
  SPECIAL(stu);
  SPECIAL(kickbasher);
  SPECIAL(icewizard);
/* Assigns for Muidguard */
SPECIAL(pink);
SPECIAL(oldman);
SPECIAL(fatlady);
/* Assigns for Canibal Haven */
SPECIAL(guardian);
SPECIAL(witchdoctor);
SPECIAL(prophet);
SPECIAL(headhunter);
SPECIAL(warrior);
SPECIAL(bodyguard);
/* Assigns for The Spiral of Death */
SPECIAL(flying_snake);
/* Assigns for Elvin Forest */
SPECIAL(trguardian);
/* NEW SPECIALS BY ACID */
SPECIAL(metaphysician);
SPECIAL(loader);
SPECIAL(big_dragon);
SPECIAL(little_dragon);
SPECIAL(poker);
SPECIAL(dealer);
SPECIAL(barker);
/* Tundra Specials by Raven */
SPECIAL(ice_devil);
SPECIAL(ice_giant);
SPECIAL(ice_wyrm);
SPECIAL(mammoth);
SPECIAL(judge_penguin);
SPECIAL(outcast_fig);
SPECIAL(yeti);
SPECIAL(ice_guard);
/* mobclass.c dual classing spec procs.. */
SPECIAL(assassin);
SPECIAL(spellcaster);
SPECIAL(fighter);
SPECIAL(healer);

  assign_kings_castle();

  ASSIGNMOB(1, puff);

  /* Immortal Zone */
  ASSIGNMOB(1200, receptionist);
  ASSIGNMOB(1201, postmaster);
  ASSIGNMOB(1202, janitor);

   /* Assigns for Muidguard  */
ASSIGNMOB(3096, pink);
ASSIGNMOB(3097, oldman);
ASSIGNMOB(3098, fatlady);
   /* Assigns for Canibal Haven */
ASSIGNMOB(31820, guardian);
ASSIGNMOB(31814, witchdoctor);
ASSIGNMOB(31821, prophet);
ASSIGNMOB(31810, headhunter);
ASSIGNMOB(31804, warrior);
ASSIGNMOB(31818, bodyguard);

/* Assigns for Elvin Forest */
ASSIGNMOB(19001, trguardian);

/* Assigns for The Spiral of Death */
ASSIGNMOB(21006, flying_snake);

  /* Midgaard */
  ASSIGNMOB(3005, receptionist);
  ASSIGNMOB(3010, postmaster);
  ASSIGNMOB(3020, guild);
  ASSIGNMOB(3021, guild);
  ASSIGNMOB(3022, guild);
  ASSIGNMOB(3023, guild);
  ASSIGNMOB(3024, guild_guard);
  ASSIGNMOB(3025, guild_guard);
  ASSIGNMOB(3026, guild_guard);
  ASSIGNMOB(3027, guild_guard);
  ASSIGNMOB(3059, cityguard);
  ASSIGNMOB(3060, cityguard);
  ASSIGNMOB(3061, janitor);
  ASSIGNMOB(3062, fido);
  ASSIGNMOB(3062, fido);
  ASSIGNMOB(3067, cityguard);
  ASSIGNMOB(3068, janitor);
  ASSIGNMOB(3095, cryogenicist);
  ASSIGNMOB(3105, mayor);


  /* LOTSA CATS */
  ASSIGNMOB(21111, cat);
  ASSIGNMOB(21117, cat);
  ASSIGNMOB(21118, cat);
  ASSIGNMOB(21129, cat);
  ASSIGNMOB(21120, cat);
  ASSIGNMOB(5443, cat);
  ASSIGNMOB(21187, cat);

  /* ZED TOWN */
  ASSIGNMOB(21184, janitor);
  ASSIGNMOB(21185, fido);

  /* MORIA */
  ASSIGNMOB(4000, snake);
  ASSIGNMOB(4001, snake);
  ASSIGNMOB(4053, snake);
  ASSIGNMOB(4100, magic_user);
  ASSIGNMOB(4102, snake);
  ASSIGNMOB(4103, thief);

  /* Redferne's */
  ASSIGNMOB(7900, cityguard);

  /* PYRAMID */
  ASSIGNMOB(5300, snake);
  ASSIGNMOB(5301, snake);
  ASSIGNMOB(5304, thief);
  ASSIGNMOB(5305, thief);
  ASSIGNMOB(5309, magic_user); /* should breath fire */
  ASSIGNMOB(5311, magic_user);
  ASSIGNMOB(5313, magic_user); /* should be a cleric */
  ASSIGNMOB(5314, magic_user); /* should be a cleric */
  ASSIGNMOB(5315, magic_user); /* should be a cleric */
  ASSIGNMOB(5316, magic_user); /* should be a cleric */
  ASSIGNMOB(5317, magic_user);

  /* High Tower Of Sorcery */
  ASSIGNMOB(2501, magic_user); /* should likely be cleric */
  ASSIGNMOB(2504, magic_user);
  ASSIGNMOB(2507, magic_user);
  ASSIGNMOB(2508, magic_user);
  ASSIGNMOB(2510, magic_user);
  ASSIGNMOB(2511, thief);
  ASSIGNMOB(2514, magic_user);
  ASSIGNMOB(2515, magic_user);
  ASSIGNMOB(2516, magic_user);
  ASSIGNMOB(2517, magic_user);
  ASSIGNMOB(2518, magic_user);
  ASSIGNMOB(2520, magic_user);
  ASSIGNMOB(2521, magic_user);
  ASSIGNMOB(2522, magic_user);
  ASSIGNMOB(2523, magic_user);
  ASSIGNMOB(2524, magic_user);
  ASSIGNMOB(2525, magic_user);
  ASSIGNMOB(2526, magic_user);
  ASSIGNMOB(2527, magic_user);
  ASSIGNMOB(2528, magic_user);
  ASSIGNMOB(2529, magic_user);
  ASSIGNMOB(2530, magic_user);
  ASSIGNMOB(2531, magic_user);
  ASSIGNMOB(2532, magic_user);
  ASSIGNMOB(2533, magic_user);
  ASSIGNMOB(2534, magic_user);
  ASSIGNMOB(2536, magic_user);
  ASSIGNMOB(2537, magic_user);
  ASSIGNMOB(2538, magic_user);
  ASSIGNMOB(2540, magic_user);
  ASSIGNMOB(2541, magic_user);
  ASSIGNMOB(2548, magic_user);
  ASSIGNMOB(2549, magic_user);
  ASSIGNMOB(2552, magic_user);
  ASSIGNMOB(2553, magic_user);
  ASSIGNMOB(2554, magic_user);
  ASSIGNMOB(2556, magic_user);
  ASSIGNMOB(2557, magic_user);
  ASSIGNMOB(2559, magic_user);
  ASSIGNMOB(2560, magic_user);
  ASSIGNMOB(2562, magic_user);
  ASSIGNMOB(2564, magic_user);

  /* SEWERS */
  ASSIGNMOB(7006, snake);
  ASSIGNMOB(7009, magic_user);
  ASSIGNMOB(7200, magic_user);
  ASSIGNMOB(7201, magic_user);
  ASSIGNMOB(7202, magic_user);

  /* FOREST */
  ASSIGNMOB(6112, little_dragon);
  ASSIGNMOB(6113, snake);
  ASSIGNMOB(6114, magic_user);
  ASSIGNMOB(6115, magic_user);
  ASSIGNMOB(6116, magic_user); /* should be a cleric */
  ASSIGNMOB(6117, magic_user);

  /* ARACHNOS */
  ASSIGNMOB(6302, little_dragon);
  ASSIGNMOB(6309, magic_user);
  ASSIGNMOB(6312, magic_user);
  ASSIGNMOB(6314, magic_user);
  ASSIGNMOB(6315, magic_user);
  ASSIGNMOB(6316, little_dragon);
  ASSIGNMOB(6317, little_dragon);

   /* DRAGONS */
  ASSIGNMOB(7040, big_dragon);
  ASSIGNMOB(9910, big_dragon);
  ASSIGNMOB(9989, little_dragon);
  

  /* Desert */
  ASSIGNMOB(5004, magic_user);
  ASSIGNMOB(5005, guild_guard); /* brass dragon */
  ASSIGNMOB(5010, magic_user);
  ASSIGNMOB(5014, magic_user);

  /* Drow City */
  ASSIGNMOB(5103, magic_user);
  ASSIGNMOB(5104, magic_user);
  ASSIGNMOB(5107, magic_user);
  ASSIGNMOB(5108, magic_user);

  /* Old Thalos */
  ASSIGNMOB(5200, magic_user);
  ASSIGNMOB(5201, magic_user);
  ASSIGNMOB(5209, magic_user);

  /* New Thalos */
/* 5481 - Cleric (or Mage... but he IS a high priest... *shrug*) */
  ASSIGNMOB(5404, receptionist);
  ASSIGNMOB(5421, magic_user);
  ASSIGNMOB(5422, magic_user);
  ASSIGNMOB(5423, magic_user);
  ASSIGNMOB(5424, magic_user);
  ASSIGNMOB(5425, magic_user);
  ASSIGNMOB(5426, magic_user);
  ASSIGNMOB(5427, magic_user);
  ASSIGNMOB(5428, magic_user);
  ASSIGNMOB(5434, cityguard);
  ASSIGNMOB(5440, magic_user);
  ASSIGNMOB(5455, magic_user);
  ASSIGNMOB(5461, cityguard);
  ASSIGNMOB(5462, cityguard);
  ASSIGNMOB(5463, cityguard);
  ASSIGNMOB(5482, cityguard);
/*
5400 - Guildmaster (Mage)
5401 - Guildmaster (Cleric)
5402 - Guildmaster (Warrior)
5403 - Guildmaster (Thief)
5456 - Guildguard (Mage)
5457 - Guildguard (Cleric)
5458 - Guildguard (Warrior)
5459 - Guildguard (Thief)
*/

  /* ROME */
  ASSIGNMOB(12009, magic_user);
  ASSIGNMOB(12018, cityguard);
  ASSIGNMOB(12020, magic_user);
  ASSIGNMOB(12021, cityguard);
  ASSIGNMOB(12025, magic_user);
  ASSIGNMOB(12030, loader);
  ASSIGNMOB(12038, magic_user);
  ASSIGNMOB(12031, magic_user);
  ASSIGNMOB(12032, magic_user);

  /* King Welmar's Castle (not covered in castle.c) */
  ASSIGNMOB(15015, thief);      /* Ergan... have a better idea? */
  ASSIGNMOB(15032, magic_user); /* Pit Fiend, have something better?  Use it */

  /* DWARVEN KINGDOM */
  ASSIGNMOB(6500, cityguard);
  ASSIGNMOB(6502, magic_user);
  ASSIGNMOB(6509, magic_user);
  ASSIGNMOB(6516, magic_user);

/* Assigns for the Town of Sundhaven  */


ASSIGNMOB(6600, sund_earl);        /* Earl of Sundhaven */
ASSIGNMOB(6601, cityguard);
ASSIGNMOB(6602, hangman);
ASSIGNMOB(6664, postmaster);
ASSIGNMOB(6656, guild_guard);
ASSIGNMOB(6655, guild_guard); 
ASSIGNMOB(6658, guild_guard);
ASSIGNMOB(6657, guild_guard);
ASSIGNMOB(6666, stu);
ASSIGNMOB(6606, rat);             /* Smoke rat */
ASSIGNMOB(6616, guild);
ASSIGNMOB(6619, guild);
ASSIGNMOB(6617, guild);  
ASSIGNMOB(6618, guild);
ASSIGNMOB(6659, cityguard);
ASSIGNMOB(6660, cityguard);    
ASSIGNMOB(6607, thief);
ASSIGNMOB(6648, butcher);
ASSIGNMOB(6661, blinder);
ASSIGNMOB(6637, silktrader);
ASSIGNMOB(6615, idiot);
ASSIGNMOB(6653, athos);
/* GHENNA */
ASSIGNMOB(9902, magic_user);
ASSIGNMOB(9907, magic_user);
ASSIGNMOB(9908, magic_user);
ASSIGNMOB(9909, magic_user);
ASSIGNMOB(9913, magic_user);
ASSIGNMOB(9914, magic_user);
ASSIGNMOB(9923, magic_user);
ASSIGNMOB(9933, magic_user);
/*WASTELAND*/
ASSIGNMOB(9002, snake);
ASSIGNMOB(9011, icewizard);
ASSIGNMOB(9012, magic_user);

/* Asterix */
ASSIGNMOB(20115, magic_user);
ASSIGNMOB(20121, magic_user);
ASSIGNMOB(20122, magic_user);
ASSIGNMOB(20123, magic_user);
ASSIGNMOB(20124, magic_user);
ASSIGNMOB(20125, magic_user);
ASSIGNMOB(20105, guild_guard);
ASSIGNMOB(20108, guild_guard);

/* castle ctlulac */
ASSIGNMOB(7515, magic_user);
ASSIGNMOB(7509, magic_user);
ASSIGNMOB(7516, magic_user);
ASSIGNMOB(7517, magic_user);

/* Tundra Mob Assigns */
ASSIGNMOB(20706, ice_devil);
ASSIGNMOB(20707, ice_giant);
ASSIGNMOB(20708, ice_wyrm);
ASSIGNMOB(20709, mammoth);
ASSIGNMOB(20713, judge_penguin);
ASSIGNMOB(20719, outcast_fig);
ASSIGNMOB(20720, yeti);
ASSIGNMOB(20721, ice_guard);

/* Special Mobs */
ASSIGNMOB(1220, metaphysician);
ASSIGNMOB(1221, barker);
ASSIGNMOB(1222, dealer);
ASSIGNMOB(1223, poker);
}



/* assign special procedures to objects */
void assign_objects(void)
{
  SPECIAL(bank);
  SPECIAL(gen_board);
  SPECIAL(marbles);

  ASSIGNOBJ(3096, gen_board);	/* social board */
  ASSIGNOBJ(3097, gen_board);	/* freeze board */
  ASSIGNOBJ(3098, gen_board);	/* immortal board */
  ASSIGNOBJ(3099, gen_board);	/* mortal board */
  ASSIGNOBJ(3087, gen_board);   /* quest board */
  ASSIGNOBJ(3088, gen_board);   /* builders board */
  ASSIGNOBJ(3089, gen_board);   /* relations board */
  ASSIGNOBJ(3090, gen_board);   /* clan leader board */
  ASSIGNOBJ(3091, gen_board);   /* very holy board */

  ASSIGNOBJ(3034, bank);	/* atm */
  ASSIGNOBJ(3036, bank);	/* cashcard */
/*  ASSIGNOBJ(6612, bank); */
  ASSIGNOBJ(6647, marbles);
  ASSIGNOBJ(6709, marbles);
}



/* assign special procedures to rooms */
void assign_rooms(void)
{
  extern int dts_are_dumps;
  int i;

  SPECIAL(dump);
  SPECIAL(pet_shops);
  SPECIAL(pray_for_items);
  SPECIAL(hospital);
  SPECIAL(extratempdam);
  /*SPECIAL(thin_ice);*/

  ASSIGNROOM(3030, dump);
  ASSIGNROOM(3068, hospital);
  ASSIGNROOM(3008, extratempdam);
  ASSIGNROOM(3031, pet_shops);
  /*ASSIGNROOM(9001, thin_ice);
  ASSIGNROOM(9002, thin_ice);
  ASSIGNROOM(9003, thin_ice);
  ASSIGNROOM(9004, thin_ice);
  ASSIGNROOM(9007, thin_ice);
  ASSIGNROOM(9013, thin_ice);
  ASSIGNROOM(9017, thin_ice);
  ASSIGNROOM(9023, thin_ice);
  ASSIGNROOM(9024, thin_ice);
  ASSIGNROOM(9025, thin_ice);
  ASSIGNROOM(9026, thin_ice);
  ASSIGNROOM(9027, thin_ice);
  ASSIGNROOM(9028, thin_ice);
  ASSIGNROOM(9029, thin_ice);
  ASSIGNROOM(9035, thin_ice);
  ASSIGNROOM(9037, thin_ice);
  ASSIGNROOM(9042, thin_ice);
  ASSIGNROOM(9043, thin_ice);
  ASSIGNROOM(9044, thin_ice); */

  if (dts_are_dumps)
    for (i = 0; i < top_of_world; i++)
      if (IS_SET(ROOM_FLAGS(i), ROOM_DEATH))
	world[i].func = dump;
}

