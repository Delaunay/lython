>>>>>>>
; Example 0
; ------------
; def fun(a: bool, b: bool) -> bool:
;     return a and b

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i1 @fun(i1 %a, i1 %b) {
entry:
  %andtmp = and i1 %a, %b
  ret i1 %andtmp
  ret void
}
<<<<<<

>>>>>>>
; Example 1
; ------------
; def fun(a: bool, b: bool) -> bool:
;     return a or b

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i1 @fun(i1 %a, i1 %b) {
entry:
  %ortmp = or i1 %a, %b
  ret i1 %ortmp
  ret void
}
<<<<<<

>>>>>>>
; Example 2
; ------------
; def fun(a: bool, b: bool, c: bool) -> bool:
;     return a or b or c

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i1 @fun(i1 %a, i1 %b, i1 %c) {
entry:
  %ortmp = or i1 %a, %b
  %ortmp7 = or i1 %ortmp, %c
  ret i1 %ortmp7
  ret void
}
<<<<<<

