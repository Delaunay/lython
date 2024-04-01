#include "dtypes.h"


namespace lython {
struct VMTestCase {
    VMTestCase(String const& c, String const& call, String const& t = ""):
        code(c), call(call), expected_type(t) {}

    String code;
    String call;
    String expected_type;
};


void write_case(std::ostream& out, int i, VMTestCase const& testcase) ;

Array<VMTestCase> load_cases(std::istream& in);

Array<VMTestCase> get_test_cases(String const& name, Array<VMTestCase> const& maybe_cases);

}