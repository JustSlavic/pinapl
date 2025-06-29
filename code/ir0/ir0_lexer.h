#ifndef IR0_LEXER_H
#define IR0_LEXER_H

#include "../lexer.h"

enum
{
    Token_Keyword = 0x10000, // 65536
    Token_KeywordInvalid = Token_Keyword,

    Token_KeywordMov,
    Token_KeywordLdr,
    Token_KeywordStr,

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


bool32 ir0_is_valid_identifier_head(char c);
bool32 ir0_is_valid_identifier_body(char c);


#endif // IR0_LEXER_H
