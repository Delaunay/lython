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
