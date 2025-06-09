#if BUILD_WEBASSEMBLY

#include <emscripten/emscripten.h>


#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif


// emcc -o hello3.html hello3.c --shell-file html_template/shell_minimal.html\
   -s NO_EXIT_RUNTIME=1 -s "EXPORTED_RUNTIME_METHODS=['ccall']"


// Library
/*
document.getElementById("mybutton").addEventListener("click", () => {
  alert("check console");
  const result = Module.ccall(
    "myFunction", // name of C function
    null, // return type
    null, // argument types
    null, // arguments
  );
});
*/

// https://emscripten.org/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html#calling-javascript-from-c-c
// emscripten_run_script("alert('hi')");


// EM_JS(void, call_alert, (), {
//   alert('hello world!');
//   throw 'all done';
// });


// EM_ASM({
//   console.log('I received: ' + $0);
// }, 100);

// EM_ASM_INT, EM_ASM_DOUBLE or EM_ASM_PTR.
// int x = EM_ASM_INT({
//   console.log('I received: ' + $0);
//   return $0 + 1;
// }, 100);
// printf("%d\n", x);


// https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#embind

//
EXTERN EMSCRIPTEN_KEEPALIVE void myFunction(int argc, char ** argv) {
    printf("MyFunction Called\n");
}


#endif