>>>>>>>
; Example 0
; ------------
; def fun():
;     return 1

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define void @fun() {
entry:
  ret i32 1
  ret void
}
<<<<<<

>>>>>>>
; Example 1
; ------------
; def fun():
;     return 2.1

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define void @fun() {
entry:
  ret double 2.100000e+00
  ret void
}
<<<<<<

>>>>>>>
; Example 2
; ------------
; def fun():
;     return None

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define void @fun() {
entry:
  ret void
  ret void
}
<<<<<<

>>>>>>>
; Example 3
; ------------
; def fun():
;     return True

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define void @fun() {
entry:
  ret i1 true
  ret void
}
<<<<<<

>>>>>>>
; Example 4
; ------------
; def fun():
;     return False

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define void @fun() {
entry:
  ret i1 false
  ret void
}
<<<<<<

