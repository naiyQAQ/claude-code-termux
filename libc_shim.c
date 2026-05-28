/*
 * glibc-to-bionic shim: libc.so.6
 *
 * Provides GLIBC_2.17 versioned symbols by forwarding to Android bionic.
 * Used to load glibc-linked .node native addons on Termux.
 *
 * All wrappers lazy-initialize by calling ensure_init() on first use
 * to handle the case where the library is loaded but constructor hasn't
 * run yet (which happens when loaded as a dependency via dlopen).
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <poll.h>
#include <time.h>
#include <sys/uio.h>
#include <sys/syscall.h>
#include <sys/auxv.h>
#include <link.h>
#include <fcntl.h>

/* ========================================================================
 * Bionic handle and function pointers
 * ======================================================================== */
static void *bionic_handle;
static int initialized;

static void do_init(void) {
    if (initialized) return;

    /* Open bionic's libc - it should already be loaded */
    bionic_handle = dlopen("libc.so", RTLD_LAZY | RTLD_NOLOAD);
    if (!bionic_handle)
        bionic_handle = dlopen("/apex/com.android.runtime/lib64/bionic/libc.so", RTLD_LAZY | RTLD_NOLOAD);
    if (!bionic_handle)
        bionic_handle = dlopen("/system/lib64/libc.so", RTLD_LAZY | RTLD_NOLOAD);
    if (!bionic_handle)
        bionic_handle = dlopen("libc.so", RTLD_LAZY);

    initialized = 1;
}

/* Resolve a symbol from bionic, with lazy init */
static void *R(const char *name) {
    if (__builtin_expect(!initialized, 0)) do_init();
    return dlsym(bionic_handle, name);
}

/* ========================================================================
 * Glibc-specific symbols (different name/signature in bionic)
 * ======================================================================== */

/* __errno_location -> bionic's __errno */
int *__errno_location(void) {
    static int *(*fptr)(void);
    if (__builtin_expect(!fptr, 0)) fptr = R("__errno");
    return fptr();
}

/* __fxstat64(ver, fd, buf) -> fstat(fd, buf) */
int __fxstat64(int ver, int fd, struct stat *buf) {
    (void)ver;
    static int (*fptr)(int, struct stat *);
    if (__builtin_expect(!fptr, 0)) fptr = R("fstat");
    return fptr(fd, buf);
}

/* __xstat64(ver, path, buf) -> stat(path, buf) */
int __xstat64(int ver, const char *path, struct stat *buf) {
    (void)ver;
    static int (*fptr)(const char *, struct stat *);
    if (__builtin_expect(!fptr, 0)) fptr = R("stat");
    return fptr(path, buf);
}

/* __xpg_strerror_r - XSI-compliant strerror_r */
int __xpg_strerror_r(int errnum, char *buf, size_t buflen) {
    if (!buf || buflen == 0) return -1;
    static char *(*b_strerror)(int);
    if (__builtin_expect(!b_strerror, 0)) b_strerror = R("strerror");
    const char *msg = b_strerror(errnum);
    if (!msg) {
        /* Use a minimal snprintf without going through our wrapper */
        static int (*b_snprintf)(char *, size_t, const char *, ...);
        if (!b_snprintf) b_snprintf = R("snprintf");
        b_snprintf(buf, buflen, "Unknown error %d", errnum);
        return -1;
    }
    static size_t (*b_strlen)(const char *);
    if (__builtin_expect(!b_strlen, 0)) b_strlen = R("strlen");
    size_t len = b_strlen(msg);
    static void *(*b_memcpy)(void *, const void *, size_t);
    if (__builtin_expect(!b_memcpy, 0)) b_memcpy = R("memcpy");
    if (len >= buflen) {
        b_memcpy(buf, msg, buflen - 1);
        buf[buflen - 1] = '\0';
        return -1;
    }
    b_memcpy(buf, msg, len + 1);
    return 0;
}

/* __cxa_thread_atexit_impl */
int __cxa_thread_atexit_impl(void (*dtor)(void *), void *obj, void *dso_symbol) {
    static int (*fptr)(void (*)(void *), void *, void *);
    if (__builtin_expect(!fptr, 0)) fptr = R("__cxa_thread_atexit_impl");
    if (fptr) return fptr(dtor, obj, dso_symbol);
    return 0;
}

/* stderr data symbol */
FILE *shim_stderr_ptr;

__attribute__((constructor))
static void init_stderr(void) {
    if (!initialized) do_init();
    FILE **p = (FILE **)dlsym(bionic_handle, "stderr");
    if (p) shim_stderr_ptr = *p;
    else shim_stderr_ptr = fdopen(2, "w");
}

/* ========================================================================
 * Forwarding wrappers - each lazy-resolves from bionic on first call
 * ======================================================================== */

/* Macro for simple forwarding functions */
#define FWD0(ret, name) \
    ret name(void) { \
        static ret (*fptr)(void); \
        if (__builtin_expect(!fptr, 0)) fptr = R(#name); \
        return fptr(); \
    }

#define FWD1(ret, name, t1) \
    ret name(t1 a1) { \
        static ret (*fptr)(t1); \
        if (__builtin_expect(!fptr, 0)) fptr = R(#name); \
        return fptr(a1); \
    }

#define FWD2(ret, name, t1, t2) \
    ret name(t1 a1, t2 a2) { \
        static ret (*fptr)(t1, t2); \
        if (__builtin_expect(!fptr, 0)) fptr = R(#name); \
        return fptr(a1, a2); \
    }

#define FWD3(ret, name, t1, t2, t3) \
    ret name(t1 a1, t2 a2, t3 a3) { \
        static ret (*fptr)(t1, t2, t3); \
        if (__builtin_expect(!fptr, 0)) fptr = R(#name); \
        return fptr(a1, a2, a3); \
    }

/* void-returning variants */
#define FWDV1(name, t1) \
    void name(t1 a1) { \
        static void (*fptr)(t1); \
        if (__builtin_expect(!fptr, 0)) fptr = R(#name); \
        fptr(a1); \
    }

/* --- Memory management --- */
FWD1(void *, malloc, size_t)
FWD2(void *, calloc, size_t, size_t)
FWD2(void *, realloc, void *, size_t)
FWDV1(free, void *)
FWD3(int, posix_memalign, void **, size_t, size_t)

/* --- String / Memory ops --- */
FWD3(void *, memcpy, void *, const void *, size_t)
FWD3(void *, memmove, void *, const void *, size_t)
FWD3(void *, memset, void *, int, size_t)
FWD1(size_t, strlen, const char *)

int bcmp(const void *s1, const void *s2, size_t n) {
    static int (*fptr)(const void *, const void *, size_t);
    if (__builtin_expect(!fptr, 0)) fptr = R("memcmp");
    return fptr(s1, s2, n);
}

/* --- I/O --- */
FWD3(ssize_t, read, int, void *, size_t)
FWD3(ssize_t, write, int, const void *, size_t)
FWD3(ssize_t, writev, int, const struct iovec *, int)
FWD1(int, close, int)

int open(const char *path, int flags, ...) {
    static int (*fptr)(const char *, int, ...);
    if (__builtin_expect(!fptr, 0)) fptr = R("open");
    if (flags & O_CREAT) {
        va_list ap; va_start(ap, flags);
        mode_t mode = (mode_t)va_arg(ap, int);
        va_end(ap);
        return fptr(path, flags, mode);
    }
    return fptr(path, flags);
}

int open64(const char *path, int flags, ...) {
    static int (*fptr)(const char *, int, ...);
    if (__builtin_expect(!fptr, 0)) {
        fptr = R("open64");
        if (!fptr) fptr = R("open");
    }
    if (flags & O_CREAT) {
        va_list ap; va_start(ap, flags);
        mode_t mode = (mode_t)va_arg(ap, int);
        va_end(ap);
        return fptr(path, flags, mode);
    }
    return fptr(path, flags);
}

off64_t lseek64(int fd, off64_t off, int whence) {
    static off64_t (*fptr)(int, off64_t, int);
    if (__builtin_expect(!fptr, 0)) {
        fptr = R("lseek64");
        if (!fptr) fptr = R("lseek");
    }
    return fptr(fd, off, whence);
}

void *mmap64(void *addr, size_t len, int prot, int flags, int fd, off64_t off) {
    static void *(*fptr)(void *, size_t, int, int, int, off64_t);
    if (__builtin_expect(!fptr, 0)) {
        fptr = R("mmap64");
        if (!fptr) fptr = R("mmap");
    }
    return fptr(addr, len, prot, flags, fd, off);
}

FWD2(int, munmap, void *, size_t)
FWD1(int, pipe, int *)
FWD3(int, poll, struct pollfd *, nfds_t, int)
FWD0(int, pause)

/* --- stdio --- */
FWD1(int, fflush, FILE *)

int fprintf(FILE *stream, const char *fmt, ...) {
    static int (*fptr)(FILE *, const char *, va_list);
    if (__builtin_expect(!fptr, 0)) fptr = R("vfprintf");
    va_list ap; va_start(ap, fmt);
    int r = fptr(stream, fmt, ap);
    va_end(ap);
    return r;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    static size_t (*fptr)(const void *, size_t, size_t, FILE *);
    if (__builtin_expect(!fptr, 0)) fptr = R("fwrite");
    return fptr(ptr, size, nmemb, stream);
}

int snprintf(char *str, size_t size, const char *fmt, ...) {
    static int (*fptr)(char *, size_t, const char *, va_list);
    if (__builtin_expect(!fptr, 0)) fptr = R("vsnprintf");
    va_list ap; va_start(ap, fmt);
    int r = fptr(str, size, fmt, ap);
    va_end(ap);
    return r;
}

/* --- Filesystem --- */
FWD2(char *, getcwd, char *, size_t)
FWD3(ssize_t, readlink, const char *, char *, size_t)
FWD2(char *, realpath, const char *, char *)

/* --- Process / System --- */
void abort(void) __attribute__((noreturn));
void abort(void) {
    static void (*fptr)(void) __attribute__((noreturn));
    if (__builtin_expect(!fptr, 0)) fptr = R("abort");
    fptr();
    __builtin_unreachable();
}

FWD1(char *, getenv, const char *)
FWD1(long, sysconf, int)

long syscall(long num, ...) {
    static long (*fptr)(long, ...);
    if (__builtin_expect(!fptr, 0)) fptr = R("syscall");
    va_list ap; va_start(ap, num);
    long a1 = va_arg(ap, long);
    long a2 = va_arg(ap, long);
    long a3 = va_arg(ap, long);
    long a4 = va_arg(ap, long);
    long a5 = va_arg(ap, long);
    long a6 = va_arg(ap, long);
    va_end(ap);
    return fptr(num, a1, a2, a3, a4, a5, a6);
}

FWD2(int, clock_gettime, clockid_t, struct timespec *)
FWD1(unsigned long, getauxval, unsigned long)

int dl_iterate_phdr(int (*cb)(struct dl_phdr_info *, size_t, void *), void *data) {
    static int (*fptr)(int (*)(struct dl_phdr_info *, size_t, void *), void *);
    if (__builtin_expect(!fptr, 0)) fptr = R("dl_iterate_phdr");
    return fptr(cb, data);
}

FWD3(int, sched_getaffinity, pid_t, size_t, cpu_set_t *)

/* --- getrandom / gettid / statx (with syscall fallback) --- */
ssize_t getrandom(void *buf, size_t buflen, unsigned int flags) {
    static ssize_t (*fptr)(void *, size_t, unsigned int);
    if (__builtin_expect(!fptr, 0)) fptr = R("getrandom");
    if (fptr) return fptr(buf, buflen, flags);
    static long (*b_syscall)(long, ...);
    if (!b_syscall) b_syscall = R("syscall");
    return b_syscall(SYS_getrandom, buf, buflen, flags);
}

pid_t gettid(void) {
    static pid_t (*fptr)(void);
    if (__builtin_expect(!fptr, 0)) fptr = R("gettid");
    if (fptr) return fptr();
    static long (*b_syscall)(long, ...);
    if (!b_syscall) b_syscall = R("syscall");
    return (pid_t)b_syscall(SYS_gettid);
}

struct statx;
int statx(int dirfd, const char *pathname, int flags, unsigned int mask, struct statx *buf) {
    static int (*fptr)(int, const char *, int, unsigned int, struct statx *);
    if (__builtin_expect(!fptr, 0)) fptr = R("statx");
    if (fptr) return fptr(dirfd, pathname, flags, mask, buf);
    static long (*b_syscall)(long, ...);
    if (!b_syscall) b_syscall = R("syscall");
    return (int)b_syscall(SYS_statx, dirfd, pathname, flags, mask, buf);
}

/* stderr asm alias */
__asm__(".global stderr\n.set stderr, shim_stderr_ptr");
