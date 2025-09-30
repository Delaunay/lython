
// extension to python AST
#include "parser/parser.h"

#define TRACE_START2(tok)                        \
    kwtrace_start((this->parsinglog),            \
                  depth,                         \
                  "{}: {} - `{}`",               \
                  to_string(tok.type()).c_str(), \
                  tok.type(),                    \
                  tok.identifier())

#define TRACE_START() TRACE_START2(token())

#define PARSER_THROW(T, err) throw T(err.message)

namespace lython {

// Primary expression
//
//  https://facebook.github.io/jsx/
//
// JSXElement :
//      JSXSelfClosingElement
//      JSXOpeningElement JSXChildrenopt JSXClosingElement
// 
// JSXSelfClosingElement :
//      < JSXElementName JSXAttributesopt / >
// 
// JSXOpeningElement :
//      < JSXElementName JSXAttributesopt >
//
// JSXClosingElement :
//      < / JSXElementName >
//
// JSXFragment :
//      < > JSXChildrenopt < / >
//
// JSXElementName :
//      JSXIdentifier
//      JSXNamespacedName
//      JSXMemberExpression
//
// JSXIdentifier :
//      IdentifierStart
//      JSXIdentifier IdentifierPart
//      JSXIdentifier [no WhiteSpace or Comment here] -
//
// Note
//  The grammar of JSXIdentifier is not parameterized the same way Identifier is. 
//  This means that <await /> is still a valid production when "[await]" appears during the derivation.
//
// JSXNamespacedName :
//      JSXIdentifier : JSXIdentifier
//
// JSXMemberExpression :
//      JSXIdentifier . JSXIdentifier
//      JSXMemberExpression . JSXIdentifier
//
    
ExprNode* Parser::parse_xml(Node* parent, int depth) {
    TRACE_START();

    auto xml = parent->new_object<XMLNode>();
    start_code_loc(xml, token());

    // Expect '<'
    expect_operator("<", true, xml, LOC);

    // Parse tag name (identifier)
    if (token().type() != tok_identifier) {
        ParsingError& error = parser_kwerror(LOC, "SyntaxError", "Expected tag name after '<'");
        add_wip_expr(error, xml);
        PARSER_THROW(SyntaxError, error);
    }

    xml->tag = get_identifier();
    next_token();

    // Parse attributes (K=V pairs)
    while (token().type() == tok_identifier) {
        String attr_name = get_identifier();
        next_token();

        // Expect '='
        expect_token(tok_assign, true, xml, LOC);

        // Parse attribute value (string, identifier, or {expression})
        ExprNode* attr_value = nullptr;
        if (token().type() == tok_string) {
            attr_value = parse_constant(xml, depth + 1);
        } else if (token().type() == tok_identifier) {
            attr_value = parse_name(xml, depth + 1);
        } else if (token().type() == tok_curly) {
            // Parse expression between {}
            next_token();  // consume '{'
            attr_value = parse_expression(xml, depth + 1);
            expect_token('}', true, xml, LOC);
        } else {
            ParsingError& error =
                parser_kwerror(LOC,
                               "SyntaxError",
                               "Expected attribute value (string, identifier, or {expression})");
            add_wip_expr(error, xml);
            PARSER_THROW(SyntaxError, error);
        }

        xml->properties[attr_name] = attr_value;
    }

    // Check for self-closing tag '/>'
    if (token().type() == tok_operator && token().operator_name() == "/") {
        next_token();
        expect_operator(">", true, xml, LOC);
        end_code_loc(xml, token());
        return xml;
    }

    // Expect '>'
    expect_operator(">", true, xml, LOC);

    // Parse children until we find the closing tag
    while (token().type() != tok_eof) {
        // Check for closing tag
        if (token().type() == tok_operator && token().operator_name() == "<") {
            // Peek ahead to see if this is a closing tag
            Token peek = peek_token();
            if (peek.type() == tok_operator && peek.operator_name() == "/") {
                break;  // This is our closing tag
            }

            // This is a child XML element
            XMLNode* child_xml = static_cast<XMLNode*>(parse_xml(xml, depth + 1));
            xml->children.push_back(child_xml);
        } else {
            // Skip other content for now (could be text nodes, expressions, etc.)
            next_token();
        }
    }

    // Parse closing tag '</tag>'
    expect_operator("<", true, xml, LOC);
    expect_operator("/", true, xml, LOC);

    if (token().type() != tok_identifier || get_identifier() != xml->tag) {
        ParsingError& error = parser_kwerror(
            LOC,
            "SyntaxError",
            fmtstr(
                "Mismatched closing tag, expected '{}' but got '{}'", xml->tag, get_identifier()));
        add_wip_expr(error, xml);
        PARSER_THROW(SyntaxError, error);
    }

    next_token();  // consume the tag name
    expect_operator(">", true, xml, LOC);

    end_code_loc(xml, token());
    return xml;
}

}  // namespace lython
