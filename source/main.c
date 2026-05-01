/* Nintendo Switch CPU stress test – libnx port of stress-ng stress-cpu */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>
#include <switch.h>
#include "stress-cpu.h"

/* Loop-control flag consumed by stress-ng-compat.h stubs */
volatile bool stress_keep_running = true;

#define ITERS_PER_METHOD  5
#define MAX_FAILS_PER_LOOP 16   /* cap on how many failure names we store */

static double time_now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

int main(int argc, char *argv[])
{
    // Avoid errors
    (void)argc;
    (void)argv;

    consoleInit(NULL);

    // Standard controller
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeDefault(&pad);

    // Count number of tests
    size_t n_methods = 0;
    while (stress_cpu_methods[n_methods].name)
        n_methods++;

    start:
    printf("\033[2J\033[H"); // For some ungodly reason libnx doesnt let you exit the console, so use this

    printf("nxcputest by Souldbminer and ColinIanKing\n");
    printf("  Press \033[1;32mA\033[0m to start\n");
    printf("  Press \033[1;31mB\033[0m to exit\n");
    consoleUpdate(NULL);

    // Wait for A or B to be pressed
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

    while (appletMainLoop()) {
        uint32_t lpass = 0, lfail = 0;
        const char *failed[MAX_FAILS_PER_LOOP];
        uint32_t nfailed = 0;

        double t_loop = time_now();

        for (size_t i = 1; i < n_methods; i++) {
            padUpdate(&pad);
            if (padGetButtonsDown(&pad) & HidNpadButton_B)
                goto stopped;

            const stress_cpu_method_info_t *m = &stress_cpu_methods[i];
            int rc = EXIT_SUCCESS;
            for (int k = 0; k < ITERS_PER_METHOD && rc == EXIT_SUCCESS; k++)
                rc = m->func(m->name);

            if (rc == EXIT_SUCCESS) {
                lpass++;
            } else {
                lfail++;
                if (nfailed < MAX_FAILS_PER_LOOP)
                    failed[nfailed++] = m->name;
            }
        }

        double ms = (time_now() - t_loop) * 1000.0;
        loop++;
        totalPassed += lpass;
        totalFailed += lfail;

        // Keep alignment
        printf("Loop \033[1m%4" PRIu64 "\033[0m  \033[1;32m%zu passed\033[0m  \033[1;31m%zu failed\033[0m  %6.0f ms\n", loop, (size_t)lpass, (size_t)lfail, ms);

        // List failures if they happened
        for (uint32_t f = 0; f < nfailed; f++)
            printf("         \033[1;31m✗\033[0m %s\n", failed[f]);

        consoleUpdate(NULL);
    }

stopped:
    printf("\nStopped after %" PRIu64 " loop%s\n\033[1;32m%" PRIu64 " tests passed\033[0m\n\033[1;31m%" PRIu64 " tests failed\033[0m\n", loop, loop == 1 ? "" : "s", totalPassed, totalFailed);
    consoleUpdate(NULL);

    // Wait for B
    printf("  Press \033[1;32mA\033[0m to return to menu\n");
    printf("  Press \033[1;31mB\033[0m to exit\n");
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
