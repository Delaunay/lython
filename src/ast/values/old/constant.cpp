// void ConstantValue::debug_print(std::ostream& out) const {
//     switch (kind) {
//     case TInvalid: out << "<Constant:Invalid>"; break;

//     case Ti8: out << "i8 " << value.i8; break;
//     case Ti16: out << "i16 " << value.i16; break;
//     case Ti32: out << "i32 " << value.i32; break;
//     case Ti64: out << "i64 " << value.i64; break;

//     case Tu8: out << "u8 " << value.u8; break;
//     case Tu16: out << "u16 " << value.u16; break;
//     case Tu32: out << "u32 " << value.u32; break;
//     case Tu64: out << "u64 " << value.u64; break;

//     case Tf32: out << "f32" << value.f32; break;

//     case Tf64:
//         // always print a float even without decimal point
//         out << "f64 " << fmtstr("{:#}", value.f64);
//         break;

//     case TBool:
//         if (value.boolean) {
//             out << "bool "
//                 << "True";
//         } else {
//             out << "bool "
//                 << "False";
//         }
//         break;

//     case TNone:
//         out << "None "
//             << "None";
//         break;

//     case TString:
//         out << "str "
//             << "\"" << value.string << "\"";
//         break;

//     case TObject: _print_object(out << "object "); break;

//     default: break;
//     }
// }

// void ConstantValue::print(std::ostream& out) const {
//     switch (kind) {
//     case TInvalid: out << "<Constant:Invalid>"; break;

//     case Ti8: out << value.i8; break;
//     case Ti16: out << value.i16; break;
//     case Ti32: out << value.i32; break;
//     case Ti64: out << value.i64; break;

//     case Tu8: out << value.u8; break;
//     case Tu16: out << value.u16; break;
//     case Tu32: out << value.u32; break;
//     case Tu64: out << value.u64; break;

//     case Tf32: out << value.f32; break;

//     case Tf64:
//         // always print a float even without decimal point
//         out << fmtstr("{:#}", value.f64);
//         break;

//     case TBool:
//         if (value.boolean) {
//             out << "True";
//         } else {
//             out << "False";
//         }
//         break;

//     case TNone: out << "None"; break;

//     case TString: out << "\"" << value.string << "\""; break;

//     case TObject: _print_object(out); break;

//     default: break;
//     }
// }

// void ConstantValue::_print_object(std::ostream& out) const {

//     if (value.object->class_id == meta::type_id<Object>()) {
//         Array<String> frags;
//         Object const* obj = reinterpret_cast<Object const*>(value.object);

//         for (auto const& attr: obj->attributes) {
//             frags.push_back(str(attr));
//         }

//         out << String("(") << join(", ", frags) << String(")");
//         return;
//     }
//     out << "<object>";
// }
