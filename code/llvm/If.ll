>>>>>>>
; Example 0
; ------------
; def fun(a: bool, b: bool) -> f64:
;     if a:
;         return 0.0
;     elif b:
;         return 1.0
;     else:
;         return 3.0
;     return 2.0

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define double @fun(i1 %a, i1 %b) {
entry:
  %b2 = alloca i1, align 1
  store i1 %b, ptr %b2, align 1
  ret double 2.000000e+00
  ret void
}
<<<<<<

