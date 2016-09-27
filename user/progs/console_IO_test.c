/** 
 * @file user/progs/console_IO_test.c
 * @author X.D. Zhai (xingdaz)
 * @brief Wrapper tests for getchar, readline, print, set_term_color, 
 *        set_cursor_pos and get_cursor_pos.
 */

#include <syscall.h>
#include <simics.h>

#define BUFSIZE 512

int 
main(int argc, char *argv[])
{
  char c;
  char buf[BUFSIZE];
  int color;
  int row, col;
  
  char *set_term_color_err = "Error setting terminal color\n";
  char *readline_err = "Error reading from input stream.\n";
  char *get_cursor_pos_err = "Error getting cursor position\n";
  char *set_cursor_pos_err = "Error setting cursor position\n";

  c = getchar();
  lprintf("Read one character %c\n", c);

  color = FGND_BGRN | BGND_BLUE;
  if (set_term_color(color) < 0)
    print(sizeof(set_term_color_err), set_term_color_err);

  if (readline(BUFSIZE, buf) < 0)
    print(sizeof(readline_err), readline_err);
  else
    print(BUFSIZE, buf);

  if (get_cursor_pos(&row, &col) < 0) 
    print(sizeof(get_cursor_pos_err), get_cursor_pos_err);
  else
    lprintf("Cursor pos row = %d, col = %d\n", row, col);

  if (set_cursor_pos(0, 0) < 0)
    print(sizeof(set_cursor_pos_err), set_cursor_pos_err);

  return 0;
}
