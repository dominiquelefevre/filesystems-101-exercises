#include "solution.h"
#include "ext2.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <linux/types.h>


static char* buffer_size = NULL;
static __le32* si_buffer_size = NULL;
static __le32* di_buffer_size = NULL;

/*
size_t urb = 0;

size_t bs = 0;



struct iovec* ci(size_t length) {
  struct iovec* iov = malloc(sizeof(struct iovec));
  
  iov->iov_base = malloc(length);
  iov->iov_len = length;

  return iov;
}








struct file_type {
  int matrix;
  int inode_nr;
  char naimenovanie[256];
  int file_type1;
  int returner;
};
*/

__attribute__((destructor)) void free_all(void)
{
	if (buffer_size != NULL)
	{
		free(buffer_size);
	}
	
	if(si_buffer_size != NULL)
	{
		free(si_buffer_size);
	}
	if(di_buffer_size != NULL)
	{
		free(di_buffer_size);		
	}
}

static __u32 bites_in_blk;
static __u32 size;
static __u32 off_tab = 0;
int copying_cur_buff(int img, int out, __le32 block_nr)
{
	if (bites_in_blk<size-off_tab)
	{
		__u32 array = bites_in_blk;
	}else
	{
		__u32 array = size-off_tab;
	}
	//__u32 array = bites_in_blk<size-off_tab?bites_in_blk:size-off_tab;
	if(array == 0)
	{
		return 0;
	}
	if(block_nr == 0)
	{
		
		memset(buffer_size, 0, bites_in_blk);
	}
	else{
		int check = pread(img, buffer_size, array, bites_in_blk * block_nr);
		if(check != array)
		{
			return -errno;
		}
	}
	if(write(out, buffer_size, array) != array)
	{
		return -errno;
	}
	off_tab = off_tab + array;
	return 0;
}

int copy_si_buffer_size(int img, int out, __le32 block_nr){
	__u32 off_tab = bites_in_blk * block_nr;
	int array = pread(img, si_buffer_size, bites_in_blk, off_tab);
	if(array < (int)bites_in_blk)
	{
		return -errno;
	}
	for(__u32 i=0; i < (__u32)(bites_in_blk / sizeof(__le32)); i++){
		int next = (block_nr!=0?si_buffer_size[i]:0);
		int back = copying_cur_buff(img, out, next);
		if(back < 0){
			return back;
		}
	}
	return 0;
}

int copy_di_buffer_size(int img, int out, __le32 block_nr){
	__u32 off_tab = bites_in_blk * block_nr;
	int array = pread(img, di_buffer_size, bites_in_blk, off_tab);
	if(array < (int)bites_in_blk)
	{
		return -errno;
	}
	for(__u32 i=0; i < (__u32)(bites_in_blk / sizeof(__le32)); i++){
		int next = (block_nr!=0?di_buffer_size[i]:0);
		int back = copy_si_buffer_size(img, out, next);
		if(back < 0){
			return back;
		}
	}
	return 0;
}

int dump_file(int img, int inode_nr, int out)
{
	struct ext2_super_block  ext2_super_block1;
	
	ssize_t array  = pread(img, &ext2_super_block1, sizeof(ext2_super_block1), 1024);
	if(array < 0){
		return -errno;
	}
	
	bites_in_blk = 1024 << ext2_super_block1.s_log_block_size;
	int block_group_nr = (inode_nr - 1) / ext2_super_block1.s_inodes_per_group;

	struct ext2_group_desc ext2_group_desc1;
	__u32 off_tab = 0; 
	off_tab = bites_in_blk * (ext2_super_block1.s_first_data_block + 1) + block_group_nr * sizeof(ext2_group_desc1);
	array  = pread(img, &ext2_group_desc1, sizeof(ext2_group_desc1), off_tab);
	if(array < 0)
	{
		return -errno;
	}

	struct ext2_inode ext2_inode1;
	int inode_in_group = (inode_nr - 1) % ext2_super_block1.s_inodes_per_group;

	off_tab = bites_in_blk * ext2_group_desc1.bg_inode_table + inode_in_group * ext2_super_block1.s_inode_size;
	array  = pread(img, &in, sizeof(in), off_tab);
	if(array < 0){
		return -errno;
	}
	size = ext2_inode1.i_size;
	buffer_size = (char*)malloc(bites_in_blk);
	int back = 0;

	for(int i = 0; i < 12; i++){
		back = copying_cur_buff(img, out, ext2_inode1.i_block[i]);
		if(back < 0){
			return back;
		}
	}
	single_inderect_block_buf = (__le32*)malloc(bites_in_blk);
	//13th block is single-indirect
	back = copy_si_buffer_size(img, out, ext2_inode1.i_block[12]);
	if(back < 0){
		return back;
	}
	di_buffer_size = (__le32*)malloc(bites_in_blk);
	back = copy_double_indirect_block(img, out, ext2_inode1.i_block[13]);
	if(back < 0)
	{
		return back;
	}
	if(off_tab != size)
	{
		fprintf(stderr, "my_size: %d, file_size: %d", off_tab, size);
		return -1;
	}
	return 0;
}
