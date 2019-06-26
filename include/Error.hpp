#ifndef ERROR_HPP
#define ERROR_HPP

#include <iostream>

/*
#define ASSERT(x) Assert(#x, x)

void Assert(const char* s, bool b) {
    if (b == false) {
        std::cout << "Assertion failed: (" << s << "), file " << __FILE__ << ", line " << __LINE__ << ".\n";
        //printf("Assertion failed: (%s), file %s, line %d.\n", s, __FILE__, __LINE__);
        abort();
        //throw 0;
    }
}
*/

#define ASSERT(x) \
{ \
    if (!(x)) \
    { \
        std::cout << "Assertion failed: (" << #x << "), file " << __FILE__ << ", line " << __LINE__ << ".\n"; \
        abort(); \
    } \
}


#endif // ERROR_HPP
