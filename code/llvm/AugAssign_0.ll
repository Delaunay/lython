; def fun(a: i32, b: i32) -> i32:
;     a += b
;     return a
; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i32 @fun(i32 %a, i32 %b) {
entry:
  %addtmp = add i32 %b, %a
  ret i32 %addtmp
  ret void
}
