# version=2
# > 0
# >> code
import aa as b, c as d, e.f as g# <<

# >> error
ModuleNotFoundError: No module named 'aa'# <<

# >> error
ModuleNotFoundError: No module named 'c'# <<

# >> error
ModuleNotFoundError: No module named 'e.f'# <<

# > 1
# >> code
import import_test as imp_test# <<

