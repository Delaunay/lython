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
