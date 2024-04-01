>>>>>>>
; Example 0
; ------------
; def myfunction(a: f64, b: f64) -> f64:
;     return a + b
; 
; def fun():
;     return myfunction(1.0, 2.0)

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define double @myfunction(double %a, double %b) {
entry:
  %addtmp = fadd double %a, %b
  ret double %addtmp
  ret void
}

define void @fun() {
entry:
  %calltmp = call double @myfunction(double 1.000000e+00, double 2.000000e+00)
  ret double %calltmp
  ret void
}
<<<<<<

