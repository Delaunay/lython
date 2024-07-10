# version=2
# > 
# >> code
from aa.b import c as d, e.f as g# <<

# >> call
ModuleNotFoundError: No module named 'aa.b'# <<

# > 
# >> code
from import_test import cls as Klass, fun as Fun, ann as Ann, var as Var# <<


# > 
# >> code
from aa.b import c as d, e.f as g# <<

# >> call
fun()# <<

# > 
# >> code
from import_test import cls as Klass, fun as Fun, ann as Ann, var as Var# <<

# >> call
fun()# <<

