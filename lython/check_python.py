from __future__ import annotations



class Custom:
    def __init__(self, *args, **kwargs) -> None:
        print('init', args, kwargs)

    def __new__(cls, *args, **kwargs):
        print('new', cls, args, kwargs)
        obj = object.__new__(cls)
        return obj



a = Custom(1, 2)


def make(cls, *args, **kwargs) -> 'cls':
    a = cls.__new__(cls, *args, **kwargs)
    a.__init__(*args, **kwargs)
    return a


a = make(Custom, 1, 2)
