; def fun(a: i32) -> i32:
;     return - a
; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i32 @fun(i32 %a) {
entry:
  %subtmp = sub i32 0, %a
  ret i32 %subtmp
  ret void
}
