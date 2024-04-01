>>>>>>>
; Example 0
; ------------
; def fun(c: i32):
;     return a := c

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define void @fun(i32 %c) {
entry:
  ret i32 %c
  ret void
}
<<<<<<

