/* pcc.c - a crash extension which show percpu_counter
 *
 * Copyright (C) 2019 
 * Author: Wang Long(w@laoqinren.net)
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "defs.h"      /* From the crash source top-level directory */

void pcc_init(void);    /* constructor function */
void pcc_fini(void);    /* destructor function (optional) */

void cmd_pcc(void);     /* Declare the commands and their help data. */
char *help_pcc[];

static struct command_table_entry command_table[] = {
	{ "pcc", cmd_pcc, help_pcc, 0},          /* One or more commands, */
	{ NULL },                                     /* terminated by NULL, */
};


void __attribute__((constructor))
pcc_init(void) /* Register the command set. */
{
	register_extension(command_table);
}
 
/* 
 *  This function is called if the shared object is unloaded. 
 *  If desired, perform any cleanups here. 
 */
void __attribute__((destructor))
pcc_fini(void) { }


/* 
 *  Arguments are passed to the command functions in the global args[argcnt]
 *  array.  See getopt(3) for info on dash arguments.  Check out defs.h and
 *  other crash commands for usage of the myriad of utility routines available
 *  to accomplish what your task.
 */
void
cmd_pcc(void)
{
	ulong count_ptr, counters_ptr;
	ulong value;
	int c;
	int tmp;
	ulong	*counters_array;

	while ((c = getopt(argcnt, args, "")) != EOF) {
		switch(c)
		{
		default:
			argerrs++;
			break;
		}
	}

	if (argerrs)
		cmd_usage(pc->curcmd, SYNOPSIS);

	if (!args[optind]) {
		error(INFO, "too few arguments\n");
		cmd_usage(pc->curcmd, SYNOPSIS);
	}

	if (args[optind] && args[optind+1]) {
		error(INFO, "too many arguments\n");
		cmd_usage(pc->curcmd, SYNOPSIS);
	}

	if (!IS_A_NUMBER(args[optind])) {
		error(FATAL, "invalid argument: %s\n",
			args[optind]);
	}
	value = stol(args[optind], FAULT_ON_ERROR, NULL);
	if (IS_KVADDR(value)) {
		/* get address of percpu_counter->count */
		readmem(value + MEMBER_OFFSET("percpu_counter", "count"),
			KVADDR, &count_ptr, sizeof(long),
			"percpu_counter->count", FAULT_ON_ERROR);
		fprintf(fp, "TOTAL COUNT:\n");
		fprintf(fp, "    count: %lu\n", count_ptr);


		/* get address of percpu_counter->counters */
		readmem(value + MEMBER_OFFSET("percpu_counter", "counters"),
			KVADDR,
			&counters_ptr, sizeof(int*),
			"percpu_counter->counters", FAULT_ON_ERROR);

		counters_array = (ulong *)GETBUF(kt->cpus * sizeof(ulong));
		for (c = 0; c < kt->cpus; c++) {
			counters_array[c] = counters_ptr
						+ kt->__per_cpu_offset[c];
		}
		fprintf(fp, "PER-CPU ADDRESSES AND COUNT\n");
		for (c = 0; c < kt->cpus; c++) {
			if (hide_offline_cpu(c)) {
				fprintf(fp, "cpu %d is OFFLINE\n", c);
				continue;
			}
			readmem(counters_array[c], KVADDR,
				&tmp, sizeof(int*),"int *", FAULT_ON_ERROR);
			fprintf(fp, "    [%d]: %lx %d\n",
					 c, counters_array[c], tmp);
		}
		FREEBUF(counters_array);
	}else {
		error(FATAL, "invalid argument: %s\n",
			args[optind]);
	}
}

char *help_pcc[] = {
	"pcc",                        /* command name */
	"show percpu_counter",   /* short description */
	"arg ...",                     /* argument synopsis, or " " if none */

	"  This command simply show percpu_counter",
	"\nEXAMPLE",
	"    crash> pcc <address>",
	NULL
};
