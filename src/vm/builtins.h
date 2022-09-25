


namespace lython {


// -
// Unary (T) -> T
// -

template<typename T, typename Implementation>
struct UnaryOperation {
    struct Params {
        T r;
        T a;
    };

    static void call(Params& params) {
        Implementation::call(params);
    }

    static T call(T a) {
        Params p;
        p.a = a;
        Implementation::call(p);
        return p.r;
    }
};

template<typename T>
struct Invert: public UnaryOperation<T, Invert<T>> {
    using Super = typename UnaryOperation<T, Invert<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = ~params.a;
    }
};

template<typename T>
struct Not : public UnaryOperation<T, Not<T>> {
    using Super = typename UnaryOperation<T, Not<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r =!params.a;
    }
};

template<typename T>
struct UAdd : public UnaryOperation<T, UAdd<T>> {
    using Super = typename UnaryOperation<T, UAdd<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = +params.a;
    }
};

template<typename T>
struct USub: public UnaryOperation<T, USub<T>> {
    using Super = typename UnaryOperation<T, USub<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = -params.a;
    }
};


template<>
struct USub<uint8>: public UnaryOperation<uint8, USub<uint8>> {
    struct Params {
        int8 r;
        uint8 a;
    };

    static void call(Params& params) {
        params.r = -int8(params.a);
    }
};

template<>
struct USub<uint16>: public UnaryOperation<uint16, USub<uint16>> {
    struct Params {
        int16 r;
        uint16 a;
    };

    static void call(Params& params) {
        params.r = -int16(params.a);
    }
};

template<>
struct USub<uint32>: public UnaryOperation<uint32, USub<uint32>> {
    struct Params {
        int32 r;
        uint32 a;
    };

    static void call(Params& params) {
        params.r = -int32(params.a);
    }
};

template<>
struct USub<uint64>: public UnaryOperation<uint64, USub<uint64>> {
    struct Params {
        int64 r;
        uint64 a;
    };

    static void call(Params& params) {
        params.r = -int64(params.a);
    }
};


// -
// Binary (T, T) -> T
// -
template<typename T, typename Implementation>
struct BinaryOperation {
    struct Params {
        T r;
        T a;
        T b;
    };

    static void call(Params& params) {
        Implementation::call(params);
    }

    static T call(T a, T b) {
        Params p;
        p.a = a;
        p.b = a;
        Implementation::call(p);
        return p.r;
    }
};

template<typename T>
struct Add: public BinaryOperation<T, Add<T>> {
    using Super = typename BinaryOperation<T, Add<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = params.a + params.b;
    }
};

template<typename T>
struct Sub: public BinaryOperation<T, Sub<T>> {
    using Super = typename BinaryOperation<T, Sub<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = params.a - params.b;
    }
};

template<typename T>
T ly_pow(T a, T b) {
    return T(std::pow(T(a), T(b)));
}

template<>
float ly_pow(float a, float b) {
    return std::powf(a, b);
}

template<>
double ly_pow(double a, double b) {
    return std::pow(a, b);
}


template<typename T>
struct Pow: public BinaryOperation<T, Pow<T>> {
    using Super = typename BinaryOperation<T, Pow<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = ly_pow(params.a, params.b);
    }
};

template<typename T>
struct LShift: public BinaryOperation<T, LShift<T>> {
    using Super = typename BinaryOperation<T, LShift<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = params.a << params.b;
    }
};

template<typename T>
struct RShift: public BinaryOperation<T, RShift<T>> {
    using Super = typename BinaryOperation<T, RShift<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = params.a >> params.b;
    }
};

template<typename T>
struct Mult: public BinaryOperation<T, Mult<T>> {
    using Super = typename BinaryOperation<T, Mult<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = params.a * params.b;
    }
};

template<typename T>
struct Div: public BinaryOperation<T, Div<T>> {
    using Super = typename BinaryOperation<T, Div<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = params.a / params.b;
    }
};

template<typename T>
struct Mod: public BinaryOperation<T, Mod<T>> {
    using Super = typename BinaryOperation<T, Mod<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = params.a % params.b;
    }
};

template<>
struct Mod<float>: public BinaryOperation<float, Mod<float>> {
    using Super = typename BinaryOperation<float, Mod<float>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = std::fmodf(params.a, params.b);
    }
};

template<>
struct Mod<double>: public BinaryOperation<double, Mod<double>> {
    using Super = typename BinaryOperation<double, Mod<double>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = std::fmod(params.a, params.b);
    }
};

template<typename T>
struct And: public BinaryOperation<T, And<T>> {
    using Super = typename BinaryOperation<T, And<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = params.a && params.b;
    }
};

template<typename T>
struct Or: public BinaryOperation<T, Or<T>> {
    using Super = typename BinaryOperation<T, Or<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = params.a || params.b;
    }
};


template<typename T>
struct BitAnd: public BinaryOperation<T, BitAnd<T>> {
    using Super = typename BinaryOperation<T, BitAnd<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = params.a & params.b;
    }
};

template<typename T>
struct BitOr: public BinaryOperation<T, BitOr<T>> {
    using Super = typename BinaryOperation<T, BitOr<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = params.a | params.b;
    }
};

template<typename T>
struct BitXor: public BinaryOperation<T, BitXor<T>> {
    using Super = typename BinaryOperation<T, BitXor<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = params.a ^ params.b;
    }
};

// -
// Comparison (T, T) -> bool
// -
template<typename T, typename Implementation>
struct ComparisonOperation {
    struct Params {
        bool r: 1;
        T a;
        T b;
    };

    static void call(Params& params) {
        Implementation::call(params);
    }

    static T call(T a, T b) {
        Params p;
        p.a = a;
        p.b = a;
        Implementation::call(p);
        return p.r;
    }
};

template<typename T>
struct Eq: public ComparisonOperation<T, Eq<T>> {
    using Super = typename ComparisonOperation<T, Eq<T>>;
    using Params = Super::Params;

    static void call(Params& params) {
        params.r = params.a == params.b;
    }
};

template<typename T>
struct NotEq: public ComparisonOperation<T, NotEq<T>> {
    using Super = typename ComparisonOperation<T, NotEq<T>>;
    using Params = Super::Params;
    static void call(Params& params) {
        params.r = params.a != params.b;
    }
};

template<typename T>
struct Lt: public ComparisonOperation<T, Lt<T>> {
    using Super = typename ComparisonOperation<T, Lt<T>>;
    using Params = Super::Params;
    static void call(Params& params) {
        params.r = params.a < params.b;
    }
};

template<typename T>
struct LtE: public ComparisonOperation<T, LtE<T>> {
    using Super = typename ComparisonOperation<T, LtE<T>>;
    using Params = Super::Params;
    static void call(Params& params) {
        params.r = params.a <= params.b;
    }
};

template<typename T>
struct Gt: public ComparisonOperation<T, Gt<T>> {
    using Super = typename ComparisonOperation<T, Gt<T>>;
    using Params = Super::Params;
    static void call(Params& params) {
        params.r = params.a > params.b;
    }
};

template<typename T>
struct GtE: public ComparisonOperation<T, GtE<T>> {
    using Super = typename ComparisonOperation<T, GtE<T>>;
    using Params = Super::Params;
    static void call(Params& params) {
        params.r = params.a >= params.b;
    }
};

template<typename T>
struct Is: public ComparisonOperation<T, Is<T>> {
    using Super = typename ComparisonOperation<T, Is<T>>;
    using Params = Super::Params;
    static void call(Params& params) {
        params.r = params.a == params.b;
    }
};

template<typename T>
struct IsNot: public ComparisonOperation<T, IsNot<T>> {
    using Super = typename ComparisonOperation<T, IsNot<T>>;
    using Params = Super::Params;
    static void call(Params& params) {
        params.r = params.a != params.b;
    }
};

template<typename T>
struct In: public ComparisonOperation<T, In<T>> {
    using Super = typename ComparisonOperation<T, In<T>>;
    using Params = Super::Params;
    static void call(Params& params) {
        params.r = params.a == params.b;
    }
};

template<typename T>
struct NotIn: public ComparisonOperation<T, NotIn<T>> {
    using Super = typename ComparisonOperation<T, NotIn<T>>;
    using Params = Super::Params;
    static void call(Params& params) {
        params.r = params.a != params.b;
    }
};

}