>>>>>>>
; Example 0
; ------------
; def fun(a: i32, b: i32) -> i32:
;     return a if True else b

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i32 @fun(i32 %a, i32 %b) {
entry:
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  ret i32 %a
  ret void
}
<<<<<<

