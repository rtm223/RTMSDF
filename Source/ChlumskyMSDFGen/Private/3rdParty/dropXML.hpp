
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
// DropXML - a simple, header-only XML parser.
// (c) Viktor Chlumsky 2024
// MIT license
// https://github.com/Chlumsky/DropXML

#pragma once

namespace dropXML {

/** Template for your parser consumer class (you can either implement it or make another with the same API)
 *  All functions should return true to continue parsing or false to abort.
 *  IMPLEMENTATION IS NOT PROVIDED!
 */
class ParserConsumer {

public:
    /// Called when a processing instruction like <?xml is encountered.
    bool processingInstruction(const char *nameStart, const char *nameEnd);
    /// Called when a <!DOCTYPE definition is encountered. The entire string between <!DOCTYPE and > is passed, without parsing its content.
    bool doctype(const char *doctypeStart, const char *doctypeEnd);
    /// Called when entering an <element> with the specified name.
    bool enterElement(const char *nameStart, const char *nameEnd);
    /// Called when leaving an </element> with the specified name. It is up to this class to check that the element matches the one currently entered. (Hint: Push onto stack in enterElement, match top of stack and pop in leaveElement)
    bool leaveElement(const char *nameStart, const char *nameEnd);
    /// Called when encountering an attribute name="value" of the last-entered element or processing instruction. All attributes will be submitted before an element's content. Call decode on value.
    bool elementAttribute(const char *nameStart, const char *nameEnd, const char *valueStart, const char *valueEnd);
    /// Called when all attributes have been submitted and before the element's content.
    bool finishAttributes();
    /// Called when a text string is encountered. Leading and trailing whitespace is removed. Multiple consecutive texts can be submitted if separated by comments. Call decode on the string.
    bool text(const char *textStart, const char *textEnd);
    /// Called when a CDATA (character data) block is encountered. Do not call decode on the string.
    bool cdata(const char *cdataStart, const char *cdataEnd);
    /// Called at the end of the XML file.
    bool finish();

};

/** Decodes XML entities in string delimited by start and end to a UTF-8 string and updates the start and end pointers.
 *  Returns true on success, false on insufficient buffer size. Invalid entities are silently copied to output without modification.
 *  Buffer must be at least as long as the string. If no '&' character is encountered, no action will be taken. You can first call this with null buffer pointers to check this without unnecessary allocation.
 */
inline bool decode(const char *&start, const char *&end, char *bufferStart, char *bufferEnd);

/// Parses the XML string delimited by xmlStart and xmlEnd and submits data to a new Consumer instance (see ParserConsumer for API). Returns true on success, false on error or if the parser consumer returns false.
template <typename Consumer = ParserConsumer>
inline bool parse(const char *xmlStart, const char *xmlEnd);

/// Parses the XML string delimited by xmlStart and xmlEnd and submits data to consumer (see ParserConsumer for API). Returns true on success, false on error or if consumer returns false.
template <typename Consumer>
inline bool parse(Consumer &consumer, const char *xmlStart, const char *xmlEnd);

// END OF API, IMPLEMENTATION FOLLOWS









namespace priv {

inline bool isWhitespace(char c) {
    switch (c) {
        case ' ': case '\t': case '\n': case '\r':
            return true;
    }
    return false;
}

inline bool isNameChar(char c) {
    switch (c) {
        case '\0':
        case ' ': case '\t': case '\n': case '\r':
        case '<': case '>': case '/': case '!': case '?': case '&': case '=': case '"': case '\'':
            return false;
    }
    return true;
}

inline void skipWhitespace(const char *&cur, const char *end) {
    while (cur < end && isWhitespace(*cur))
        ++cur;
}

inline void skipWhitespaceAndComments(const char *&cur, const char *end) {
    skipWhitespace(cur, end);
    while (cur+4 <= end && cur[0] == '<' && cur[1] == '!' && cur[2] == '-' && cur[3] == '-') {
        cur += 4;
        while (cur < end) {
            if (*cur == '-' && cur+3 <= end && cur[1] == '-' && cur[2] == '>') {
                cur += 3;
                skipWhitespace(cur, end);
                break;
            }
            ++cur;
        }
    }
}

inline void skipString(const char *&cur, const char *end) {
    char delimiter = *cur++;
    while (cur < end && *cur != delimiter)
        ++cur;
}

inline bool skipDTD(const char *&cur, const char *end) {
    int depth = 1;
    while (cur < end) {
        switch (*cur) {
            case '>':
                if (--depth == 0)
                    return true;
                break;
            case '<':
                ++depth;
                break;
            case '"': case '\'':
                skipString(cur, end);
                if (cur >= end)
                    return false;
                break;
        }
        ++cur;
    }
    return false;
}

template <typename Consumer>
inline bool parseAttribute(Consumer &consumer, const char *&cur, const char *end) {
    const char *nameStart = cur;
    while (cur < end && isNameChar(*cur))
        ++cur;
    const char *nameEnd = cur;
    if (nameEnd <= nameStart)
        return false;
    skipWhitespace(cur, end);
    if (!(cur < end && *cur == '='))
        return false;
    ++cur;
    skipWhitespace(cur, end);
    if (!(cur < end && (*cur == '"' || *cur == '\'')))
        return false;
    const char *valueStart = cur+1;
    skipString(cur, end);
    const char *valueEnd = cur;
    if (!(cur < end && consumer.elementAttribute(nameStart, nameEnd, valueStart, valueEnd)))
        return false;
    ++cur;
    return true;
}

inline bool encodeUTF8(char *&cur, char *end, unsigned long codepoint) {
    if (cur >= end)
        return false;
    if (codepoint < 0x80u) {
        *cur++ = char(codepoint);
        return true;
    }
    int length = 1;
    while (codepoint>>(1+5*length) && length < 6)
        ++length;
    if (cur+length > end)
        return false;
    *cur++ = char((0xffu<<(8-length))|(codepoint>>((length-1)*6)));
    for (int i = 1; i < length; ++i)
        *cur++ = char(0x80u|((codepoint>>((length-i-1)*6))&0x3fu));
    return true;
}

inline bool readNumericEntity(const char *&cur, const char *end, unsigned long &charCode) {
    // Hexadecimal
    if (cur < end && (*cur == 'x' || *cur == 'X')) {
        ++cur;
        while (cur < end) {
            switch (*cur++) {
                case '0': charCode = 0x10*charCode+0x00; continue;
                case '1': charCode = 0x10*charCode+0x01; continue;
                case '2': charCode = 0x10*charCode+0x02; continue;
                case '3': charCode = 0x10*charCode+0x03; continue;
                case '4': charCode = 0x10*charCode+0x04; continue;
                case '5': charCode = 0x10*charCode+0x05; continue;
                case '6': charCode = 0x10*charCode+0x06; continue;
                case '7': charCode = 0x10*charCode+0x07; continue;
                case '8': charCode = 0x10*charCode+0x08; continue;
                case '9': charCode = 0x10*charCode+0x09; continue;
                case 'a': case 'A': charCode = 0x10*charCode+0x0a; continue;
                case 'b': case 'B': charCode = 0x10*charCode+0x0b; continue;
                case 'c': case 'C': charCode = 0x10*charCode+0x0c; continue;
                case 'd': case 'D': charCode = 0x10*charCode+0x0d; continue;
                case 'e': case 'E': charCode = 0x10*charCode+0x0e; continue;
                case 'f': case 'F': charCode = 0x10*charCode+0x0f; continue;
                case ';': return true;
                default: return false;
            }
        }
    // Decimal
    } else {
        while (cur < end) {
            switch (*cur++) {
                case '0': charCode = 10*charCode+0; continue;
                case '1': charCode = 10*charCode+1; continue;
                case '2': charCode = 10*charCode+2; continue;
                case '3': charCode = 10*charCode+3; continue;
                case '4': charCode = 10*charCode+4; continue;
                case '5': charCode = 10*charCode+5; continue;
                case '6': charCode = 10*charCode+6; continue;
                case '7': charCode = 10*charCode+7; continue;
                case '8': charCode = 10*charCode+8; continue;
                case '9': charCode = 10*charCode+9; continue;
                case ';': return true;
                default: return false;
            }
        }
    }
    return false;
}

}

template <typename Consumer>
inline bool parse(const char *xmlStart, const char *xmlEnd) {
    Consumer consumer;
    return parse(consumer, xmlStart, xmlEnd);
}

template <typename Consumer>
inline bool parse(Consumer &consumer, const char *xmlStart, const char *xmlEnd) {
    using namespace priv;
    const char *cur = xmlStart, *end = xmlEnd;
    int depth = 0;
    // BOM
    if (cur+3 <= end && cur[0] == '\xef' && cur[1] == '\xbb' && cur[2] == '\xbf')
        cur += 3;
    for (skipWhitespaceAndComments(cur, end); cur < end; skipWhitespaceAndComments(cur, end)) {
        if (*cur == '<') {
            ++cur;
            // CDATA
            if (cur+8 <= end && cur[0] == '!' && cur[1] == '[' && cur[2] == 'C' && cur[3] == 'D' && cur[4] == 'A' && cur[5] == 'T' && cur[6] == 'A' && cur[7] == '[') {
                cur += 8;
                const char *cdataStart = cur;
                bool terminated = false;
                while (cur < end) {
                    if (*cur == ']' && cur+3 <= end && cur[1] == ']' && cur[2] == '>') {
                        const char *cdataEnd = cur;
                        if (!consumer.cdata(cdataStart, cdataEnd))
                            return false;
                        terminated = true;
                        cur += 3;
                        break;
                    }
                    ++cur;
                }
                if (!terminated)
                    return false;
                continue;
            }
            // DOCTYPE
            if (cur+8 <= end && cur[0] == '!' && cur[1] == 'D' && cur[2] == 'O' && cur[3] == 'C' && cur[4] == 'T' && cur[5] == 'Y' && cur[6] == 'P' && cur[7] == 'E' && !isNameChar(cur[8])) {
                cur += 8;
                skipWhitespace(cur, end);
                const char *doctypeStart = cur;
                if (!skipDTD(cur, end))
                    return false;
                const char *doctypeEnd = cur++;
                while (doctypeEnd > doctypeStart && isWhitespace(doctypeEnd[-1]))
                    --doctypeEnd;
                if (!consumer.doctype(doctypeStart, doctypeEnd))
                    return false;
                continue;
            }
            skipWhitespace(cur, end);
            // XML element (beginning / end) or processing instruction
            bool processingInstruction = cur < end && *cur == '?';
            bool elementEnd = cur < end && *cur == '/' && !processingInstruction;
            if (elementEnd || processingInstruction) {
                ++cur;
                skipWhitespace(cur, end);
            }
            const char *nameStart = cur;
            while (cur < end && isNameChar(*cur))
                ++cur;
            const char *nameEnd = cur;
            if (nameEnd <= nameStart)
                return false;
            skipWhitespace(cur, end);
            if (!elementEnd) {
                if (processingInstruction) {
                    if (!consumer.processingInstruction(nameStart, nameEnd))
                        return false;
                } else {
                    ++depth;
                    if (!consumer.enterElement(nameStart, nameEnd))
                        return false;
                }
                while (cur < end && isNameChar(*cur)) {
                    if (!parseAttribute(consumer, cur, end))
                        return false;
                    skipWhitespace(cur, end);
                }
                if (cur < end && *cur == '/' && !processingInstruction) {
                    elementEnd = true;
                    ++cur;
                    skipWhitespace(cur, end);
                }
                if (!consumer.finishAttributes())
                    return false;
            }
            if (elementEnd) {
                if (!(--depth >= 0 && consumer.leaveElement(nameStart, nameEnd)))
                    return false;
            }
            if (processingInstruction) {
                if (!(cur < end && *cur == '?'))
                    return false;
                ++cur;
                skipWhitespace(cur, end);
            }
            if (!(cur < end && *cur == '>'))
                return false;
            ++cur;
        // Text
        } else {
            const char *textStart = cur;
            while (cur < end && *cur != '<')
                ++cur;
            const char *textEnd = cur;
            while (textEnd > textStart && isWhitespace(textEnd[-1]))
                --textEnd;
            if (!consumer.text(textStart, textEnd))
                return false;
        }
    }
    return depth == 0 && consumer.finish();
}

inline bool decode(const char *&start, const char *&end, char *bufferStart, char *bufferEnd) {
    using namespace priv;
    for (const char *cur = start, *inputEnd = end; cur < inputEnd; ++cur) {
        if (*cur == '&') {
            const char *inputCur = start;
            char *bufferCur = bufferStart;
            while (inputCur < inputEnd) {
                if (bufferCur >= bufferEnd)
                    return false;
                if (*inputCur == '&') {
                    if (inputCur+4 <= inputEnd) {
                        // &lt;
                        if (inputCur[1] == 'l' && inputCur[2] == 't' && inputCur[3] == ';') {
                            inputCur += 4;
                            *bufferCur++ = '<';
                            continue;
                        }
                        // &gt;
                        if (inputCur[1] == 'g' && inputCur[2] == 't' && inputCur[3] == ';') {
                            inputCur += 4;
                            *bufferCur++ = '>';
                            continue;
                        }
                        // &amp;
                        if (inputCur[1] == 'a' && inputCur[2] == 'm' && inputCur[3] == 'p' && inputCur+5 <= inputEnd && inputCur[4] == ';') {
                            inputCur += 5;
                            *bufferCur++ = '&';
                            continue;
                        }
                        // &apos;
                        if (inputCur[1] == 'a' && inputCur[2] == 'p' && inputCur[3] == 'o' && inputCur+6 <= inputEnd && inputCur[4] == 's' && inputCur[5] == ';') {
                            inputCur += 6;
                            *bufferCur++ = '\'';
                            continue;
                        }
                        // &quot;
                        if (inputCur[1] == 'q' && inputCur[2] == 'u' && inputCur[3] == 'o' && inputCur+6 <= inputEnd && inputCur[4] == 't' && inputCur[5] == ';') {
                            inputCur += 6;
                            *bufferCur++ = '"';
                            continue;
                        }
                    }
                    // Numeric character code
                    if (inputCur+2 <= inputEnd && inputCur[1] == '#') {
                        const char *inputPrev = inputCur;
                        inputCur += 2;
                        unsigned long charCode = 0;
                        if (readNumericEntity(inputCur, inputEnd, charCode)) {
                            if (!encodeUTF8(bufferCur, bufferEnd, charCode))
                                return false;
                            continue;
                        }
                        inputCur = inputPrev;
                    }
                    // Return false here to fail on invalid entity, otherwise it will be copied as-is
                }
                *bufferCur++ = *inputCur++;
            }
            start = bufferStart;
            end = bufferCur;
            break;
        }
    }
    return true;
}

}
