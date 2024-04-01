; def fun(a: i32, b: i32):
;     del a, b
; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define void @fun(i32 %a, i32 %b) {
entry:
  ret void
}
