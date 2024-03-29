###########################################################################
# This is the include file for the make file.
###########################################################################
# You should have to edit only this file to get things to build.
#

###########################################################################
# Tab stops
###########################################################################
# If you use tabstops set to something other than the international
# standard of eight characters, this is your opportunity to inform
# our print scripts.
TABSTOP = 4

###########################################################################
# The method for acquiring project updates.
###########################################################################
# This should be "afs" for any Andrew machine, "web" for non-andrew machines
# and "offline" for machines with no network access.
#
# "offline" is strongly not recommended as you may miss important project
# updates.
#
UPDATE_METHOD = afs

###########################################################################
# WARNING: Do not put extraneous test programs into the REQPROGS variables.
#          Doing so will put your grader in a bad mood which is likely to
#          make him or her less forgiving for other issues.
###########################################################################

###########################################################################
# Mandatory programs whose source is provided by course staff
###########################################################################
# A list of the programs in 410user/progs which are provided in source
# form and NECESSARY FOR THE KERNEL TO RUN
#
# The idle process is a really good thing to keep here.
#
410REQPROGS = idle

###########################################################################
# Mandatory programs provided in binary form by course staff
###########################################################################
# A list of the programs in 410user/progs which are provided in binary
# form and NECESSARY FOR THE KERNEL TO RUN
#
# You may move these to 410BINPROGS to test your syscall stubs once
# they exist, but we will grade you against the binary versions.
# This should be fine.
#
410REQBINPROGS = shell init

###########################################################################
# WARNING: When we test your code, the two TESTS variables below will be
# ignored.  Your kernel MUST RUN WITHOUT THEM.
###########################################################################

###########################################################################
# Test programs provided by course staff you wish to run
###########################################################################
# A list of the test programs you want compiled in from the 410user/progs
# directory
#
410TESTS = actual_wait \
					 agility_drill \
					 beady_test \
					 bistromath \
					 cat \
					 cvar_test \
					 cyclone \
					 excellent \
					 getpid_test1 \
					 halt_test \
					 join_specific_test \
					 juggle \
					 largetest \
					 mandelbrot \
					 misbehave_wrap \
					 multitest \
					 mutex_destroy_test \
					 nibbles \
					 paraguay \
					 racer \
					 rwlock_downgrade_read_test \
					 stack_test1 \
					 startle \
					 switzerland \
					 thr_exit_join \
					 wild_test1 

##########################################################################d#
# Test programs you have written which you wish to run
###########################################################################
# A list of the test programs you want compiled in from the user/progs
# directory
#
STUDENTTESTS = virgin life_cycle_test thread_management_test \
							 memory_management_test console_IO_test misc_test

###########################################################################
# Object files for your thread library
###########################################################################
THREAD_OBJS = malloc.o panic.o mutex.o asm.o cvar.o list.o thread.o \
							swexn_handler.o rwlock.o sem.o

# Thread Group Library Support.
#
# Since libthrgrp.a depends on your thread library, the "buildable blank
# P2" we give you can't build libthrgrp.a.  Once you set up your thread
# library and fix THREAD_OBJS above, uncomment this line to enable building
# libthrgrp.a:
410USER_LIBS_EARLY += libthrgrp.a

###########################################################################
# Object files for your syscall wrappers
###########################################################################
SYSCALL_OBJS = syscall.o fork.o exec.o set_status.o vanish.o wait.o \
							 task_vanish.o gettid.o yield.o deschedule.o make_runnable.o \
							 get_ticks.o sleep.o swexn.o \
							 new_pages.o remove_pages.o \
							 getchar.o readline.o print.o set_term_color.o set_cursor_pos.o \
							 get_cursor_pos.o \
							 readfile.o halt.o

###########################################################################
# Object files for your automatic stack handling
###########################################################################
AUTOSTACK_OBJS = autostack.o
