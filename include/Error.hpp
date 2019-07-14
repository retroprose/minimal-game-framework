#ifndef ERROR_HPP
#define ERROR_HPP

#include <iostream>

/*
    I am defining my own assert here just for added
    flexibility and the ability to remove it from all
    files by changing one line.
    (just uncomment the next line, and comment out the next)
*/

//#define ASSERT(x)
#define ASSERT(x) Assert(#x, x, __FILE__, __LINE__)

bool Assert(const char* s, bool b, const char* file, int line) {
    if (b == false) {
        std::cout << "Assertion failed: (" << s << "), file " << file << ", line " << line << ".\n";
        //printf("Assertion failed: (%s), file %s, line %d.\n", s, file, line);
        abort();
        //throw 0;
    }
    return b;
}

#endif // ERROR_HPP
