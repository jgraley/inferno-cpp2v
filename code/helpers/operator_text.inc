#ifndef PREFIX
#define PREFIX(TOK, TEXT, NODE)
#endif

#ifndef POSTFIX
#define POSTFIX(TOK, TEXT, NODE)
#endif

#ifndef BINARY
#define BINARY(TOK, TEXT, NODE)
#endif

#ifndef ASSIGN
#define ASSIGN(TOK, TEXT, NODE)
#endif

PREFIX(amp                  , "&",   AddressOf )
BINARY(amp                  , "&",   BitwiseAnd )
BINARY(ampamp               , "&&",  LocicalAnd )
ASSIGN(ampequal             , "&=",  BitwiseAnd )
BINARY(star                 , "*",   Multiply )
PREFIX(star                 , "*",   Dereference )
ASSIGN(starequal            , "*=",  Multiply )
BINARY(plus                 , "+",   Add )
PREFIX(plusplus             , "++",  PreIncrement )
POSTFIX(plusplus            , "++",  PostIncrement )
ASSIGN(plusequal            , "+=",  Add )
PREFIX(minus                , "-",   Negate )
BINARY(minus                , "-",   Subtract )
PREFIX(minusminus           , "--",  PreDecrement )
POSTFIX(minusminus          , "--",  PostDecrement )
ASSIGN(minusequal           , "-=",  Subtract )
PREFIX(tilde                , "~",   BitwiseNot )
PREFIX(exclaim              , "!",   LogicalNot )
BINARY(exclaimequal         , "!=",  NotEqual )
BINARY(slash                , "/",   Divide )
ASSIGN(slashequal           , "/=",  Divide )
BINARY(percent              , "%",   Modulo )
ASSIGN(percentequal         , "%=",  Modulo )
BINARY(less                 , "<",   Less )
BINARY(lessless             , "<<",  ShiftLeft )
BINARY(lessequal            , "<=",  LessOrEqual )
ASSIGN(lesslessequal        , "<<=", ShiftLeft )
BINARY(greater              , ">",   Greater )
BINARY(greatergreater       , ">>",  ShiftRight )
BINARY(greaterequal         , ">=",  GreaterOrEqual )
ASSIGN(greatergreaterequal  , ">>=", ShiftRight )
BINARY(caret                , "^",   Eor )
ASSIGN(caretequal           , "^=",  Eor )
BINARY(pipe                 , "|",   BitwiseOr )
BINARY(pipepipe             , "||",  LogicalOr )
ASSIGN(pipeequal            , "|=",  BitwiseOr )
ASSIGN(equal                , "=",   Assign )
BINARY(equalequal           , "==",  Equal )
BINARY(comma                , ",",   Comma ) 

#undef PREFIX
#undef POSTFIX
#undef BINARY
#undef ASSIGN
