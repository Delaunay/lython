#include "ast/nodes.h"
#include "dtypes.h"
#include "utilities/names.h"

#if KMETA_PROCESSING
#define KMETA(...) __attribute__((annotate(#__VA_ARGS__)))
#else
#define KMETA(...)
#endif

#define KCLASS(...)    KMETA(class, __VA_ARGS__)
#define KSTRUCT(...)   KMETA(struct, __VA_ARGS__)
#define KPROPERTY(...) KMETA(proprety, __VA_ARGS__)
#define KFUNCTION(...) KMETA(function, __VA_ARGS__)
#define KIGNORE(...)   KMETA(__VA_ARGS__, reflected = 0)

struct KSTRUCT(a, b, c) MyStruct1 {

    int a;

    KPROPERTY(FirstFlag, SomeValue = Elaborate, SecondFlag)
    int b;

    KPROPERTY(reflected = 0, SomeValue = Elaborate, SecondFlag)
    int d;

    int c;

    KFUNCTION()
    void fun() {}
};

namespace lython {
template <>
struct meta::ReflectionTrait<MyStruct1> {
    static int register_members() {
        meta::register_member<&MyStruct1::a>("a");
        meta::register_member<&MyStruct1::b>("b");
        meta::register_member<&MyStruct1::c>("c");
        meta::register_member<&MyStruct1::fun>("fun");
        return 1;
    }
};
}  // namespace lython
