
#include "native.h"

namespace lython {
NativeObject newobj(NativeObject cls, NativeObject args) { return cls.__new__(args); }
void         del(NativeObject& self) { return self.__del__(); }
NativeObject init(NativeObject& self) { return self.__init__(); }
NativeObject copy(NativeObject& self) { return self.__copy__(); }
NativeObject deepcopy(NativeObject& self, NativeObject memodict) {
    return self.__deepcopy__(memodict);
}

// Pickling
NativeObject getinitargs(NativeObject& self) { return self.__getinitargs__(); }
NativeObject getnewargs(NativeObject& self) { return self.__getnewargs__(); }
NativeObject getstate(NativeObject& self) { return self.__getstate__(); }
void         setstate(NativeObject& self, NativeObject state) { self.__setstate__(state); }
NativeObject reduce(NativeObject& self) { return self.__reduce__(); }
NativeObject reduce_ex(NativeObject& self) { return self.__reduce_ex__(); }

// Unary
NativeObject pos(NativeObject& self) { return self.__pos__(); }
NativeObject neg(NativeObject& self) { return self.__neg__(); }
NativeObject abs(NativeObject& self) { return self.__abs__(); }
NativeObject invert(NativeObject& self) { return self.__invert__(); }
NativeObject round(NativeObject& self, int n) { return self.__round__(n); }
NativeObject floor(NativeObject& self) { return self.__floor__(); }
NativeObject ceil(NativeObject& self) { return self.__ceil__(); }
NativeObject trunc(NativeObject& self) { return self.__trunc__(); }

// Conversions
NativeObject cvt_int(NativeObject& self) { return self.__int__(); }
NativeObject cvt_long(NativeObject& self) { return self.__long__(); }
NativeObject cvt_float(NativeObject& self) { return self.__float__(); }
NativeObject cvt_complex(NativeObject& self) { return self.__komplex__(); }
NativeObject cvt_oct(NativeObject& self) { return self.__oct__(); }
NativeObject cvt_hex(NativeObject& self) { return self.__hex__(); }
NativeObject cvt_index(NativeObject& self) { return self.__index__(); }
NativeObject cvt_coerce(NativeObject& self, NativeObject type) { return self.__coerce__(type); }

// Format
NativeObject str(NativeObject& self) { return self.__str__(); }
NativeObject repr(NativeObject& self) { return self.__repr__(); }
NativeObject unicode(NativeObject& self) { return self.__unicode__(); }
NativeObject format(NativeObject& self, String formatstr) { return self.__format__(formatstr); }
NativeObject hash(NativeObject& self) { return self.__hash__(); }
NativeObject nonzero(NativeObject& self) { return self.__nonzero__(); }
NativeObject dir(NativeObject& self) { return self.__dir__(); }
NativeObject lysizeof(NativeObject& self) { return self.__sizeof__(); }

// Descriptor
NativeObject get(NativeObject& self, NativeObject instance, NativeObject owner) {
    return self.__get__(instance, owner);
}
void set(NativeObject& self, NativeObject instance, NativeObject value) {
    return self.__set__(instance, value);
}

// Sequence
NativeObject len(NativeObject& self) { return self.__len__(); }
NativeObject getitem(NativeObject& self, NativeObject key) { return self.__getitem__(key); }
NativeObject setitem(NativeObject& self, NativeObject key, NativeObject value) {
    return self.__setitem__(key, value);
}
NativeObject delitem(NativeObject& self, NativeObject key) { return self.__delitem__(key); }
NativeObject iter(NativeObject& self) { return self.__iter__(); }
NativeObject reversed(NativeObject& self) { return self.__reversed__(); }
bool contains(NativeObject& self, NativeObject const& item) { return self.__contains__(item); }
NativeObject missing(NativeObject& self, NativeObject key) { return self.__missing__(key); }
NativeObject next(NativeObject& self) { return self.__next__(); }

// types
NativeObject instancecheck(NativeObject& self, NativeObject key) {
    return self.__instancecheck__(key);
}
NativeObject subclasscheck(NativeObject& self, NativeObject key) {
    return self.__subclasscheck__(key);
}

NativeObject call(NativeObject& self, NativeObject args) { return self.__call__(args); }
NativeObject enter(NativeObject& self) { return self.__enter__(); }
NativeObject exit(NativeObject& self,
                  NativeObject  except_type,
                  NativeObject  except_value,
                  NativeObject  traceback) {
    return self.__exit__(except_type, except_value, traceback);
}

// Attribute access
NativeObject getattr(NativeObject& self, String const& name) { return self.__getattr__(name); }
NativeObject setattr(NativeObject& self, String const& name, NativeObject value) {
    return self.__setattr__(name, value);
}
NativeObject delattr(NativeObject& self, String const& name) { return self.__delattr__(name); }
NativeObject getattribute(NativeObject& self, String const& name) {
    return self.__getattribute__(name);
}

// Binary
NativeObject add(NativeObject& self, NativeObject const& other) { return self.__add__(other); }
NativeObject sub(NativeObject& self, NativeObject const& other) { return self.__sub__(other); }
NativeObject mul(NativeObject& self, NativeObject const& other) { return self.__mul__(other); }
NativeObject floordiv(NativeObject& self, NativeObject const& other) {
    return self.__floordiv__(other);
}
NativeObject div(NativeObject& self, NativeObject const& other) { return self.__div__(other); }
NativeObject truediv(NativeObject& self, NativeObject const& other) {
    return self.__truediv__(other);
}
NativeObject mod(NativeObject& self, NativeObject const& other) { return self.__mod__(other); }
NativeObject divmod(NativeObject& self, NativeObject const& other) {
    return self.__divmod__(other);
}
NativeObject pow(NativeObject& self, NativeObject const& other) { return self.__pow__(other); }
NativeObject lshift(NativeObject& self, NativeObject const& other) {
    return self.__lshift__(other);
}
NativeObject rshift(NativeObject& self, NativeObject const& other) {
    return self.__rshift__(other);
}
NativeObject lyand(NativeObject& self, NativeObject const& other) { return self.__and__(other); }
NativeObject lyor(NativeObject& self, NativeObject const& other) { return self.__or__(other); }
NativeObject lyxor(NativeObject& self, NativeObject const& other) { return self.__xor__(other); }

NativeObject radd(NativeObject& self, NativeObject const& other) { return self.__radd__(other); }
NativeObject rsub(NativeObject& self, NativeObject const& other) { return self.__rsub__(other); }
NativeObject rmul(NativeObject& self, NativeObject const& other) { return self.__rmul__(other); }
NativeObject rfloordiv(NativeObject& self, NativeObject const& other) {
    return self.__rfloordiv__(other);
}
NativeObject rdiv(NativeObject& self, NativeObject const& other) { return self.__rdiv__(other); }
NativeObject rtruediv(NativeObject& self, NativeObject const& other) {
    return self.__rtruediv__(other);
}
NativeObject rmod(NativeObject& self, NativeObject const& other) { return self.__rmod__(other); }
NativeObject rdivmod(NativeObject& self, NativeObject const& other) {
    return self.__rdivmod__(other);
}
NativeObject rpow(NativeObject& self, NativeObject const& other) { return self.__rpow__(other); }
NativeObject rlshift(NativeObject& self, NativeObject const& other) {
    return self.__rlshift__(other);
}
NativeObject rrshift(NativeObject& self, NativeObject const& other) {
    return self.__rrshift__(other);
}
NativeObject rand(NativeObject& self, NativeObject const& other) { return self.__rand__(other); }
NativeObject ror(NativeObject& self, NativeObject const& other) { return self.__ror__(other); }
NativeObject rxor(NativeObject& self, NativeObject const& other) { return self.__rxor__(other); }

void iadd(NativeObject& self, NativeObject const& other) { self.__iadd__(other); }
void isub(NativeObject& self, NativeObject const& other) { self.__isub__(other); }
void imul(NativeObject& self, NativeObject const& other) { self.__imul__(other); }
void ifloordiv(NativeObject& self, NativeObject const& other) { self.__ifloordiv__(other); }
void idiv(NativeObject& self, NativeObject const& other) { self.__idiv__(other); }
void itruediv(NativeObject& self, NativeObject const& other) { self.__itruediv__(other); }
void imod(NativeObject& self, NativeObject const& other) { self.__imod__(other); }
void ipow(NativeObject& self, NativeObject const& other) { self.__ipow__(other); }
void ilshift(NativeObject& self, NativeObject const& other) { self.__ilshift__(other); }
void irshift(NativeObject& self, NativeObject const& other) { self.__irshift__(other); }
void iand(NativeObject& self, NativeObject const& other) { self.__iand__(other); }
void ior(NativeObject& self, NativeObject const& other) { self.__ior__(other); }
void ixor(NativeObject& self, NativeObject const& other) { self.__ixor__(other); }

// Compare
int  cmp(NativeObject& self, NativeObject const& other) { return self.__cmp__(other); }
bool eq(NativeObject& self, NativeObject const& other) { return self.__eq__(other); }
bool ne(NativeObject& self, NativeObject const& other) { return self.__ne__(other); }
bool lt(NativeObject& self, NativeObject const& other) { return self.__lt__(other); }
bool le(NativeObject& self, NativeObject const& other) { return self.__le__(other); }
bool gt(NativeObject& self, NativeObject const& other) { return self.__gt__(other); }
bool ge(NativeObject& self, NativeObject const& other) { return self.__ge__(other); }
bool is(NativeObject& self, NativeObject const& other) { return self.__is__(other); }

NativeObject match_args(NativeObject& self) { return self.__match_args__(); }

// Native Object
// =============
NativeObject::~NativeObject() {}

NativeObject NativeObject::__new__(NativeObject args) { return raise<NotImplemented>(); }
void         NativeObject::__del__() {
    raise<NotImplemented>();
    return;
}
NativeObject NativeObject::__init__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__copy__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__deepcopy__(NativeObject memodict) { return raise<NotImplemented>(); }

// Pickling
NativeObject NativeObject::__getinitargs__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__getnewargs__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__getstate__() { return raise<NotImplemented>(); }
void         NativeObject::__setstate__(NativeObject state) {
    raise<NotImplemented>();
    return;
}
NativeObject NativeObject::__reduce__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__reduce_ex__() { return raise<NotImplemented>(); }

// Unary
NativeObject NativeObject::__pos__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__neg__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__abs__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__invert__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__round__(int n) { return raise<NotImplemented>(); }
NativeObject NativeObject::__floor__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__ceil__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__trunc__() { return raise<NotImplemented>(); }

// Conversions
NativeObject NativeObject::__int__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__long__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__float__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__komplex__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__oct__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__hex__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__index__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__coerce__(NativeObject type) { return raise<NotImplemented>(); }

// Format
NativeObject NativeObject::__str__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__repr__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__unicode__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__format__(String formatstr) { return raise<NotImplemented>(); }
NativeObject NativeObject::__hash__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__nonzero__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__dir__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__sizeof__() { return raise<NotImplemented>(); }

// Descriptor
NativeObject NativeObject::__get__(NativeObject instance, NativeObject owner) {
    return raise<NotImplemented>();
}
void __set__(NativeObject instance, NativeObject value) { raise<NotImplemented>(); }
void __delete__(NativeObject instance) { raise<NotImplemented>(); }

// Sequence
NativeObject NativeObject::__len__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__getitem__(NativeObject key) { return raise<NotImplemented>(); }
NativeObject NativeObject::__setitem__(NativeObject key, NativeObject value) {
    return raise<NotImplemented>();
}
NativeObject NativeObject::__delitem__(NativeObject key) { return raise<NotImplemented>(); }
NativeObject NativeObject::__iter__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__reversed__() { return raise<NotImplemented>(); }
bool         NativeObject::__contains__(NativeObject item) {
    raise<NotImplemented>();
    return false;
}
NativeObject NativeObject::__missing__(NativeObject key) { return raise<NotImplemented>(); }
NativeObject NativeObject::__next__() { return raise<StopIteration>(); }

// types
NativeObject NativeObject::__instancecheck__(NativeObject key) { return raise<NotImplemented>(); }
NativeObject NativeObject::__subclasscheck__(NativeObject key) { return raise<NotImplemented>(); }
NativeObject NativeObject::__call__(NativeObject key) { return raise<NotImplemented>(); }
NativeObject NativeObject::__enter__() { return raise<NotImplemented>(); }
NativeObject NativeObject::__exit__(NativeObject exception_type,
                                    NativeObject exception_value,  //
                                    NativeObject traceback) {
    return raise<NotImplemented>();
}

// Attribute access
NativeObject NativeObject::__getattr__(String const& name) { return raise<NotImplemented>(); }
NativeObject NativeObject::__setattr__(String const& name, NativeObject value) {
    return raise<NotImplemented>();
}
NativeObject NativeObject::__delattr__(String const& name) { return raise<NotImplemented>(); }
NativeObject NativeObject::__getattribute__(String const& name) { return raise<NotImplemented>(); }

// Binary
NativeObject NativeObject::__add__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__sub__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__mul__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__floordiv__(NativeObject const& other) {
    return raise<NotImplemented>();
}
NativeObject NativeObject::__div__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__truediv__(NativeObject const& other) {
    return raise<NotImplemented>();
}
NativeObject NativeObject::__mod__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__divmod__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__pow__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__lshift__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__rshift__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__and__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__or__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__xor__(NativeObject const& other) { return raise<NotImplemented>(); }

NativeObject NativeObject::__radd__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__rsub__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__rmul__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__rfloordiv__(NativeObject const& other) {
    return raise<NotImplemented>();
}
NativeObject NativeObject::__rdiv__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__rtruediv__(NativeObject const& other) {
    return raise<NotImplemented>();
}
NativeObject NativeObject::__rmod__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__rdivmod__(NativeObject const& other) {
    return raise<NotImplemented>();
}
NativeObject NativeObject::__rpow__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__rlshift__(NativeObject const& other) {
    return raise<NotImplemented>();
}
NativeObject NativeObject::__rrshift__(NativeObject const& other) {
    return raise<NotImplemented>();
}
NativeObject NativeObject::__rand__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__ror__(NativeObject const& other) { return raise<NotImplemented>(); }
NativeObject NativeObject::__rxor__(NativeObject const& other) { return raise<NotImplemented>(); }

void NativeObject::__iadd__(NativeObject const& other) { raise<NotImplemented>(); }
void NativeObject::__isub__(NativeObject const& other) { raise<NotImplemented>(); }
void NativeObject::__imul__(NativeObject const& other) { raise<NotImplemented>(); }
void NativeObject::__ifloordiv__(NativeObject const& other) { raise<NotImplemented>(); }
void NativeObject::__idiv__(NativeObject const& other) { raise<NotImplemented>(); }
void NativeObject::__itruediv__(NativeObject const& other) { raise<NotImplemented>(); }
void NativeObject::__imod__(NativeObject const& other) { raise<NotImplemented>(); }
void NativeObject::__ipow__(NativeObject const& other) { raise<NotImplemented>(); }
void NativeObject::__ilshift__(NativeObject const& other) { raise<NotImplemented>(); }
void NativeObject::__irshift__(NativeObject const& other) { raise<NotImplemented>(); }
void NativeObject::__iand__(NativeObject const& other) { raise<NotImplemented>(); }
void NativeObject::__ior__(NativeObject const& other) { raise<NotImplemented>(); }
void NativeObject::__ixor__(NativeObject const& other) { raise<NotImplemented>(); }

// Compare
int NativeObject::__cmp__(NativeObject const& other) {
    raise<NotImplemented>();
    return -1;
}
bool NativeObject::__eq__(NativeObject const& other) { return cmp(*this, other) == 0; }
bool NativeObject::__ne__(NativeObject const& other) { return cmp(*this, other) != 0; }
bool NativeObject::__lt__(NativeObject const& other) { return cmp(*this, other) < 0; }
bool NativeObject::__le__(NativeObject const& other) { return cmp(*this, other) <= 0; }
bool NativeObject::__gt__(NativeObject const& other) { return cmp(*this, other) > 0; }
bool NativeObject::__ge__(NativeObject const& other) { return cmp(*this, other) >= 0; }
bool NativeObject::__is__(NativeObject const& other) { return this == &other; }

NativeObject NativeObject::__match_args__() { return raise<NotImplemented>(); }
}  // namespace lython