/*
 * glibc-to-bionic shim: libm.so.6
 *
 * Provides GLIBC_2.17 versioned math symbols by forwarding to bionic.
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stddef.h>

static void *bionic_libm;
static int inited;

static void *RM(const char *name) {
    if (__builtin_expect(!inited, 0)) {
        bionic_libm = dlopen("libm.so", RTLD_LAZY | RTLD_NOLOAD);
        if (!bionic_libm)
            bionic_libm = dlopen("/apex/com.android.runtime/lib64/bionic/libm.so", RTLD_LAZY | RTLD_NOLOAD);
        inited = 1;
    }
    return dlsym(bionic_libm, name);
}

double pow(double base, double exp) {
    static double (*fptr)(double, double);
    if (__builtin_expect(!fptr, 0)) fptr = RM("pow");
    return fptr(base, exp);
}

float sinf(float x) {
    static float (*fptr)(float);
    if (__builtin_expect(!fptr, 0)) fptr = RM("sinf");
    return fptr(x);
}
