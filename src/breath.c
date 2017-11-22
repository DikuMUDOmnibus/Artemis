ASPELL(spell_acid_breath)
{
  int dam;
  struct obj_data *burn;

  assert(victim && ch);
  dam = (level*level)>>1;
  if(mag_savingthrow(victim, SAVING_BREATH) )
    dam >>= 1;
  damage(ch, victim, dam, SPELL_ACID_BREATH);

  /* And now for the damage on inventory */

  if (!mag_savingthrow(victim, SAVING_BREATH) ) {
    for(burn=victim->carrying ; burn ; burn=burn->next_content){
      if(number(1,7) == 3){
        if((burn->obj_flags.type_flag==ITEM_CONTAINER)&&
           (IS_SET(burn->obj_flags.extra_flags,ITEM_SFX)))
          return;
        else {
          act("$o melts away to nothing",0,victim,burn,0,TO_CHAR);
          extract_obj(burn);
          return;
        }
      }
    }
  }
}

ASPELL(spell_fire_breath)
{
  int dam;
  struct obj_data *burn;

  assert(victim && ch);
  dam = (level*level)>>1;
  if(mag_savingthrow(victim, SAVING_BREATH) )
    dam >>= 1;
  damage(ch, victim, dam, SPELL_FIRE_BREATH);

  /* And now for the damage on inventory */

  if (!mag_savingthrow(victim, SAVING_BREATH) ) {
    for(burn=victim->carrying ; burn ; burn=burn->next_content){
      if(number(1,7) == 3){
        if((burn->obj_flags.type_flag==ITEM_CONTAINER)&&
           (IS_SET(burn->obj_flags.extra_flags,ITEM_SFX)))
          return;
        else {
          act("$o burns to charred remains",0,victim,burn,0,TO_CHAR);
          extract_obj(burn);
          return;
        }
      }
    }
  }
}


ASPELL(spell_frost_breath)
{
  int dam;
  struct obj_data *frozen;

  assert(victim && ch);

  dam = (level*level)>>2;
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
    }
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

ASPELL(spell_gas_breath)
{
  int dam;

  assert(victim && ch);

  dam = level*level;
  if ( mag_savingthrow(victim, SAVING_BREATH) )
    dam >>= 1;
  damage(ch, victim, dam, SPELL_GAS_BREATH);

}

ASPELL(spell_lightning_breath)
{
  int dam;

  assert(victim && ch);

  dam = (level*level)>>2;

  if ( mag_savingthrow(victim, SAVING_BREATH) )
    dam >>= 1;
  damage(ch, victim, dam, SPELL_LIGHTNING_BREATH);
}

