#ifndef PREFIX
#define PREFIX(TOK, TEXT, NODE, ASS, BASE)
#endif

#ifndef POSTFIX
#define POSTFIX(TOK, TEXT, NODE, ASS, BASE)
#endif

#ifndef BINARY
#define BINARY(TOK, TEXT, NODE, ASS, BASE)
#endif

#ifndef ALTBIN
#define ALTBIN(TOK, NODE, ASS)
#endif

PREFIX(amp                  , "&",   AddressOf, NonAssignment, Operator )
BINARY(amp                  , "&",   BitwiseAnd, NonAssignment, Bitwise )
BINARY(ampamp               , "&&",  LocicalAnd, NonAssignment, Logical )
ALTBIN(ampequal             ,        BitwiseAnd, Assignment )
BINARY(star                 , "*",   Multiply, NonAssignment, Arithmetic )
PREFIX(star                 , "*",   Dereference, NonAssignment, Operator )
ALTBIN(starequal            ,        Multiply, Assignment )
BINARY(plus                 , "+",   Add, NonAssignment, Arithmetic )
PREFIX(plusplus             , "++",  PreIncrement, Assignment, Arithmetic )
POSTFIX(plusplus            , "++",  PostIncrement, Assignment, Arithmetic )
ALTBIN(plusequal            ,        Add, Assignment )
PREFIX(minus                , "-",   Negate, NonAssignment, Arithmetic )
BINARY(minus                , "-",   Subtract, NonAssignment, Arithmetic )
PREFIX(minusminus           , "--",  PreDecrement, Assignment, Arithmetic )
POSTFIX(minusminus          , "--",  PostDecrement, Assignment, Arithmetic )
ALTBIN(minusequal           ,        Subtract, Assignment )
PREFIX(tilde                , "~",   BitwiseNot, NonAssignment, Bitwise )
PREFIX(exclaim              , "!",   LogicalNot, NonAssignment, Bitwise )
BINARY(exclaimequal         , "!=",  NotEqual, NonAssignment, Comparison )
BINARY(slash                , "/",   Divide, NonAssignment, Arithmetic )
ALTBIN(slashequal           ,        Divide, Assignment )
BINARY(percent              , "%",   Modulo, NonAssignment, Arithmetic )
ALTBIN(percentequal         ,        Modulo, Assignment )
BINARY(less                 , "<",   Less, NonAssignment, Comparison )
BINARY(lessless             , "<<",  ShiftLeft, NonAssignment, Shift )
BINARY(lessequal            , "<=",  LessOrEqual, NonAssignment, Comparison )
ALTBIN(lesslessequal        ,        ShiftLeft, Assignment )
BINARY(greater              , ">",   Greater, NonAssignment, Comparison )
BINARY(greatergreater       , ">>",  ShiftRight, NonAssignment, Shift )
BINARY(greaterequal         , ">=",  GreaterOrEqual, NonAssignment, Comparison )
ALTBIN(greatergreaterequal  ,        ShiftRight, Assignment )
BINARY(caret                , "^",   Eor, NonAssignment, Bitwise )
ALTBIN(caretequal           ,        Eor, Assignment )
BINARY(pipe                 , "|",   BitwiseOr, NonAssignment, Bitwise )
BINARY(pipepipe             , "||",  LogicalOr, NonAssignment, Logical )
ALTBIN(pipeequal            ,        BitwiseOr, Assignment )
BINARY(equal                , "",    Assign, Assignment, Operator )
BINARY(equalequal           , "==",  Equal, NonAssignment, Comparison )
BINARY(comma                , ",",   Comma, NonAssignment, Operator ) 

#undef PREFIX
#undef POSTFIX
#undef BINARY
#undef ALTBIN
