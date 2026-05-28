/*
 * glibc-to-bionic shim: libasound.so.2
 *
 * Provides ALSA_0.9 / ALSA_0.9.0rc4 / ALSA_0.9.0rc8 versioned symbols
 * by forwarding to Termux's alsa-lib (libasound.so).
 *
 * The .node file expects libasound.so.2 with versioned ALSA symbols.
 * Termux's alsa-lib has SONAME libasound.so and no ALSA version tags.
 * This shim bridges the gap.
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stddef.h>
#include <stdint.h>

/* Handle to real libasound */
static void *alsa_handle;

__attribute__((constructor))
static void init_alsa(void) {
    /* Try to load termux's libasound.so */
    alsa_handle = dlopen("libasound.so", RTLD_LAZY | RTLD_GLOBAL);
    if (!alsa_handle) {
        /* Try with full path */
        alsa_handle = dlopen("/data/data/com.termux/files/usr/lib/libasound.so", RTLD_LAZY | RTLD_GLOBAL);
    }
}

static void *alsa_resolve(const char *name) {
    if (!alsa_handle) return NULL;
    return dlsym(alsa_handle, name);
}

/* Opaque ALSA types - we just pass pointers through */
typedef void snd_pcm_t;
typedef void snd_pcm_hw_params_t;
typedef void snd_pcm_sw_params_t;
typedef void snd_pcm_status_t;
typedef long snd_pcm_sframes_t;
typedef unsigned long snd_pcm_uframes_t;
typedef int snd_pcm_stream_t;
typedef int snd_pcm_access_t;
typedef int snd_pcm_format_t;

/* timespec for htstamp */
#include <time.h>
#include <poll.h>

/*
 * ALSA_0.9 symbols
 */
int snd_pcm_open(snd_pcm_t **pcm, const char *name, snd_pcm_stream_t stream, int mode) {
    static int (*fptr)(snd_pcm_t **, const char *, snd_pcm_stream_t, int);
    if (!fptr) fptr = alsa_resolve("snd_pcm_open");
    if (!fptr) return -1;
    return fptr(pcm, name, stream, mode);
}

int snd_pcm_close(snd_pcm_t *pcm) {
    static int (*fptr)(snd_pcm_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_close");
    if (!fptr) return -1;
    return fptr(pcm);
}

int snd_pcm_prepare(snd_pcm_t *pcm) {
    static int (*fptr)(snd_pcm_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_prepare");
    if (!fptr) return -1;
    return fptr(pcm);
}

int snd_pcm_start(snd_pcm_t *pcm) {
    static int (*fptr)(snd_pcm_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_start");
    if (!fptr) return -1;
    return fptr(pcm);
}

int snd_pcm_pause(snd_pcm_t *pcm, int enable) {
    static int (*fptr)(snd_pcm_t *, int);
    if (!fptr) fptr = alsa_resolve("snd_pcm_pause");
    if (!fptr) return -1;
    return fptr(pcm, enable);
}

int snd_pcm_recover(snd_pcm_t *pcm, int err, int silent) {
    static int (*fptr)(snd_pcm_t *, int, int);
    if (!fptr) fptr = alsa_resolve("snd_pcm_recover");
    if (!fptr) return -1;
    return fptr(pcm, err, silent);
}

snd_pcm_sframes_t snd_pcm_readi(snd_pcm_t *pcm, void *buffer, snd_pcm_uframes_t size) {
    static snd_pcm_sframes_t (*fptr)(snd_pcm_t *, void *, snd_pcm_uframes_t);
    if (!fptr) fptr = alsa_resolve("snd_pcm_readi");
    if (!fptr) return -1;
    return fptr(pcm, buffer, size);
}

snd_pcm_sframes_t snd_pcm_writei(snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size) {
    static snd_pcm_sframes_t (*fptr)(snd_pcm_t *, const void *, snd_pcm_uframes_t);
    if (!fptr) fptr = alsa_resolve("snd_pcm_writei");
    if (!fptr) return -1;
    return fptr(pcm, buffer, size);
}

snd_pcm_sframes_t snd_pcm_avail(snd_pcm_t *pcm) {
    static snd_pcm_sframes_t (*fptr)(snd_pcm_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_avail");
    if (!fptr) return -1;
    return fptr(pcm);
}

snd_pcm_sframes_t snd_pcm_bytes_to_frames(snd_pcm_t *pcm, ssize_t bytes) {
    static snd_pcm_sframes_t (*fptr)(snd_pcm_t *, ssize_t);
    if (!fptr) fptr = alsa_resolve("snd_pcm_bytes_to_frames");
    if (!fptr) return -1;
    return fptr(pcm, bytes);
}

int snd_pcm_get_params(snd_pcm_t *pcm, snd_pcm_uframes_t *buffer_size, snd_pcm_uframes_t *period_size) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_uframes_t *, snd_pcm_uframes_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_get_params");
    if (!fptr) return -1;
    return fptr(pcm, buffer_size, period_size);
}

int snd_pcm_poll_descriptors_count(snd_pcm_t *pcm) {
    static int (*fptr)(snd_pcm_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_poll_descriptors_count");
    if (!fptr) return 0;
    return fptr(pcm);
}

int snd_pcm_poll_descriptors(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int space) {
    static int (*fptr)(snd_pcm_t *, struct pollfd *, unsigned int);
    if (!fptr) fptr = alsa_resolve("snd_pcm_poll_descriptors");
    if (!fptr) return -1;
    return fptr(pcm, pfds, space);
}

int snd_pcm_poll_descriptors_revents(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int nfds, unsigned short *revents) {
    static int (*fptr)(snd_pcm_t *, struct pollfd *, unsigned int, unsigned short *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_poll_descriptors_revents");
    if (!fptr) return -1;
    return fptr(pcm, pfds, nfds, revents);
}

/* hw_params family */
int snd_pcm_hw_params_malloc(snd_pcm_hw_params_t **ptr) {
    static int (*fptr)(snd_pcm_hw_params_t **);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_malloc");
    if (!fptr) return -1;
    return fptr(ptr);
}

void snd_pcm_hw_params_free(snd_pcm_hw_params_t *obj) {
    static void (*fptr)(snd_pcm_hw_params_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_free");
    if (fptr) fptr(obj);
}

int snd_pcm_hw_params_any(snd_pcm_t *pcm, snd_pcm_hw_params_t *params) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_hw_params_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_any");
    if (!fptr) return -1;
    return fptr(pcm, params);
}

int snd_pcm_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_hw_params_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params");
    if (!fptr) return -1;
    return fptr(pcm, params);
}

int snd_pcm_hw_params_can_pause(snd_pcm_hw_params_t *params) {
    static int (*fptr)(snd_pcm_hw_params_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_can_pause");
    if (!fptr) return 0;
    return fptr(params);
}

int snd_pcm_hw_params_set_access(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_access_t access) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_hw_params_t *, snd_pcm_access_t);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_set_access");
    if (!fptr) return -1;
    return fptr(pcm, params, access);
}

int snd_pcm_hw_params_set_format(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_format_t format) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_hw_params_t *, snd_pcm_format_t);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_set_format");
    if (!fptr) return -1;
    return fptr(pcm, params, format);
}

int snd_pcm_hw_params_set_channels(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_set_channels");
    if (!fptr) return -1;
    return fptr(pcm, params, val);
}

int snd_pcm_hw_params_set_rate(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val, int dir) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int, int);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_set_rate");
    if (!fptr) return -1;
    return fptr(pcm, params, val, dir);
}

int snd_pcm_hw_params_set_buffer_size(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t val) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_hw_params_t *, snd_pcm_uframes_t);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_set_buffer_size");
    if (!fptr) return -1;
    return fptr(pcm, params, val);
}

int snd_pcm_hw_params_test_channels(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_test_channels");
    if (!fptr) return -1;
    return fptr(pcm, params, val);
}

int snd_pcm_hw_params_test_format(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_format_t val) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_hw_params_t *, snd_pcm_format_t);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_test_format");
    if (!fptr) return -1;
    return fptr(pcm, params, val);
}

int snd_pcm_hw_params_test_rate(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val, int dir) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int, int);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_test_rate");
    if (!fptr) return -1;
    return fptr(pcm, params, val, dir);
}

/* ALSA_0.9.0rc4 symbols (different versioned API) */
int snd_pcm_hw_params_get_buffer_size_min(const snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val) {
    static int (*fptr)(const snd_pcm_hw_params_t *, snd_pcm_uframes_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_get_buffer_size_min");
    if (!fptr) return -1;
    return fptr(params, val);
}

int snd_pcm_hw_params_get_buffer_size_max(const snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val) {
    static int (*fptr)(const snd_pcm_hw_params_t *, snd_pcm_uframes_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_get_buffer_size_max");
    if (!fptr) return -1;
    return fptr(params, val);
}

int snd_pcm_hw_params_get_channels_min(const snd_pcm_hw_params_t *params, unsigned int *val) {
    static int (*fptr)(const snd_pcm_hw_params_t *, unsigned int *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_get_channels_min");
    if (!fptr) return -1;
    return fptr(params, val);
}

int snd_pcm_hw_params_get_channels_max(const snd_pcm_hw_params_t *params, unsigned int *val) {
    static int (*fptr)(const snd_pcm_hw_params_t *, unsigned int *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_get_channels_max");
    if (!fptr) return -1;
    return fptr(params, val);
}

int snd_pcm_hw_params_get_rate_min(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir) {
    static int (*fptr)(const snd_pcm_hw_params_t *, unsigned int *, int *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_get_rate_min");
    if (!fptr) return -1;
    return fptr(params, val, dir);
}

int snd_pcm_hw_params_get_rate_max(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir) {
    static int (*fptr)(const snd_pcm_hw_params_t *, unsigned int *, int *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_get_rate_max");
    if (!fptr) return -1;
    return fptr(params, val, dir);
}

int snd_pcm_hw_params_set_buffer_time_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int *, int *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_set_buffer_time_near");
    if (!fptr) return -1;
    return fptr(pcm, params, val, dir);
}

int snd_pcm_hw_params_set_period_size_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val, int *dir) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_hw_params_t *, snd_pcm_uframes_t *, int *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_set_period_size_near");
    if (!fptr) return -1;
    return fptr(pcm, params, val, dir);
}

int snd_pcm_hw_params_set_period_time_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_hw_params_t *, unsigned int *, int *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_hw_params_set_period_time_near");
    if (!fptr) return -1;
    return fptr(pcm, params, val, dir);
}

/* sw_params family */
int snd_pcm_sw_params_malloc(snd_pcm_sw_params_t **ptr) {
    static int (*fptr)(snd_pcm_sw_params_t **);
    if (!fptr) fptr = alsa_resolve("snd_pcm_sw_params_malloc");
    if (!fptr) return -1;
    return fptr(ptr);
}

void snd_pcm_sw_params_free(snd_pcm_sw_params_t *obj) {
    static void (*fptr)(snd_pcm_sw_params_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_sw_params_free");
    if (fptr) fptr(obj);
}

int snd_pcm_sw_params_current(snd_pcm_t *pcm, snd_pcm_sw_params_t *params) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_sw_params_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_sw_params_current");
    if (!fptr) return -1;
    return fptr(pcm, params);
}

int snd_pcm_sw_params(snd_pcm_t *pcm, snd_pcm_sw_params_t *params) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_sw_params_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_sw_params");
    if (!fptr) return -1;
    return fptr(pcm, params);
}

int snd_pcm_sw_params_set_avail_min(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_sw_params_t *, snd_pcm_uframes_t);
    if (!fptr) fptr = alsa_resolve("snd_pcm_sw_params_set_avail_min");
    if (!fptr) return -1;
    return fptr(pcm, params, val);
}

int snd_pcm_sw_params_set_start_threshold(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_sw_params_t *, snd_pcm_uframes_t);
    if (!fptr) fptr = alsa_resolve("snd_pcm_sw_params_set_start_threshold");
    if (!fptr) return -1;
    return fptr(pcm, params, val);
}

int snd_pcm_sw_params_set_tstamp_mode(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, int val) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_sw_params_t *, int);
    if (!fptr) fptr = alsa_resolve("snd_pcm_sw_params_set_tstamp_mode");
    if (!fptr) return -1;
    return fptr(pcm, params, val);
}

int snd_pcm_sw_params_set_tstamp_type(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, int val) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_sw_params_t *, int);
    if (!fptr) fptr = alsa_resolve("snd_pcm_sw_params_set_tstamp_type");
    if (!fptr) return -1;
    return fptr(pcm, params, val);
}

/* status */
int snd_pcm_status(snd_pcm_t *pcm, snd_pcm_status_t *status) {
    static int (*fptr)(snd_pcm_t *, snd_pcm_status_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_status");
    if (!fptr) return -1;
    return fptr(pcm, status);
}

snd_pcm_sframes_t snd_pcm_status_get_delay(const snd_pcm_status_t *obj) {
    static snd_pcm_sframes_t (*fptr)(const snd_pcm_status_t *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_status_get_delay");
    if (!fptr) return 0;
    return fptr(obj);
}

size_t snd_pcm_status_sizeof(void) {
    static size_t (*fptr)(void);
    if (!fptr) fptr = alsa_resolve("snd_pcm_status_sizeof");
    if (!fptr) return 0;
    return fptr();
}

/* ALSA_0.9.0rc8 symbols */
void snd_pcm_status_get_htstamp(const snd_pcm_status_t *obj, struct timespec *ts) {
    static void (*fptr)(const snd_pcm_status_t *, struct timespec *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_status_get_htstamp");
    if (fptr) fptr(obj, ts);
}

void snd_pcm_status_get_trigger_htstamp(const snd_pcm_status_t *obj, struct timespec *ts) {
    static void (*fptr)(const snd_pcm_status_t *, struct timespec *);
    if (!fptr) fptr = alsa_resolve("snd_pcm_status_get_trigger_htstamp");
    if (fptr) fptr(obj, ts);
}
