; def fun():
;     return True
; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define void @fun() {
entry:
  ret i1 true
  ret void
}
