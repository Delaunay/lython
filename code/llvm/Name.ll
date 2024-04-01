>>>>>>>
; Example 0
; ------------
; def fun(a: i32) -> i32:
;     return a

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i32 @fun(i32 %a) {
entry:
  ret i32 %a
  ret void
}
<<<<<<

