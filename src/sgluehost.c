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
#include <stdlib.h>
#include <time.h> // clock
#include "sglue.h"
#include "scottfree.h"

extern void LoadDatabase(FILE *f, int loud);
extern void LoadGame(char *name);

// stubs
int WriteSaveFile(char* buf, int bz) { return 0; }
int ReadSaveFile(char* buf, int bz) { return 0; }
void Pause() {}

void Output(const char* b)
{
    char c;
    for (;;)
    {
        c = *b++;
        if (!c) break;
        putchar(c);
        ++pos;
        if (c == '\n') pos = 0;
        lastChar = c;
    }
}

void OutputNumber(int v) { printf("%d", v); }

void Exit()
{
    Output("Game Over\n");
    exit(0);
}

void Fatal(const char *x)
{
	printf("%s.\n",x);
	Exit();
}

void ClearScreen(void)
{
    ;
}

unsigned char RandomPercent(unsigned char n)
{
    unsigned int rv = rand() % 100;
	if(rv<n)
		return(1);
	return(0);
}

void LineInput(const char* prompt, char *buf, unsigned char sz)
{
    Output("\n");
    Output(prompt);
    fgets(buf, sz, stdin);
}

void emitTopLine(char* s) { Output(s); }
void Intro() { Output(INTRO_TEXT); }

char CharInput(const char* prompt)
{
    Output(prompt);
    return getchar();
}

int main(int argc, char *argv[])
{
	FILE *f;

#ifdef GAME
    ;
#else    
	
	if(argc!=2 && argc!=3)
	{
		fprintf(stderr,"%s <database> <savefile>.\n",argv[0]);
		Exit(1);
	}
	f=fopen(argv[1],"r");
	if(f==NULL)
	{
		perror(argv[1]);
		Exit(1);
	}

    LoadDatabase(f, 1);

	fclose(f);
	if(argc==3) LoadGame(argv[2]);
#endif

    srand(clock());

    Intro();    

    extern void rungame();
    rungame();

    return 0;
}





