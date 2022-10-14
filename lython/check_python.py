



class Custom:
    def __init__(self, *args, **kwargs) -> None:
        print('init', args, kwargs)

    def __new__(cls, *args, **kwargs):
        print('new', cls, args, kwargs)
        obj = object.__new__(cls)
        return obj



a = Custom(1, 2)
