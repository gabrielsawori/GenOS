#include "mem.h"
#include "screen.h"

// ==========================================
// 1. PHYSICAL MEMORY MANAGER (PMM) - BITMAP
// ==========================================

// Kelola 32MB RAM Fisik (32MB / 4KB = 8192 blok)
#define MAX_BLOCKS 8192 

// 1 angka u32 (32 bit) bisa menyimpan status 32 blok memori (0 = Kosong, 1 = Terpakai)
u32 memory_bitmap[MAX_BLOCKS / 32];
u32 max_blocks = 0;
u32 used_blocks = 0;

void pmm_set_block(u32 bit) { memory_bitmap[bit / 32] |= (1 << (bit % 32)); }
void pmm_clear_block(u32 bit) { memory_bitmap[bit / 32] &= ~(1 << (bit % 32)); }
int pmm_test_block(u32 bit) { return memory_bitmap[bit / 32] & (1 << (bit % 32)); }

u32 pmm_find_first_free() {
    for (u32 i = 0; i < max_blocks / 32; i++) {
        if (memory_bitmap[i] != 0xFFFFFFFF) { // Jika tidak penuh dengan angka 1
            for (int j = 0; j < 32; j++) {
                int bit = 1 << j;
                if (!(memory_bitmap[i] & bit)) return i * 32 + j;
            }
        }
    }
    return (u32)-1;
}

void pmm_init(u32 mem_size) {
    max_blocks = mem_size / PMM_BLOCK_SIZE;
    used_blocks = max_blocks; 
    // Mengamankan seluruh RAM di awal (Semua bit jadi 1)
    for (u32 i = 0; i < max_blocks / 32; i++) memory_bitmap[i] = 0xFFFFFFFF;
}

void pmm_init_region(u32 start, u32 size) {
    int align = start / PMM_BLOCK_SIZE;
    int blocks = size / PMM_BLOCK_SIZE;
    for (int i = 0; i < blocks; i++) {
        pmm_clear_block(align + i);
        used_blocks--;
    }
    pmm_set_block(0); // Kunci blok 0 (Alamat NULL) agar tidak pernah dialokasikan
}

void* pmm_alloc_block() {
    if (used_blocks >= max_blocks) return 0; // Memori Fisik Habis
    u32 frame = pmm_find_first_free();
    if (frame == (u32)-1) return 0;
    pmm_set_block(frame);
    used_blocks++;
    return (void*)(frame * PMM_BLOCK_SIZE);
}

void pmm_free_block(void* p) {
    u32 addr = (u32)p;
    u32 frame = addr / PMM_BLOCK_SIZE;
    pmm_clear_block(frame);
    used_blocks--;
}

// ==========================================
// 2. HEAP MEMORY MANAGER (Kmalloc / Kfree)
// ==========================================

typedef struct mem_block {
    u32 size;
    int is_free;
    struct mem_block *next;
} mem_block_t;

mem_block_t *free_list = 0;

void mem_init() {
    // A. Inisialisasi PMM: Kelola 32MB RAM fisik pertama
    pmm_init(32 * 1024 * 1024);
    
    // B. Buka area RAM dari 2MB hingga 32MB agar bisa dipakai
    // (Alamat 0x0 hingga 0x1FFFFF dikunci khusus untuk Kernel dan Memory Mapped I/O)
    pmm_init_region(0x200000, 30 * 1024 * 1024);

    // C. Meminta 25 Blok Fisik (100KB) ke PMM untuk gudang awal kmalloc kita
    u8* heap_start = (u8*)pmm_alloc_block();
    for(int i = 0; i < 24; i++) {
        pmm_alloc_block(); // Paksa PMM memberi blok berurutan
    }

    // D. Siapkan struktur Linked List di atas RAM hardware yang sesungguhnya!
    free_list = (mem_block_t*)heap_start;
    free_list->size = (25 * PMM_BLOCK_SIZE) - sizeof(mem_block_t);
    free_list->is_free = 1;
    free_list->next = 0;
}

void* kmalloc(u32 size) {
    mem_block_t *curr = free_list;
    while (curr != 0) {
        if (curr->is_free && curr->size >= size) {
            if (curr->size > size + sizeof(mem_block_t) + 4) {
                mem_block_t *new_block = (mem_block_t*)((u8*)curr + sizeof(mem_block_t) + size);
                new_block->size = curr->size - size - sizeof(mem_block_t);
                new_block->is_free = 1;
                new_block->next = curr->next;
                curr->size = size;
                curr->next = new_block;
            }
            curr->is_free = 0;
            return (void*)((u8*)curr + sizeof(mem_block_t));
        }
        curr = curr->next;
    }
    return 0; // Error: Kehabisan Memori (Out of Memory)
}

void kfree(void *ptr) {
    if (ptr == 0) return;
    mem_block_t *block = (mem_block_t*)((u8*)ptr - sizeof(mem_block_t));
    block->is_free = 1;

    // FITUR BARU: Coalesce (Penggabungan blok kosong)
    // Mencegah fragmentasi memori agar RAM tidak bocor!
    mem_block_t *curr = free_list;
    while (curr != 0) {
        if (curr->is_free && curr->next != 0 && curr->next->is_free) {
            curr->size += curr->next->size + sizeof(mem_block_t);
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}