#pragma once
#include <stddef.h>

typedef int (*stress_cpu_func)(const char *name);

typedef struct {
    const char        *name;
    stress_cpu_func    func;
    double             bogo_op_rate;
} stress_cpu_method_info_t;

extern const stress_cpu_method_info_t stress_cpu_methods[];
extern const size_t stress_cpu_methods_count;
