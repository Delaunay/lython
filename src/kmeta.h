#pragma once

#if KMETA_PROCESSING
#define KMETA(...) __attribute__((annotate(#__VA_ARGS__)))
#else
#define KMETA(...)
#endif

#define KCLASS(...)    KMETA(class, __VA_ARGS__)
#define KSTRUCT(...)   KMETA(struct, __VA_ARGS__)
#define KPROPERTY(...) KMETA(proprety, __VA_ARGS__)
#define KFUNCTION(...) KMETA(function, __VA_ARGS__)
#define KIGNORE(...)   KMETA(__VA_ARGS__, reflected=0)

