/** 
 * @file user/progs/memory_management_test.c
 * @author X.D. Zhai (xingdaz)
 * @brief Wrapper test new_pages and remove_pages 
 */

#include <syscall.h>
#include <simics.h>

int 
main(int argc, char *argv[])
{
  if (new_pages(0x0, 10) < 0)
    lprintf("Can't allocate new pages\n");
  if (remove_pages(0x0) < 0)
    lprintf("Can't remove pages\n");
}
