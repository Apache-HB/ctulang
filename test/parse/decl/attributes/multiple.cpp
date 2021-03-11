#include "tstream.hpp"
#include "tlexer.hpp"
#include "tparse.hpp"

int main() {
    auto stream = StringStream(R"(
        @attribute1
        @attribute2
        using num = int;
    )");
    auto lexer = TestLexer(&stream);
    auto parse = TestParser(&lexer);

    parse.expect(
        [&]{ return parse.parseDecl(); }, 
        MAKE<Decorated>(
            vec<ptr<Attribute>>({
                MAKE<Attribute>(
                    parse.qualified({ "attribute1" }),
                    vec<ptr<CallArg>>()
                ),
                MAKE<Attribute>(
                    parse.qualified({ "attribute2" }),
                    vec<ptr<CallArg>>()
                )
            }),
            MAKE<Alias>(
                MAKE<Ident>(lexer.ident("num")),
                parse.qualified({ "int" })
            )
        )
    );

    parse.finish();
}
