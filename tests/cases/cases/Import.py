# version=2
# > 
# >> code
import aa as b, c as d, e.f as g# <<

# >> error:: ModuleNotFoundError: No module named 'aa'
# >> error:: ModuleNotFoundError: No module named 'c'
# >> error:: ModuleNotFoundError: No module named 'e.f'
# > 
# >> code
import import_test as imp_test# <<



# > 
# >> code
import aa as b, c as d, e.f as g# <<

# >> call
fun()# <<

# > 
# >> code
import import_test as imp_test# <<

# >> call
fun()# <<

