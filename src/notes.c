/*
 * Support for noting information about a given player.
 * 7 July 1996 Stuart Lamble (aka Danae)
 */

#include "structs.h"
#include "interpreter.h"

ACMD(do_note) {
/* struct char_data *ch, char *argument, int cmd, int subcmd */
  struct char_data *vict;

  half_chop(argument, arg, buf);

  if (!*arg)
    send_to_char("Whom do you which to make a note about?\r\n", ch);
  else if (!*buf)
    send_to_char("Ok, fine, sure, but WHAT??\r\n", ch);
  else {
    if ((vict = get_char_vis(ch, arg)) == NULL)
      send_to_char("They don't seem to be around. Sorry.\r\n", ch);
    else if (GET_LEVEL(vict) >= GET_LEVEL(ch))
      send_to_char("Are you crazy??\r\n", ch);
    else {
      vict->player_specials->num_notes++;
      vict->player_specials->notes = realloc(vict->player_specials->notes,
		sizeof(char *) * num_notes);
      vict->player_specials->notes[num_notes - 1] =
		malloc(strlen(buf) + 1);
      strcpy(vict->player_specials->notes[num_notes - 1], buf);
      send_to_char("Done.\r\n", ch);
    }
  }
}

ACMD(do_unnote) {
  send_to_char("Sorry, not yet written. Annoy Danae. (:-)\r\n", ch);
}
