>>>>>>>
; Example 0
; ------------
; def fun(a: f64, b: f64) -> f64:
;     return a + b

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define double @fun(double %a, double %b) {
entry:
  %addtmp = fadd double %a, %b
  ret double %addtmp
  ret void
}
<<<<<<

>>>>>>>
; Example 1
; ------------
; def fun(a: f64, b: f64) -> f64:
;     return a - b

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define double @fun(double %a, double %b) {
entry:
  %subtmp = fsub double %a, %b
  ret double %subtmp
  ret void
}
<<<<<<

>>>>>>>
; Example 2
; ------------
; def fun(a: f64, b: f64) -> f64:
;     return a * b

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define double @fun(double %a, double %b) {
entry:
  %multtmp = fmul double %a, %b
  ret double %multtmp
  ret void
}
<<<<<<

>>>>>>>
; Example 3
; ------------
; def fun(a: i32, b: i32) -> i32:
;     return a << b

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i32 @fun(i32 %a, i32 %b) {
entry:
  %addtmp = lshr i32 %a, %b
  ret i32 %addtmp
  ret void
}
<<<<<<

>>>>>>>
; Example 4
; ------------
; def fun(a: i32, b: i32) -> i32:
;     return a ^ b

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i32 @fun(i32 %a, i32 %b) {
entry:
  %bitxortmp = xor i32 %b, %a
  ret i32 %bitxortmp
  ret void
}
<<<<<<

