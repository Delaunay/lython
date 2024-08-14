

/*
#if KMETA_PROCESSING
template<typename... Args>
void kmeta_annotation(Args...) {}

#define KMETA(...) kmeta_annotation(__VA_ARGS__);
#else
#define KMETA(...)
#endif

KMETA(a, b, c)
struct MyStruct {

    KMETA(1, 2, 3)
    int a;
};
//*/


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

struct KSTRUCT(a, b, c) MyStruct1 {

    int a;

    KPROPERTY(FirstFlag, SomeValue=Elaborate, SecondFlag)
    int b;

    KPROPERTY(reflected=0, SomeValue=Elaborate, SecondFlag)
    int d;

    int c;

    KFUNCTION()
    void  fun() {}
};




