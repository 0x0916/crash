/* echo.c - simple example of a crash extension
 *
 * Copyright (C) 2001, 2002 Mission Critical Linux, Inc.
 * Copyright (C) 2002-2005, 2007, 2013 David Anderson
 * Copyright (C) 2002-2005, 2007, 2013 Red Hat, Inc. All rights reserved.
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

#define FS_REQUIRES_DEV         1 
#define FS_BINARY_MOUNTDATA     2
#define FS_HAS_SUBTYPE          4
#define FS_USERNS_MOUNT         8       /* Can be mounted by userns root */
#define FS_HAS_RM_XQUOTA        256     /* KABI: fs has the rm_xquota quota op */
#define FS_HAS_INVALIDATE_RANGE 512     /* FS has new ->invalidatepage with length arg */
#define FS_HAS_DIO_IODONE2      1024    /* KABI: fs supports new iodone */
#define FS_HAS_NEXTDQBLK        2048    /* KABI: fs has the ->get_nextdqblk op */
#define FS_HAS_DOPS_WRAPPER     4096    /* kabi: fs is using dentry_operations_wrapper. sb->s_d_op points to dentry_operations_wrapper */
#define FS_RENAME_DOES_D_MOVE   32768   /* FS will handle d_move() during rename() internally. */
#define FS_HAS_FO_EXTEND        65536   /* fs is using the file_operations_extend struture */
#define FS_HAS_WBLIST           131072  /* KABI: fs has writeback list super ops */

/*
 * struct declaration
 */
struct file_system_type_offset_table {
	long file_system_type_fs_flags;
	long file_system_type_next;
	long file_system_type_fs_supers;
};

static struct file_system_type_offset_table file_system_type_offset_table = { 0 };

#define FILE_SYSTEM_TYPE_OFFSET_INIT(X, Y, Z) \
	(file_system_type_offset_table.X=MEMBER_OFFSET(Y,Z))
#define FILE_SYSTEM_TYPE_OFFSET(X) \
	(OFFSET_verify(file_system_type_offset_table.X, (char *)__FUNCTION__, __FILE__, __LINE__, #X))


void filesystemtype_init(void);    /* constructor function */
void filesystemtype_fini(void);    /* destructor function (optional) */

void cmd_filesystemtype(void);     /* Declare the commands and their help data. */
char *help_filesystemtype[];

static struct command_table_entry command_table[] = {
        { "filesystemtype", cmd_filesystemtype, help_filesystemtype, 0},          /* One or more commands, */
        { NULL },                                     /* terminated by NULL, */
};


void __attribute__((constructor))
filesystemtype_init(void) /* Register the command set. */
{ 
	FILE_SYSTEM_TYPE_OFFSET_INIT(file_system_type_fs_flags,
			"file_system_type", "fs_flags");
	FILE_SYSTEM_TYPE_OFFSET_INIT(file_system_type_next,
			"file_system_type", "next");
	FILE_SYSTEM_TYPE_OFFSET_INIT(file_system_type_fs_supers,
			"file_system_type", "fs_supers");
        register_extension(command_table);
}
 
/* 
 *  This function is called if the shared object is unloaded. 
 *  If desired, perform any cleanups here. 
 */
void __attribute__((destructor))
filesystemtype_fini(void) { }

void
show_system_file_type()
{
	char buf1[BUFSIZE];
	char buf2[BUFSIZE];
	char buf3[BUFSIZE];
	char *outputbuffer;
	int bufferindex;
	int others;
	unsigned long file_systems_addr = symbol_value("file_systems");
	unsigned long file_system_addr = file_systems_addr;
	unsigned long name;
	unsigned int fs_flags;
	char file_system_type_header[BUFSIZE] = {0};
	sprintf(file_system_type_header, "%s %s %s\n",
			mkstring(buf1, VADDR_PRLEN, CENTER, "FILE_SYSTEM_TYPE"),
			mkstring(buf2, VADDR_PRLEN, LJUST, "TYPE"),
			mkstring(buf3, VADDR_PRLEN, LJUST, "FS_FLAGS")
		);

	fprintf(fp, "%s", file_system_type_header);

	readmem(file_systems_addr, KVADDR, &file_system_addr, sizeof(void *),
		"file_systems value", FAULT_ON_ERROR);

	while (file_system_addr) {
		// Read the file_system_type's name
		readmem(file_system_addr + OFFSET(file_system_type_name),
			KVADDR, &name, sizeof(void *),
			"file_system_type name", FAULT_ON_ERROR);
		if (read_string(name, buf1, (BUFSIZE/2)-1)) 
			sprintf(buf2, "%s", buf1);
		else
			sprintf(buf2, "unknown ");
		// Read the file system type's fs_flags
		readmem(file_system_addr + FILE_SYSTEM_TYPE_OFFSET(file_system_type_fs_flags),
			KVADDR, &fs_flags, sizeof(int), "file_system_type fs_flags", FAULT_ON_ERROR);

		others = 0;
		outputbuffer = GETBUF(BUFSIZE);
		bufferindex = 0;
		#define sprintflag(X) sprintf(outputbuffer + bufferindex, X, others++ ? "," : "")
		if (fs_flags & FS_REQUIRES_DEV)
			bufferindex += sprintflag("%sFS_REQUIRES_DEV");
		if (fs_flags & FS_BINARY_MOUNTDATA)
			bufferindex += sprintflag("%sFS_BINARY_MOUNTDATA");
		if (fs_flags & FS_HAS_SUBTYPE)
			bufferindex += sprintflag("%sFS_HAS_SUBTYPE");
		if (fs_flags & FS_USERNS_MOUNT)
			bufferindex += sprintflag("%sFS_USERNS_MOUNT");
		if (fs_flags & FS_HAS_RM_XQUOTA)
			bufferindex += sprintflag("%sFS_HAS_RM_XQUOTA");
		if (fs_flags & FS_HAS_INVALIDATE_RANGE)
			bufferindex += sprintflag("%sFS_HAS_INVALIDATE_RANGE");
		if (fs_flags & FS_HAS_DIO_IODONE2)
			bufferindex += sprintflag("%sFS_HAS_DIO_IODONE2");
		if (fs_flags & FS_HAS_NEXTDQBLK)
			bufferindex += sprintflag("%sFS_HAS_NEXTDQBLK");
		if (fs_flags & FS_HAS_DOPS_WRAPPER)
			bufferindex += sprintflag("%sFS_HAS_DOPS_WRAPPER");
		if (fs_flags & FS_RENAME_DOES_D_MOVE)
			bufferindex += sprintflag("%sFS_RENAME_DOES_D_MOVE");
		if (fs_flags & FS_HAS_FO_EXTEND)
			bufferindex += sprintflag("%sFS_HAS_FO_EXTEND");
		if (fs_flags & FS_HAS_WBLIST)
			bufferindex += sprintflag("%sFS_HAS_WBLIST");

		if (bufferindex == 0)
			fprintf(fp, "%s %s 0x%x\n",
				mkstring(buf1, VADDR_PRLEN, RJUST|LONG_HEX,
					MKSTR(file_system_addr)),
				mkstring(buf3, VADDR_PRLEN, LJUST, buf2),
				fs_flags
				);
		else
			fprintf(fp, "%s %s 0x%x (%s)\n",
				mkstring(buf1, VADDR_PRLEN, RJUST|LONG_HEX,
					MKSTR(file_system_addr)),
				mkstring(buf3, VADDR_PRLEN, LJUST, buf2),
				fs_flags,
				outputbuffer
				);
		FREEBUF(outputbuffer);
		
		// Read the file_system_type next
		readmem(file_system_addr + FILE_SYSTEM_TYPE_OFFSET(file_system_type_next),
			KVADDR, &file_system_addr, sizeof(void *), "file_system_type next", FAULT_ON_ERROR);
	}
}

int super_block_callback(void * super_block, void * arg)
{
//	char buf1[BUFSIZE];

	fprintf(fp, "    %lx\n", (unsigned long)super_block);

//	fprintf(fp, "    %s\n",
//		mkstring(buf1, VADDR_PRLEN, RJUST|LONG_HEX, MKSTR(super_block))
//		);

	return 0;
}

void show_all_super_blocks(unsigned long file_system_addr)
{
	struct list_data list_data, *ld;
	struct datatype_member struct_member, *sm;
	unsigned long fs_supers;
	char offset[50] = "super_block.s_instances";

	sm = &struct_member;
	ld = &list_data;
	BZERO(ld, sizeof(struct list_data));

	readmem(file_system_addr + FILE_SYSTEM_TYPE_OFFSET(file_system_type_fs_supers),
		KVADDR, &fs_supers, sizeof(void *), "file_system_type fs_supers", FAULT_ON_ERROR);

	if (arg_to_datatype(offset, sm, RETURN_ON_ERROR) > 1)
		ld->struct_list_offset = sm->member_offset;

	ld->list_head_offset = sm->member_offset;
	ld->flags |= LIST_CALLBACK;
	ld->start = fs_supers;
	ld->callback_func = super_block_callback;

	if (ld->start != 0) {
		fprintf(fp, "\nSUPERBLOCKS:\n");

		hq_open();
		do_list(ld);
		hq_close();
	}else
		fprintf(fp, "no superblocks for the file_system_type\n");

}
void show_system_type_and_superblock(const char *type)
{
	char buf1[BUFSIZE];
	char buf2[BUFSIZE];
	unsigned long file_systems_addr = symbol_value("file_systems");
	unsigned long file_system_addr = file_systems_addr;
	unsigned long name;
	int found = 0;
	char file_system_type_header[BUFSIZE] = {0};

	readmem(file_systems_addr, KVADDR, &file_system_addr, sizeof(void *),
		"file_systems value", FAULT_ON_ERROR);

	while (file_system_addr) {
		// Read the file_system_type's name
		readmem(file_system_addr + OFFSET(file_system_type_name),
			KVADDR, &name, sizeof(void *),
			"file_system_type name", FAULT_ON_ERROR);
		if (read_string(name, buf1, (BUFSIZE/2)-1)) 
			sprintf(buf2, "%s", buf1);
		else
			sprintf(buf2, "unknown ");

		if (!strcmp(type, buf2)) {
			found = 1;
			break;
		}
		// Read the file_system_type next
		readmem(file_system_addr + FILE_SYSTEM_TYPE_OFFSET(file_system_type_next),
			KVADDR, &file_system_addr, sizeof(void *), "file_system_type next", FAULT_ON_ERROR);
	}
	if (found == 1) {
		sprintf(file_system_type_header, "%s %s\n",
			mkstring(buf1, VADDR_PRLEN, CENTER, "FILE_SYSTEM_TYPE"),
			mkstring(buf2, VADDR_PRLEN, LJUST, "TYPE")
		);
		fprintf(fp, "%s", file_system_type_header);
		fprintf(fp, "%s %s\n",
			mkstring(buf1, VADDR_PRLEN, RJUST|LONG_HEX, MKSTR(file_system_addr)),
			mkstring(buf2, VADDR_PRLEN, LJUST, type)
			);

		show_all_super_blocks(file_system_addr);
	}
	else
		fprintf(fp, "Can not found the %s file_system_type in current system.", type);
}

/* 
 *  Arguments are passed to the command functions in the global args[argcnt]
 *  array.  See getopt(3) for info on dash arguments.  Check out defs.h and
 *  other crash commands for usage of the myriad of utility routines available
 *  to accomplish what your task.
 */
void
cmd_filesystemtype(void)
{
        int c;

        while ((c = getopt(argcnt, args, "s:")) != EOF) {
                switch(c)
                {
		case 's':
			show_system_type_and_superblock(optarg);
			return;;
                default:
                        argerrs++;
                        break;
                }
        }

        if (argerrs)
                cmd_usage(pc->curcmd, SYNOPSIS);

	show_system_file_type();
}


/* 
 *  The optional help data is simply an array of strings in a defined format.
 *  For example, the "help echo" command will use the help_echo[] string
 *  array below to create a help page that looks like this:
 * 
 *    NAME
 *      echo - echoes back its arguments
 *
 *    SYNOPSIS
 *      echo arg ...
 *
 *    DESCRIPTION
 *      This command simply echoes back its arguments.
 *
 *    EXAMPLE
 *      Echo back all command arguments:
 *
 *        crash> echo hello, world
 *        hello, world
 *
 */
 
char *help_filesystemtype[] = {
        "filesystemtype",                        /* command name */
        "echoes back its arguments",   /* short description */
        "arg ...",                     /* argument synopsis, or " " if none */
 
        "  This command simply echoes back its arguments.",
        "\nEXAMPLE",
        "  Echo back all command arguments:\n",
        "    crash> echo hello, world",
        "    hello, world",
        NULL
};
