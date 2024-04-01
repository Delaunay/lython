; def fun(b: i32) -> i32:
;     a = b
;     return a
; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i32 @fun(i32 %b) {
entry:
  ret i32 %b
  ret void
}
