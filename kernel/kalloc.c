// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

#define PGPERCPU ((PHYSTOP - KERNBASE) / NCPU)
void freerange(void *pa_start, void *pa_end, int cpu_id);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

#define min(a, b) ((a) < (b) ? (a) : (b))

void kinit() {
  for (int id = 0; id < NCPU; ++id) {
    initlock(&kmem[id].lock, "kmem");
    freerange(end + PGPERCPU * id,
              min((uint64)(end + PGPERCPU * (id + 1)), PHYSTOP), id);
  }
}

void kfree_wrapper(void *pa, int cpu_id) {
  struct run *r;

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP) {
    panic("kfree");
  }

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);
  r = (struct run *)pa;
  acquire(&kmem[cpu_id].lock);
  r->next = kmem[cpu_id].freelist;
  kmem[cpu_id].freelist = r;
  release(&kmem[cpu_id].lock);
}

void freerange(void *pa_start, void *pa_end, int cpu_id) {
  char *p;
  p = (char *)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char *)pa_end; p += PGSIZE)
    kfree_wrapper(p, cpu_id);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void *pa) {
  push_off();
  int id = cpuid();
  kfree_wrapper(pa, id);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *kalloc(void) {
  struct run *r;

  push_off();
  int id = cpuid();
  acquire(&kmem[id].lock);
  r = kmem[id].freelist;
  if (r)
    kmem[id].freelist = r->next;
  release(&kmem[id].lock);

  if (r) {
    memset((char *)r, 5, PGSIZE); // fill with junk
    pop_off();
    return (void *)r;
  }

  for (int i = 0; i < NCPU; ++i) {
    if (i == id)
      continue;
    acquire(&kmem[i].lock);
    r = kmem[i].freelist;
    if (r) {
      kmem[i].freelist = r->next;
      release(&kmem[i].lock);
      break;
    }
    release(&kmem[i].lock);
  }

  pop_off();
  return (void *)r;
}
