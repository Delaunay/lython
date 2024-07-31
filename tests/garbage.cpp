
//
// GC
//


#include <cstdio>
#include <cstdlib>
#include <cstring>

void register_root_range(void *start, void *end);
void identify_global_roots();
bool is_pointer_in_root_set(void *ptr);
void identify_stack_roots();


void identify_global_roots() {
    FILE *maps = fopen("/proc/self/maps", "r");
    if (!maps) {
        perror("fopen");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), maps)) {
        if (strstr(line, " rw-p ") && !strstr(line, "stack")) {
            unsigned long start, end;
            sscanf(line, "%lx-%lx", &start, &end);
            // Register this range as a potential root set
            register_root_range((void *)start, (void *)end);
        }
    }
    fclose(maps);
}

typedef struct RootRange {
    void *start;
    void *end;
    struct RootRange *next;
} RootRange;

RootRange *root_ranges = NULL;

void register_root_range(void *start, void *end) {
    printf("Found root\n");
    RootRange *range = (RootRange *)malloc(sizeof(RootRange));
    range->start = start;
    range->end = end;
    range->next = root_ranges;
    root_ranges = range;
}

bool is_pointer_in_root_set(void *ptr) {
    RootRange *range = root_ranges;
    while (range) {
        if (ptr >= range->start && ptr < range->end) {
            return true;
        }
        range = range->next;
    }
    return false;
}


#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

void identify_stack_roots() {
    pthread_attr_t attr;
    pthread_getattr_np(pthread_self(), &attr);

    void *stack_addr;
    size_t stack_size;
    pthread_attr_getstack(&attr, &stack_addr, &stack_size);

    register_root_range(stack_addr, stack_addr + stack_size);

    pthread_attr_destroy(&attr);
}


struct Memheader {
    int size;
};

struct GarbageCollector {

    Memheader* header(void* ptr) {
        return (Memheader*)((char*)(ptr) - sizeof(Memheader));
    }

    GarbageCollector& instance() {
        static GarbageCollector _;
        return _;
    }

    void* memalloc(std::size_t n) {
        _alloc += 1;
        Memheader* data = (Memheader*) malloc(n + sizeof(Memheader));
        data->size = n;
        return (char*)(data) + sizeof(Memheader);
    }

    void memfree(void* ptr) {
        _free += 1;
        free(header(ptr));
    }

    int _alloc = 0;
    int _free = 0;

    
};

int main() {

    void* data = malloc(1024 * 1024);
    
    return 0;
}