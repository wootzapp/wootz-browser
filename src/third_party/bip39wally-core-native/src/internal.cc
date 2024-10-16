#include "internal.h"
#include "wally_crypto.h"
#include "ccan/ccan/build_assert/build_assert.h"
#include "ccan/ccan/crypto/sha256/sha256.h"
#include "ccan/ccan/crypto/sha512/sha512.h"
#include <stdbool.h>

#undef malloc
#undef free

int wally_free_string(char *str)
{
    if (!str)
        return WALLY_EINVAL;
    wally_clear(str, strlen(str));
    wally_free(str);
    return WALLY_OK;
}

int wally_bzero(void *bytes, size_t len)
{
    if (!bytes)
        return WALLY_EINVAL;
    wally_clear(bytes, len);
    return WALLY_OK;
}

int wally_sha256(const unsigned char *bytes, size_t bytes_len,
                 unsigned char *bytes_out, size_t len)
{
    struct sha256Wally sha;
    bool aligned = alignment_ok(bytes_out, sizeof(sha.u.u32));

    if (!bytes || !bytes_out || len != SHA256_LEN)
        return WALLY_EINVAL;

    sha256Wally(aligned ? (struct sha256Wally *)bytes_out : &sha, bytes, bytes_len);
    if (!aligned) {
        memcpy(bytes_out, &sha, sizeof(sha));
        wally_clear(&sha, sizeof(sha));
    }
    return WALLY_OK;
}

int wally_sha256d(const unsigned char *bytes, size_t bytes_len,
                  unsigned char *bytes_out, size_t len)
{
    struct sha256Wally sha_1, sha_2;
    bool aligned = alignment_ok(bytes_out, sizeof(sha_1.u.u32));

    if (!bytes || !bytes_out || len != SHA256_LEN)
        return WALLY_EINVAL;

    sha256Wally(&sha_1, bytes, bytes_len);
    sha256Wally(aligned ? (struct sha256Wally *)bytes_out : &sha_2, &sha_1, sizeof(sha_1));
    if (!aligned) {
        memcpy(bytes_out, &sha_2, sizeof(sha_2));
        wally_clear(&sha_2, sizeof(sha_2));
    }
    wally_clear(&sha_1, sizeof(sha_1));
    return WALLY_OK;
}

int wally_sha512(const unsigned char *bytes, size_t bytes_len,
                 unsigned char *bytes_out, size_t len)
{
    struct sha512Wally sha;
    bool aligned = alignment_ok(bytes_out, sizeof(sha.u.u64));

    if (!bytes || !bytes_out || len != SHA512_LEN)
        return WALLY_EINVAL;

    sha512Wally(aligned ? (struct sha512Wally *)bytes_out : &sha, bytes, bytes_len);
    if (!aligned) {
        memcpy(bytes_out, &sha, sizeof(sha));
        wally_clear(&sha, sizeof(sha));
    }
    return WALLY_OK;
}

static void wally_internal_bzero(void *dest, size_t len)
{
#ifdef HAVE_MEMSET_S
    memset_s(dest, len, 0, len);
#else
    memset(dest, 0, len);
#if 0
    /* This is used by boringssl to prevent memset from being elided. It
     * works by forcing a memory barrier and so can be slow.
     */
    __asm__ __volatile__ ("" : : "r" (dest) : "memory");
#endif
#endif
}

static void *wally_internal_malloc(size_t size)
{
    return malloc(size);
}

static void wally_internal_free(void *ptr)
{
    if (ptr)
        free(ptr);
}

static struct wally_operations _ops = {
    wally_internal_malloc,
    wally_internal_free,
    wally_internal_bzero
};

void *wally_malloc(size_t size)
{
    return _ops.malloc_fn(size);
}

void wally_free(void *ptr)
{
    _ops.free_fn(ptr);
}

char *wally_strdup(const char *str)
{
    size_t len = strlen(str) + 1;
    char *new_str = (char *)wally_malloc(len);
    if (new_str)
        memcpy(new_str, str, len); /* Copies terminating nul */
    return new_str;
}

const struct wally_operations *wally_ops(void)
{
    return &_ops;
}

int wally_get_operations(struct wally_operations *output)
{
    if (!output)
        return WALLY_EINVAL;
    memcpy(output, &_ops, sizeof(_ops));
    return WALLY_OK;
}

int wally_set_operations(const struct wally_operations *ops)
{
    if (!ops)
        return WALLY_EINVAL;
#define COPY_FN_PTR(name) if (ops->name) _ops.name = ops->name
    COPY_FN_PTR(malloc_fn);
    COPY_FN_PTR(free_fn);
    COPY_FN_PTR (bzero_fn);
    COPY_FN_PTR (ec_nonce_fn);
#undef COPY_FN_PTR
    return WALLY_OK;
}

void wally_clear(void *p, size_t len){
    _ops.bzero_fn(p, len);
}

void wally_clear_2(void *p, size_t len, void *p2, size_t len2){
    _ops.bzero_fn(p, len);
    _ops.bzero_fn(p2, len2);
}

void wally_clear_3(void *p, size_t len, void *p2, size_t len2,
                   void *p3, size_t len3){
    _ops.bzero_fn(p, len);
    _ops.bzero_fn(p2, len2);
    _ops.bzero_fn(p3, len3);
}

void wally_clear_4(void *p, size_t len, void *p2, size_t len2,
                   void *p3, size_t len3, void *p4, size_t len4){
    _ops.bzero_fn(p, len);
    _ops.bzero_fn(p2, len2);
    _ops.bzero_fn(p3, len3);
    _ops.bzero_fn(p4, len4);
}

void wally_clear_5(void *p, size_t len, void *p2, size_t len2,
                   void *p3, size_t len3, void *p4, size_t len4,
                   void *p5, size_t len5){
    _ops.bzero_fn(p, len);
    _ops.bzero_fn(p2, len2);
    _ops.bzero_fn(p3, len3);
    _ops.bzero_fn(p4, len4);
    _ops.bzero_fn(p5, len5);
}

void wally_clear_6(void *p, size_t len, void *p2, size_t len2,
                   void *p3, size_t len3, void *p4, size_t len4,
                   void *p5, size_t len5, void *p6, size_t len6){
    _ops.bzero_fn(p, len);
    _ops.bzero_fn(p2, len2);
    _ops.bzero_fn(p3, len3);
    _ops.bzero_fn(p4, len4);
    _ops.bzero_fn(p5, len5);
    _ops.bzero_fn(p6, len6);
}

static bool wally_init_done = false;

int wally_init(uint32_t flags)
{
    if (flags)
        return WALLY_EINVAL;

    if (!wally_init_done) {
        sha256_optimize();
        wally_init_done = true;
    }

    return WALLY_OK;
}

int wally_cleanup(uint32_t flags)
{
    if (flags)
        return WALLY_EINVAL;
    return WALLY_OK;
}


#ifdef __ANDROID__
#define malloc(size) wally_malloc(size)
#define free(ptr) wally_free(ptr)
#include "cpufeatures/cpu-features.c"
#endif
