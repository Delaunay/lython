; def fun():
;     return False
; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define void @fun() {
entry:
  ret i1 false
  ret void
}
