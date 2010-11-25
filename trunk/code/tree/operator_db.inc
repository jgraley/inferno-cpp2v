// Not a header file - this should be included wherever a list of info about 
// all the operators is required - if any of INFIX, PREFIX, POSTFIX are defined
// they are invoked for each operator with pertinent info in the parameters 

// save typing by defining any macros not yet defined
#ifndef PREFIX
#define PREFIX(TOK, TEXT, NODE, BASE, CAT)
#endif
#ifndef POSTFIX
#define POSTFIX(TOK, TEXT, NODE, BASE, CAT)
#endif
#ifndef INFIX
#define INFIX(TOK, TEXT, NODE, BASE, CAT)
#endif
#ifndef OTHER
#define OTHER(TOK, TEXT, NODE, BASE, CAT)
#endif

/*
Syntax  Clang token          Text   Inferno node          Intermediate         Category */

PREFIX(amp                 , "&",   AddressOf,            Unop,                   SPECIAL )
INFIX(amp                  , "&",   BitwiseAnd,           CommutativeBinop,       BITWISE )
INFIX(ampamp               , "&&",  LogicalAnd,           Binop,                  LOGICAL )
INFIX(ampequal             , "&=",  AssignmentBitwiseAnd, AssignmentOperator,     BITWISE )
INFIX(star                 , "*",   Multiply,             CommutativeBinop,       ARITHMETIC )
PREFIX(star                , "*",   Dereference,          Unop,                   SPECIAL )
INFIX(starequal            , "*=",  AssignmentMultiply,   AssignmentOperator,     ARITHMETIC )
INFIX(plus                 , "+",   Add,                  CommutativeBinop,       ARITHMETIC )
PREFIX(plusplus            , "++",  PreIncrement,         AssignmentOperator,     ARITHMETIC ) // operator++()
POSTFIX(plusplus           , "++",  PostIncrement,        AssignmentOperator,     ARITHMETIC ) // operator++(int)
INFIX(plusequal            , "+=",  AssignmentAdd,        AssignmentOperator,     ARITHMETIC )
PREFIX(minus               , "-",   Negate,               Unop,                   ARITHMETIC )
INFIX(minus                , "-",   Subtract,             Binop,                  ARITHMETIC )
PREFIX(minusminus          , "--",  PreDecrement,         AssignmentOperator,     ARITHMETIC ) // operator--()
POSTFIX(minusminus         , "--",  PostDecrement,        AssignmentOperator,     ARITHMETIC ) // operator--(int)
INFIX(minusequal           , "-=",  AssignmentSubtract,   AssignmentOperator,     ARITHMETIC )
PREFIX(tilde               , "~",   BitwiseNot,           Unop,                   BITWISE )
PREFIX(exclaim             , "!",   LogicalNot,           Unop,                   LOGICAL )
INFIX(exclaimequal         , "!=",  NotEqual,             CommutativeBinop,       COMPARISON )
INFIX(slash                , "/",   Divide,               Binop,                  ARITHMETIC )
INFIX(slashequal           , "/=",  AssignmentDivide,     AssignmentOperator,     ARITHMETIC )
INFIX(percent              , "%",   Modulo,               Binop,                  ARITHMETIC )
INFIX(percentequal         , "%=",  AssignmentModulo,     AssignmentOperator,     ARITHMETIC )
INFIX(less                 , "<",   Less,                 Binop,                  COMPARISON )
INFIX(lessless             , "<<",  ShiftLeft,            Binop,                  SHIFT )
INFIX(lessequal            , "<=",  LessOrEqual,          Binop,                  COMPARISON )
INFIX(lesslessequal        , "<<=", AssignmentShiftLeft,  AssignmentOperator,     SHIFT )
INFIX(greater              , ">",   Greater,              Binop,                  COMPARISON )
INFIX(greatergreater       , ">>",  ShiftRight,           Binop,                  SHIFT )
INFIX(greaterequal         , ">=",  GreaterOrEqual,       Binop,                  COMPARISON )
INFIX(greatergreaterequal  , ">>=", AssignmentShiftRight, AssignmentOperator,     SHIFT )
INFIX(caret                , "^",   Eor,                  CommutativeBinop,       BITWISE )
INFIX(caretequal           , "^=",  AssignmentEor,        AssignmentOperator,     BITWISE )
INFIX(pipe                 , "|",   BitwiseOr,            CommutativeBinop,       BITWISE )
INFIX(pipepipe             , "||",  LogicalOr,            Binop,                  LOGICAL )
INFIX(pipeequal            , "|=",  AssignmentBitwiseOr,  AssignmentOperator,     BITWISE )
INFIX(equal                , "=",   Assign,               AssignmentOperator,     SPECIAL )
INFIX(equalequal           , "==",  Equal,                CommutativeBinop,       COMPARISON )
INFIX(comma                , ",",   Comma,                Binop,                  SPECIAL ) 
OTHER(0                    , "",    Multiplexor,          Ternop,                 SPECIAL )  // a?b:c
OTHER(0                    , "",    Subscript,            Binop,                  SPECIAL )  // a[b]
OTHER(0                    , "",    MakeArray,            NonCommutativeOperator, SPECIAL ) // {a, b, c}
OTHER(0                    , "",    This,                 Operator,               SPECIAL ) // this

// save typing by undefining macros
#undef PREFIX
#undef POSTFIX
#undef INFIX
#undef OTHER
