#include <stdio.h>

#include <base.h>
#include <console.hpp>
#include <util.hpp>

#include "../unit_testing.hpp"

#include "../lexer.hpp"
#include "../parser.hpp"


#include "parse_types.hpp"


int main()
{
    TEST_BEGIN();

    RUN_TEST_SUITE__PARSING();

    TEST_END();
    return 0;
}

#include "../lexer.cpp"
#include "../parser.cpp"
#include <util.cpp>
#include <lexer.cpp>
