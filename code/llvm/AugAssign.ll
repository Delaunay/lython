>>>>>>>
; Example 0
; ------------
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
<<<<<<

>>>>>>>
; Example 1
; ------------
; def fun(a: i32, b: i32) -> i32:
;     a -= b
;     return a

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i32 @fun(i32 %a, i32 %b) {
entry:
  %subtmp = sub i32 %a, %b
  ret i32 %subtmp
  ret void
}
<<<<<<

