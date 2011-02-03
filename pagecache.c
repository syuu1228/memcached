/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * Slabs memory allocation, based on powers-of-N. Slabs are up to 1MB in size
 * and are divided into chunks. The chunk sizes start off at the size of the
 * "item" structure plus space for a small key and value. They increase by
 * a multiplier factor from there, up to half the maximum slab size. The last
 * slab size is always 1MB, since that's the maximum item size allowed by the
 * memcached protocol.
 */
#include "memcached.h"
#include "slabs.h"
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

typedef struct _pagecache_item {
    void *addr;
    unsigned int hdr_id;
    int hdr_len;
} pagecache_item;

/**
 * Access to the slab allocator is protected by this lock
 */
static inline pagecache_item *_pagecache_item_pit(const item *it, const size_t nkey, const uint8_t nsuffix, const int cas) {
    return ((pagecache_item*) &(it->data) + nkey + 1
            + nsuffix
            + ((cas) ? sizeof(uint64_t) : 0));
}

static inline pagecache_item *pagecache_item_pit(const item *it) {
    return _pagecache_item_pit(it, it->nkey, it->nsuffix, it->it_flags & ITEM_CAS);
}

static unsigned int pagecache_clsid(const size_t size) {
    return 1;
}

/**
 * Determines the chunk sizes and initializes the slab class descriptors
 * accordingly.
 */
static void pagecache_init(const size_t limit, const double factor, const bool prealloc) {
    slabs_init(limit, factor, prealloc);
}

static item *pagecache_alloc(const char *key, const size_t nkey, const int nbytes, const uint8_t nsuffix, const unsigned int id) {
    item *it = NULL;
    pagecache_item *pit = NULL;
    const size_t hdr_len = sizeof(item) + nkey + nsuffix +
        sizeof(pagecache_item) +
        (settings.use_cas) ? sizeof(uint64_t) : 0;
    unsigned int hdr_id;
    
    hdr_id = slabs_clsid(hdr_len);
    it = slabs_alloc(key, nkey, 0, nsuffix, hdr_id);
    if (!it) {
        return 0;
    }
    
    pit = _pagecache_item_pit(it, nkey, nsuffix, settings.use_cas);
    pit->addr = mmap(NULL, nbytes, PROT_READ|PROT_WRITE,
                     MAP_PRIVATE|MAP_ANONYMOUS,
                     -1, 0);
    pit->hdr_len = hdr_len;
    pit->hdr_id = hdr_id;
    if (pit->addr == MAP_FAILED) {
        slabs_free(it, hdr_len, hdr_id);
        return 0;
    }
    
    return it;
}

static void pagecache_free(item *it, size_t ntotal, unsigned int id) {
    pagecache_item *pit = pagecache_item_pit(it);
    munmap(pit->addr, it->nbytes);
    slabs_free(it, pit->hdr_len, pit->hdr_id);
    return;
}

static void pagecache_adjust_mem_requested(unsigned int id, size_t old, size_t ntotal)
{
    abort();
}

static uint64_t pagecache_item_get_cas(item *i) {
    return (i->it_flags & ITEM_CAS) ? i->data->cas : (uint64_t)0;
}

static void pagecache_item_set_cas(item *i, uint64_t v) {
    if (i->it_flags & ITEM_CAS)
        i->data->cas = v;
}

static char *pagecache_item_key(item *item) {
    return (((char*)&(item->data)) 
            + ((item->it_flags & ITEM_CAS) ? sizeof(uint64_t) : 0));
}

static char *pagecache_item_suffix(item *item) {
    return ((char*) &(item->data) + item->nkey + 1
            + ((item->it_flags & ITEM_CAS) ? sizeof(uint64_t) : 0));
}

static char *pagecache_item_data(item *item) {
    return (char *)pagecache_item_pit(item)->addr;
}

static int pagecache_item_ntotal(item *item) {
    return (sizeof(struct _stritem) + item->nkey + 1
            + item->nsuffix + item->nbytes
            + ((item->it_flags & ITEM_CAS) ? sizeof(uint64_t) : 0));
}

struct storage pagecache_storage = {
    .init = pagecache_init,
    .clsid = pagecache_clsid,
    .alloc = pagecache_alloc,
    .free = pagecache_free,
    .adjust_mem_requested = pagecache_adjust_mem_requested,
    .get_stats = get_stats,
    .stats = slabs_stats,
    .item_get_cas = pagecache_item_get_cas,
    .item_set_cas = pagecache_item_set_cas,
    .item_key = pagecache_item_key,
    .item_suffix = pagecache_item_suffix,
    .item_data = pagecache_item_data,
    .item_ntotal = pagecache_item_ntotal
};
