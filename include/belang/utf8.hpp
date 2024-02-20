#pragma once

#ifndef BELCOMP_BELANG_UTF8
#define BELCOMP_BELANG_UTF8 1

#include <tuple>
#include <string>

#define UTF8_INVALID ((unsigned char)0)

/*
Possible UTF-8 chars (CORRECT(x)):
1. Single-byte: 0xxxxxxx
2. Two-byte:    110xxxxx 10xxxxxx
3. Three-byte:  1110xxxx 10xxxxxx 10xxxxxx
4. Four-byte:   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

Impossible UTF-8 chars (INCORRECT(x)):
1. 1xxxxxxx 0xxxxxxx
2. 1xxxxxxx 11xxxxxx
3. 11111xxx
4. 10xxxxxx
*/

namespace belang {

union Utf8Char {
    char bytes[4];
    unsigned int codePoint;
};

std::tuple<unsigned char, Utf8Char> getNextUnicode(const char *inputBytes);
Utf8Char utfFromCodePoint(unsigned int codePoint);
Utf8Char fromAsciiChar(char chr);
Utf8Char fromCharArray(char chr[4]);
void utfStringAppend(std::string *string, Utf8Char chr);
bool utfCharInString(const char *stringStart, const char *stringEnd, Utf8Char chr);
bool utfCompareSingle(const char *string, Utf8Char chr2);
bool isValidUtf8(const char *stringStart, const char *stringEnd);
std::string utfCharToString(Utf8Char chr);

std::string charToString(char chr);

}

#endif // BELCOMP_BELANG_UTF8
