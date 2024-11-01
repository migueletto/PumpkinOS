#undef WIN32
#define USE_DL_PREFIX 1
#define HAVE_MMAP 0
#define MORECORE_CONTIGUOUS 0
#define MORECORE_CANNOT_TRIM 1
#define MORECORE_FAILURE 0
#define DL_DEBUG 1
#define MORECORE heap_morecore
#define MALLOC_FAILURE_ACTION
#define malloc_getpagesize sys_getpagesize()

#include "sys.h"
#include "heap.h"
#include "dlmalloc.h"
#include "debug.h"

#define assert(__e) ((__e) ? (void)0 : heap_assert(h, __FILE__, __LINE__, __FUNCTION__, #__e))

#define INTERNAL_SIZE_T   sys_size_t
#define SIZE_SZ           (sizeof(INTERNAL_SIZE_T))
#define MALLOC_ALIGNMENT  (2 * SIZE_SZ)
#define MALLOC_ALIGN_MASK (MALLOC_ALIGNMENT - 1)
#define CHUNK_SIZE_T      sys_size_t

#define NBINS              96
#define NSMALLBINS         32
#define SMALLBIN_WIDTH      8
#define MIN_LARGE_SIZE    256

#define in_smallbin_range(sz)  ((CHUNK_SIZE_T)(sz) < (CHUNK_SIZE_T)MIN_LARGE_SIZE)
#define smallbin_index(sz)     (((unsigned)(sz)) >> 3)
#define bin_index(sz) ((in_smallbin_range(sz)) ? smallbin_index(sz) : largebin_index(sz))

#define BINMAPSHIFT      5
#define BITSPERMAP       (1U << BINMAPSHIFT)
#define BINMAPSIZE       (NBINS / BITSPERMAP)

#define idx2block(i)     ((i) >> BINMAPSHIFT)
#define idx2bit(i)       ((1U << ((i) & ((1U << BINMAPSHIFT)-1))))

#define mark_bin(m,i)    ((m)->binmap[idx2block(i)] |=  idx2bit(i))
#define unmark_bin(m,i)  ((m)->binmap[idx2block(i)] &= ~(idx2bit(i)))
#define get_binmap(m,i)  ((m)->binmap[idx2block(i)] &   idx2bit(i))

#define bin_at(m, i) ((mbinptr)((char*)&((m)->bins[(i)<<1]) - (SIZE_SZ<<1)))

#define next_bin(b)  ((mbinptr)((char*)(b) + (sizeof(mchunkptr)<<1)))

#define first(b)     ((b)->fd)
#define last(b)      ((b)->bk)

#define unlink(P, BK, FD) {                                            \
  FD = P->fd;                                                          \
  BK = P->bk;                                                          \
  FD->bk = BK;                                                         \
  BK->fd = FD;                                                         \
}

#define M_MXFAST            1
#define DEFAULT_MXFAST     64

#define M_TOP_PAD              -2
#define DEFAULT_TOP_PAD        (0)

#define DEFAULT_MMAP_MAX       (0)
#define M_MMAP_THRESHOLD      -3
#define DEFAULT_MMAP_THRESHOLD (256 * 1024)

#define M_TRIM_THRESHOLD       -1
#define DEFAULT_TRIM_THRESHOLD (256 * 1024)

#define MORECORE_CONTIGUOUS_BIT  (1U)

#define contiguous(M) (((M)->morecore_properties &  MORECORE_CONTIGUOUS_BIT))
#define noncontiguous(M) (((M)->morecore_properties &  MORECORE_CONTIGUOUS_BIT) == 0)
#define set_contiguous(M) ((M)->morecore_properties |=  MORECORE_CONTIGUOUS_BIT)
#define set_noncontiguous(M) ((M)->morecore_properties &= ~MORECORE_CONTIGUOUS_BIT)

#define ANYCHUNKS_BIT        (1U)

#define have_anychunks(M)     (((M)->max_fast &  ANYCHUNKS_BIT))
#define set_anychunks(M)      ((M)->max_fast |=  ANYCHUNKS_BIT)
#define clear_anychunks(M)    ((M)->max_fast &= ~ANYCHUNKS_BIT)

#define FASTCHUNKS_BIT        (2U)

#define have_fastchunks(M)   (((M)->max_fast &  FASTCHUNKS_BIT))
#define set_fastchunks(M)    ((M)->max_fast |=  (FASTCHUNKS_BIT|ANYCHUNKS_BIT))
#define clear_fastchunks(M)  ((M)->max_fast &= ~(FASTCHUNKS_BIT))

#define set_max_fast(M, s) \
  (M)->max_fast = (((s) == 0)? SMALLBIN_WIDTH: request2size(s)) | \
  ((M)->max_fast &  (FASTCHUNKS_BIT|ANYCHUNKS_BIT))

#define get_max_fast(M) \
  ((M)->max_fast & ~(FASTCHUNKS_BIT | ANYCHUNKS_BIT))

#define set_max_fast(M, s) \
  (M)->max_fast = (((s) == 0)? SMALLBIN_WIDTH: request2size(s)) | \
  ((M)->max_fast &  (FASTCHUNKS_BIT|ANYCHUNKS_BIT))

#define get_max_fast(M) ((M)->max_fast & ~(FASTCHUNKS_BIT | ANYCHUNKS_BIT))

#define FIRST_SORTED_BIN_SIZE MIN_LARGE_SIZE

#define unsorted_chunks(M)          (bin_at(M, 1))
#define initial_top(M)              (unsorted_chunks(M))

#define PTR_UINT sys_size_t

#define get_malloc_state(h) ((struct malloc_state *)dlmalloc_get_state(h))

struct mallinfo {
  int arena;
  int ordblks;
  int smblks;
  int hblks;
  int hblkhd;
  int usmblks;
  int fsmblks;
  int uordblks;
  int fordblks;
  int keepcost;
};

void *dlmemalign(void *h, sys_size_t, sys_size_t);
void *dlvalloc(void *, sys_size_t);
int dlmallopt(void *h, int, int);
struct mallinfo dlmallinfo(void *h);
void **dlindependent_calloc(void *, sys_size_t, sys_size_t, void **);
void **dlindependent_comalloc(void *, sys_size_t, sys_size_t*, void **);
void *dlpvalloc(void *, sys_size_t);
void dlcfree(void *, void *);
int dlmalloc_trim(void *h, sys_size_t);
sys_size_t dlmalloc_usable_size(void*);
void dlmalloc_stats(void);

struct malloc_chunk {
  sys_size_t prev_size;
  sys_size_t size;
  struct malloc_chunk *fd;
  struct malloc_chunk *bk;
};

#define chunk2mem(p)   ((void*)((char*)(p) + 2*SIZE_SZ))
#define mem2chunk(mem) ((mchunkptr)((char*)(mem) - 2*SIZE_SZ))

/* The smallest possible chunk */
#define MIN_CHUNK_SIZE        (sizeof(struct malloc_chunk))

/* The smallest size we can malloc is an aligned minimal chunk */

#define MINSIZE  \
  (CHUNK_SIZE_T)(((MIN_CHUNK_SIZE+MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK))

/* Check if m has acceptable alignment */

#define aligned_OK(m)  (((PTR_UINT)((m)) & (MALLOC_ALIGN_MASK)) == 0)


/* 
   Check if a request is so large that it would wrap around zero when
   padded and aligned. To simplify some other code, the bound is made
   low enough so that adding MINSIZE will also not wrap around sero.
*/

#define REQUEST_OUT_OF_RANGE(req)                                 \
  ((CHUNK_SIZE_T)(req) >=                                        \
   (CHUNK_SIZE_T)(INTERNAL_SIZE_T)(-2 * MINSIZE))    

/* pad request bytes into a usable size -- internal version */

#define request2size(req)                                         \
  (((req) + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE)  ?             \
   MINSIZE :                                                      \
   ((req) + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)

/*  Same, except also perform argument check */

#define checked_request2size(req, sz)                             \
  if (REQUEST_OUT_OF_RANGE(req)) {                                \
    MALLOC_FAILURE_ACTION;                                        \
    return 0;                                                     \
  }                                                               \
  (sz) = request2size(req);                                              

/*
  --------------- Physical chunk operations ---------------
*/


/* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */
#define PREV_INUSE 0x1

/* extract inuse bit of previous chunk */
#define prev_inuse(p)       ((p)->size & PREV_INUSE)


/* size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap() */
#define IS_MMAPPED 0x2

/* check for mmap()'ed chunk */
#define chunk_is_mmapped(p) ((p)->size & IS_MMAPPED)

/* 
  Bits to mask off when extracting size 

  Note: IS_MMAPPED is intentionally not masked off from size field in
  macros for which mmapped chunks should never be seen. This should
  cause helpful core dumps to occur if it is tried by accident by
  people extending or adapting this malloc.
*/
#define SIZE_BITS (PREV_INUSE|IS_MMAPPED)

/* Get size, ignoring use bits */
#define chunksize(p)         ((p)->size & ~(SIZE_BITS))


/* Ptr to next physical malloc_chunk. */
#define next_chunk(p) ((mchunkptr)( ((char*)(p)) + ((p)->size & ~PREV_INUSE) ))

/* Ptr to previous physical malloc_chunk */
#define prev_chunk(p) ((mchunkptr)( ((char*)(p)) - ((p)->prev_size) ))

/* Treat space at ptr + offset as a chunk */
#define chunk_at_offset(p, s)  ((mchunkptr)(((char*)(p)) + (s)))

/* extract p's inuse bit */
#define inuse(p)\
((((mchunkptr)(((char*)(p))+((p)->size & ~PREV_INUSE)))->size) & PREV_INUSE)

/* set/clear chunk as being inuse without otherwise disturbing */
#define set_inuse(p)\
((mchunkptr)(((char*)(p)) + ((p)->size & ~PREV_INUSE)))->size |= PREV_INUSE

#define clear_inuse(p)\
((mchunkptr)(((char*)(p)) + ((p)->size & ~PREV_INUSE)))->size &= ~(PREV_INUSE)


/* check/set/clear inuse bits in known places */
#define inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size & PREV_INUSE)

#define set_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size |= PREV_INUSE)

#define clear_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size &= ~(PREV_INUSE))


/* Set size at head, without disturbing its use bit */
#define set_head_size(p, s)  ((p)->size = (((p)->size & PREV_INUSE) | (s)))

/* Set size/use field */
#define set_head(p, s)       ((p)->size = (s))

/* Set size at footer (only when chunk is not in use) */
#define set_foot(p, s)       (((mchunkptr)((char*)(p) + (s)))->prev_size = (s))

typedef struct malloc_chunk *mchunkptr;
typedef struct malloc_chunk *mbinptr;

static int largebin_index(unsigned int sz) {
  unsigned int x = sz >> SMALLBIN_WIDTH;
  unsigned int m;

  if (x >= 0x10000) return NBINS-1;

  unsigned int n = ((x - 0x100) >> 16) & 8;
  x <<= n;
  m = ((x - 0x1000) >> 16) & 4;
  n += m;
  x <<= m;
  m = ((x - 0x4000) >> 16) & 2;
  n += m;
  x = (x << m) >> 14;
  m = 13 - n + (x & ~(x>>1));

  return NSMALLBINS + (m << 2) + ((sz >> (m + 6)) & 3);
}

typedef struct malloc_chunk *mfastbinptr;

#define fastbin_index(sz) ((((unsigned int)(sz)) >> 3) - 2)

#define MAX_FAST_SIZE 80

#define NFASTBINS (fastbin_index(request2size(MAX_FAST_SIZE))+1)

#define FASTBIN_CONSOLIDATION_THRESHOLD  ((unsigned long)(DEFAULT_TRIM_THRESHOLD) >> 1)

struct malloc_state {
  sys_size_t max_fast;
  mfastbinptr fastbins[NFASTBINS];
  mchunkptr top;
  mchunkptr last_remainder;
  mchunkptr bins[NBINS * 2];
  unsigned int binmap[BINMAPSIZE+1];
  sys_size_t trim_threshold;
  sys_size_t top_pad;
  sys_size_t mmap_threshold;
  int n_mmaps;
  int n_mmaps_max;
  int max_n_mmaps;
  unsigned int pagesize;
  unsigned int morecore_properties;
  sys_size_t mmapped_mem;
  sys_size_t sbrked_mem;
  sys_size_t max_sbrked_mem;
  sys_size_t max_mmapped_mem;
  sys_size_t max_total_mem;
};

typedef struct malloc_state *mstate;

static void malloc_consolidate(void *h, mstate);
static void **iALLOc(void *h, sys_size_t, sys_size_t *, int, void **);

int dlmalloc_state_size(void) {
  return sizeof(struct malloc_state);
}

static void malloc_init_state(mstate av) {
  int i;
  mbinptr bin;

  for (i = 1; i < NBINS; ++i) {
    bin = bin_at(av,i);
    bin->fd = bin->bk = bin;
  }

  av->top_pad = DEFAULT_TOP_PAD;
  av->n_mmaps_max = DEFAULT_MMAP_MAX;
  av->mmap_threshold = DEFAULT_MMAP_THRESHOLD;
  av->trim_threshold = DEFAULT_TRIM_THRESHOLD;

  set_noncontiguous(av);

  set_max_fast(av, DEFAULT_MXFAST);

  av->top = initial_top(av);
  av->pagesize = sys_getpagesize();
}

static void do_check_chunk(void *h, mchunkptr p) {
  mstate av = get_malloc_state(h);
  sys_size_t sz = chunksize(p);

  char* max_address = (char *)(av->top) + chunksize(av->top);
  char* min_address = max_address - av->sbrked_mem;

  if (!chunk_is_mmapped(p)) {
    if (p != av->top) {
      if (contiguous(av)) {
        assert(((char *)p) >= min_address);
        assert(((char *)p + sz) <= ((char *)(av->top)));
      }
    } else {
      assert((sys_size_t)(sz) >= MINSIZE);
      assert(prev_inuse(p));
    }
  } else {
    assert(!chunk_is_mmapped(p));
  }
}

static void do_check_free_chunk(void *h, mchunkptr p) {
  mstate av = get_malloc_state(h);

  sys_size_t sz = p->size & ~PREV_INUSE;
  mchunkptr next = chunk_at_offset(p, sz);

  do_check_chunk(h, p);

  assert(!inuse(p));
  assert (!chunk_is_mmapped(p));

  if ((sys_size_t)(sz) >= MINSIZE) {
    assert((sz & MALLOC_ALIGN_MASK) == 0);
    assert(aligned_OK(chunk2mem(p)));

    if (next->prev_size != sz) {
      debug(DEBUG_ERROR, "Heap", "do_check_free_chunk p=%p next=%p next->prev_size=%u sz=%u", p, next, (uint32_t)next->prev_size, (uint32_t)sz);
      debug_bytes(DEBUG_ERROR, "Heap", (uint8_t *)p, sz + 16);
    }
    assert(next->prev_size == sz);

    assert(prev_inuse(p));
    assert (next == av->top || inuse(next));

    assert(p->fd != NULL);
    assert(p->bk != NULL);
    assert(p->fd->bk == p);
    assert(p->bk->fd == p);
  } else
    assert(sz == SIZE_SZ);
}

static void do_check_inuse_chunk(void *h, mchunkptr p) {
  mstate av = get_malloc_state(h);
  mchunkptr next;
  do_check_chunk(h, p);

  if (chunk_is_mmapped(p))
    return;

  if (!inuse(p)) {
    next = next_chunk(p);
    debug(DEBUG_ERROR, "Heap", "do_check_inuse_chunk p=%p", p);
    debug_bytes(DEBUG_ERROR, "Heap", (uint8_t *)p, 256);;
    debug(DEBUG_ERROR, "Heap", "do_check_inuse_chunk next=%p", next);
    debug_bytes(DEBUG_ERROR, "Heap", (uint8_t *)next, 16);;
  }
  assert(inuse(p));

  next = next_chunk(p);

  if (!prev_inuse(p)) {
    mchunkptr prv = prev_chunk(p);
    assert(next_chunk(prv) == p);
    do_check_free_chunk(h, prv);
  }

  if (next == av->top) {
    assert(prev_inuse(next));
    assert(chunksize(next) >= MINSIZE);
  } else if (!inuse(next))
    do_check_free_chunk(h, next);
}

static void do_check_remalloced_chunk(void *h, mchunkptr p, sys_size_t s) {
  sys_size_t sz = p->size & ~PREV_INUSE;

  do_check_inuse_chunk(h, p);

  assert((sz & MALLOC_ALIGN_MASK) == 0);
  assert((sys_size_t)(sz) >= MINSIZE);
  assert(aligned_OK(chunk2mem(p)));
  assert((long)(sz) - (long)(s) >= 0);
  assert((long)(sz) - (long)(s + MINSIZE) < 0);
}

static void do_check_malloced_chunk(void *h, mchunkptr p, sys_size_t s) {
  do_check_remalloced_chunk(h, p, s);
  assert(prev_inuse(p));
}

static void do_check_malloc_state(void *h) {
  mstate av = get_malloc_state(h);
  int i;
  mchunkptr p;
  mchunkptr q;
  mbinptr b;
  unsigned int binbit;
  int empty;
  unsigned int idx;
  sys_size_t size;
  sys_size_t total = 0;
  int max_fast_bin;

  assert(sizeof(sys_size_t) <= sizeof(char*));
  assert((MALLOC_ALIGNMENT & (MALLOC_ALIGNMENT-1)) == 0);

  if (av->top == 0 || av->top == initial_top(av))
    return;

  assert((av->pagesize & (av->pagesize-1)) == 0);
  assert(get_max_fast(av) <= request2size(MAX_FAST_SIZE));
  max_fast_bin = fastbin_index(av->max_fast);

  for (i = 0; NFASTBINS-i > 0; ++i) {
    p = av->fastbins[i];

    if (i > max_fast_bin)
      assert(p == 0);

    while (p != 0) {
      do_check_inuse_chunk(h, p);
      total += chunksize(p);

      assert(fastbin_index(chunksize(p)) == i);
      p = p->fd;
    }
  }

  if (total != 0)
    assert(have_fastchunks(av));
  else if (!have_fastchunks(av))
    assert(total == 0);

  for (i = 1; i < NBINS; ++i) {
    b = bin_at(av,i);

    if (i >= 2) {
      binbit = get_binmap(av,i);
      empty = last(b) == b;
      if (!binbit)
        assert(empty);
      else if (!empty)
        assert(binbit);
    }

    for (p = last(b); p != b; p = p->bk) {
      do_check_free_chunk(h, p);
      size = chunksize(p);
      total += size;

      if (i >= 2) {
        idx = bin_index(size);
        assert(idx == i);

        if ((sys_size_t) size >= (sys_size_t)(FIRST_SORTED_BIN_SIZE)) {
          assert(p->bk == b || (sys_size_t)chunksize(p->bk) >= (sys_size_t)chunksize(p));
        }
      }

      for (q = next_chunk(p); (q != av->top && inuse(q) && (sys_size_t)(chunksize(q)) >= MINSIZE); q = next_chunk(q)) {
        do_check_inuse_chunk(h, q);
      }
    }
  }

  do_check_chunk(h, av->top);

  assert(total <= (sys_size_t)(av->max_total_mem));
  assert(av->n_mmaps >= 0);
  assert(av->n_mmaps <= av->max_n_mmaps);
  assert((sys_size_t)(av->sbrked_mem) <= (sys_size_t)(av->max_sbrked_mem));
  assert((sys_size_t)(av->mmapped_mem) <= (sys_size_t)(av->max_mmapped_mem));
  assert((sys_size_t)(av->max_total_mem) >= (sys_size_t)(av->mmapped_mem) + (sys_size_t)(av->sbrked_mem));
}

static void* sYSMALLOc(void *h, sys_size_t nb, mstate av) {
  mchunkptr old_top;
  sys_size_t old_size;
  char* old_end;

  long size;
  char* brk;

  long correction;
  char* snd_brk;

  sys_size_t front_misalign;
  sys_size_t end_misalign;
  char* aligned_brk;

  mchunkptr p;
  mchunkptr remainder;
  sys_size_t remainder_size;
  sys_size_t sum;
  sys_size_t pagemask = av->pagesize - 1;

  if (have_fastchunks(av)) {
    assert(in_smallbin_range(nb));
    malloc_consolidate(h, av);
    return dlmalloc(h, nb - MALLOC_ALIGN_MASK);
  }

  old_top = av->top;
  old_size = chunksize(old_top);
  old_end = (char *)(chunk_at_offset(old_top, old_size));

  brk = snd_brk = (char *)(0);

  assert((old_top == initial_top(av) && old_size == 0) || ((sys_size_t) (old_size) >= MINSIZE && prev_inuse(old_top)));
  assert((sys_size_t)(old_size) < (sys_size_t)(nb + MINSIZE));
  assert(!have_fastchunks(av));
  size = nb + av->top_pad + MINSIZE;

  if (contiguous(av))
    size -= old_size;

  size = (size + pagemask) & ~pagemask;

  if (size > 0)
    brk = (char *)(heap_morecore(h, size));

  if (brk != (char *)(0)) {
    av->sbrked_mem += size;

    if (brk == old_end && snd_brk == (char *)(0)) {
      set_head(old_top, (size + old_size) | PREV_INUSE);
    } else {
      front_misalign = 0;
      end_misalign = 0;
      correction = 0;
      aligned_brk = brk;
      if (contiguous(av) && old_size != 0 && brk < old_end) {
        set_noncontiguous(av);
      }

      if (contiguous(av)) {
        if (old_size != 0)
          av->sbrked_mem += brk - old_end;

        front_misalign = (sys_size_t)chunk2mem(brk) & MALLOC_ALIGN_MASK;
        if (front_misalign > 0) {
          correction = MALLOC_ALIGNMENT - front_misalign;
          aligned_brk += correction;
        }

        correction += old_size;

        end_misalign = (sys_size_t)(brk + size + correction);
        correction += ((end_misalign + pagemask) & ~pagemask) - end_misalign;

        assert(correction >= 0);
        snd_brk = (char *)(heap_morecore(h, correction));

        if (snd_brk == (char *)(0)) {
          correction = 0;
          snd_brk = (char *)(heap_morecore(h, 0));
        } else if (snd_brk < brk) {
          snd_brk = brk + size;
          correction = 0;
          set_noncontiguous(av);
        }
      } else {
        assert(aligned_OK(chunk2mem(brk)));
        if (snd_brk == (char *)(0)) {
          snd_brk = (char *)(heap_morecore(h, 0));
          av->sbrked_mem += snd_brk - brk - size;
        }
      }

      if (snd_brk != (char *)(0)) {
        av->top = (mchunkptr)aligned_brk;
        set_head(av->top, (snd_brk - aligned_brk + correction) | PREV_INUSE);
        av->sbrked_mem += correction;

        if (old_size != 0) {
          old_size = (old_size - 3*SIZE_SZ) & ~MALLOC_ALIGN_MASK;
          set_head(old_top, old_size | PREV_INUSE);
          chunk_at_offset(old_top, old_size )->size = SIZE_SZ|PREV_INUSE;
          chunk_at_offset(old_top, old_size + SIZE_SZ)->size = SIZE_SZ|PREV_INUSE;

          if (old_size >= MINSIZE) {
            sys_size_t tt = av->trim_threshold;
            av->trim_threshold = (sys_size_t)(-1);
            dlfree(h, chunk2mem(old_top));
            av->trim_threshold = tt;
          }
        }
      }
    }

    sum = av->sbrked_mem;
    if (sum > (sys_size_t)(av->max_sbrked_mem))
      av->max_sbrked_mem = sum;

    sum += av->mmapped_mem;
    if (sum > (sys_size_t)(av->max_total_mem))
      av->max_total_mem = sum;

    do_check_malloc_state(h);

    p = av->top;
    size = chunksize(p);

    if ((sys_size_t)(size) >= (sys_size_t)(nb + MINSIZE)) {
      remainder_size = size - nb;
      remainder = chunk_at_offset(p, nb);
      av->top = remainder;
      set_head(p, nb | PREV_INUSE);
      set_head(remainder, remainder_size | PREV_INUSE);
      do_check_malloced_chunk(h, p, nb);
      return chunk2mem(p);
    }
  }

  return 0;
}

void *dlmalloc(void *h, sys_size_t bytes) {
  mstate av = get_malloc_state(h);

  sys_size_t nb;
  unsigned int idx;
  mbinptr bin;
  mfastbinptr* fb;

  mchunkptr victim;
  sys_size_t size;
  int victim_index;

  mchunkptr remainder;
  sys_size_t remainder_size;

  unsigned int block;
  unsigned int bit;
  unsigned int map;

  mchunkptr fwd;
  mchunkptr bck;
  checked_request2size(bytes, nb);

  if (!have_anychunks(av)) {
    if (av->max_fast == 0)
      malloc_consolidate(h, av);
    goto use_top;
  }

  if ((sys_size_t)(nb) <= (sys_size_t)(av->max_fast)) {
    fb = &(av->fastbins[(fastbin_index(nb))]);
    if ( (victim = *fb) != 0) {
      *fb = victim->fd;
      do_check_remalloced_chunk(h, victim, nb);
      return chunk2mem(victim);
    }
  }

  if (in_smallbin_range(nb)) {
    idx = smallbin_index(nb);
    bin = bin_at(av,idx);

    if ( (victim = last(bin)) != bin) {
      bck = victim->bk;
      set_inuse_bit_at_offset(victim, nb);
      bin->bk = bck;
      bck->fd = bin;

      do_check_malloced_chunk(h, victim, nb);
      return chunk2mem(victim);
    }
  } else {
    idx = largebin_index(nb);
    if (have_fastchunks(av))
      malloc_consolidate(h, av);
  }
  while ( (victim = unsorted_chunks(av)->bk) != unsorted_chunks(av)) {
    bck = victim->bk;
    size = chunksize(victim);
    if (in_smallbin_range(nb) &&
        bck == unsorted_chunks(av) &&
        victim == av->last_remainder &&
        (sys_size_t)(size) > (sys_size_t)(nb + MINSIZE)) {

      remainder_size = size - nb;
      remainder = chunk_at_offset(victim, nb);
      unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
      av->last_remainder = remainder;
      remainder->bk = remainder->fd = unsorted_chunks(av);

      set_head(victim, nb | PREV_INUSE);
      set_head(remainder, remainder_size | PREV_INUSE);
      set_foot(remainder, remainder_size);

      do_check_malloced_chunk(h, victim, nb);
      return chunk2mem(victim);
    }

    unsorted_chunks(av)->bk = bck;
    bck->fd = unsorted_chunks(av);

    if (size == nb) {
      set_inuse_bit_at_offset(victim, size);
      do_check_malloced_chunk(h, victim, nb);
      return chunk2mem(victim);
    }

    if (in_smallbin_range(size)) {
      victim_index = smallbin_index(size);
      bck = bin_at(av, victim_index);
      fwd = bck->fd;
    } else {
      victim_index = largebin_index(size);
      bck = bin_at(av, victim_index);
      fwd = bck->fd;

      if (fwd != bck) {

        if ((sys_size_t)(size) < (sys_size_t)(bck->bk->size)) {
          fwd = bck;
          bck = bck->bk;
        } else if ((sys_size_t)(size) >=
                 (sys_size_t)(FIRST_SORTED_BIN_SIZE)) {


          size |= PREV_INUSE;
          while ((sys_size_t)(size) < (sys_size_t)(fwd->size))
            fwd = fwd->fd;
          bck = fwd->bk;
        }
      }
    }

    mark_bin(av, victim_index);
    victim->bk = bck;
    victim->fd = fwd;
    fwd->bk = victim;
    bck->fd = victim;
  }
  if (!in_smallbin_range(nb)) {
    bin = bin_at(av, idx);

    for (victim = last(bin); victim != bin; victim = victim->bk) {
      size = chunksize(victim);

      if ((sys_size_t)(size) >= (sys_size_t)(nb)) {
        remainder_size = size - nb;
        unlink(victim, bck, fwd);

        if (remainder_size < MINSIZE) {
          set_inuse_bit_at_offset(victim, size);
          do_check_malloced_chunk(h, victim, nb);
          return chunk2mem(victim);
        } else {
          remainder = chunk_at_offset(victim, nb);
          unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
          remainder->bk = remainder->fd = unsorted_chunks(av);
          set_head(victim, nb | PREV_INUSE);
          set_head(remainder, remainder_size | PREV_INUSE);
          set_foot(remainder, remainder_size);
          do_check_malloced_chunk(h, victim, nb);
          return chunk2mem(victim);
        }
      }
    }
  }
  ++idx;
  bin = bin_at(av,idx);
  block = idx2block(idx);
  map = av->binmap[block];
  bit = idx2bit(idx);

  for (;;) {
    if (bit > map || bit == 0) {
      do {
        if (++block >= BINMAPSIZE)
          goto use_top;
      } while ( (map = av->binmap[block]) == 0);

      bin = bin_at(av, (block << BINMAPSHIFT));
      bit = 1;
    }

    while ((bit & map) == 0) {
      bin = next_bin(bin);
      bit <<= 1;
      assert(bit != 0);
    }

    victim = last(bin);


    if (victim == bin) {
      av->binmap[block] = map &= ~bit;
      bin = next_bin(bin);
      bit <<= 1;
    } else {
      size = chunksize(victim);
      assert((sys_size_t)(size) >= (sys_size_t)(nb));
      remainder_size = size - nb;

      bck = victim->bk;
      bin->bk = bck;
      bck->fd = bin;


      if (remainder_size < MINSIZE) {
        set_inuse_bit_at_offset(victim, size);
        do_check_malloced_chunk(h, victim, nb);
        return chunk2mem(victim);
      } else {
        remainder = chunk_at_offset(victim, nb);

        unsorted_chunks(av)->bk = unsorted_chunks(av)->fd = remainder;
        remainder->bk = remainder->fd = unsorted_chunks(av);

        if (in_smallbin_range(nb))
          av->last_remainder = remainder;

        set_head(victim, nb | PREV_INUSE);
        set_head(remainder, remainder_size | PREV_INUSE);
        set_foot(remainder, remainder_size);
        do_check_malloced_chunk(h, victim, nb);
        return chunk2mem(victim);
      }
    }
  }

  use_top:
  victim = av->top;
  size = chunksize(victim);

  if ((sys_size_t)(size) >= (sys_size_t)(nb + MINSIZE)) {
    remainder_size = size - nb;
    remainder = chunk_at_offset(victim, nb);
    av->top = remainder;
    set_head(victim, nb | PREV_INUSE);
    set_head(remainder, remainder_size | PREV_INUSE);

    do_check_malloced_chunk(h, victim, nb);
    return chunk2mem(victim);
  }

  return sYSMALLOc(h, nb, av);
}

void dlfree(void *h, void *mem) {
  mstate av = get_malloc_state(h);

  mchunkptr p;
  sys_size_t size;
  mfastbinptr* fb;
  mchunkptr nextchunk;
  sys_size_t nextsize;
  int nextinuse;
  sys_size_t prevsize;
  mchunkptr bck;
  mchunkptr fwd;

  if (mem != 0) {
    p = mem2chunk(mem);
    size = chunksize(p);

    do_check_inuse_chunk(h, p);

    if ((sys_size_t)(size) <= (sys_size_t)(av->max_fast)) {
      set_fastchunks(av);
      fb = &(av->fastbins[fastbin_index(size)]);
      p->fd = *fb;
      *fb = p;
    } else if (!chunk_is_mmapped(p)) {
      set_anychunks(av);
      nextchunk = chunk_at_offset(p, size);
      nextsize = chunksize(nextchunk);

      if (!prev_inuse(p)) {
        prevsize = p->prev_size;
        size += prevsize;
        p = chunk_at_offset(p, -((long) prevsize));
        unlink(p, bck, fwd);
      }

      if (nextchunk != av->top) {
        nextinuse = inuse_bit_at_offset(nextchunk, nextsize);
        set_head(nextchunk, nextsize);

        if (!nextinuse) {
          unlink(nextchunk, bck, fwd);
          size += nextsize;
        }

        bck = unsorted_chunks(av);
        fwd = bck->fd;
        p->bk = bck;
        p->fd = fwd;
        bck->fd = p;
        fwd->bk = p;

        set_head(p, size | PREV_INUSE);
        set_foot(p, size);

        do_check_free_chunk(h, p);
      } else {
        size += nextsize;
        set_head(p, size | PREV_INUSE);
        av->top = p;
        do_check_chunk(h, p);
      }
      if ((sys_size_t)(size) >= FASTBIN_CONSOLIDATION_THRESHOLD) {
        if (have_fastchunks(av))
          malloc_consolidate(h, av);
      }
    }
  }
}

static void malloc_consolidate(void *h, mstate av) {
  mfastbinptr* fb;
  mfastbinptr* maxfb;
  mchunkptr p;
  mchunkptr nextp;
  mchunkptr unsorted_bin;
  mchunkptr first_unsorted;

  mchunkptr nextchunk;
  sys_size_t size;
  sys_size_t nextsize;
  sys_size_t prevsize;
  int nextinuse;
  mchunkptr bck;
  mchunkptr fwd;

  if (av->max_fast != 0) {
    clear_fastchunks(av);

    unsorted_bin = unsorted_chunks(av);
    maxfb = &(av->fastbins[fastbin_index(av->max_fast)]);
    fb = &(av->fastbins[0]);
    do {
      if ( (p = *fb) != 0) {
        *fb = 0;

        do {
          do_check_inuse_chunk(h, p);
          nextp = p->fd;

          size = p->size & ~PREV_INUSE;
          nextchunk = chunk_at_offset(p, size);
          nextsize = chunksize(nextchunk);

          if (!prev_inuse(p)) {
            prevsize = p->prev_size;
            size += prevsize;
            p = chunk_at_offset(p, -((long) prevsize));
            unlink(p, bck, fwd);
          }

          if (nextchunk != av->top) {
            nextinuse = inuse_bit_at_offset(nextchunk, nextsize);
            set_head(nextchunk, nextsize);

            if (!nextinuse) {
              size += nextsize;
              unlink(nextchunk, bck, fwd);
            }

            first_unsorted = unsorted_bin->fd;
            unsorted_bin->fd = p;
            first_unsorted->bk = p;

            set_head(p, size | PREV_INUSE);
            p->bk = unsorted_bin;
            p->fd = first_unsorted;
            set_foot(p, size);
          } else {
            size += nextsize;
            set_head(p, size | PREV_INUSE);
            av->top = p;
          }
        } while ( (p = nextp) != 0);
      }
    } while (fb++ != maxfb);
  } else {
    malloc_init_state(av);
    do_check_malloc_state(h);
  }
  dlmalloc_stats();
}

void dlmalloc_init_state(void *h) {
  mstate av = get_malloc_state(h);
  malloc_init_state(av);
}

void* dlrealloc(void *h, void* oldmem, sys_size_t bytes) {
  mstate av = get_malloc_state(h);

  sys_size_t nb;

  mchunkptr oldp;
  sys_size_t oldsize;

  mchunkptr newp;
  sys_size_t newsize;
  void* newmem;

  mchunkptr next;

  mchunkptr remainder;
  sys_size_t remainder_size;

  mchunkptr bck;
  mchunkptr fwd;

  sys_size_t copysize;
  unsigned int ncopies;
  sys_size_t* s;
  sys_size_t* d;
  if (oldmem == 0) return dlmalloc(h, bytes);

  checked_request2size(bytes, nb);

  oldp = mem2chunk(oldmem);
  oldsize = chunksize(oldp);

  do_check_inuse_chunk(h, oldp);

  if (!chunk_is_mmapped(oldp)) {
    if ((sys_size_t)(oldsize) >= (sys_size_t)(nb)) {
      newp = oldp;
      newsize = oldsize;
    } else {
      next = chunk_at_offset(oldp, oldsize);

      if (next == av->top &&
          (sys_size_t)(newsize = oldsize + chunksize(next)) >=
          (sys_size_t)(nb + MINSIZE)) {
        set_head_size(oldp, nb);
        av->top = chunk_at_offset(oldp, nb);
        set_head(av->top, (newsize - nb) | PREV_INUSE);
        return chunk2mem(oldp);
      } else if (next != av->top &&
               !inuse(next) &&
               (sys_size_t)(newsize = oldsize + chunksize(next)) >=
               (sys_size_t)(nb)) {
        newp = oldp;
        unlink(next, bck, fwd);
      } else {
        newmem = dlmalloc(h, nb - MALLOC_ALIGN_MASK);
        if (newmem == 0)
          return 0;

        newp = mem2chunk(newmem);
        newsize = chunksize(newp);

        if (newp == next) {
          newsize += oldsize;
          newp = oldp;
        } else {
          copysize = oldsize - SIZE_SZ;
          s = (sys_size_t*)(oldmem);
          d = (sys_size_t*)(newmem);
          ncopies = copysize / sizeof(sys_size_t);
          assert(ncopies >= 3);

          if (ncopies > 9)
            sys_memcpy(d, s, copysize);

          else {
            *(d+0) = *(s+0);
            *(d+1) = *(s+1);
            *(d+2) = *(s+2);
            if (ncopies > 4) {
              *(d+3) = *(s+3);
              *(d+4) = *(s+4);
              if (ncopies > 6) {
                *(d+5) = *(s+5);
                *(d+6) = *(s+6);
                if (ncopies > 8) {
                  *(d+7) = *(s+7);
                  *(d+8) = *(s+8);
                }
              }
            }
          }

          dlfree(h, oldmem);
          do_check_inuse_chunk(h, newp);
          return chunk2mem(newp);
        }
      }
    }

    assert((sys_size_t)(newsize) >= (sys_size_t)(nb));
    remainder_size = newsize - nb;

    if (remainder_size < MINSIZE) {
      set_head_size(newp, newsize);
      set_inuse_bit_at_offset(newp, newsize);
    } else {
      remainder = chunk_at_offset(newp, nb);
      set_head_size(newp, nb);
      set_head(remainder, remainder_size | PREV_INUSE);

      set_inuse_bit_at_offset(remainder, remainder_size);
      dlfree(h, chunk2mem(remainder));
    }

    do_check_inuse_chunk(h, newp);
    return chunk2mem(newp);
  } else {
    do_check_malloc_state(h);
    return 0;
  }
}

void* dlmemalign(void *h, sys_size_t alignment, sys_size_t bytes) {
  sys_size_t nb;
  char* m;
  mchunkptr p;
  char* brk;
  mchunkptr newp;
  sys_size_t newsize;
  sys_size_t leadsize;
  mchunkptr remainder;
  sys_size_t remainder_size;
  sys_size_t size;

  if (alignment <= MALLOC_ALIGNMENT) return dlmalloc(h, bytes);
  if (alignment < MINSIZE) alignment = MINSIZE;

  if ((alignment & (alignment - 1)) != 0) {
    sys_size_t a = MALLOC_ALIGNMENT * 2;
    while ((sys_size_t)a < (sys_size_t)alignment) a <<= 1;
    alignment = a;
  }

  checked_request2size(bytes, nb);
  m = (char*)(dlmalloc(h, nb + alignment + MINSIZE));

  if (m == 0) return 0;

  p = mem2chunk(m);

  if ((((sys_size_t)(m)) % alignment) != 0) {
    brk = (char*)mem2chunk((sys_size_t)(((sys_size_t)(m + alignment - 1)) &
                           -((signed long) alignment)));
    if ((sys_size_t)(brk - (char*)(p)) < MINSIZE)
      brk += alignment;

    newp = (mchunkptr)brk;
    leadsize = brk - (char*)(p);
    newsize = chunksize(p) - leadsize;

    if (chunk_is_mmapped(p)) {
      newp->prev_size = p->prev_size + leadsize;
      set_head(newp, newsize|IS_MMAPPED);
      return chunk2mem(newp);
    }

    set_head(newp, newsize | PREV_INUSE);
    set_inuse_bit_at_offset(newp, newsize);
    set_head_size(p, leadsize);
    dlfree(h, chunk2mem(p));
    p = newp;

    assert (newsize >= nb &&
            (((sys_size_t)(chunk2mem(p))) % alignment) == 0);
  }

  if (!chunk_is_mmapped(p)) {
    size = chunksize(p);
    if ((sys_size_t)(size) > (sys_size_t)(nb + MINSIZE)) {
      remainder_size = size - nb;
      remainder = chunk_at_offset(p, nb);
      set_head(remainder, remainder_size | PREV_INUSE);
      set_head_size(p, nb);
      dlfree(h, chunk2mem(remainder));
    }
  }

  do_check_inuse_chunk(h, p);
  return chunk2mem(p);
}

void* dlcalloc(void *h, sys_size_t n_elements, sys_size_t elem_size) {
  mchunkptr p;
  sys_size_t clearsize;
  sys_size_t nclears;
  sys_size_t* d;

  void* mem = dlmalloc(h, n_elements * elem_size);

  if (mem != 0) {
    p = mem2chunk(mem);

    if (!chunk_is_mmapped(p)) {
      d = (sys_size_t*)mem;
      clearsize = chunksize(p) - SIZE_SZ;
      nclears = clearsize / sizeof(sys_size_t);
      assert(nclears >= 3);

      if (nclears > 9)
        sys_memset(d, 0, clearsize);

      else {
        *(d+0) = 0;
        *(d+1) = 0;
        *(d+2) = 0;
        if (nclears > 4) {
          *(d+3) = 0;
          *(d+4) = 0;
          if (nclears > 6) {
            *(d+5) = 0;
            *(d+6) = 0;
            if (nclears > 8) {
              *(d+7) = 0;
              *(d+8) = 0;
            }
          }
        }
      }
    } else {
      d = (sys_size_t*)mem;
      clearsize = chunksize(p) - 2*SIZE_SZ;
      sys_memset(d, 0, clearsize);
    }

  }
  return mem;
}

void dlcfree(void *h, void *mem) {
  dlfree(h, mem);
}

void** dlindependent_calloc(void *h, sys_size_t n_elements, sys_size_t elem_size, void* chunks[]) {
  sys_size_t sz = elem_size;
  return iALLOc(h, n_elements, &sz, 3, chunks);
}

void** dlindependent_comalloc(void *h, sys_size_t n_elements, sys_size_t sizes[], void* chunks[]) {
  return iALLOc(h, n_elements, sizes, 0, chunks);
}

static void** iALLOc(void *h, sys_size_t n_elements, sys_size_t* sizes, int opts, void* chunks[]) {
  mstate av = get_malloc_state(h);
  sys_size_t element_size;
  sys_size_t contents_size;
  sys_size_t array_size;
  void* mem;
  mchunkptr p;
  sys_size_t remainder_size;
  void** marray;
  mchunkptr array_chunk;
  int mmx;
  sys_size_t size;
  sys_size_t i;

  if (av->max_fast == 0) malloc_consolidate(h, av);

  if (chunks != 0) {
    if (n_elements == 0)
      return chunks;
    marray = chunks;
    array_size = 0;
  } else {
    if (n_elements == 0)
      return (void**) dlmalloc(h, 0);
    marray = 0;
    array_size = request2size(n_elements * (sizeof(void*)));
  }

  if (opts & 0x1) {
    element_size = request2size(*sizes);
    contents_size = n_elements * element_size;
  } else {
    element_size = 0;
    contents_size = 0;
    for (i = 0; i != n_elements; ++i)
      contents_size += request2size(sizes[i]);
  }

  size = contents_size + array_size - MALLOC_ALIGN_MASK;

  mmx = av->n_mmaps_max;
  av->n_mmaps_max = 0;
  mem = dlmalloc(h, size);
  av->n_mmaps_max = mmx;
  if (mem == 0)
    return 0;

  p = mem2chunk(mem);
  assert(!chunk_is_mmapped(p));
  remainder_size = chunksize(p);

  if (opts & 0x2) {
    sys_memset(mem, 0, remainder_size - SIZE_SZ - array_size);
  }

  if (marray == 0) {
    array_chunk = chunk_at_offset(p, contents_size);
    marray = (void**) (chunk2mem(array_chunk));
    set_head(array_chunk, (remainder_size - contents_size) | PREV_INUSE);
    remainder_size = contents_size;
  }

  for (i = 0; ; ++i) {
    marray[i] = chunk2mem(p);
    if (i != n_elements-1) {
      if (element_size != 0)
        size = element_size;
      else
        size = request2size(sizes[i]);
      remainder_size -= size;
      set_head(p, size | PREV_INUSE);
      p = chunk_at_offset(p, size);
    } else {
      set_head(p, remainder_size | PREV_INUSE);
      break;
    }
  }

  if (marray != chunks) {
    if (element_size != 0)
      assert(remainder_size == element_size);
    else
      assert(remainder_size == request2size(sizes[i]));
    do_check_inuse_chunk(h, mem2chunk(marray));
  }

  for (i = 0; i != n_elements; ++i) {
    do_check_inuse_chunk(h, mem2chunk(marray[i]));
  }

  return marray;
}

void* dlvalloc(void *h, sys_size_t bytes) {
  mstate av = get_malloc_state(h);
  if (av->max_fast == 0) malloc_consolidate(h, av);
  return dlmemalign(h, av->pagesize, bytes);
}

void* dlpvalloc(void *h, sys_size_t bytes) {
  mstate av = get_malloc_state(h);
  sys_size_t pagesz;

  if (av->max_fast == 0) malloc_consolidate(h, av);
  pagesz = av->pagesize;
  return dlmemalign(h, pagesz, (bytes + pagesz - 1) & ~(pagesz - 1));
}

int dlmalloc_trim(void *h, sys_size_t pad) {
  mstate av = get_malloc_state(h);
  malloc_consolidate(h, av);
  return 0;
}

sys_size_t dlmalloc_usable_size(void* mem) {
  mchunkptr p;
  if (mem != 0) {
    p = mem2chunk(mem);
    if (chunk_is_mmapped(p))
      return chunksize(p) - 2*SIZE_SZ;
    else if (inuse(p))
      return chunksize(p) - SIZE_SZ;
  }
  return 0;
}

struct mallinfo dlmallinfo(void *h) {
  mstate av = get_malloc_state(h);
  struct mallinfo mi;
  int i;
  mbinptr b;
  mchunkptr p;
  sys_size_t avail;
  sys_size_t fastavail;
  int nblocks;
  int nfastblocks;

  if (av->top == 0) malloc_consolidate(h, av);

  do_check_malloc_state(h);
  avail = chunksize(av->top);
  nblocks = 1;
  nfastblocks = 0;
  fastavail = 0;

  for (i = 0; NFASTBINS-i>0; ++i) {
    for (p = av->fastbins[i]; p != 0; p = p->fd) {
      ++nfastblocks;
      fastavail += chunksize(p);
    }
  }

  avail += fastavail;

  for (i = 1; i < NBINS; ++i) {
    b = bin_at(av, i);
    for (p = last(b); p != b; p = p->bk) {
      ++nblocks;
      avail += chunksize(p);
    }
  }

  mi.smblks = nfastblocks;
  mi.ordblks = nblocks;
  mi.fordblks = avail;
  mi.uordblks = av->sbrked_mem - avail;
  mi.arena = av->sbrked_mem;
  mi.hblks = av->n_mmaps;
  mi.hblkhd = av->mmapped_mem;
  mi.fsmblks = fastavail;
  mi.keepcost = chunksize(av->top);
  mi.usmblks = av->max_total_mem;

  return mi;
}

void dlmalloc_stats(void)
{
}

int dlmallopt(void *h, int param_number, int value) {
  mstate av = get_malloc_state(h);

  malloc_consolidate(h, av);

  switch(param_number) {
  case 1:
    if (value >= 0 && value <= MAX_FAST_SIZE) {
      set_max_fast(av, value);
      return 1;
    }
    else
      return 0;

  case -1:
    av->trim_threshold = value;
    return 1;

  case -2:
    av->top_pad = value;
    return 1;

  case -3:
    av->mmap_threshold = value;
    return 1;

  case -4:
    if (value != 0)
      return 0;

    av->n_mmaps_max = value;
    return 1;

  default:
    return 0;
  }
}
