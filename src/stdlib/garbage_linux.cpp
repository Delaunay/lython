#include "garbage.h"


#if __has_include(<pthread.h>)
#include <pthread.h>
#define BUILD_LINUX 1
#endif

namespace lython {

#if BUILD_LINUX
void BoehmGarbageCollector::mark_registers(GCGen gen) {

#if BUILD_WEBASSEMBLY

#else
    void* registers[16];

    // Inline assembly to capture register values
    __asm__ volatile (
        "movq %%rax, %0\n"
        "movq %%rbx, %1\n"
        "movq %%rcx, %2\n"
        "movq %%rdx, %3\n"
        "movq %%rsi, %4\n"
        "movq %%rdi, %5\n"
        "movq %%rbp, %6\n"
        "movq %%rsp, %7\n"
        "movq %%r8, %8\n"
        "movq %%r9, %9\n"
        "movq %%r10, %10\n"
        "movq %%r11, %11\n"
        "movq %%r12, %12\n"
        "movq %%r13, %13\n"
        "movq %%r14, %14\n"
        "movq %%r15, %15\n"
        : "=g"(registers[0]), "=g"(registers[1]), "=g"(registers[2]), "=g"(registers[3]),
          "=g"(registers[4]), "=g"(registers[5]), "=g"(registers[6]), "=g"(registers[7]),
          "=g"(registers[8]), "=g"(registers[9]), "=g"(registers[10]), "=g"(registers[11]),
          "=g"(registers[12]), "=g"(registers[13]), "=g"(registers[14]), "=g"(registers[15])
        :  // No input operands
        : "memory" // Clobbers memory to avoid reordering
    );

    // Check each register to see if it contains a pointer
    for (int i = 0; i < 16; ++i) {
        // relocating registers mean changing their values
        if (is_pointer(gen, registers[i])) {
            mark_obj(registers[i], gen, Mark::Register);
        }
    }
#endif
}
void BoehmGarbageCollector::mark_stack(GCGen gen) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // Get the thread attributes for the current thread
    pthread_getattr_np(pthread_self(), &attr);

    void *stack_base;
    size_t stack_size;

    // Get the stack base and stack size
    pthread_attr_getstack(&attr, &stack_base, &stack_size);

    // Calculate the stack bounds
    void **stack_pointer = (void**)__builtin_frame_address(0);
    void **stack_end = (void**)((char*)stack_base + stack_size);

    // Iterate over the stack
    for (void **current = stack_pointer; current < stack_end; ++current) {
        possible_pointer(gen, current, Mark::Stack);
    }

    // Clean up
    pthread_attr_destroy(&attr);
}

void BoehmGarbageCollector::mark_globals(GCGen gen) {
    FILE *maps = fopen("/proc/self/maps", "r");
    if (!maps) {
        kwdebug(outlog(), "could not open file to scan for globals");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), maps)) {
        unsigned long start, end;
        char perms[5];
        char pathname[256] = "";

        if (sscanf(line, "%lx-%lx %4s %*s %*s %*s %255s", &start, &end, perms, pathname) < 4) {
            continue;
        }

        // Check if the section is writable and belongs to the main executable or shared libraries
        if (strchr(perms, 'w') != NULL && strstr(pathname, "[heap]") == NULL) {
            // Scan each potential pointer location
            for (void **current = (void **)start; current < (void **)end; ++current) {
                possible_pointer(gen, current, Mark::Global);
            }
        }
    }
    fclose(maps);
}

void BoehmGarbageCollector::get_heap_bounds() {
    FILE *maps = fopen("/proc/self/maps", "r");
    if (!maps) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    unsigned long lowestAddress = (unsigned long)-1;
    unsigned long highestAddress = 0;
    char line[256];

    while (fgets(line, sizeof(line), maps)) {
        unsigned long start, end;
        char perms[5], path[256];

        // Initialize path to empty string
        path[0] = '\0';

        // Read the memory region line
        if (sscanf(line, "%lx-%lx %4s %*s %*s %*s %255s", &start, &end, perms, path) < 3) {
            continue;
        }

        // Look for heap regions which usually have "[heap]" in their path
        if (strstr(path, "[heap]") != NULL) {
            if (start < lowestAddress) {
                lowestAddress = start;
            }
            if (end > highestAddress) {
                highestAddress = end;
            }
        }
    }

    fclose(maps);

    heap_start = (void*)lowestAddress;
    heap_end = (void*)highestAddress;
}
#endif

}