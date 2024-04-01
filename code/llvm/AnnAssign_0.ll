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
