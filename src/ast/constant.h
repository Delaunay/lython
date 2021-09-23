#ifndef LYTHON_CONSTANT_HEADER
#define LYTHON_CONSTANT_HEADER

namespace lython {

struct ConstantValue {
    public:
    enum Type
    {
        TInvalid,
        TInt,
        TFloat,
        TDouble,
        TString
    };

    ConstantValue() = default;

    ConstantValue(int v): kind(TInt) { value.integer = v; }

    ConstantValue(float v): kind(TFloat) { value.single = v; }

    ConstantValue(double v): kind(TDouble) { value.decimal = v; }

    ConstantValue(String const &v): kind(TInvalid) { set_string(v); }

    ConstantValue(ConstantValue const &v): kind(TInvalid) { copy_union(v.kind, v.value); }

    ~ConstantValue() {
        if (kind == TString) {
            value.string.~String();
        }
    }

    ConstantValue &operator=(ConstantValue const &v) {
        copy_union(v.kind, v.value);
        return *this;
    }

    template <typename T>
    ConstantValue &operator=(T const &v) {
        *this = ConstantValue(v);
        return *this;
    }

    void print(std::ostream &out) const;

    private:
    // ast.Str, ast.Bytes, ast.NameConstant, ast.Ellipsis
    union ValueVariant {
        ValueVariant() {}
        ~ValueVariant() {}

        int    integer;
        double decimal;
        float  single;
        String string;
    };

    ValueVariant value;
    Type         kind = TInvalid;

    void set_string(const String &data) {
        if (kind != Type::TString) {
            new (&value.string) String(data);
        } else {
            value.string = data;
        }
        kind = Type::TString;
    }

    void copy_union(Type k, ValueVariant const &v) {
        switch (kind) {
        case TString:
            set_string(v.string);
        case TDouble:
            value.decimal = v.decimal;
        case TFloat:
            value.single = v.single;
        case TInt:
            value.integer = v.integer;
        }
        kind = k;
    }
};

} // namespace lython

#endif