/** 
 * @file user/progs/misc_test.c
 * @author X.D. Zhai (xingdaz)
 * @brief Wrapper tests for readfile and halt 
 */

#include <syscall.h>

#define BUFSIZE 512

int 
main(int argc, char *argv[])
{
  char buf[BUFSIZE];
  char *filename = "fake_file";
  char *readfile_err = "Error reading file.\n";
  if (readfile(filename, buf, 5, 0) < 0)
    print(sizeof(readfile_err), readfile_err);
  
  halt();
}
