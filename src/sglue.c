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

#include <stdio.h>

#include "defs.h"
#include "sglue.h"
#include "os.h"

/* These functions "glue" scottfree to trs-80 implementations */

void Output(const char* b)
{
    outs(b);
}

void Exit()
{
    for (;;)
    {
        printf("Game Over\n");
        getkey();
    }
}

void Fatal(const char *x)
{
	printf("%s.\n",x);
	Exit();
}

void ClearScreen(void)
{
    // clear top window 

    char* b = vidaddrfor(scrollPos);
    memset(VIDRAM, ' ', b - VIDRAM);

    cursorPos = 0;
    scrollPos = 0;


}

unsigned char RandomPercent(unsigned char n)
{
    unsigned int rv = rand16() % 100;
	if(rv<n)
		return(1);
	return(0);
}

void LineInput(const char* prompt, char *buf, unsigned char sz)
{
    //outchar('\n');
    outs(prompt);

    getline2(buf, sz);
    outchar('\n');
}

void emitTopLine(char* s)
{
    // emit the break line which separates the top and bottom, then
    // set the scroll position to where we are.
    outs(s);
    scrollPos = cursorPos;

    // cause the bottom window to scroll
    setcursor(63, 15);
    outchar('\n');
}




