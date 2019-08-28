/**
 *
 *    _    __        _      __                           
 *   | |  / /____   (_)____/ /_      __ ____ _ _____ ___ 
 *   | | / // __ \ / // __  /| | /| / // __ `// ___// _ \
 *   | |/ // /_/ // // /_/ / | |/ |/ // /_/ // /   /  __/
 *   |___/ \____//_/ \__,_/  |__/|__/ \__,_//_/    \___/ 
 *                                                       
 *  Copyright (©) Voidware 2019.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 * 
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 * 
 *  contact@voidware.com
 */

// helper utility functions

#include <ctype.h>

signed char u_strnicmp(const char* s1, const char* s2, unsigned int n)
{
    signed char v = 0;
    while (n) 
    {
        --n;

        char c1 = tolower(*s1);
        char c2 = tolower(*s2);

        v = (c2 - c1);
        if (v || !*s1) break;
        ++s1;
        ++s2;
    }
    return v;
}

signed char u_stricmp(const char* s1, const char* s2)
{
    signed char v;
    for (;;)
    {
        char c1 = tolower(*s1);
        char c2 = tolower(*s2);
        v = c2 - c1;
        if (v || !*s1) break;
        ++s1;
        ++s2;
    }
    return v;
}


#if 0
unsigned char isqrt16(unsigned short a)
{
    // 16 bit version, valid up to 16383
    unsigned short rem = 0;
    unsigned char root = 0;
    unsigned char i;
    for (i = 0; i < 8; ++i)
    {
        root <<= 1;
        rem = (rem << 2) + (a >> 14);
        a <<= 2;
        if (root < rem)
        {
            ++root;
            rem -= root;
            ++root;
        }
    }
    return root >> 1;
}
#endif
