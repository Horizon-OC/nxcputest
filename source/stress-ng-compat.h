#pragma once

#include <complex.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include <time.h>

#define TARGET_CLONES
#define OPTIMIZE3 __attribute__((optimize(3)))
#define OPTIMIZE0 __attribute__((optimize(0)))
#define OPTIMIZE_FAST_MATH __attribute__((optimize("fast-math")))
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define STRINGIFY(x) #x
#define PRAGMA_UNROLL_N(n) _Pragma(STRINGIFY(GCC unroll n))

#define SIZEOF_ARRAY(a) (sizeof(a) / sizeof((a)[0]))
#define UNEXPECTED // Don't bother

#define HAVE_INT128_T
#define HAVE_COMPLEX_H
#define HAVE_COMPLEX
#define HAVE_SRAND48
#define HAVE_LRAND48
#define HAVE_DRAND48

#define OPT_FLAGS_VERIFY (1ULL << 0)
static uint64_t g_opt_flags = 0;
static uint64_t g_opt_timeout = 1;

#define pr_fail(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define pr_inf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define pr_dbg(fmt, ...)

#define HAVE_ASM_NOP
static inline void stress_asm_nop(void) { asm volatile("nop"); }

#define shim_sqrt sqrt
#define shim_rint rint
#define shim_cos cos
#define shim_cosf cosf
#define shim_sin sin
#define shim_sinf sinf
#define shim_cosh cosh
#define shim_coshf coshf
#define shim_sinh sinh
#define shim_sinhf sinhf
#define shim_fabs fabs
#define shim_cexp cexp
#define shim_log log
#define shim_logf logf
#define shim_exp exp
#define shim_expf expf
#define shim_pow pow
#define shim_powf powf
#define shim_cpow cpow
#define shim_memcpy memcpy

#define shim_csinf csinf
#define shim_ccosf ccosf
#define shim_csin csin
#define shim_ccos ccos

static inline long double shim_sqrtl(long double x) {
  return (long double)sqrt((double)x);
}
static inline long double shim_rintl(long double x) {
  return (long double)rint((double)x);
}
static inline long double shim_cosl(long double x) {
  return (long double)cos((double)x);
}
static inline long double shim_sinl(long double x) {
  return (long double)sin((double)x);
}
static inline long double shim_coshl(long double x) {
  return (long double)cosh((double)x);
}
static inline long double shim_sinhl(long double x) {
  return (long double)sinh((double)x);
}
static inline long double shim_fabsl(long double x) {
  return (long double)fabs((double)x);
}
static inline long double shim_logl(long double x) {
  return (long double)log((double)x);
}
static inline long double shim_expl(long double x) {
  return (long double)exp((double)x);
}
static inline long double shim_powl(long double b, long double e) {
  return (long double)pow((double)b, (double)e);
}
static inline long double shim_lgammal(long double x) {
  return (long double)lgamma((double)x);
}
static inline long double shim_roundl(long double x) {
  return (long double)round((double)x);
}

static inline long double complex shim_csinl(long double complex z) {
  return (long double complex)csin((double complex)z);
}
static inline long double complex shim_ccosl(long double complex z) {
  return (long double complex)ccos((double complex)z);
}
static inline double shim_cabsl(long double complex z) {
  return cabs((double complex)z);
}

/* Memory shims */
#define shim_memset memset

/* Extra compiler attributes */
#define CONST __attribute__((const))
#define ALWAYS_INLINE __attribute__((always_inline))
#define ALIGN64 __attribute__((aligned(64)))

#define STRESS_MINIMUM(a, b) ((a) < (b) ? (a) : (b))
#define STRESS_MAXIMUM(a, b) ((a) > (b) ? (a) : (b))

#define STRESS_GETBIT(a, i) (((uint8_t *)(a))[(i) >> 3] & (1u << ((i) & 7u)))
#define STRESS_CLRBIT(a, i) (((uint8_t *)(a))[(i) >> 3] &= ~(1u << ((i) & 7u)))

static inline void shim_nanosleep_uint64(uint64_t ns) {
  svcSleepThread((s64)ns);
}

static uint32_t mwc_w = 0xdeadbeef;
static uint32_t mwc_z = 0x12345678;

static inline void stress_mwc_seed_default(void) {
  mwc_w = 0xdeadbeef;
  mwc_z = 0x12345678;
}

static inline uint32_t stress_mwc32(void) {
  mwc_z = 36969u * (mwc_z & 65535u) + (mwc_z >> 16);
  mwc_w = 18000u * (mwc_w & 65535u) + (mwc_w >> 16);
  return (mwc_z << 16) + mwc_w;
}

static inline uint64_t stress_mwc64(void) {
  return ((uint64_t)stress_mwc32() << 32) | (uint64_t)stress_mwc32();
}

static inline uint16_t stress_mwc16(void) { return (uint16_t)stress_mwc32(); }

static inline uint8_t stress_mwc8(void) { return (uint8_t)stress_mwc32(); }

static inline uint64_t stress_mwc64modn(uint64_t n) {
  return n ? (stress_mwc64() % n) : 0;
}

static inline uint32_t stress_bitops_bitreverse32(uint32_t x) {
  x = ((x >> 1) & 0x55555555u) | ((x & 0x55555555u) << 1);
  x = ((x >> 2) & 0x33333333u) | ((x & 0x33333333u) << 2);
  x = ((x >> 4) & 0x0f0f0f0fu) | ((x & 0x0f0f0f0fu) << 4);
  x = ((x >> 8) & 0x00ff00ffu) | ((x & 0x00ff00ffu) << 8);
  return (x >> 16) | (x << 16);
}

static inline uint32_t stress_bitops_parity32(uint32_t x) {
  x ^= x >> 16;
  x ^= x >> 8;
  x ^= x >> 4;
  x &= 0xfu;
  return (0x6996u >> x) & 1u;
}

static inline uint32_t stress_bitops_popcount32(uint32_t x) {
  return (uint32_t)__builtin_popcount(x);
}

static inline uint32_t stress_bitops_nextpwr2(uint32_t x) {
  if (x == 0)
    return 1u;
  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x + 1u;
}

static volatile double _sink_d;
static volatile float _sink_f;
static volatile uint64_t _sink_u64;
static volatile uint32_t _sink_u32;
static volatile uint16_t _sink_u16;

static inline void stress_put_double(double v) { _sink_d = v; }
static inline void stress_put_float(float v) { _sink_f = v; }
static inline void stress_put_long_double(long double v) {
  _sink_d = (double)v;
}
static inline void stress_put_uint64(uint64_t v) { _sink_u64 = v; }
static inline void stress_put_uint32(uint32_t v) { _sink_u32 = v; }
static inline void stress_put_uint16(uint16_t v) { _sink_u16 = v; }
static inline void stress_put_uint8(uint8_t v) { _sink_u32 = v; }

#include "stress-cpu.h"

typedef struct {
  const char *name;
} stress_args_t;

extern volatile bool stress_keep_running;

static inline bool stress_continue(stress_args_t *args) {
  (void)args;
  return stress_keep_running;
}

static inline bool stress_continue_flag(void) { return stress_keep_running; }

static inline void stress_bogo_set(stress_args_t *args, uint64_t v) {
  (void)args;
  (void)v;
}

static inline void stress_signal_catch_sigill(void) {}

static inline bool stress_setting_get(const char *k, void *v) {
  (void)k;
  (void)v;
  return false;
}

static inline bool stress_instance_zero(stress_args_t *args) {
  (void)args;
  return true;
}

#define STRESS_STATE_DEINIT 0
#define STRESS_STATE_SYNC_WAIT 1
#define STRESS_STATE_RUN 2

static inline void stress_proc_state_set(const char *n, int s) {
  (void)n;
  (void)s;
}
static inline void stress_sync_start_wait(stress_args_t *args) { (void)args; }
static inline void stress_cpu_fp_subnormals_disable(void) {}
static inline void stress_cpu_fp_subnormals_enable(void) {}

static inline unsigned int sleep(unsigned int seconds) {
  svcSleepThread((s64)seconds * 1000000000LL);
  return 0;
}

static inline uint16_t stress_net_ipv4_checksum(const void *buf, size_t len) {
  uint32_t sum = 0;
  const uint16_t *p = (const uint16_t *)buf;
  while (len > 1) {
    sum += *p++;
    len -= 2;
  }
  if (len)
    sum += *(const uint8_t *)p;
  while (sum >> 16)
    sum = (sum & 0xffffu) + (sum >> 16);
  return (uint16_t)~sum;
}

#define STRESS_DBL_NANOSECOND 1.0e9
#define STRESS_DBL_MICROSECOND 1.0e6
#define STRESS_DBL_MILLISECOND 1.0e3

static inline double stress_time_now(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec * 1.0e-9;
}

static inline uint32_t stress_cpus_online_get(void) { return 4; }
typedef struct {
  const char *opt_s;
  const char *opt_l;
  const char *description;
} stress_help_t;

typedef uint64_t opt_val_t;
typedef struct {
  int opt_id;
  const char *name;
  int type_id;
  uint64_t min_val;
  uint64_t max_val;
  void *fn;
} stress_opt_t;

enum {
  OPT_cpu_load = 1,
  OPT_cpu_load_slice,
  OPT_cpu_method,
  OPT_cpu_old_metrics,
};

#define TYPE_ID_INT32 1
#define TYPE_ID_SIZE_T_METHOD 2
#define TYPE_ID_BOOL 3
#define END_OPT {0, NULL, 0, 0, 0, NULL}

#define CLASS_CPU (1u << 0)
#define CLASS_COMPUTE (1u << 1)
#define VERIFY_OPTIONAL 0

typedef int (*stress_stressor_fn)(stress_args_t *args);

typedef struct {
  stress_stressor_fn stressor;
  unsigned classifier;
  const stress_opt_t *opts;
  int verify;
  const void *help;
} stressor_info_t;
