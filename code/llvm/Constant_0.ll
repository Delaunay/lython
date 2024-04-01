; def fun():
;     return 1
; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define void @fun() {
entry:
  ret i32 1
  ret void
}
