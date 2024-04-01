; def fun(a: bool, b: bool, c: bool) -> bool:
;     return a or b or c
; ModuleID = 'KiwiJIT'
source_filename = "KiwiJIT"

define i1 @fun(i1 %a, i1 %b, i1 %c) {
entry:
  %ortmp = or i1 %a, %b
  %ortmp7 = or i1 %ortmp, %c
  ret i1 %ortmp7
  ret void
}
