; def fun() -> i32:
;     a: i32 = 1
;     return a
; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i32 @fun() {
entry:
  ret i32 1
  ret void
}
