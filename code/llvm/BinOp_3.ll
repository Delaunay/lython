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
