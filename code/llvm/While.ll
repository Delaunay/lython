>>>>>>>
; Example 0
; ------------
; def fun(limit: i32):
;     a = 0
;     while a < limit:
;         a += 1
;     else:
;         pass

; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define void @fun(i32 %limit) {
entry:
  ret void
}
<<<<<<

