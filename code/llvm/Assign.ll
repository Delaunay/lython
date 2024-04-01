>>>>>>>
; Example 0
; ------------
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
<<<<<<

>>>>>>>
; Example 1
; ------------
; def fun(c: Tuple[i32, i32]) -> i32:
;     a, b = c
;     return a

<<<<<<

