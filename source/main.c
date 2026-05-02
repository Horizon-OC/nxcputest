// TODO: improve the formatting
#include "stress-cpu.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include <time.h>

volatile bool stress_keep_running = true;

#define ITERS_PER_METHOD 5
#define MAX_FAILS_PER_LOOP 16
#define NUM_THREADS 3
#define THREAD_STACK_SIZE (256 * 1024)
#define THREAD_PRIO 0x2C

typedef struct {
  int core;
  size_t n_methods;
  volatile bool *stop;
  uint32_t lpass;
  uint32_t lfail;
  const char *failed[MAX_FAILS_PER_LOOP];
  uint32_t nfailed;
  volatile int *running_count;
} thread_arg_t;

static void stress_mwc_seed_for_core(int core);

static void stress_thread_func(void *arg_) {
  thread_arg_t *a = (thread_arg_t *)arg_;

  stress_mwc_seed_for_core(a->core);

  for (size_t i = 1; i < a->n_methods; i++) {
    if (*a->stop)
      break;

    const stress_cpu_method_info_t *m = &stress_cpu_methods[i];
    int rc = EXIT_SUCCESS;
    for (int k = 0; k < ITERS_PER_METHOD && rc == EXIT_SUCCESS; k++)
      rc = m->func(m->name);

    if (rc == EXIT_SUCCESS) {
      a->lpass++;
    } else {
      a->lfail++;
      if (a->nfailed < MAX_FAILS_PER_LOOP)
        a->failed[a->nfailed++] = m->name;
    }
  }

  __atomic_fetch_sub(a->running_count, 1, __ATOMIC_RELEASE);
}

static double time_now(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}
#include "stress-ng-compat.h"

static void stress_mwc_seed_for_core(int core) {
  uint32_t mix = (uint32_t)core * 0x9e3779b9u;
  stress_mwc_seed(0xdeadbeef ^ mix, 0x12345678 ^ ~mix);
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  consoleInit(NULL);
  padConfigureInput(1, HidNpadStyleSet_NpadStandard);

  PadState pad;
  padInitializeDefault(&pad);

  size_t n_methods = 0;
  while (stress_cpu_methods[n_methods].name)
    n_methods++;

start:
  printf("\033[2J\033[H");
  printf("nxcputest by Souldbminer and ColinIanKing\n");
  printf("  Press \033[1;32mA\033[0m to start");
  printf("  Press \033[1;31mB\033[0m to exit\n");
  consoleUpdate(NULL);

  while (appletMainLoop()) {
    padUpdate(&pad);
    u64 kDown = padGetButtonsDown(&pad);
    if (kDown & HidNpadButton_A)
      break;
    if (kDown & HidNpadButton_B)
      goto exit;
    consoleUpdate(NULL);
  }

  printf("\033[2J\033[H");

  consoleUpdate(NULL);

  uint64_t loop = 0;
  uint64_t totalPassed = 0;
  uint64_t totalFailed = 0;

  Thread threads[NUM_THREADS];
  thread_arg_t args[NUM_THREADS];
  volatile bool stop_flag = false;
  volatile int running = 0;

  while (appletMainLoop()) {
    stop_flag = false;
    running = NUM_THREADS;

    for (int t = 0; t < NUM_THREADS; t++) {
        args[t] = (thread_arg_t){
            .core = t,
            .n_methods = n_methods,
            .stop = &stop_flag,
            .lpass = 0,
            .lfail = 0,
            .nfailed = 0,
            .running_count = &running,
        };
        threadCreate(&threads[t], stress_thread_func, &args[t], NULL,
                    THREAD_STACK_SIZE, THREAD_PRIO, t);
        threadStart(&threads[t]);
    }

    double t_loop = time_now();

    while (__atomic_load_n(&running, __ATOMIC_ACQUIRE) > 0) {
        padUpdate(&pad);
        if (padGetButtonsDown(&pad) & HidNpadButton_B)
            stop_flag = true;
        svcSleepThread(8000000LL); // 8ms, avoid too much looping
        consoleUpdate(NULL);
    }

    double ms = (time_now() - t_loop) * 1000.0;

    for (int t = 0; t < NUM_THREADS; t++) {
        threadWaitForExit(&threads[t]);
        threadClose(&threads[t]);
    }

    uint32_t lpass = 0, lfail = 0;
    const char *failed[MAX_FAILS_PER_LOOP];
    uint32_t nfailed = 0;

    for (int t = 0; t < NUM_THREADS; t++) {
        lpass += args[t].lpass;
        lfail += args[t].lfail;
        for (uint32_t f = 0; f < args[t].nfailed && nfailed < MAX_FAILS_PER_LOOP; f++) {
            bool dup = false;
            for (uint32_t d = 0; d < nfailed; d++)
            if (failed[d] == args[t].failed[f]) {
                dup = true;
                break;
            }
            if (!dup)
            failed[nfailed++] = args[t].failed[f];
        }
    }

    loop++;
    totalPassed += lpass;
    totalFailed += lfail;

    printf("Loop \033[1m%4" PRIu64 "\033[0m  "
        "\033[1;32m%u passed\033[0m  "
        "\033[1;31m%u failed\033[0m  "
        "%6.0f ms\n",
        loop, lpass, lfail, ms);

    if(nfailed != 0)
        printf("Failed:\n");

    for (uint32_t f = 0; f < nfailed; f++)
    printf("  \033[0m %s\n", failed[f]);

    consoleUpdate(NULL);

    if (stop_flag)
    goto stopped;
    }

stopped:
  printf("\nStopped after %" PRIu64 " loop%s\n"
         "\033[1;32m%" PRIu64 " tests passed\033[0m\n"
         "\033[1;31m%" PRIu64 " tests failed\033[0m\n",
         loop, loop == 1 ? "" : "s", totalPassed, totalFailed);
  consoleUpdate(NULL);

  printf(" Press \033[1;32mA\033[0m to return to menu\n");
  printf(" Press \033[1;31mB\033[0m to exit\n");
  consoleUpdate(NULL);

  while (appletMainLoop()) {
    padUpdate(&pad);
    u64 kDown = padGetButtonsDown(&pad);
    if (kDown & HidNpadButton_A)
      goto start;
    if (kDown & HidNpadButton_B)
      goto exit;
    consoleUpdate(NULL);
  }

exit:
  consoleExit(NULL);
  return 0;
}
