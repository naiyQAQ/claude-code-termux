/*
 * glibc-to-bionic shim: libpthread.so.0
 *
 * Provides GLIBC_2.17 versioned pthread symbols by forwarding to bionic.
 * All pthread functions live in bionic's libc.so.
 */

#define _GNU_SOURCE
#include <pthread.h>
#include <dlfcn.h>
#include <stddef.h>

static void *bionic_libc;
static int inited;

static void *RP(const char *name) {
    if (__builtin_expect(!inited, 0)) {
        bionic_libc = dlopen("libc.so", RTLD_LAZY | RTLD_NOLOAD);
        if (!bionic_libc)
            bionic_libc = dlopen("/apex/com.android.runtime/lib64/bionic/libc.so", RTLD_LAZY | RTLD_NOLOAD);
        inited = 1;
    }
    return dlsym(bionic_libc, name);
}

int pthread_create(pthread_t *t, const pthread_attr_t *a, void *(*fn)(void *), void *arg) {
    static int (*fptr)(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);
    if (__builtin_expect(!fptr, 0)) fptr = RP("pthread_create");
    return fptr(t, a, fn, arg);
}
int pthread_join(pthread_t t, void **retval) {
    static int (*fptr)(pthread_t, void **);
    if (__builtin_expect(!fptr, 0)) fptr = RP("pthread_join");
    return fptr(t, retval);
}
int pthread_detach(pthread_t t) {
    static int (*fptr)(pthread_t);
    if (__builtin_expect(!fptr, 0)) fptr = RP("pthread_detach");
    return fptr(t);
}
pthread_t pthread_self(void) {
    static pthread_t (*fptr)(void);
    if (__builtin_expect(!fptr, 0)) fptr = RP("pthread_self");
    return fptr();
}
int pthread_attr_init(pthread_attr_t *a) {
    static int (*fptr)(pthread_attr_t *);
    if (__builtin_expect(!fptr, 0)) fptr = RP("pthread_attr_init");
    return fptr(a);
}
int pthread_attr_destroy(pthread_attr_t *a) {
    static int (*fptr)(pthread_attr_t *);
    if (__builtin_expect(!fptr, 0)) fptr = RP("pthread_attr_destroy");
    return fptr(a);
}
int pthread_attr_setstacksize(pthread_attr_t *a, size_t s) {
    static int (*fptr)(pthread_attr_t *, size_t);
    if (__builtin_expect(!fptr, 0)) fptr = RP("pthread_attr_setstacksize");
    return fptr(a, s);
}
int pthread_key_create(pthread_key_t *k, void (*dtor)(void *)) {
    static int (*fptr)(pthread_key_t *, void (*)(void *));
    if (__builtin_expect(!fptr, 0)) fptr = RP("pthread_key_create");
    return fptr(k, dtor);
}
int pthread_key_delete(pthread_key_t k) {
    static int (*fptr)(pthread_key_t);
    if (__builtin_expect(!fptr, 0)) fptr = RP("pthread_key_delete");
    return fptr(k);
}
int pthread_setspecific(pthread_key_t k, const void *v) {
    static int (*fptr)(pthread_key_t, const void *);
    if (__builtin_expect(!fptr, 0)) fptr = RP("pthread_setspecific");
    return fptr(k, v);
}
int pthread_setname_np(pthread_t t, const char *name) {
    static int (*fptr)(pthread_t, const char *);
    if (__builtin_expect(!fptr, 0)) fptr = RP("pthread_setname_np");
    return fptr(t, name);
}
int pthread_rwlock_rdlock(pthread_rwlock_t *l) {
    static int (*fptr)(pthread_rwlock_t *);
    if (__builtin_expect(!fptr, 0)) fptr = RP("pthread_rwlock_rdlock");
    return fptr(l);
}
int pthread_rwlock_wrlock(pthread_rwlock_t *l) {
    static int (*fptr)(pthread_rwlock_t *);
    if (__builtin_expect(!fptr, 0)) fptr = RP("pthread_rwlock_wrlock");
    return fptr(l);
}
int pthread_rwlock_unlock(pthread_rwlock_t *l) {
    static int (*fptr)(pthread_rwlock_t *);
    if (__builtin_expect(!fptr, 0)) fptr = RP("pthread_rwlock_unlock");
    return fptr(l);
}
