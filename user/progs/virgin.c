/** 
 * @file user/progs/virgin.c
 * @author X.D. Zhai (xingdaz)
 * @brief Test set_status(), vanish() and print() 
 */

#include <syscall.h>

int 
main(int argc, char *argv[])
{
  char buf[] = "I don't give a fuck\n";
  print(sizeof(buf), buf);
  set_status(1);
}
