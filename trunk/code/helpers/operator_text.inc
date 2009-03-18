#ifndef PREFIX
#define PREFIX(TOK, TEXT, NODE, ASS)
#endif

#ifndef POSTFIX
#define POSTFIX(TOK, TEXT, NODE, ASS)
#endif

#ifndef BINARY
#define BINARY(TOK, TEXT, NODE, ASS)
#endif

#ifndef ALTBIN
#define ALTBIN(TOK, TEXT, NODE, ASS)
#endif

PREFIX(amp                  , "&",   AddressOf, NonAssignment )
BINARY(amp                  , "&",   BitwiseAnd, NonAssignment )
BINARY(ampamp               , "&&",  LocicalAnd, NonAssignment )
ALTBIN(ampequal             , "&=",  BitwiseAnd, Assignment )
BINARY(star                 , "*",   Multiply, NonAssignment )
PREFIX(star                 , "*",   Dereference, NonAssignment )
ALTBIN(starequal            , "*=",  Multiply, Assignment )
BINARY(plus                 , "+",   Add, NonAssignment )
PREFIX(plusplus             , "++",  PreIncrement, Assignment )
POSTFIX(plusplus            , "++",  PostIncrement, Assignment )
ALTBIN(plusequal            , "+=",  Add, Assignment )
PREFIX(minus                , "-",   Negate, NonAssignment )
BINARY(minus                , "-",   Subtract, NonAssignment )
PREFIX(minusminus           , "--",  PreDecrement, Assignment )
POSTFIX(minusminus          , "--",  PostDecrement, Assignment )
ALTBIN(minusequal           , "-=",  Subtract, Assignment )
PREFIX(tilde                , "~",   BitwiseNot, NonAssignment )
PREFIX(exclaim              , "!",   LogicalNot, NonAssignment )
BINARY(exclaimequal         , "!=",  NotEqual, NonAssignment )
BINARY(slash                , "/",   Divide, NonAssignment )
ALTBIN(slashequal           , "/=",  Divide, Assignment )
BINARY(percent              , "%",   Modulo, NonAssignment )
ALTBIN(percentequal         , "%=",  Modulo, Assignment )
BINARY(less                 , "<",   Less, NonAssignment )
BINARY(lessless             , "<<",  ShiftLeft, NonAssignment )
BINARY(lessequal            , "<=",  LessOrEqual, NonAssignment )
ALTBIN(lesslessequal        , "<<=", ShiftLeft, Assignment )
BINARY(greater              , ">",   Greater, NonAssignment )
BINARY(greatergreater       , ">>",  ShiftRight, NonAssignment )
BINARY(greaterequal         , ">=",  GreaterOrEqual, NonAssignment )
ALTBIN(greatergreaterequal  , ">>=", ShiftRight, Assignment )
BINARY(caret                , "^",   Eor, NonAssignment )
ALTBIN(caretequal           , "^=",  Eor, Assignment )
BINARY(pipe                 , "|",   BitwiseOr, NonAssignment )
BINARY(pipepipe             , "||",  LogicalOr, NonAssignment )
ALTBIN(pipeequal            , "|=",  BitwiseOr, Assignment )
BINARY(equal                , "",   Assign, Assignment )
BINARY(equalequal           , "==",  Equal, NonAssignment )
BINARY(comma                , ",",   Comma, NonAssignment ) 

#undef PREFIX
#undef POSTFIX
#undef BINARY
#undef ALTBIN
