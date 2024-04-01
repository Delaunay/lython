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
