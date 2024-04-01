>>>>>>>
; Example 0
; ------------
; def fun(a: i32) -> i32:
;     return + a

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i32 @fun(i32 %a) {
entry:
  ret i32 %a
  ret void
}
<<<<<<

>>>>>>>
; Example 1
; ------------
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
<<<<<<

>>>>>>>
; Example 2
; ------------
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
<<<<<<

>>>>>>>
; Example 3
; ------------
; def fun(a: i32) -> i32:
;     return ! a

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i32 @fun(i32 %a) {
entry:
  %0 = xor i32 %a, 1
  ret i32 %0
  ret void
}
<<<<<<

