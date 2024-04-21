#pragma once

#include "dtypes.h"


namespace lython {


#define KW_TESTING_TYPE(field, attr)    \
    struct Test ## field {              \
        Test ## field(String const& v): \
            attr(v)                     \
        {}                              \
        String attr;                    \
    };

KW_TESTING_TYPE(Name, name);
KW_TESTING_TYPE(Code, code);
KW_TESTING_TYPE(Call, call);
KW_TESTING_TYPE(Expected, expected);

struct VMTestCase {
    VMTestCase(String const& c, String const& call, String const& t = ""):
        code(c), call(call), expected_type(t) {}

    VMTestCase(TestName const& n, String const& c, String const& call, String const& t):
        name(n.name), code(c), call(call), expected_type(t)
    {}

    String code;
    String call;
    String expected_type;
    String name;


    bool operator==(VMTestCase const& other) const {
        return code == other.code && call == other.call && expected_type == other.expected_type;
    }
};


void write_case(std::ostream& out, int i, VMTestCase const& testcase) ;

Array<VMTestCase> load_cases(std::istream& in);

Array<VMTestCase> get_test_cases(String const& name, Array<VMTestCase> const& maybe_cases);
Array<VMTestCase> get_test_cases(String const& folder, String const& name);

void transition(String const& folder, String const& name, Array<VMTestCase> const& cases);


}