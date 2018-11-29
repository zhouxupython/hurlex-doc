/*
 * =====================================================================================
 *
 *       Filename:  pmm.c
 *
 *    Description:  页内存管理
 *
 *        Version:  1.0
 *        Created:  2013年11月16日 12时00分35秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Hurley (LiuHuan), liuhuan1992@gmail.com
 *        Company:  Class 1107 of Computer Science and Technology
 *
 * =====================================================================================
 */

#include "multiboot.h"
#include "common.h"
#include "debug.h"
#include "pmm.h"

// 物理内存页面管理的栈
static uint32_t pmm_stack[PAGE_MAX_SIZE+1];

// 物理内存管理的栈指针
static uint32_t pmm_stack_top;

// 物理内存页的数量
uint32_t phy_page_count;

//void show_memory_map()
//{
//	uint32_t mmap_addr = glb_mboot_ptr->mmap_addr;
//	uint32_t mmap_length = glb_mboot_ptr->mmap_length;
//
//	printk("mmap_length: %d\n", mmap_length);
//
//	printk("Memory map:\n");
//
////	mmap_entry_t *mmap = (mmap_entry_t *)mmap_addr;
////	for (mmap = (mmap_entry_t *)mmap_addr; (uint32_t)mmap < mmap_addr + mmap_length; mmap++) {
////		printk("base_addr = 0x%X%08X, length = 0x%X%08X, type = 0x%X\n",
////			(uint32_t)mmap->base_addr_high, (uint32_t)mmap->base_addr_low,
////			(uint32_t)mmap->length_high, (uint32_t)mmap->length_low,
////			(uint32_t)mmap->type);
////	}
//
//	mmap_entry_t *mmap = (mmap_entry_t *)mmap_addr;
//	int i;
//	for (i = 0; i < mmap_length; i++) {
//		printk("base_addr = 0x%X%08X, length = 0x%X%08X, type = 0x%X\n",
//			(uint32_t)mmap->base_addr_high, (uint32_t)mmap->base_addr_low,
//			(uint32_t)mmap->length_high, (uint32_t)mmap->length_low,
//			(uint32_t)mmap->type);
//
//		mmap++;// mmap is a ptr of type mmap_entry_t, mmap++ means ptr pointe to next mmap_entry_t
//	}
//}


void show_memory_map()
{
	uint32_t mmap_addr = glb_mboot_ptr->mmap_addr;
	uint32_t mmap_entry_num = glb_mboot_ptr->mmap_length / sizeof(mmap_entry_t);

	printk("mmap_entry_num: %d\n", mmap_entry_num);

	printk("Memory map:\n");

	mmap_entry_t *mmap_entry = (mmap_entry_t *)mmap_addr;
	int i;
	for (i = 0; i < mmap_entry_num; i++) {
		printk("base_addr = 0x%X%08X, length = 0x%X%08X, type = 0x%X\n",
			(uint32_t)mmap_entry->base_addr_high, (uint32_t)mmap_entry->base_addr_low,
			(uint32_t)mmap_entry->length_high, (uint32_t)mmap_entry->length_low,
			(uint32_t)mmap_entry->type);

		mmap_entry++;// mmap is a ptr of type mmap_entry_t, mmap++ means ptr pointe to next mmap_entry_t
	}
}


//void init_pmm()
//{
//	mmap_entry_t *mmap_start_addr = (mmap_entry_t *)glb_mboot_ptr->mmap_addr;
//	mmap_entry_t *mmap_end_addr = (mmap_entry_t *)glb_mboot_ptr->mmap_addr + glb_mboot_ptr->mmap_length;
//
//	mmap_entry_t *map_entry;
//
//	for (map_entry = mmap_start_addr; map_entry < mmap_end_addr; map_entry++) {
//
//		// 如果是可用内存 ( 按照协议，1 表示可用内存，其它数字指保留区域 )
//		if (map_entry->type == 1 && map_entry->base_addr_low == 0x100000) {
//
//			// 把内核结束位置到结束位置的内存段，按页存储到页管理栈里
//			// 最多支持512MB的物理内存
//			uint32_t page_addr = map_entry->base_addr_low + (uint32_t)(kern_end - kern_start);
//			uint32_t length = map_entry->base_addr_low + map_entry->length_low;
//
//			while (page_addr < length && page_addr <= PMM_MAX_SIZE) {
//				pmm_free_page(page_addr);
//				page_addr += PMM_PAGE_SIZE;
//				phy_page_count++;
//			}
//		}
//	}
//}


void init_pmm()
{
	uint32_t mmap_addr = glb_mboot_ptr->mmap_addr;
	uint32_t mmap_entry_num = glb_mboot_ptr->mmap_length / sizeof(mmap_entry_t);

	mmap_entry_t *mmap_entry = (mmap_entry_t *)mmap_addr;
	int i;
	for (i = 0; i < mmap_entry_num; i++) {
		// 如果是可用内存 ( 按照协议，1 表示可用内存，其它数字指保留区域 )
		if (mmap_entry->type == 1 && mmap_entry->base_addr_low == 0x100000) {

			// 把内核结束位置到结束位置的内存段，按页存储到页管理栈里
			// 最多支持512MB的物理内存
			uint32_t page_low_start = mmap_entry->base_addr_low + (uint32_t)(kern_end - kern_start);
			uint32_t page_low_end = mmap_entry->base_addr_low + mmap_entry->length_low;

			while (page_low_start < page_low_end && page_low_start <= PMM_MAX_SIZE) {
				pmm_free_page(page_low_start);
				page_low_start += PMM_PAGE_SIZE;
				phy_page_count++;
			}
		}

		mmap_entry++;// mmap is a ptr of type mmap_entry_t, mmap++ means ptr pointe to next mmap_entry_t
	}
}


uint32_t pmm_alloc_page()
{
	assert(pmm_stack_top != 0, "out of memory");

	uint32_t page = pmm_stack[pmm_stack_top--];

	return page;
}

void pmm_free_page(uint32_t p)
{
	assert(pmm_stack_top != PAGE_MAX_SIZE, "out of pmm_stack stack");

	pmm_stack[++pmm_stack_top] = p;
}

