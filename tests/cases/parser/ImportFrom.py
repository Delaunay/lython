# >>> case: 0
# >>> code
from aa.b import c as d, e.f as g# <<<

# >>> call
ModuleNotFoundError: No module named 'aa.b'# <<<

# >>> case: 1
# >>> code
from import_test import cls as Klass, fun as Fun, ann as Ann, var as Var# <<<

