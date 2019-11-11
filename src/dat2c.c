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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "sglue.h"
#include "scottfree.h"

extern void LoadDatabase(FILE *f, int loud);
extern void LoadGame(char *name);


void Output(const char* b) { printf(b); }
void OutputNumber(int v) { printf("%d", v); }

void Exit()
{
    printf("Game Over\n");
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
    fgets(buf, sz, stdin);
}

void emitTopLine(char* s) { Output(s); }
void Intro() { Output(INTRO_TEXT); }
int ReadSaveFile(const char* name, char* buf, int bz) { return 0; }
int WriteSaveFile(const char* name, char* buf, int bz) { return 0; }
char CharInput(const char* prompt) { return 0; }

FILE* out;

void EmitHeader()
{
    fprintf(out,"Header GameHeader = {\n");
    fprintf(out,"\t%d,\n", GameHeader.Unknown);
    fprintf(out,"\t%d,\n", GameHeader.NumItems);
    fprintf(out,"\t%d,\n", GameHeader.NumActions);
    fprintf(out,"\t%d,\n", GameHeader.NumWords);
    fprintf(out,"\t%d,\n", GameHeader.NumRooms);
    fprintf(out,"\t%d,\n", GameHeader.MaxCarry);
    fprintf(out,"\t%d,\n", GameHeader.PlayerRoom);
    fprintf(out,"\t%d,\n", GameHeader.Treasures);
    fprintf(out,"\t%d,\n", GameHeader.WordLength);
    fprintf(out,"\t%d,\n", GameHeader.LightTime);
    fprintf(out,"\t%d,\n", GameHeader.NumMessages);
    fprintf(out,"\t%d\n", GameHeader.TreasureRoom);
    fprintf(out,"};\n");
}


#define ENDREC fprintf(out, "},\n")
#define COMMA fprintf(out, ", ")
#define TAB fputc('\t', out)
#define NEWLINE fputc('\n', out)
#define QUOTE fputc('"', out)

void emitString(const char* s)
{
    if (s)
    {
        QUOTE;
        const char* p = s;
        while (*p)
        {
            if (*p == '\n')
            {
                fprintf(out, "\\n");
            }
            else if (*p == '"')
            {
                fprintf(out, "\\\"");
            }
            else
            {
                fputc(*p, out);
            }
            ++p;
        }
        QUOTE;
    }
    else fprintf(out, "0");
}

void emitShortArray(short* s, int n)
{
    int i;
    fprintf(out, "{ ");
    for (i = 0; i < n; ++i)
    {
        if (i) fprintf(out, ", ");
        fprintf(out, "%d", s[i]);
    }
    fprintf(out, "}");
}

void emitByteArray(uchar* s, int n)
{
    int i;
    fprintf(out, "{ ");
    for (i = 0; i < n; ++i)
    {
        if (i) fprintf(out, ", ");
        fprintf(out, "%d", s[i]);
    }
    fprintf(out, "}");
}


void EmitActions()
{
    int i;
    int n = GameHeader.NumActions + 1;
    fprintf(out,"Action Actions[%d] = {\n", n);
    for (i = 0; i < n; ++i)
    {
        Action* it = Actions + i;
        fprintf(out, "\t{ %d, ", it->Vocab);
        emitShortArray(it->Condition, 5);
        COMMA;
        emitShortArray(it->Action, 2);
        ENDREC;
    }
    fprintf(out,"};\n");    
}

void EmitItems()
{
    int i;
    int n = GameHeader.NumItems + 1;
    fprintf(out,"Item Items[%d] = {\n", n);
    for (i = 0; i < n; ++i)
    {
        Item* it = Items + i;
        fprintf(out,"\t{ ");
        emitString(it->Text);
        fprintf(out,", %d, %d, ",
                it->Location,
                it->InitialLoc);
        emitString(it->AutoGet);
        ENDREC;
    }
    fprintf(out,"};\n");
}

void EmitVerbs()
{
    int i;
    int n = GameHeader.NumWords + 1;
    fprintf(out,"char* Verbs[%d] = {\n", n);
    for (i = 0; i < n; ++i)
    {
        TAB;
        emitString(Verbs[i]);
        COMMA;
        NEWLINE;
    }
        
    fprintf(out,"};\n");
}

void EmitNouns()
{
    int i;
    int n = GameHeader.NumWords + 1;
    fprintf(out,"char* Nouns[%d] = {\n", n);
    for (i = 0; i < n; ++i)
    {
        TAB;
        emitString(Nouns[i]);
        COMMA;
        NEWLINE;
    }
        
    fprintf(out,"};\n");
}

void EmitRooms()
{
    int i;
    int n = GameHeader.NumRooms + 1;
    fprintf(out,"Room Rooms[%d] = {\n", n);
    for (i = 0; i < n; ++i)
    {
        Room* it = Rooms + i;
        fprintf(out, "\t{ ");
        emitString(it->Text);
        fprintf(out, ", ");
        emitByteArray(it->Exits, 6);
        ENDREC;
    }
        
    fprintf(out,"};\n");    
}

void EmitMessages()
{
    int i;
    int n = GameHeader.NumMessages + 1;
    fprintf(out,"char* Messages[%d] = {\n", n);
    for (i = 0; i < n; ++i)
    {
        TAB;
        emitString(Messages[i]);
        COMMA;
        NEWLINE;
    }
        
    fprintf(out,"};\n");    
}

void DumpDatabase()
{
    EmitHeader();
    EmitActions();
    EmitVerbs();
    EmitNouns();
    EmitRooms();
    EmitMessages();
    EmitItems();

}

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
        }
    }

    const char* fname = 0;

    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-')
        {
            fname = argv[i];
        }
    }

    if (!fname)
    {
        printf("Usage: %s filename\n", argv[0]);
        return 1;
    }

    FILE* fp = fopen(fname, "r");
    if (fp)
    {
        char foutname[256];
        strcat(strcpy(foutname, fname), ".h");
        out = fopen(foutname, "w");
        if (out)
        {
            LoadDatabase(fp, 1);
            DumpDatabase();
            fclose(out);
        }
        else
        {
            printf("Can't open output file '%s'\n", foutname);
        }
        fclose(fp);
    }
    else
    {
        printf("Can't open '%s'\n", fname);
        return -1;
    }

    return 0;
}
