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
