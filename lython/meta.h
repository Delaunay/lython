

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
#define KMETA2(...) __attribute__((annotate(#__VA_ARGS__)))
#else
#define KMETA2(...)
#endif

struct KMETA2(a, b, c) MyStruct1 {

    int a;


    KMETA2(1, 2, 3)
    int b;


    int c;

};




