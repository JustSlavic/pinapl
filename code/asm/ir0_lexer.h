#ifndef IR0_LEXER_H
#define IR0_LEXER_H

#include "lexer.h"

enum
{
    Token_Keyword = 0x10000, // 65536
    Token_KeywordInvalid = Token_Keyword,

    Token_KeywordMov,
    Token_KeywordAdd,
    Token_KeywordSub,
    Token_KeywordMul,
    Token_KeywordDiv,
    Token_KeywordRet,

    Token_KeywordCmp,

    Token_KeywordJmp,
    Token_KeywordJeq,
    Token_KeywordJne,
    Token_KeywordJlt,
    Token_KeywordJle,
    Token_KeywordJgt,
    Token_KeywordJge,
};


#endif // IR0_LEXER_H
