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
#include <stdarg.h>

#include "defs.h"
#include "os.h"

// if we want to scan the keymatrix ourselves
#define IMPLEMENT_SCAN
#define IMPLEMENT_GETLINE

// store our own cursor position (do not use the OS location)
unsigned int cursorPos;

// are we in 80 col mode?
uchar cols80;

// location of video ram 0x3c00 or 0xf800
uchar* vidRam;

// what model? (set up by initModel)
uchar TRSModel;

// How much memory in K  (initModel)
uchar TRSMemory;
uchar* TRSMemoryFail;

// random number seed
static uint seed;
static uchar* oldVidRam;

static uchar* OldStack;
static uchar* NewStack;

// point to scroll from (usually 0)
unsigned int scrollPos;

void pushVideo(uchar* a)
{
    oldVidRam = vidRam;
    vidRam = a;
}

void popVideo()
{
    memcpy(oldVidRam, vidRam, (cols80 ? VIDSIZE80 : VIDSIZE));
    vidRam = oldVidRam;
}

static uint vidoff(char x, char y)
{
    // calculate the video offset from the screen base for CHARACTER pos (x,y)
    uint a;

    a = (uint)y<<6;
    if (cols80) a += (uint)y<<4;
    return a + x;
}

uchar* vidaddrfor(uint a)
{
    // find the video ram address for offset `a'
    if (a >= VIDSIZE && !cols80 || a >= VIDSIZE80) return 0;
    return vidRam + a;
}

uchar* vidaddr(char x, char y)
{
    // return the video address of (x,y) or 0 if off end.
    return vidaddrfor(vidoff(x,y));
}

void outcharat(char x, char y, uchar c)
{
    // set video character directly without affecting cursor position
    *vidaddr(x,y) = c;
}


static uint nextLinePos()
{
    // calculate the next line from the current cursor pos
    uint a = cursorPos;
    if (cols80)
    {
        // bump a to the next multiple of 80
        uchar q = a/80;
        a = (q + 1)*80;
    }
    else
    {
        a = (a + 64) & ~63;  // start of next line
    }
    return a;
}

static void clearLine()
{
    // clear from current cursor pos to end of line
    uint a = cursorPos;
    uint b = nextLinePos();
    memset(vidaddrfor(a), ' ', b - a);
}

void lastLine()
{
    // put the cursor on the last line 
    if (cols80)
    {
        setcursor(0, 23);
    }
    else
    {
        setcursor(0, 15);
    }
}

void nextLine()
{
    uchar sc;

    char* p = vidRam + scrollPos;
    cursorPos = nextLinePos();

    if (cols80)
    {
        sc = (cursorPos >= VIDSIZE80);
        if (sc)
        {
            // scroll
            memmove(p, p + 80, VIDSIZE80 - 80 - scrollPos);
        }
    }
    else
    {
        sc = (cursorPos >= VIDSIZE);
        if (sc)
        {
            // scroll
            memmove(p, p + 64, VIDSIZE - 64 - scrollPos);
        }
    }

    if (sc)
    {
        // place at last line and clear line
        lastLine();
        clearLine();
    }
}


void outchar(char c)
{
    uint a = cursorPos;
    uchar* p = vidaddrfor(a);
    
    if (c == '\b')
    {
        *p = ' ';
        if (a) a--;
    }
    else if (c == '\n')
    {
        clearLine();
        nextLine();
        return;
    }
    else if (c == '\r')
    {
        // ignore
    }
    else
    {
        *p = c;
        ++a;
        if (a >= VIDSIZE && !cols80 || a >= VIDSIZE80)
        {
            // scroll and place on last line
            nextLine();
            return;
        }
    }
    cursorPos = a;
}

void setcursor(char x, char y)
{
    cursorPos = vidoff(x, y);
}

void clsc(uchar c)
{
    memset(vidRam, c, (cols80 ? VIDSIZE80 : VIDSIZE));
    cursorPos = 0;
    setWide(0);
}

void cls()
{
    clsc(' ');
}


static void outPort(uchar port, uchar val)
{
    __asm
        pop hl          ; ret
        pop bc          ; port->c, val->b
        push bc
        push hl
        out (c),b
    __endasm;
}

static uchar inPort(uchar port)
{
    __asm
        pop hl          ; ret
        pop bc          ; port->c
        push bc
        push hl
        in  l,(c)
    __endasm;
}

static uchar ramAt(uchar* p) __naked
{
    // return 1 if we have RAM at address `p', 0 otherwise
    __asm
        pop bc
        pop hl
        push hl         // p -> hl
        push bc
        ld  a,(hl)      // get original
        ld  b,a         // save
        xor #0xff       // flip bits
        ld  (hl),a      // change all bits in RAM
        xor (hl)        // mask
        ld  (hl),b      // restore original
        ld   l,#1       // return result if ok
        ret  z          // return if ok
        dec  l          // return 0 if bad
        ret
    __endasm;
}

char* getHigh() __naked
{
    __asm
        ld  hl,#0
        ld  b,h
        ld  a,#100 // select HIGH
        RST 0x28   // @HIGH@ -> hl
        ret
    __endasm;
}

static uchar testBlock(uchar a)
{
    // test 256 bytes of RAM at address `a'00
    // return 1, ok, 0 fail
    uchar* p = (uchar*)(a << 8);
    uint r;
    for (;;)
    {
        r = ramAt(p);
        if (!r) 
        {
            // test failed, remember failure address
            TRSMemoryFail = p;
            break;
        }
        ++p;
        if (((uchar)p) == 0) break;
    } 
    return r;
}

#define ADDR(_n) ((uint)&_n)
#define ADDRH(_n) ((uchar)(ADDR(_n)>>8))

uchar ramTest(uchar a, uchar n)
{
    // test `n' blocks of 256 at `a'00
    uchar r = 1;
    do
    {
        if (a < ADDRH(ramAt) || a > ADDRH(testBlock)) r = testBlock(a);
        ++a;
        --n;
    } while (r && n);
    return r;
}

static uchar getModel()
{
    uchar m = 1;
    
    // attempt to change to M4 bank 1, which maps RAM over 14K ROM
    // will work if we _are_ M4.
    outPort(0x84, 1); 

    // if we have RAM, then M4
    if (ramAt((uchar*)0x2000))
    {
        // this is a 4 or 4P.
        // NB: leave at bank 1 for now...
        m = 4;
    }
    else
    {
        // M3 or M1
        uchar v = inPort(0xff);
        
        // toggle DISWAIT
        outPort(0xec, v ^ 0x20);

        if (inPort(0xff) != v)  // read back from mirror
        {
            // changed, we are M3
            outPort(0xEC, v);  // restore original
            m = 3;
        }
    }

    return m;
}

static void setSpeed(uchar fast)
{
    if (TRSModel >= 4)
    {
        // M4 runs at 2.02752 or 4.05504 MHz
        outPort(0xec, fast ? 0x40 : 0);
    }
}

#ifdef IMPLEMENT_SCAN

static const char keyMatrix[] =
{
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
    'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
    'X', 'Y', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z',
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', ':', ';', ',', '-', '.', '/',
    '\r', '\b', 'Z', KEY_ARROW_UP, KEY_ARROW_DOWN, KEY_ARROW_LEFT, KEY_ARROW_RIGHT, ' ',
    'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 'Z', 
};

static uchar readKeyRowCol(uchar* row, uchar* col)
{
    uchar r = 1;
    uchar i;
    uchar hit = 0;
    
    static uchar kbrows[8];

    for (i = 0; i < 8; ++i)
    {
        uchar v = cols80 ? *(KBBASE80 + r) : *(KBBASE + r);
        r <<= 1;

        uchar t = v ^ kbrows[i];

        if (t)
        {
            kbrows[i] = v;

            // press not release            
            if (v & t)
            {
                hit = 1;
                
                *row = i;
                *col = 0;
                
                while (t > 1)
                {
                    t >>= 1;
                    ++*col;
                }
            }
        }
    }

    return hit;
}


char scanKey()
{
    // return key if pressed or 0
    uchar row, col;

    // ignore shift & control
    return (readKeyRowCol(&row, &col) && row != 7) ? keyMatrix[row*8 + col] : 0;
}

#else // IMPLEMENT_SCAN

// otherwise call the ROM to scan a key

char scanKey()
{
    // return key pressed or 0 if none

    if (TRSModel >= 4)
    {
    __asm
        ld    a,#8      // @KBD
        RST   0x28      // uses DE
        ld    l,a
    __endasm;
    }
    else
    {
      __asm
        call    0x2b
        ld      l,a
    __endasm;
    }
}

#endif // IMPLEMENT_SCAN

static uchar keyIdleState;
static IdleHandler idleHandler;
static uchar idleDelay;
static uchar idleCount;

void setIdleHandler(IdleHandler h, uchar d)
{
    idleHandler = h;
    idleDelay = d;
}

static void _keyIdle()
{
    // stir random number
    ++seed;
    
    if (idleHandler)
    {
        if (!idleCount) idleCount = idleDelay + 1;
        
        if (!--idleCount)
        {
            keyIdleState = ~keyIdleState;
            (*idleHandler)(keyIdleState);
        }
    }
}

char getkey()
{
    // wait for a key
    char c;
    for (;;)
    {
        c = scanKey();
        if (c)
        {
            if (keyIdleState) 
            {
                idleCount = 1;
                _keyIdle(); // force revert to state 0
            }
            return c;
        }
        else
        {
            _keyIdle();
        }
    }
}

#ifdef IMPLEMENT_GETLINE
uchar getline(char* buf, uchar nmax)
{
    // our own version of `getline' 
    // the os version always prints the newline, but this one
    // does not. This can be useful when we dont want the screen to scroll
    // because of our input.
    uchar pos = 0;
    for (;;)
    {
        char c;

        // emit prompt
        vidRam[cursorPos] = '_';
        
        // wait for key
        c = getkey();

        if (c == '\b') // backspace
        {
            if (pos)
            {
                --pos;
                outchar(c);
            }
        }
        else 
        {
            if (pos < nmax)
            {
                buf[pos++] = c;
                buf[pos] = 0; // maintain termination
                outchar(c);
            }
            if (c == '\r') break;  // enter hit
        }
    }
    return pos;
}
#else // IMPLEMENT_GETLINE

// use ROM

uchar rom4_getline(char* buf, uchar nmax) __naked
{
    // model 4 version
    __asm
        pop  bc     // ret
        pop  hl     // buf
        pop  de     // e = nmax
        push de
        push hl
        push bc
        ld   b,e    // nmax
        ld   c,#0
        ld   a,#9   // @KEYIN
        RST  0x28
        ld   l,b    // number typed
        ret        
    __endasm;
}

uchar rom_getline(char* buf, uchar nmax) __naked
{
    // emit prompt and handle backspace etc
    __asm
        pop  bc     // ret
        pop  hl     // buf
        pop  de     // e = nmax
        push de
        push hl
        push bc
        ld   b,e    // nmax
        call 0x40
        ld   l,b    // number typed
        ret
    __endasm;
}

static uchar c4row;
static uchar c4col;
static void setROMCursor()
{
    if (TRSModel >= 4)
    {
        c4row = cursorPos/80;
        c4col = cursorPos - c4row*80;
        
        __asm
            ld  a,#15   // @VDCTL
            ld  b,#3    // set cursor
            ld  hl,#_c4row
            ld  d,(hl)
            ld  hl,#_c4col
            ld  e,(hl)
            ex  de,hl
            RST 0x28
        __endasm;
    }
    else
    {
        // set the ROM cursor to our cursor
        *ROM_CURSOR = VIDRAM + cursorPos;
    }
}

uchar getline(char* buf, uchar nmax)
{
    //printf("CURSOR %04X\n", (int)*ROM_CURSOR);
    setROMCursor();

    uchar n;

    if (TRSModel >= 4)
    {
        n = rom4_getline(buf, nmax);
    }
    else
    {
        n = rom_getline(buf, nmax);
    }

    // terminate
    buf[n] = 0;
    return n;
}

#endif // IMPLEMENT_GETLINE

void pause()
{
    // delay, unless key pressed
    int c = 1000;  // XX scale delay by machine speed
    while (--c)
    {
        if (scanKey()) return;
    }
}

void outs(const char* s)
{
    while (*s) outchar(*s++);
}

int putchar(int c)
{
    outchar(c);
    return c;
}

void outsWide(const char* s)
{
    // write text in wide mode
    
   // arrange even location
    if (cursorPos & 1) outchar(' ');
    
    // write each char followed by a space
    while (*s)
    {
        outchar(*s++);
        outchar(' ');
    }
}

void printfat(uchar x, uchar y, const char* fmt, ...)
{
    // printf at (x,y) character position
    // NB: text is automatically flushed (without need for "\n")
    
    va_list args;
    setcursor(x, y);    
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}



void setWide(uchar v)
{
    if (TRSModel == 1)
    {
        // model I
        outPort(0xFF, v << 3); // 8 or 0
    }
    else
    {
        // get MODOUT (mirror of port 0xec)
        // NB: do not read from 0xEC
        uchar m = inPort(0xff);
        
        // set or clear MODSEL bit 
        if (v) m |= 4;
        else m &= ~4;

        outPort(0xEC, m);
    }
}

#if 0
static uint alloca_ret;
uchar* alloca(uint a)
{
    __asm
        pop  bc         // ret
        pop  de         // a
        push de
        ld   (_alloca_ret),sp  // point to a is result
        xor a
        ld   h,a
        ld   l,a        // hl = 0
        sbc  hl,de      // hl = -a
        add  hl,sp      // hl = sp - a
        ld   sp,hl      
        push bc
    __endasm;
    return alloca_ret;
}
#endif

void initModel()
{
    uchar* rp = (uchar*)0x4000;
    
    cols80 = 0;
    vidRam = VIDRAM;
    TRSMemory = 0;

    TRSModel = getModel();

    if (TRSModel >= 4)
    {
        char* h = getHigh();
        
        cols80 = 1;
        vidRam = VIDRAM80;
        
        outPort(0x84, 0x86); // M4 map, 80cols, page 1
        setSpeed(0); // slow (for now..)
        
        TRSMemory = 64;

        // m4 0xf400 - vidram is keyboard area
        NewStack = (uchar*)0xF400; 
        if (NewStack > h) NewStack = h;
    }
    else
    {
        // how much RAM do we have?
        for (;;)
        {
            TRSMemory += 16;
            rp += 0x4000;
            if (!ramAt(rp)) break;
        }
        NewStack = rp;
    }
}

void setStack() __naked
{
    // locate the stack to `NewStack`
    // ASSUME we are called from main
    
    __asm
        pop hl
        ld (_OldStack),sp
        ld sp,(_NewStack)
        jp  (hl)
    __endasm;
}

void revertStack() __naked
{
    // put stack back to original
    // ASSUME we are called from main
    __asm
        pop hl
        ld sp,(_OldStack)
        jp (hl)
    __endasm;
}




void srand(uint v)
{
    seed = v;
}

unsigned int rand16()
{
    uint v;
    uchar a;

    v = (seed + 1)*75;
    a = v;
    a -= (v >> 8);
    seed = ((v & 0xff00) | a) - 1;
    return seed;
}

uint randn(uint n)
{
    // random [0,n-1]
    // 16 bit version
    
    uint c = 1;
    uint v;

    while (c < n) c <<= 1;
    --c;
    
    do
    {
        v = rand16() & c;
    } while (v >= n);
    
    return v;
}

uchar randc(uchar n)
{
    // random [0,n-1]
    // 8 bit version

    uchar v;
    uchar c = 0xff;
    
    if (n <= 128)
    {
        c = 1;
        while (c < n) c <<= 1;
        --c;
    }

    do
    {
        v = rand16() & c;
    } while (v >= n);
    
    return v;
}

