#ifndef LYTHON_NATIVE_HEADER
#define LYTHON_NATIVE_HEADER

// Actual method signature is WIP
// most mght become references

#include "dtypes.h"

namespace lython {

struct StopIteration;
struct NotImplemented;
struct NativeObject;

template <typename Exception>
NativeObject raise();

NativeObject newobj(NativeObject cls, NativeObject args);
void         del(NativeObject& self);
// void         del(NativeObject instance);
NativeObject init(NativeObject& self);
NativeObject copy(NativeObject& self);
NativeObject deepcopy(NativeObject& self, NativeObject memodict);

// Pickling
NativeObject getinitargs(NativeObject& self);
NativeObject getnewargs(NativeObject& self);
NativeObject getstate(NativeObject& self);
void         setstate(NativeObject& self, NativeObject state);
NativeObject reduce(NativeObject& self);
NativeObject reduce_ex(NativeObject& self);

// Unary
NativeObject pos(NativeObject& self);
NativeObject neg(NativeObject& self);
NativeObject abs(NativeObject& self);
NativeObject invert(NativeObject& self);
NativeObject round(NativeObject& self, int n);
NativeObject floor(NativeObject& self);
NativeObject ceil(NativeObject& self);
NativeObject trunc(NativeObject& self);

// Conversions
NativeObject cvt_int(NativeObject& self);
NativeObject cvt_long(NativeObject& self);
NativeObject cvt_float(NativeObject& self);
NativeObject cvt_complex(NativeObject& self);
NativeObject cvt_oct(NativeObject& self);
NativeObject cvt_hex(NativeObject& self);
NativeObject cvt_index(NativeObject& self);
NativeObject cvt_coerce(NativeObject type);

// Format
NativeObject str(NativeObject& self);
NativeObject repr(NativeObject& self);
NativeObject unicode(NativeObject& self);
NativeObject format(NativeObject& self, String formatstr);
NativeObject hash(NativeObject& self);
NativeObject nonzero(NativeObject& self);
NativeObject dir(NativeObject& self);
NativeObject lysizeof(NativeObject& self);

// Descriptor
NativeObject get(NativeObject instance, NativeObject owner);
void         set(NativeObject instance, NativeObject value);

// Sequence
NativeObject len(NativeObject& self);
NativeObject getitem(NativeObject& self, NativeObject key);
NativeObject setitem(NativeObject& self, NativeObject key, NativeObject value);
NativeObject delitem(NativeObject& self, NativeObject key);
NativeObject iter(NativeObject& self);
NativeObject next(NativeObject& self);
NativeObject reversed(NativeObject& self);
bool         contains(NativeObject& self, NativeObject const& other);
NativeObject missing(NativeObject& self, NativeObject key);

// types
NativeObject instancecheck(NativeObject& self, NativeObject key);
NativeObject subclasscheck(NativeObject& self, NativeObject key);

NativeObject call(NativeObject& self, NativeObject key);
NativeObject enter(NativeObject& self, NativeObject key);
NativeObject exit(NativeObject& self,
                  NativeObject  except_type,
                  NativeObject  except_value,
                  NativeObject  traceback);

// Attribute access
NativeObject getattr(NativeObject& self, String const& name);
NativeObject setattr(NativeObject& self, String const& name, NativeObject value);
NativeObject delattr(NativeObject& self, String const& name);
NativeObject getattribute(NativeObject& self, String const& name);

// Binary
NativeObject add(NativeObject& self, NativeObject const& other);
NativeObject sub(NativeObject& self, NativeObject const& other);
NativeObject mul(NativeObject& self, NativeObject const& other);
NativeObject floordiv(NativeObject& self, NativeObject const& other);
NativeObject div(NativeObject& self, NativeObject const& other);
NativeObject truediv(NativeObject& self, NativeObject const& other);
NativeObject mod(NativeObject& self, NativeObject const& other);
NativeObject divmod(NativeObject& self, NativeObject const& other);
NativeObject pow(NativeObject& self, NativeObject const& other);
NativeObject lshift(NativeObject& self, NativeObject const& other);
NativeObject rshift(NativeObject& self, NativeObject const& other);
NativeObject lyand(NativeObject& self, NativeObject const& other);
NativeObject lyor(NativeObject& self, NativeObject const& other);
NativeObject lyxor(NativeObject& self, NativeObject const& other);
NativeObject radd(NativeObject& self, NativeObject const& other);
NativeObject rsub(NativeObject& self, NativeObject const& other);
NativeObject rmul(NativeObject& self, NativeObject const& other);
NativeObject rfloordiv(NativeObject& self, NativeObject const& other);
NativeObject rdiv(NativeObject& self, NativeObject const& other);
NativeObject rtruediv(NativeObject& self, NativeObject const& other);
NativeObject rmod(NativeObject& self, NativeObject const& other);
NativeObject rdivmod(NativeObject& self, NativeObject const& other);
NativeObject rpow(NativeObject& self, NativeObject const& other);
NativeObject rlshift(NativeObject& self, NativeObject const& other);
NativeObject rrshift(NativeObject& self, NativeObject const& other);
NativeObject rand(NativeObject& self, NativeObject const& other);
NativeObject ror(NativeObject& self, NativeObject const& other);
NativeObject rxor(NativeObject& self, NativeObject const& other);
void         iadd(NativeObject& self, NativeObject const& other);
void         isub(NativeObject& self, NativeObject const& other);
void         imul(NativeObject& self, NativeObject const& other);
void         ifloordiv(NativeObject& self, NativeObject const& other);
void         idiv(NativeObject& self, NativeObject const& other);
void         itruediv(NativeObject& self, NativeObject const& other);
void         imod(NativeObject& self, NativeObject const& other);
void         ipow(NativeObject& self, NativeObject const& other);
void         ilshift(NativeObject& self, NativeObject const& other);
void         irshift(NativeObject& self, NativeObject const& other);
void         iand(NativeObject& self, NativeObject const& other);
void         ior(NativeObject& self, NativeObject const& other);
void         ixor(NativeObject& self, NativeObject const& other);

// Compare
int  cmp(NativeObject& self, NativeObject const& other);
bool eq(NativeObject& self, NativeObject const& other);
bool ne(NativeObject& self, NativeObject const& other);
bool lt(NativeObject& self, NativeObject const& other);
bool le(NativeObject& self, NativeObject const& other);
bool gt(NativeObject& self, NativeObject const& other);
bool ge(NativeObject& self, NativeObject const& other);
bool is(NativeObject& self, NativeObject const& other);

NativeObject match_args(NativeObject& self);

struct NativeObject {
    virtual ~NativeObject();

    // FIXME: this needs to be static AND virtual
    virtual NativeObject __new__(NativeObject args);
    virtual void         __del__();
    virtual NativeObject __init__();
    virtual NativeObject __copy__();
    virtual NativeObject __deepcopy__(NativeObject memodict);

    // Pickling
    virtual NativeObject __getinitargs__();
    virtual NativeObject __getnewargs__();
    virtual NativeObject __getstate__();
    virtual void         __setstate__(NativeObject state);
    virtual NativeObject __reduce__();
    virtual NativeObject __reduce_ex__();

    // Unary
    virtual NativeObject __pos__();
    virtual NativeObject __neg__();
    virtual NativeObject __abs__();
    virtual NativeObject __invert__();
    virtual NativeObject __round__(int n);
    virtual NativeObject __floor__();
    virtual NativeObject __ceil__();
    virtual NativeObject __trunc__();

    // Conversions
    virtual NativeObject __int__();
    virtual NativeObject __long__();
    virtual NativeObject __float__();
    virtual NativeObject __komplex__();
    virtual NativeObject __oct__();
    virtual NativeObject __hex__();
    virtual NativeObject __index__();
    virtual NativeObject __coerce__(NativeObject type);

    // Format
    virtual NativeObject __str__();
    virtual NativeObject __repr__();
    virtual NativeObject __unicode__();
    virtual NativeObject __format__(String formatstr);
    virtual NativeObject __hash__();
    virtual NativeObject __nonzero__();
    virtual NativeObject __dir__();
    virtual NativeObject __sizeof__();

    // Descriptor
    virtual NativeObject __get__(NativeObject instance, NativeObject owner);
    virtual void         __set__(NativeObject instance, NativeObject value);
    virtual void         __delete__(NativeObject instance);

    // Sequence
    virtual NativeObject __len__();
    virtual NativeObject __getitem__(NativeObject key);
    virtual NativeObject __setitem__(NativeObject key, NativeObject value);
    virtual NativeObject __delitem__(NativeObject key);
    virtual NativeObject __iter__();
    virtual NativeObject __next__();
    virtual NativeObject __reversed__();
    virtual bool         __contains__(NativeObject item);
    virtual NativeObject __missing__(NativeObject key);

    // types
    virtual NativeObject __instancecheck__(NativeObject key);
    virtual NativeObject __subclasscheck__(NativeObject key);
    virtual NativeObject __call__(NativeObject key);
    virtual NativeObject __enter__();
    virtual NativeObject __exit__(NativeObject exception_type,
                                  NativeObject exception_value,  //
                                  NativeObject traceback);
    // Attribute access
    virtual NativeObject __getattr__(String const& name);
    virtual NativeObject __setattr__(String const& name, NativeObject value);
    virtual NativeObject __delattr__(String const& name);
    virtual NativeObject __getattribute__(String const& name);

    // Binary
    virtual NativeObject __add__(NativeObject const& other);
    virtual NativeObject __sub__(NativeObject const& other);
    virtual NativeObject __mul__(NativeObject const& other);
    virtual NativeObject __floordiv__(NativeObject const& other);
    virtual NativeObject __div__(NativeObject const& other);
    virtual NativeObject __truediv__(NativeObject const& other);
    virtual NativeObject __mod__(NativeObject const& other);
    virtual NativeObject __divmod__(NativeObject const& other);
    virtual NativeObject __pow__(NativeObject const& other);
    virtual NativeObject __lshift__(NativeObject const& other);
    virtual NativeObject __rshift__(NativeObject const& other);
    virtual NativeObject __and__(NativeObject const& other);
    virtual NativeObject __or__(NativeObject const& other);
    virtual NativeObject __xor__(NativeObject const& other);
    virtual NativeObject __radd__(NativeObject const& other);
    virtual NativeObject __rsub__(NativeObject const& other);
    virtual NativeObject __rmul__(NativeObject const& other);
    virtual NativeObject __rfloordiv__(NativeObject const& other);
    virtual NativeObject __rdiv__(NativeObject const& other);
    virtual NativeObject __rtruediv__(NativeObject const& other);
    virtual NativeObject __rmod__(NativeObject const& other);
    virtual NativeObject __rdivmod__(NativeObject const& other);
    virtual NativeObject __rpow__(NativeObject const& other);
    virtual NativeObject __rlshift__(NativeObject const& other);
    virtual NativeObject __rrshift__(NativeObject const& other);
    virtual NativeObject __rand__(NativeObject const& other);
    virtual NativeObject __ror__(NativeObject const& other);
    virtual NativeObject __rxor__(NativeObject const& other);
    virtual void         __iadd__(NativeObject const& other);
    virtual void         __isub__(NativeObject const& other);
    virtual void         __imul__(NativeObject const& other);
    virtual void         __ifloordiv__(NativeObject const& other);
    virtual void         __idiv__(NativeObject const& other);
    virtual void         __itruediv__(NativeObject const& other);
    virtual void         __imod__(NativeObject const& other);
    virtual void         __ipow__(NativeObject const& other);
    virtual void         __ilshift__(NativeObject const& other);
    virtual void         __irshift__(NativeObject const& other);
    virtual void         __iand__(NativeObject const& other);
    virtual void         __ior__(NativeObject const& other);
    virtual void         __ixor__(NativeObject const& other);

    // Compare
    virtual int  __cmp__(NativeObject const& other);
    virtual bool __eq__(NativeObject const& other);
    virtual bool __ne__(NativeObject const& other);
    virtual bool __lt__(NativeObject const& other);
    virtual bool __le__(NativeObject const& other);
    virtual bool __gt__(NativeObject const& other);
    virtual bool __ge__(NativeObject const& other);
    virtual bool __is__(NativeObject const& other);

    // Pattern matching
    NativeObject __match_args__();
};

}  // namespace lython

#endif