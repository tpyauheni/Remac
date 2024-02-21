#include <remac/utf8.hpp>

#include <tuple>
#include <string>

// TODO: Check for string end (0 byte)

namespace remac {

std::tuple<unsigned char, Utf8Char> getNextUnicode(const char *inputBytes) {
    Utf8Char utfChar;
    utfChar.codePoint = 0;

    if (inputBytes[0] & 0x80) {
        if (!(inputBytes[0] & 0x40)) {
            // 10xxxxxx
            // INCORRECT(4)
            return { UTF8_INVALID, utfChar };
        }

        // 11111xxx
        // INCORRECT(3)
        if (
            inputBytes[0] & 0x40 && inputBytes[0] & 0x20 && \
            inputBytes[0] & 0x10 && inputBytes[0] & 0x08
        ) {
            return { UTF8_INVALID, utfChar };
        }

        // 11xxxxxx 0xxxxxxx
        // INCORRECT(1)
        // 11xxxxxx 11xxxxxx
        // INCORRECT(2)
        if ((!(inputBytes[1] & 0x80)) || (inputBytes[1] & 0x40)) {
            return { UTF8_INVALID, utfChar };
        }

        utfChar.bytes[0] = inputBytes[0];
        utfChar.bytes[1] = inputBytes[1];

        if (inputBytes[0] & 0x20) {
            // 11xxxxxx 0xxxxxxx
            // INCORRECT(1)
            // 11xxxxxx 11xxxxxx
            // INCORRECT(2)
            if ((!(inputBytes[1] & 0x80)) || (inputBytes[1] & 0x40)) {
                return { UTF8_INVALID, utfChar };
            }

            // 11xxxxxx xxxxxxxx 0xxxxxxx
            // INCORRECT(1)
            // 11xxxxxx xxxxxxxx 11xxxxxx
            // INCORRECT(2)
            if ((!(inputBytes[2] & 0x80)) || (inputBytes[2] & 0x40)) {
                return { UTF8_INVALID, utfChar };
            }

            utfChar.bytes[2] = inputBytes[2];
        } else {
            return { 2, utfChar };
        }

        if (inputBytes[0] & 0x10) {
            // 11xxxxxx 0xxxxxxx
            // INCORRECT(1)
            // 11xxxxxx 11xxxxxx
            // INCORRECT(2)
            if ((!(inputBytes[1] & 0x80)) || (inputBytes[1] & 0x40)) {
                return { UTF8_INVALID, utfChar };
            }

            // 11xxxxxx xxxxxxxx 0xxxxxxx
            // INCORRECT(1)
            // 11xxxxxx xxxxxxxx 11xxxxxx
            // INCORRECT(2)
            if ((!(inputBytes[2] & 0x80)) || (inputBytes[2] & 0x40)) {
                return { UTF8_INVALID, utfChar };
            }

            // 11xxxxxx xxxxxxxx xxxxxxxx 0xxxxxxx
            // INCORRECT(1)
            // 11xxxxxx xxxxxxxx xxxxxxxx 11xxxxxx
            // INCORRECT(2)
            if ((!(inputBytes[3] & 0x80)) || (inputBytes[3] & 0x40)) {
                return { UTF8_INVALID, utfChar };
            }

            utfChar.bytes[3] = inputBytes[3];
        } else {
            return { 3, utfChar };
        }

        return { 4, utfChar };
    } else {
        // 0xxxxxxx
        // CORRECT(1)
        utfChar.bytes[0] = inputBytes[0];
        return { 1, utfChar };
    }
}

Utf8Char utfFromCodePoint(unsigned int codePoint) {
    return Utf8Char { .codePoint = codePoint };
}

Utf8Char fromAsciiChar(char chr) {
    return Utf8Char { .bytes = { chr, 0, 0, 0 } };
}

Utf8Char fromCharArray(char chr[4]) {
    return Utf8Char { .bytes = { chr[0], chr[1], chr[2], chr[3] } };
}

void utfStringAppend(std::string *string, Utf8Char chr) {
    if (chr.bytes[0]) {
        string->push_back(chr.bytes[0]);

        if (chr.bytes[1]) {
            string->push_back(chr.bytes[1]);

            if (chr.bytes[2]) {
                string->push_back(chr.bytes[2]);

                if (chr.bytes[3]) {
                    string->push_back(chr.bytes[3]);
                }
            }
        }
    }
}

bool utfCharInString(const char *stringStart, const char *stringEnd, Utf8Char chr) {
    const char *ptr = stringStart;

    while (ptr < stringEnd) {
        std::tuple<unsigned char, Utf8Char> utf8_tuple = getNextUnicode(ptr);
        ptr += std::get<0>(utf8_tuple);

        if (std::get<1>(utf8_tuple).codePoint == chr.codePoint) {
            return true;
        }
    }

    return false;
}

bool utfCompareSingle(const char *str, Utf8Char chr2) {
    Utf8Char chr1 = std::get<1>(getNextUnicode(str));
    return chr1.codePoint == chr2.codePoint;
}

bool isValidUtf8(const char *stringStart, const char *stringEnd) {
    const char *ptr = stringStart;

    while (ptr < stringEnd) {
        std::tuple<unsigned char, Utf8Char> utf8_tuple = getNextUnicode(ptr);
        unsigned char length = std::get<0>(utf8_tuple);

        if (length == UTF8_INVALID) {
            return false;
        }

        ptr += length;
    }

    return ptr == stringEnd;
}

std::string utfCharToString(Utf8Char chr) {
    std::string str;
    utfStringAppend(&str, chr);
    return str;
}

std::string charToString(char chr) {
    std::string str;
    str.push_back(chr);
    return str;
}

}
