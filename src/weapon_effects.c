void weapon_effects(struct char_data *ch, struct char_data *vict,
 struct obj_data *weapon)
{
  int n;
  extern struct index_data *obj_index;
  extern struct index_data *mob_index;
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

