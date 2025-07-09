//
// >>>> malloc challenge! <<<<
//
// Your task is to improve utilization and speed of the following malloc
// implementation.
// Initial implementation is the same as the one implemented in simple_malloc.c.
// For the detailed explanation, please refer to simple_malloc.c.

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define BIN_COUNT 10 // 10個のbinを用意

//
// Interfaces to get memory pages from OS
//

void *mmap_from_system(size_t size);
void munmap_to_system(void *ptr, size_t size);

//
// Struct definitions
//

typedef struct my_metadata_t {
  size_t size;
  struct my_metadata_t *next;
} my_metadata_t;

typedef struct my_heap_t {
  my_metadata_t *free_head;
  my_metadata_t dummy;
  // my_metadata_t *bins[BIN_COUNT];  // 各 bin（サイズ別の格納場所）の先頭ポインタ
} my_heap_t;

//
// Static variables (DO NOT ADD ANOTHER STATIC VARIABLES!)
//
my_heap_t my_heap;

//
// Helper functions (feel free to add/remove/edit!)
//

void my_add_to_free_list(my_metadata_t *metadata) {
  assert(!metadata->next);
  metadata->next = my_heap.free_head;
  my_heap.free_head = metadata;
}

void my_remove_from_free_list(my_metadata_t *metadata, my_metadata_t *prev) {
  if (prev) {
    prev->next = metadata->next;
  } else {
    my_heap.free_head = metadata->next;
  }
  metadata->next = NULL;
}

// int get_bin_index(size_t size) {
//   if (size <= 32) return 0;
//   else if (size <= 64) return 1;
//   else if (size <= 128) return 2;
//   else if (size <= 256) return 3;
//   else if (size <= 512) return 4;
//   else if (size <= 1024) return 5;
//   else if (size <= 2048) return 6;
//   else if (size <= 3072) return 7;
//   else if (size <= 4096) return 8;
//   else return 9;
// }

//
// Interfaces of malloc (DO NOT RENAME FOLLOWING FUNCTIONS!)
//

// This is called at the beginning of each challenge.
void my_initialize() {
  my_heap.free_head = &my_heap.dummy;
  my_heap.dummy.size = 0;
  my_heap.dummy.next = NULL;
}

// my_malloc() is called every time an object is allocated.
// |size| is guaranteed to be a multiple of 8 bytes and meets 8 <= |size| <=
// 4000. You are not allowed to use any library functions other than
// mmap_from_system() / munmap_to_system().
void *my_malloc(size_t size) {
  my_metadata_t *metadata = my_heap.free_head;
  my_metadata_t *prev = NULL;
  my_metadata_t *best_fit = NULL;
  my_metadata_t *best_fit_prev = NULL;
  my_metadata_t *worst_fit = NULL;
  my_metadata_t *worst_fit_prev = NULL;
  my_metadata_t *curr = my_heap.free_head;
  size_t min_diff = (size_t)-1; // 最小の差分を記録する変数。初期値は大きく取っておく。
  size_t max_diff = 0; // 最小の差分を記録する変数。初期値は大きく取っておく。

  // First-fit: Find the first free slot the object fits.
  // TODO: Update this logic to Best-fit!
  // while (metadata && metadata->size < size) {
  //   prev = metadata;
  //   metadata = metadata->next;
  // }
  // now, metadata points to the first free slot
  // and prev is the previous entry.



  // if (!metadata) {
  //   // There was no free slot available. We need to request a new memory region
  //   // from the system by calling mmap_from_system().
  //   //
  //   //     | metadata | free slot |
  //   //     ^
  //   //     metadata
  //   //     <---------------------->
  //   //            buffer_size
  //   size_t buffer_size = 4096;
  //   my_metadata_t *metadata = (my_metadata_t *)mmap_from_system(buffer_size);
  //   metadata->size = buffer_size - sizeof(my_metadata_t);
  //   metadata->next = NULL;
  //   // Add the memory region to the free list.
  //   my_add_to_free_list(metadata);
  //   // Now, try my_malloc() again. This should succeed.
  //   return my_malloc(size);
  // }


  // // |ptr| is the beginning of the allocated object.
  // //
  // // ... | metadata | object | ...
  // //     ^          ^
  // //     metadata   ptr
  // void *ptr = metadata + 1;
  // size_t remaining_size = metadata->size - size;
  // // Remove the free slot from the free list.
  // my_remove_from_free_list(metadata, prev);

  // if (remaining_size > sizeof(my_metadata_t)) {
  //   // Shrink the metadata for the allocated object
  //   // to separate the rest of the region corresponding to remaining_size.
  //   // If the remaining_size is not large enough to make a new metadata,
  //   // this code path will not be taken and the region will be managed
  //   // as a part of the allocated object.
  //   metadata->size = size;
  //   // Create a new metadata for the remaining free slot.
  //   //
  //   // ... | metadata | object | metadata | free slot | ...
  //   //     ^          ^        ^
  //   //     metadata   ptr      new_metadata
  //   //                 <------><---------------------->
  //   //                   size       remaining size
  //   my_metadata_t *new_metadata = (my_metadata_t *)((char *)ptr + size);
  //   new_metadata->size = remaining_size - sizeof(my_metadata_t);
  //   new_metadata->next = NULL;
  //   // Add the remaining free slot to the free list.
  //   my_add_to_free_list(new_metadata);

  // ################# Best-fit: Find the smallest free slot that can fit the object. ######################
  while (curr) { //リストの終わりまで見ていく
    if (curr->size >= size) { // 必要なサイズ以上の空き領域がある場合
      size_t diff = curr->size - size; // 見ている領域のサイズと必要なサイズの差分を計算
      if (diff < min_diff) { //
        best_fit = curr;
        best_fit_prev = prev; // best_fit の１つ前の要素。
        /*
        補足:　↑最後まで見ていくからこそ、後でprevを見ると、bestの１つ前ではなくなるので、best_fit のprev を保存しておく必要がある。
        */
        min_diff = diff;
      }
    }
    prev = curr;
    curr = curr->next;
  }

  if (!best_fit) { // best fit が見つからなかった（つまり 必要容量を入れられるメモリがない）場合
    // 4096バイトのメモリをosからもらい、再度 my_malloc() を呼び出す。
    size_t buffer_size = 4096;
    my_metadata_t *metadata = (my_metadata_t *)mmap_from_system(buffer_size);
    metadata->size = buffer_size - sizeof(my_metadata_t); // メタデータのサイズを引いて、実際に使えるサイズを計算
    metadata->next = NULL;
    my_add_to_free_list(metadata); // 確保したメモリを free_list に追加
    return my_malloc(size);
  }

  void *ptr = best_fit + 1;
  size_t remaining_size = best_fit->size - size; // 余っているサイズを計算
  my_remove_from_free_list(best_fit, best_fit_prev); // best_fitブロックは今から使用するので、free_list からは削除

  // 空きに余裕があれば、余りを free_list に追加する処理
  if (remaining_size > sizeof(my_metadata_t)) {
      best_fit->size = size;
      my_metadata_t *new_metadata = (my_metadata_t *)((char *)ptr + size);
      new_metadata->size = remaining_size - sizeof(my_metadata_t);
      new_metadata->next = NULL;
      my_add_to_free_list(new_metadata);
    }

  // //  ################# Worst-fit: Find the largest free slot that can fit the object. ######################
  // while (curr) { //リストの終わりまで見ていく
  //   if (curr->size >= size) { // 必要なサイズ以上の空き領域がある場合
  //     size_t diff = curr->size - size; // 見ている領域のサイズと必要なサイズの差分を計算
  //     if (diff > max_diff) { //
  //       worst_fit = curr;
  //       worst_fit_prev = prev; // worst_fit の１つ前の要素。
  //       max_diff = diff;
  //     }
  //   }
  //   prev = curr;
  //   curr = curr->next;
  // }

  // if (!worst_fit) { // worst fit が見つからなかった（つまり 必要容量を入れられるメモリがない）場合
  //   // 4096バイトのメモリをosからもらい、再度 my_malloc() を呼び出す。
  //   size_t buffer_size = 4096;
  //   my_metadata_t *metadata = (my_metadata_t *)mmap_from_system(buffer_size);
  //   metadata->size = buffer_size - sizeof(my_metadata_t); // メタデータのサイズを引いて、実際に使えるサイズを計算
  //   metadata->next = NULL;
  //   my_add_to_free_list(metadata); // 確保したメモリを free_list に追加
  //   return my_malloc(size);
  // }

  // void *ptr = worst_fit + 1;
  // size_t remaining_size = worst_fit->size - size; // 余っているサイズを計算
  // my_remove_from_free_list(worst_fit, worst_fit_prev); // best_fitブロックは今から使用するので、free_list からは削除

  // // 空きに余裕があれば、余りを free_list に追加する処理
  // if (remaining_size > sizeof(my_metadata_t)) {
  //     worst_fit->size = size;
  //     my_metadata_t *new_metadata = (my_metadata_t *)((char *)ptr + size);
  //     new_metadata->size = remaining_size - sizeof(my_metadata_t);
  //     new_metadata->next = NULL;
  //     my_add_to_free_list(new_metadata);
  //   }

    return ptr;
  }

// This is called every time an object is freed.  You are not allowed to
// use any library functions other than mmap_from_system / munmap_to_system.
void my_free(void *ptr) {
  // Look up the metadata. The metadata is placed just prior to the object.
  //
  // ... | metadata | object | ...
  //     ^          ^
  //     metadata   ptr
  my_metadata_t *metadata = (my_metadata_t *)ptr - 1;
  // Add the free slot to the free list.
  my_add_to_free_list(metadata);
}

// This is called at the end of each challenge.
void my_finalize() {
  // Nothing is here for now.
  // feel free to add something if you want!
}

void test() {
  // Implement here!
  assert(1 == 1); /* 1 is 1. That's always true! (You can remove this.) */
}
