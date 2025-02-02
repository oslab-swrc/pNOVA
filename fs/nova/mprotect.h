// SPDX-License-Identifier: GPL-2.0-only
// Modified by June-hyung Kim
// Major modified : Few bug fix for pNOVA
// To publish the pNOVA, Updated by Hongsu Byun
// SPDX-FileCopyrightText: Copyright (c) 2021 Sogang University
:
/*
 * BRIEF DESCRIPTION
 *
 * Memory protection definitions for the NOVA filesystem.
 *
 * Copyright 2015-2016 Regents of the University of California,
 * UCSD Non-Volatile Systems Lab, Andiry Xu <jix024@cs.ucsd.edu>
 * Copyright 2012-2013 Intel Corporation
 * Copyright 2009-2011 Marco Stornelli <marco.stornelli@gmail.com>
 * Copyright 2003 Sony Corporation
 * Copyright 2003 Matsushita Electric Industrial Co., Ltd.
 * 2003-2004 (c) MontaVista Software, Inc. , Steve Longerbeam
 *
 * This program is free software; you can redistribute it and/or modify it
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __WPROTECT_H
#define __WPROTECT_H

#include <linux/fs.h>
#include "nova_def.h"
#include "super.h"

extern void nova_error_mng(struct super_block *sb, const char *fmt, ...);

static inline int nova_range_check(struct super_block *sb, void *p,
					 unsigned long len)
{
	struct nova_sb_info *sbi = NOVA_SB(sb);
/*
	if (p < sbi->virt_addr ||
			p + len > sbi->virt_addr + sbi->initsize) {
		nova_err(sb, "access pmem out of range: pmem range 0x%lx - 0x%lx, "
				"access range 0x%lx - 0x%lx\n",
				(unsigned long)sbi->virt_addr,
				(unsigned long)(sbi->virt_addr + sbi->initsize),
				(unsigned long)p, (unsigned long)(p + len));
		dump_stack();
		return -EINVAL;
	}
*/
	return 0;
}

extern int nova_writeable(void *vaddr, unsigned long size, int rw);

static inline int nova_is_protected(struct super_block *sb)
{
	struct nova_sb_info *sbi = (struct nova_sb_info *)sb->s_fs_info;

	if (wprotect)
		return wprotect;

	return sbi->s_mount_opt & NOVA_MOUNT_PROTECT;
}

static inline int nova_is_wprotected(struct super_block *sb)
{
	return nova_is_protected(sb);
}

static inline void
__nova_memunlock_range(void *p, unsigned long len)
{
	/*
	 * NOTE: Ideally we should lock all the kernel to be memory safe
	 * and avoid to write in the protected memory,
	 * obviously it's not possible, so we only serialize
	 * the operations at fs level. We can't disable the interrupts
	 * because we could have a deadlock in this path.
	 */
	nova_writeable(p, len, 1);
}

static inline void
__nova_memlock_range(void *p, unsigned long len)
{
	nova_writeable(p, len, 0);
}

static inline void nova_memunlock_range(struct super_block *sb, void *p,
					 unsigned long len)
{
	if (nova_range_check(sb, p, len))
		return;

	if (nova_is_protected(sb))
		__nova_memunlock_range(p, len);
}

static inline void nova_memlock_range(struct super_block *sb, void *p,
				       unsigned long len)
{
	if (nova_is_protected(sb))
		__nova_memlock_range(p, len);
}

static inline void nova_memunlock_super(struct super_block *sb)
{
	struct nova_super_block *ps = nova_get_super(sb);

	if (nova_is_protected(sb))
		__nova_memunlock_range(ps, NOVA_SB_SIZE);
}

static inline void nova_memlock_super(struct super_block *sb)
{
	struct nova_super_block *ps = nova_get_super(sb);

	if (nova_is_protected(sb))
		__nova_memlock_range(ps, NOVA_SB_SIZE);
}

static inline void nova_memunlock_reserved(struct super_block *sb,
					 struct nova_super_block *ps)
{
	struct nova_sb_info *sbi = NOVA_SB(sb);

	if (nova_is_protected(sb))
		__nova_memunlock_range(ps,
			sbi->head_reserved_blocks * NOVA_DEF_BLOCK_SIZE_4K);
}

static inline void nova_memlock_reserved(struct super_block *sb,
				       struct nova_super_block *ps)
{
	struct nova_sb_info *sbi = NOVA_SB(sb);

	if (nova_is_protected(sb))
		__nova_memlock_range(ps,
			sbi->head_reserved_blocks * NOVA_DEF_BLOCK_SIZE_4K);
}

static inline void nova_memunlock_journal(struct super_block *sb)
{
	void *addr = nova_get_block(sb, NOVA_DEF_BLOCK_SIZE_4K * JOURNAL_START);

	if (nova_range_check(sb, addr, NOVA_DEF_BLOCK_SIZE_4K))
		return;

	if (nova_is_protected(sb))
		__nova_memunlock_range(addr, NOVA_DEF_BLOCK_SIZE_4K);
}

static inline void nova_memlock_journal(struct super_block *sb)
{
	void *addr = nova_get_block(sb, NOVA_DEF_BLOCK_SIZE_4K * JOURNAL_START);

	if (nova_is_protected(sb))
		__nova_memlock_range(addr, NOVA_DEF_BLOCK_SIZE_4K);
}

static inline void nova_memunlock_inode(struct super_block *sb,
					 struct nova_inode *pi)
{
	if (nova_range_check(sb, pi, NOVA_INODE_SIZE))
		return;

	if (nova_is_protected(sb))
		__nova_memunlock_range(pi, NOVA_INODE_SIZE);
}

static inline void nova_memlock_inode(struct super_block *sb,
				       struct nova_inode *pi)
{
	/* nova_sync_inode(pi); */
	if (nova_is_protected(sb))
		__nova_memlock_range(pi, NOVA_INODE_SIZE);
}

static inline void nova_memunlock_block(struct super_block *sb, void *bp)
{
	if (nova_range_check(sb, bp, sb->s_blocksize))
		return;

	if (nova_is_protected(sb))
		__nova_memunlock_range(bp, sb->s_blocksize);
}

static inline void nova_memlock_block(struct super_block *sb, void *bp)
{
	if (nova_is_protected(sb))
		__nova_memlock_range(bp, sb->s_blocksize);
}


#endif
