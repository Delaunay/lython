>>>>>>>
; Example 0
; ------------
; def fun() -> bool:
;     a: bool = True
;     return a

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i1 @fun() {
entry:
  ret i1 true
  ret void
}
<<<<<<

>>>>>>>
; Example 1
; ------------
; def fun() -> i32:
;     a: i32 = 1
;     return a

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i32 @fun() {
entry:
  ret i32 1
  ret void
}
<<<<<<

>>>>>>>
; Example 2
; ------------
; def fun() -> f64:
;     a: f64 = 2.0

<<<<<<

