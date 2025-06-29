#ifndef ASCII_H
#define ASCII_H

#include "common.h"

bool32 is_newline(char c);
bool32 is_crlf(char c);
bool32 is_ascii_space(char c);
bool32 is_ascii_whitespace(char c);
bool32 is_ascii_lowercase_alpha(char c);
bool32 is_ascii_capital_alpha(char c);
bool32 is_ascii_alpha(char c);
bool32 is_ascii_digit(char c);
bool32 is_ascii_hex(char c);
bool32 is_ascii_double_quote(char c);

#endif // ASCII_H
