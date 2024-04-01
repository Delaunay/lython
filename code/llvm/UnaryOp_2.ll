; def fun(a: i32) -> i32:
;     return ~ a
; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i32 @fun(i32 %a) {
entry:
  %0 = xor i32 %a, -1
  ret i32 %0
  ret void
}
