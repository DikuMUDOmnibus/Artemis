void do_flick(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  struct obj_data *obj;
  char victim_name[240];
  char obj_name[240];
  int eq_pos;

  argument = one_argument(argument, obj_name);
  one_argument(argument, victim_name);
  if (!(victim = get_char_vis(ch, victim_name))) {
    send_to_char("Who?\n\r", ch);
    return;
  } else if (victim == ch) {
    send_to_char("Odd?\n\r", ch);
    return;
  } else if(GET_LEVEL(ch) <= GET_LEVEL(victim)){
    send_to_char("Bad idea.\n\r",ch);
    return;
  }
  if (!(obj = get_obj_in_list(obj_name, victim->carrying))) {
    for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
      if(victim->equipment[eq_pos] &&
        (isname(obj_name, victim->equipment[eq_pos]->name))){
        obj = victim->equipment[eq_pos];
        break;
      }
    if (!obj) {
      send_to_char("Can't find that item.\n\r",ch);
      return;
    } else { /* It is equipment */
      obj_to_char(unequip_char(victim, eq_pos), ch);
      send_to_char("Done.\n\r", ch);
    }
  } else {  /* obj found in inventory */
    obj_from_char(obj);
    obj_to_char(obj, ch);
    send_to_char("Done.\n\r", ch);
  }
}
