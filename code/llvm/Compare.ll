>>>>>>>
; Example 0
; ------------
; def fun(a: i32, b: i32, c: i32, d: i32) -> bool:
;     return a < b > c != d

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i1 @fun(i32 %a, i32 %b, i32 %c, i32 %d) {
entry:
  %0 = icmp slt i32 %a, %b
  %1 = icmp sgt i32 %b, %c
  %2 = icmp ne i32 %c, %d
  %3 = and i1 %0, %1
  %4 = and i1 %3, %2
  ret i1 %4
  ret void
}
<<<<<<

>>>>>>>
; Example 1
; ------------
; def fun():
;     return a not in b

<<<<<<

