// Not a header file - this should be included wherever a list of info about 
// all the operators is required - if any of INFIX, PREFIX, POSTFIX are defined
// they are invoked for each operator with pertinent info in the parameters 

// save typing by defining any macros not yet defined
#ifndef PREFIX
#define PREFIX(TOK, TEXT, NODE, BASE)
#endif
#ifndef POSTFIX
#define POSTFIX(TOK, TEXT, NODE, BASE)
#endif
#ifndef INFIX
#define INFIX(TOK, TEXT, NODE, BASE)
#endif

/*
Syntax  Clang enum           Text   Inferno node          Intermediate          */
PREFIX(amp                 , "&",   AddressOf,            Operator )
INFIX(amp                  , "&",   BitwiseAnd,           Bitwise )
INFIX(ampamp               , "&&",  LocicalAnd,           Logical )
INFIX(ampequal             , "&=",  AssignmentBitwiseAnd, AssignmentOperator )
INFIX(star                 , "*",   Multiply,             Arithmetic )
PREFIX(star                , "*",   Dereference,          Operator )
INFIX(starequal            , "*=",  AssignmentMultiply,   AssignmentOperator )
INFIX(plus                 , "+",   Add,                  Arithmetic )
PREFIX(plusplus            , "++",  PreIncrement,         AssignmentOperator )
POSTFIX(plusplus           , "++",  PostIncrement,        AssignmentOperator )
INFIX(plusequal            , "+=",  AssignmentAdd,        AssignmentOperator )
PREFIX(minus               , "-",   Negate,               Arithmetic )
INFIX(minus                , "-",   Subtract,             Arithmetic )
PREFIX(minusminus          , "--",  PreDecrement,         AssignmentOperator )
POSTFIX(minusminus         , "--",  PostDecrement,        AssignmentOperator )
INFIX(minusequal           , "-=",  AssignmentSubtract,   AssignmentOperator )
PREFIX(tilde               , "~",   BitwiseNot,           Bitwise )
PREFIX(exclaim             , "!",   LogicalNot,           Bitwise )
INFIX(exclaimequal         , "!=",  NotEqual,             Comparison )
INFIX(slash                , "/",   Divide,               Arithmetic )
INFIX(slashequal           , "/=",  AssignmentDivide,     AssignmentOperator )
INFIX(percent              , "%",   Modulo,               Arithmetic )
INFIX(percentequal         , "%=",  AssignmentModulo,     AssignmentOperator )
INFIX(less                 , "<",   Less,                 Comparison )
INFIX(lessless             , "<<",  ShiftLeft,            Shift )
INFIX(lessequal            , "<=",  LessOrEqual,          Comparison )
INFIX(lesslessequal        , "<<=", AssignmentShiftLeft,  AssignmentOperator )
INFIX(greater              , ">",   Greater,              Comparison )
INFIX(greatergreater       , ">>",  ShiftRight,           Shift )
INFIX(greaterequal         , ">=",  GreaterOrEqual,       Comparison )
INFIX(greatergreaterequal  , ">>=", AssignmentShiftRight, AssignmentOperator )
INFIX(caret                , "^",   Eor,                  Bitwise )
INFIX(caretequal           , "^=",  AssignmentEor,        AssignmentOperator )
INFIX(pipe                 , "|",   BitwiseOr,            Bitwise )
INFIX(pipepipe             , "||",  LogicalOr,            Logical )
INFIX(pipeequal            , "|=",  AssignmentBitwiseOr,  AssignmentOperator )
INFIX(equal                , "=",   Assign,               Operator )
INFIX(equalequal           , "==",  Equal,                Comparison )
INFIX(comma                , ",",   Comma,                Operator ) 

// save typing by undefining macros
#undef PREFIX
#undef POSTFIX
#undef INFIX
