/**
 *
 *    _    __        _      __                           
 *   | |  / /____   (_)____/ /_      __ ____ _ _____ ___ 
 *   | | / // __ \ / // __  /| | /| / // __ `// ___// _ \
 *   | |/ // /_/ // // /_/ / | |/ |/ // /_/ // /   /  __/
 *   |___/ \____//_/ \__,_/  |__/|__/ \__,_//_/    \___/ 
 *                                                       
 *  Copyright (�) Voidware 2019.
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
#include <setjmp.h>
#include <ctype.h>

#include "defs.h"
#include "sglue.h"
#include "scottfree.h"
#include "os.h"

/* These functions "glue" scottfree to trs-80 implementations */

void Output(const char* b)
{
    char c;
    for (;;)
    {
        c = *b++;
        if (!c) break;
        outchar(c);
        ++pos;
        if (c == '\n') pos = 0;
        lastChar = c;
    }
}

void OutputNumber(int a)
{
    outint(a);
}

extern jmp_buf main_env;

void Exit()
{
    resetGame();
    scrollPos = 0;
    longjmp(main_env, 1);
}

void Fatal(const char *x)
{
	printf_simple("%s.\n",x);
	Exit();
}

void ClearScreen(void)
{
    // clear top window 

    char* b = vidaddrfor(scrollPos);
    memset(vidRam, ' ', b - vidRam);

    cursorPos = 0;
    scrollPos = 0;
}

unsigned char RandomPercent(unsigned char n)
{
    uchar r = 0;
    if (n)
    {
        r = (rand16() % 100) < n;
    }
    return r;
}

void LineInput(const char* prompt, char *buf, unsigned char sz)
{
    Output(prompt);

    getline(buf, sz);
    outchar('\n');
}

char CharInput(const char* prompt)
{
    char c = getSingleChar(prompt);
    //outchar('\n');
    return c;
}

void emitTopLine(char* s)
{
    // emit the break line which separates the top and bottom, then
    // set the scroll position to where we are.
    Output(s);
    scrollPos = cursorPos;

    // cause the bottom window to scroll
    lastLine();
    nextLine(); // scroll
}

void Intro()
{
    Output(INTRO_TEXT);
}

const char* getFileName()
{
    static char savename[16];
    static char* f = savename;
    
    if (!*f)
    {
        f = CmdLine;
        strcat(strcpy(savename, f), ".SAV");
    }
    return savename;
}

int ReadSaveFile(char* buf, int bz)
{
    return readFile(getFileName(), buf, bz);
}

int WriteSaveFile(char* buf, int bz)
{
    return writeFile(getFileName(), buf, bz);
}

void Pause()
{
    pause();
    pause();
}




