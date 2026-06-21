#ifndef PINAPL_ASCII_H_
#define PINAPL_ASCII_H_

#include "base.h"


bool ascii_is_newline(char c);
bool ascii_is_crlf(char c);
bool ascii_is_space(char c);
bool ascii_is_whitespace(char c);
bool ascii_is_lowercase_alpha(char c);
bool ascii_is_capital_alpha(char c);
bool ascii_is_alpha(char c);
bool ascii_is_digit(char c);
bool ascii_is_double_quote(char c);

bool ascii_is_oct(char c);
int32 ascii_to_oct(char c);

bool ascii_is_dec(char c);
int32 ascii_to_dec(char c);

bool ascii_is_hex(char c);
int32 ascii_to_hex(char c);


#endif /* PINAPL_ASCII_H_ */
