        if (wielded)
      dam += dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2));
      if(IS_SET(wielded->obj_flags.extra_flags,ITEM_SFX)){
        ff=(ch->char_specials.fighting != NULL);
        weapon_effects(ch,victim,wielded);
        if(ch->char_specials.fighting){
          if(ch->char_specials.fighting->in_room != ch->in_room) return;
        } else {
          if(ff) return;
      }
    }
