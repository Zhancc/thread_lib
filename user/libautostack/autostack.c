/* If you want to use assembly language instead of C,
 * delete this autostack.c and provide an autostack.S
 * instead.
 */

/* for legacy single threaded program, this should used solely to handle stack growth
 * for multithreaded program, this should handle the stack growth for root thread in a limited way
 */
void
install_autostack(void *stack_high, void *stack_low)
{

}
