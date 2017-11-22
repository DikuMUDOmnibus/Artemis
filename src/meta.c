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
#define ANTI_FULL_COST 10000000
#define ANTI_THIRST_COST 5000000
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
   send_to_char("This will cost 10,000,000 experience points.\n\r",ch);
    send_to_char(" 4 - Freedom from hunger\n\r",ch);
   send_to_char("This will cost 5,000,000 experience points.\n\r",ch);
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
       case  1: k=ch->points.max_hit;
                ch->points.max_hit+=1+
                  (k<300)+(k<1000)+(k<3000)+(k<10000)+(k<30000)+(k<100000);
               break;
       case  2: k=ch->points.max_mana;
                ch->points.max_mana+=1+
                  (k<300)+(k<1000)+(k<3000)+(k<10000)+(k<30000)+(k<100000);
                break;
       case  3: k=ch->points.max_move;
                ch->points.max_move+=1+
                  (k<300)+(k<1000)+(k<3000)+(k<10000)+(k<30000)+(k<100000);
                break;
       case  4: GET_EXP(ch)-=ANTI_FULL_COST;
                GET_COND(ch,FULL) = -1; 
                break;
       case  5: GET_EXP(ch)-=ANTI_THIRST_COST;
                GET_COND(ch,THIRST) = -1; 
                break;
      }
    }
    sprintf(buf,"$n tells you '%s - %d times.'",metasign[opt-1],i);
    act(buf,TRUE,meta,0,ch,TO_VICT);
    return(TRUE);
  }
  return(FALSE);
}

