; def fun():
;     return 2.1
; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define void @fun() {
entry:
  ret double 2.100000e+00
  ret void
}
