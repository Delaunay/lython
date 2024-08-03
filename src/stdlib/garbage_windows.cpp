#include "garbage.h"

#if __has_include(<windows.h>)
#ifndef BUILD_WINDOWS
#define BUILD_WINDOWS 1
#endif
#include <intrin.h>
#include <windows.h>
#endif

namespace lython {

#if BUILD_WINDOWS
void BoehmGarbageCollector::mark_registers(GCGen gen) {
    void* registers[16];

    CONTEXT context;
    RtlCaptureContext(&context);

    registers[0] = (void*)context.Rax;
    registers[1] = (void*)context.Rbx;
    registers[2] = (void*)context.Rcx;
    registers[3] = (void*)context.Rdx;
    registers[4] = (void*)context.Rsi;
    registers[5] = (void*)context.Rdi;
    registers[6] = (void*)context.Rbp;
    registers[7] = (void*)context.Rsp;
    registers[8] = (void*)context.R8;
    registers[9] = (void*)context.R9;
    registers[10] = (void*)context.R10;
    registers[11] = (void*)context.R11;
    registers[12] = (void*)context.R12;
    registers[13] = (void*)context.R13;
    registers[14] = (void*)context.R14;
    registers[15] = (void*)context.R15;

    // Check each register to see if it contains a pointer
    for (int i = 0; i < 16; ++i) {
        if (is_pointer(gen, registers[i])) {
            mark_obj(registers[i], gen, Mark::Register);
        }
    }
}


bool is_aligned(void* ptr) {
    constexpr size_t alignment = alignof(void*);
    return reinterpret_cast<uintptr_t>(ptr) % alignment == 0;
}


void BoehmGarbageCollector::mark_stack(GCGen gen) {
    // Obtain stack bounds for the current thread
    CONTEXT context;
    RtlCaptureContext(&context);
    PNT_TIB pTib = (PNT_TIB)NtCurrentTeb();

    // Use Rsp for stack pointer in x86_64
    void** stack_pointer = (void**)context.Rsp;
    void** stack_base = (void**)pTib->StackBase;
    void** stack_limit = (void**)pTib->StackLimit;

    // Iterate over stack
    for (void** current = stack_pointer; current < stack_base; ++current) {
        if (current >= stack_limit && current < stack_base) { 
            if (is_pointer(gen, *current)) {
                mark_obj(*current, gen, Mark::Stack);
            }
        }
    }
}

void BoehmGarbageCollector::mark_globals(GCGen gen) {
// i.e root set
    // Get a handle to the current module (application)
    HMODULE hModule = GetModuleHandle(NULL);

    // Variables to store section info
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((char*)hModule + dosHeader->e_lfanew);

    // Get the data section
    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(ntHeaders);
    for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i, ++section) {
        if (strcmp((char*)section->Name, ".data") == 0 ||
            strcmp((char*)section->Name, ".bss") == 0) {
            void* start = (char*)hModule + section->VirtualAddress;
            void* end   = (char*)start + section->Misc.VirtualSize;

            // Scan each potential pointer location
            for (void** current = (void**)start; current < (void**)end; ++current) {
                if (is_pointer(gen, *current)) {
                    // Mark object as reachable in GC
                    mark_obj(*current, gen, Mark::Global);
                }
            }
        }
    }
}

void BoehmGarbageCollector::get_heap_bounds() {
    HANDLE             heap = GetProcessHeap();  // Get the default process heap
    PROCESS_HEAP_ENTRY entry;
    entry.lpData = NULL;

    SIZE_T lowestAddress  = (SIZE_T)-1;  // Start with the highest possible address
    SIZE_T highestAddress = 0;           // Start with the lowest possible address

    while (HeapWalk(heap, &entry) != 0) {
        if (entry.wFlags & PROCESS_HEAP_ENTRY_BUSY) {  // Check if this entry is used
            SIZE_T start = (SIZE_T)entry.lpData;
            SIZE_T end   = start + entry.cbData;

            // Update the lowest and highest addresses
            if (start < lowestAddress) {
                lowestAddress = start;
            }
            if (end > highestAddress) {
                highestAddress = end;
            }
        }
    }

    heap_start = (void*)lowestAddress;
    heap_end   = (void*)highestAddress;

    kwdebug(outlog(),
            "heap {} - {} = {}",
            heap_start,
            heap_end,
            (char*)(heap_end) - (char*)(heap_start));
}

#endif

}