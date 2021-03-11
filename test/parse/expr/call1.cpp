#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream("name()");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    vec<ptr<NameType>> names = {
        MAKE<NameType>(
            MAKE<Ident>(
                lexer.ident("name")
            )
        )
    };

    parse.expect(
        [&]{ return parse.parseExpr(); }, 
        MAKE<CallExpr>(
            MAKE<NameExpr>(
                MAKE<QualifiedType>(names)
            ),
            vec<ptr<CallArg>>()
        )
    );

    parse.finish();
}