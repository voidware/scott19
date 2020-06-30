/*
 *  ScottFree Revision 1.14
 *  Some changed taken from Stefano Bodrato scottzx.
 *  Other changes by voidware.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

#include "defs.h"
#include "scottfree.h"
#include "sglue.h"
#include "utils.h"

#define MAX_WORDLEN 16
#define MAX_SAVESIZE 256

#define STRINGIZE_AUX(a) #a
#define STRINGIZE(a) STRINGIZE_AUX(a)

#ifndef GAME
Header GameHeader;
Item *Items;
Room *Rooms;
char **Verbs;
char **Nouns;
char **Messages;
Action *Actions;
#else
#include STRINGIZE(GAME)

#include "defs.h"
#include "os.h"

#define printf printf_simple
#define sprintf sprintf_simple

#endif

int LightRefill;
char NounText[MAX_WORDLEN];
int Counters[16];   /* Range unknown */
int CurrentCounter;
uchar SavedRoom;
uchar RoomSaved[16];  /* Range unknown */
int Redraw = 1;     /* Update item window */
char lastChar;
uchar pos;
uchar Width = 64;

#define TRS80_LINE  "<------------------------------------------------------------->\n"

#define MyLoc   (GameHeader.PlayerRoom)

unsigned long BitFlags=0;   /* Might be >32 flags - I haven't seen >32 yet */
static char startSave[MAX_SAVESIZE];

static uchar bfTest(uchar b)
{
    uchar v = 0;
    if (BitFlags & (1UL<<b)) v = 1;
    return v;
}

static void bfSet(uchar b)
{
    BitFlags |= (1UL<<b);
}

static void bfClear(uchar b)
{
    BitFlags &= ~(1UL<<b);
}

static uchar WordMatch(const char* s1, const char* s2)
{
    signed char v = 0;
    uchar n = GameHeader.WordLength;
    while (n) 
    {
        --n;
        
        v = toupper(*s2) - toupper(*s1);
        if (v || !*s1) break;
        ++s1;
        ++s2;
    }
    return !v;
}

uchar CountCarried()
{
    uchar n=0;
    Item* it = Items;
    int ct = GameHeader.NumItems;
    while (ct)
    {
        ct--;
        if(it++->Location==CARRIED) ++n;
    }
    return(n);
}

#if 0
char *MapSynonym(char *word)
{
    uchar n;
    static char lastword[MAX_WORDLEN];  /* Last non synonym */
    
    for (n = 1; n <= GameHeader.NumWords; ++n)
    {
        char* tp=Nouns[n];
        
        if(*tp=='*') tp++;
        else strcpy(lastword,tp);

        if (WordMatch(word, tp)) return lastword;
    }
    return(NULL);
}

int MatchUpItem(char *text, int loc)
{
    char *word=MapSynonym(text);
    int ct;
    
    if(word==NULL) word=text;
    
    for (ct = 0; ct<=GameHeader.NumItems; ++ct)
    {
        if(Items[ct].Location==loc &&
           Items[ct].AutoGet &&
           WordMatch(word, Items[ct].AutoGet))
            return(ct);
    }
    return(-1);
}
#endif

int MatchItem(int no, uchar loc)
{
    assert(no >= 0);
    char* nt = Nouns[no];


    Item* it = Items;
    int ct = GameHeader.NumItems;
    while (ct)
    {
        if(it->Location==loc &&
           it->AutoGet &&
           WordMatch(nt, it->AutoGet))
        {
            return GameHeader.NumItems - ct;
        }
        ++it;
        ct--;
    }

    return -1;
}

#ifdef DAT_FILES
char *ReadString(FILE *f)
{
    char tmp[1024];
    char *t;
    int c,nc;
    int ct=0;
    do
    {
        c=fgetc(f);
    }
    while(c!=EOF && isspace(c));
    if(c!='"')
    {
        Fatal("Initial quote expected");
    }
    do
    {
        c=fgetc(f);
        if(c==EOF)
            Fatal("EOF in string");
        if(c=='"')
        {
            nc=fgetc(f);
            if(nc!='"')
            {
                ungetc(nc,f);
                break;
            }
        }
        if(c==0x60) 
            c='"'; /* pdd */
        tmp[ct++]=c;
    }
    while(1);
    tmp[ct]=0;
    t=(char*)malloc(ct+1);
    memcpy(t,tmp,ct+1);
    return(t);
}
    
void LoadDatabase(FILE *f, int loud)
{
    int ni,na,nw,nr,mc,pr,tr,wl,lt,mn,trm;
    int ct;
    short lo;
    Action *ap;
    Room *rp;
    Item *ip;
/* Load the header */
    
    if(fscanf(f,"%*d %d %d %d %d %d %d %d %d %d %d %d",
        &ni,&na,&nw,&nr,&mc,&pr,&tr,&wl,&lt,&mn,&trm,&ct)<10)
        Fatal("Invalid database(bad header)");

    assert(ni < 255);
    assert(nw < 255);
    assert(nr < 255);
    assert(trm < 255);
    assert(tr < 255);
    
    GameHeader.NumItems=ni;
    Items=(Item *)malloc(sizeof(Item)*(ni+1));
    GameHeader.NumActions=na;
    Actions=(Action *)malloc(sizeof(Action)*(na+1));
    GameHeader.NumWords=nw;
    GameHeader.WordLength=wl;
    Verbs=(char **)malloc(sizeof(char *)*(nw+1));
    Nouns=(char **)malloc(sizeof(char *)*(nw+1));
    GameHeader.NumRooms=nr;
    Rooms=(Room *)malloc(sizeof(Room)*(nr+1));
    GameHeader.MaxCarry=mc;
    GameHeader.PlayerRoom=pr;
    GameHeader.Treasures=tr;
    GameHeader.LightTime=lt;
    LightRefill=lt;
    GameHeader.NumMessages=mn;
    Messages=(char **)malloc(sizeof(char *)*(mn+1));
    GameHeader.TreasureRoom=trm;
    
/* Load the actions */

    ct=0;
    ap=Actions;
    if(loud) printf("Reading %d actions.\n",na);
    for (ct = 0; ct<=na; ++ct)
    {
        int t1;
        if(fscanf(f,"%d %hd %hd %hd %hd %hd %hd %hd",
                  &t1,
                  &ap->Condition[0],
                  &ap->Condition[1],
                  &ap->Condition[2],
                  &ap->Condition[3],
                  &ap->Condition[4],
                  &ap->Action[0],
                  &ap->Action[1])!=8)
        {
            printf("Bad action line (%d)\n",ct);
            Exit();
        }
        ap->Vocab = t1;
        ap++;
    }           
    ct=0;
    if(loud)
        printf("Reading %d word pairs.\n",nw);
    while(ct<nw+1)
    {
        Verbs[ct]=ReadString(f);
        Nouns[ct]=ReadString(f);
        ct++;
    }
    rp=Rooms;
    if(loud) printf("Reading %d rooms.\n",nr);
    for (ct = 0; ct<=nr; ++ct)
    {
        uchar i;
        int t[6];
        fscanf(f,"%d %d %d %d %d %d",
               &t[0],&t[1],&t[2], &t[3],&t[4],&t[5]);

        for (i = 0; i < 6; ++i)
        {
            assert(t[i] < 256);
            rp->Exits[i] = t[i];
        }
        
        rp->Text=ReadString(f);
        rp++;
    }
    ct=0;
    if(loud) printf("Reading %d messages.\n",mn);
    while(ct<mn+1)
    {
        Messages[ct]=ReadString(f);
        ct++;
    }
    ct=0;
    if(loud) printf("Reading %d items.\n",ni);
    ip=Items;
    while(ct<ni+1)
    {
        ip->Text=ReadString(f);
        ip->AutoGet=strchr(ip->Text,'/');
        /* Some games use // to mean no auto get/drop word! */
        if(ip->AutoGet && strcmp(ip->AutoGet,"//") && strcmp(ip->AutoGet,"/*"))
        {
            char *t;
            *ip->AutoGet++=0;
            t=strchr(ip->AutoGet,'/');
            if(t!=NULL)
                *t=0;
        }
        fscanf(f,"%hd",&lo);
        ip->Location=(unsigned char)lo;
        ip->InitialLoc=ip->Location;
        ip++;
        ct++;
    }
    ct=0;
    /* Discard Comment Strings */
    while(ct<na+1)
    {
        free(ReadString(f));
        ct++;
    }
    fscanf(f,"%d",&ct);
    if(loud) printf("Version %d.%02d of Adventure ",
        ct/100,ct%100);
    fscanf(f,"%d",&ct);
    if(loud) printf("%d.\nLoad Complete.\n\n",ct);
}
#endif // DAT_FILES

void OutputWord(char* s)
{
    if (pos + strlen(s) > Width - 4) Output("\n");
    Output(s);
}


static const char *ExitNames[]=
{
    "NORTH","SOUTH","EAST","WEST","UP","DOWN"
};

void Look()
{
    uchar c,f;
    
    ClearScreen();

    if(bfTest(DARKBIT) && Items[LIGHT_SOURCE].Location!= CARRIED
                && Items[LIGHT_SOURCE].Location!= MyLoc)
    {
#ifdef YOUARE
        Output("You can't see. It is too dark!\n");
#else
        Output("I can't see. It is too dark!\n");
#endif
    }
    else
    {
        Room* r=&Rooms[MyLoc];
        char* t = r->Text;
        if(*t=='*') Output(++t);
        else
        {
#ifdef YOUARE
            Output("You are in a ");
#else
            Output("I am in a ");
#endif            
            Output(t);
        }
        if (lastChar != '.') Output(".");

        f = 0;
        for (c = 0; c <= GameHeader.NumItems; ++c)
        {
            if(Items[c].Location==MyLoc)
            {
                if (!f++)
                {
#ifdef YOUARE
                    Output("\nYou can also see: ");
#else
                    Output(" Visible Items:\n\n");
#endif
                }
                else
                {
                    Output(" ");
                }

                OutputWord(Items[c].Text);
                Output(".");
            }
        }
        Output("\n");
        
        f=0;
        for (c = 0; c < 6; ++c)
        {
            if(r->Exits[c])
            {
                if (!f++) Output("\n   Some obvious exits are: ");
                else Output(" ");
                Output(ExitNames[c]);
            }
        }
        if (f) Output("\n");
    }
    emitTopLine(TRS80_LINE);
}


#define NOTFOUND ((uchar)-1)

uchar WhichWord(char *word, char **list)
{
    // lookup a word from a list
    if (*word)
    {
        uchar ne;
        uchar n;
        for (ne = 1; ne<=GameHeader.NumWords; ++ne)
        {
            char* tp= *++list; // list[ne]
            if(*tp=='*') tp++;
            else n = ne;

            if (WordMatch(word, tp)) return n;
        }

    }
    return NOTFOUND;
}

static const char* getword(char* buf, const char* s)
{
    while (isspace(*s)) ++s;
    while (*s && !isspace(*s)) *buf++ = *s++;
    *buf = 0;
    return s;
}

void GetInput(uchar* vb, uchar* no)
{
    char buf[100];
    char verb[MAX_WORDLEN],noun[MAX_WORDLEN];
    uchar vc,nc;
    do
    {
        do
        {
            LineInput("  -------> Tell me what to do? ", buf, sizeof(buf));
            getword(noun, getword(verb, buf));
            
        }
        while (!*verb);

        if(*noun==0 && strlen(verb)==1)
        {
            switch(toupper(*verb))
            {
                case 'N':strcpy(verb,"NORTH");break;
                case 'E':strcpy(verb,"EAST");break;
                case 'S':strcpy(verb,"SOUTH");break;
                case 'W':strcpy(verb,"WEST");break;
                case 'U':strcpy(verb,"UP");break;
                case 'D':strcpy(verb,"DOWN");break;
                /* Brian Howarth interpreter also supports this */
                case 'I':strcpy(verb,"INVE");break;
            }
        }
        
        nc=WhichWord(verb,Nouns);
        
        /* The Scott Adams system has a hack to avoid typing 'go' */
        if(nc>=1 && nc <=6)
        {
            vc=1;
        }
        else
        {
            vc=WhichWord(verb,Verbs);
            nc=WhichWord(noun,Nouns);
        }
        
        *vb=vc;
        *no=nc;
        
        if(vc==NOTFOUND)
        {
            Output("You use word(s) I don't know!\n");
        }
    }
    while(vc==NOTFOUND);
    strcpy(NounText,noun);  /* Needed by GET/DROP hack */

    //printf("DEBUG '%s' (%d) '%s' (%d)\n", verb, vc, noun, nc);
}

int SaveBuf(char* b, int sz)
{
    // return actual bytes to save
    uchar c;
    int ct;
    char* b0 = b;
    for(c=0;c<16;c++)
        b += sprintf(b,"%d %d\n",Counters[c],(int)RoomSaved[c]);

    b += sprintf(b, "%ld %d %d %d %d\n",
                 BitFlags,
                 (int)MyLoc,
                 CurrentCounter,
                 (int)SavedRoom,
                 (int)GameHeader.LightTime);
    
    for(ct=0;ct<=GameHeader.NumItems;ct++)
        b += sprintf(b,"%d\n", (int)Items[ct].Location);

    ct = b - b0;
    if (ct >= sz) printf("SaveBuf Too small! Need %d\n", ct);
    return ct;
}

static char* inb;

static void skipL()
{
    while (*inb++ != '\n') ;
}

static void skipD()
{
    if (*inb == '-') ++inb;
    while (isdigit(*inb)) ++inb;
}

static void skipS()
{
    while (isspace(*inb)) ++inb;
}

static int scanD()
{
    int v;
    skipS();
    v = atoi(inb);
    skipD();
    return v;
}

void LoadBuf(char* b)
{
    uchar c;
    int ct;

    inb = b;
    
    for(c=0;c<16;c++)
    {
        //sscanf(b,"%d %d\n",&t1, &t2);
        Counters[c] = scanD();
        RoomSaved[c] = scanD();
        skipL();
    }

    BitFlags = atol(inb);
    skipD();

    /*
    sscanf(b,"%ld %d %d %d %d\n",
           &BitFlags,
           &t1,
           &t2,
           &t3,
           &t4);
    */

    MyLoc = scanD();
    CurrentCounter = scanD();
    SavedRoom = scanD();
    GameHeader.LightTime = scanD();
    skipL();
        
    for(ct=0;ct<=GameHeader.NumItems;ct++)
    {
        //sscanf(b,"%d\n",&t1);
        Items[ct].Location=scanD();
        skipL();
    }
}



void SaveGame()
{
    char buf[MAX_SAVESIZE];
    int n = SaveBuf(buf, sizeof(buf));
    WriteSaveFile(buf, n);
}

int LoadGame()
{
    char buf[MAX_SAVESIZE];
    int n = ReadSaveFile(buf, MAX_SAVESIZE);
    if (n > 0) LoadBuf(buf);
    return n;
}


int PerformLine(int ct)
{
    int continuation=0;
    int param[5],pptr=0;
    int act[4];
    int cc=0;
    while(cc<5)
    {
        int cv,dv;
        cv=Actions[ct].Condition[cc];
        dv=cv/20;
        cv%=20;
        switch(cv)
        {
            case 0:
                param[pptr++]=dv;
                break;
            case 1:
                if(Items[dv].Location!=CARRIED)
                    return(0);
                break;
            case 2:
                if(Items[dv].Location!=MyLoc)
                    return(0);
                break;
            case 3:
                if(Items[dv].Location!=CARRIED&&
                    Items[dv].Location!=MyLoc)
                    return(0);
                break;
            case 4:
                if(MyLoc!=dv)
                    return(0);
                break;
            case 5:
                if(Items[dv].Location==MyLoc)
                    return(0);
                break;
            case 6:
                if(Items[dv].Location==CARRIED)
                    return(0);
                break;
            case 7:
                if(MyLoc==dv)
                    return(0);
                break;
            case 8:
                //if((BitFlags&(1U<<dv))==0)
                if(!bfTest(dv))
                    return(0);
                break;
            case 9:
                //if(BitFlags&(1U<<dv))
                if(bfTest(dv))
                    return(0);
                break;
            case 10:
                if(CountCarried()==0)
                    return(0);
                break;
            case 11:
                if(CountCarried())
                    return(0);
                break;
            case 12:
                if(Items[dv].Location==CARRIED||Items[dv].Location==MyLoc)
                    return(0);
                break;
            case 13:
                if(Items[dv].Location==0)
                    return(0);
                break;
            case 14:
                if(Items[dv].Location)
                    return(0);
                break;
            case 15:
                if(CurrentCounter>dv)
                    return(0);
                break;
            case 16:
                if(CurrentCounter<=dv)
                    return(0);
                break;
            case 17:
                if(Items[dv].Location!=Items[dv].InitialLoc)
                    return(0);
                break;
            case 18:
                if(Items[dv].Location==Items[dv].InitialLoc)
                    return(0);
                break;
            case 19:/* Only seen in Brian Howarth games so far */
                if(CurrentCounter!=dv)
                    return(0);
                break;
        }
        cc++;
    }
    /* Actions */
    act[0]=Actions[ct].Action[0];
    act[2]=Actions[ct].Action[1];
    act[1]=act[0]%150;
    act[3]=act[2]%150;
    act[0]/=150;
    act[2]/=150;
    cc=0;
    pptr=0;
    while(cc<4)
    {
        if(act[cc]>=1 && act[cc]<52)
        {
            Output(Messages[act[cc]]);
            Output("\n");
        }
        else if(act[cc]>101)
        {
            Output(Messages[act[cc]-50]);
            Output("\n");
        }
        else switch(act[cc])
        {
            case 0:/* NOP */
                break;
            case 52:
                if(CountCarried()==GameHeader.MaxCarry)
                {
                    #ifdef YOUARE
                        Output("You are carrying too much.\n");
                    #else
                        Output("I've too much to carry!\n");
                    #endif
                    break;
                }
                if(Items[param[pptr]].Location==MyLoc)
                    Redraw=1;
                Items[param[pptr++]].Location= CARRIED;
                break;
            case 53:
                Redraw=1;
                Items[param[pptr++]].Location=MyLoc;
                break;
            case 54:
                Redraw=1;
                MyLoc=param[pptr++];
                break;
            case 55:
                if(Items[param[pptr]].Location==MyLoc)
                    Redraw=1;
                Items[param[pptr++]].Location=0;
                break;
            case 56:
                //BitFlags|=(1U<<DARKBIT);
                bfSet(DARKBIT);
                break;
            case 57:
                //BitFlags&=~(1U<<DARKBIT);
                bfClear(DARKBIT);
                break;
            case 58:
                //BitFlags|=(1U<<param[pptr++]);
                bfSet(param[pptr++]);
                break;
            case 59:
                if(Items[param[pptr]].Location==MyLoc)
                    Redraw=1;
                Items[param[pptr++]].Location=0;
                break;
            case 60:
                //BitFlags&=~(1U<<param[pptr++]);
                bfClear(param[pptr++]);
                break;
            case 61:
                //if(Options&YOUARE)
                #ifdef YOUARE
                    Output("You are dead.\n");
                #else
                    Output("I am dead.\n");
                #endif
                //BitFlags&=~(1U<<DARKBIT);
                bfClear(DARKBIT);
                MyLoc=GameHeader.NumRooms;/* It seems to be what the code says! */
                Look();
                break;
            case 62:
            {
                /* Bug fix for some systems - before it could get parameters wrong */
                int i=param[pptr++];
                Items[i].Location=param[pptr++];
                Redraw=1;
                break;
            }
            case 63:
doneit:             Output("The game is now over.\n");
                //wrefresh(Bottom);
                //sleep(5);
                //endwin();
                Exit();
            case 64:
                Look();
                break;
            case 65:
            {
                int ct;
                int n=0;
                for (ct = 0; ct<=GameHeader.NumItems; ++ct)
                {
                    if(Items[ct].Location==GameHeader.TreasureRoom &&
                       *Items[ct].Text=='*') n++;
                }
                //if(Options&YOUARE)
                #ifdef YOUARE
                    Output("You have stored ");
                #else
                    Output("I stored ");
                #endif
                OutputNumber(n);
                Output(" TREASURES. On a scale of 0 to 100 that rates: ");
                OutputNumber((n*100)/GameHeader.Treasures);
                Output("\n");
                if(n==GameHeader.Treasures)
                {
                    Output("Well done.\n");
                    goto doneit;
                }
                break;
            }
            case 66:
            {
                int ct;
                uchar f=0;
                
                #ifdef YOUARE
                    Output("You are carrying:\n");
                #else
                    Output("I am carrying the following:\n");
                #endif
                    
                for (ct = 0; ct<=GameHeader.NumItems; ++ct)
                {
                    if(Items[ct].Location==CARRIED)
                    {
                        if (f++) Output(". ");
                        OutputWord(Items[ct].Text);
                    }
                }
                if(f==0) Output("Nothing");
                Output(".\n");
                break;
            }
            case 67:
                //BitFlags|=(1U<<0);
                bfSet(0);
                break;
            case 68:
                //BitFlags&=~(1U<<0);
                bfClear(0);
                break;
            case 69:
                GameHeader.LightTime=LightRefill;
                if(Items[LIGHT_SOURCE].Location==MyLoc)
                    Redraw=1;
                Items[LIGHT_SOURCE].Location=CARRIED;
                //BitFlags&=~(1L<<LIGHTOUTBIT);
                bfClear(LIGHTOUTBIT);
                break;
            case 70:
                /* not sure what this does.
                 * for example "read advertisement" in adv01.
                 * can't clear screen otherwise breaks top window.
                 */
                //ClearScreen(); /* pdd. */
                //OutReset();
                break;
            case 71:
                SaveGame();
                break;
            case 72:
            {
                int i1=param[pptr++];
                int i2=param[pptr++];
                int t=Items[i1].Location;
                if(t==MyLoc || Items[i2].Location==MyLoc)
                    Redraw=1;
                Items[i1].Location=Items[i2].Location;
                Items[i2].Location=t;
                break;
            }
            case 73:
                continuation=1;
                break;
            case 74:
                if(Items[param[pptr]].Location==MyLoc)
                    Redraw=1;
                Items[param[pptr++]].Location= CARRIED;
                break;
            case 75:
            {
                int i1,i2;
                i1=param[pptr++];
                i2=param[pptr++];
                if(Items[i1].Location==MyLoc)
                    Redraw=1;
                Items[i1].Location=Items[i2].Location;
                if(Items[i2].Location==MyLoc)
                    Redraw=1;
                break;
            }
            case 76:    /* Looking at adventure .. */
                Look();
                break;
            case 77:
                if(CurrentCounter>=0)
                    CurrentCounter--;
                break;
            case 78:
                OutputNumber(CurrentCounter);
                break;
            case 79:
                CurrentCounter=param[pptr++];
                break;
            case 80:
            {
                int t=MyLoc;
                MyLoc=SavedRoom;
                SavedRoom=t;
                Redraw=1;
                break;
            }
            case 81:
            {
                /* This is somewhat guessed. Claymorgue always
                   seems to do select counter n, thing, select counter n,
                   but uses one value that always seems to exist. Trying
                   a few options I found this gave sane results on ageing */
                int t=param[pptr++];
                int c1=CurrentCounter;
                CurrentCounter=Counters[t];
                Counters[t]=c1;
                break;
            }
            case 82:
                CurrentCounter+=param[pptr++];
                break;
            case 83:
                CurrentCounter-=param[pptr++];
                if(CurrentCounter< -1)
                    CurrentCounter= -1;
                /* Note: This seems to be needed. I don't yet
                   know if there is a maximum value to limit too */
                break;
            case 84:
                Output(NounText);
                break;
            case 85:
                Output(NounText);
                Output("\n");
                break;
            case 86:
                Output("\n");
                break;
            case 87:
            {
                /* Changed this to swap location<->roomflag[x]
                   not roomflag 0 and x */
                int p=param[pptr++];
                int sr=MyLoc;
                MyLoc=RoomSaved[p];
                RoomSaved[p]=sr;
                Redraw=1;
                break;
            }
            case 88:
                //wrefresh(Top);
                //wrefresh(Bottom);
                //sleep(2); /* DOC's say 2 seconds. Spectrum times at 1.5 */
                // XXX need to add a delay here
                break;
            case 89:
                pptr++;
                /* SAGA draw picture n */
                /* Spectrum Seas of Blood - start combat ? */
                /* Poking this into older spectrum games causes a crash */
                break;
            default:
                printf("ERROR: Unknown action %d [Param begins %d %d]\n",
                    act[cc],param[pptr],param[pptr+1]);
                break;
        }
        cc++;
    }
    return(1+continuation);     
}


int PerformActions(uchar vb, uchar no)
{
    static int disable_sysfunc=0;   /* Recursion lock */
    uchar d = bfTest(DARKBIT);
    
    int ct;
    int fl;
    int doagain=0;
    if(vb==1 && no == NOTFOUND)
    {
        Output("Give me a direction too.\n");
        return(0);
    }
    if(vb==1 && no>=1 && no<=6) //go
    {
        uchar nl;
        if(Items[LIGHT_SOURCE].Location==MyLoc ||
           Items[LIGHT_SOURCE].Location==CARRIED) d=0;
        if(d)
            Output("Dangerous to move in the dark!\n");
        
        nl=Rooms[MyLoc].Exits[no-1];
        if(nl!=0)
        {
            MyLoc=nl;

            Output("OK");
            Look();
            
            return(0);
        }
        if(d)
        {
            #ifdef YOUARE
                Output("You fell down and broke your neck.\n");
            #else
                Output("I fell down and broke my neck.\n");
            #endif
            Exit();
        }

        #ifdef YOUARE
            Output("You can't go in that direction.\n");
        #else
            Output("I can't go in THAT direction.\n");
        #endif
        return(0);
    }
    fl= -1;
    for (ct = 0; ct<=GameHeader.NumActions;)
    {
            int vv,nv;
            vv=Actions[ct].Vocab;
            /* Think this is now right. If a line we run has an action73
               run all following lines with vocab of 0,0 */
            
            if(vb && doagain && vv)
                break;

            /* Oops.. added this minor cockup fix 1.11 */
            if(vb!=0 && !doagain && fl== 0)
                break;

            nv=vv%150;
            vv/=150;
            
            if(vv == vb || (doagain && Actions[ct].Vocab==0))
            {
                if ((vv!=0 && (nv==no||nv==0))
                    || doagain
                    || (vv==0 && RandomPercent(nv)))
                {
                    int f2;
                    if(fl== -1) fl= -2;
                    if((f2=PerformLine(ct))>0)
                    {
                        /* ahah finally figured it out ! */
                        fl=0;
                        if(f2==2)
                            doagain=1;
                        if(vb!=0 && doagain==0)
                            return 0;
                    }
                }
            }
            ct++;
            if(Actions[ct].Vocab!=0) doagain=0;
    }
    if(fl!=0 && disable_sysfunc==0)
    {
        int i;
        if(Items[LIGHT_SOURCE].Location==MyLoc ||
           Items[LIGHT_SOURCE].Location==CARRIED)
            d=0;
        
        /* Yes they really _are_ hardcoded values */
        if(vb==10) // get
        {
            if (!u_stricmp(NounText, "all"))
            {
                int ct;
                uchar f;
                    
                if(d)
                {
                    Output("It is dark.\n");
                    return 0;
                }

                f = 0;
                for (ct = 0; ct <= GameHeader.NumItems; ++ct)
                {
                    if(Items[ct].Location==MyLoc && Items[ct].AutoGet!=NULL && Items[ct].AutoGet[0]!='*')
                    {
                        no=WhichWord(Items[ct].AutoGet,Nouns);
                        disable_sysfunc=1;  /* Don't recurse into auto get ! */
                        PerformActions(vb,no);  /* Recursively check each items table code */
                        disable_sysfunc=0;
                        if(CountCarried()==GameHeader.MaxCarry)
                        {
#ifdef YOUARE
                            Output("You are carrying too much.\n");
#else
                            Output("I've too much to carry.\n");
#endif
                            return(0);
                        }
                        Items[ct].Location= CARRIED;
                        Redraw=1;
                        Output(Items[ct].Text);
                        Output(": OK\n");
                        f=1;
                    }
                }
                if(f==0) Output("Nothing taken.\n");
                return(0);
            }
            if(no==NOTFOUND)
            {
                Output("What?\n");
                return(0);
            }
            if(CountCarried()==GameHeader.MaxCarry)
            {
#ifdef YOUARE
                Output("You are carrying too much.\n");
#else
                Output("I've too much to carry.\n");
#endif
                return(0);
            }
            //i=MatchUpItem(NounText,MyLoc);
            i=MatchItem(no,MyLoc);
            if(i==-1)
            {
#ifdef YOUARE
                Output("It is beyond your power to do that.\n");
#else
                Output("It's beyond my power to do that.\n");
#endif
                return(0);
            }
            Items[i].Location= CARRIED;
            Output("OK");
            Redraw=1;
            return(0);
        }
        else if(vb==18) // drop
        {
            if(!u_stricmp(NounText,"ALL"))
            {
                int ct=0;
                int f=0;
                while(ct<=GameHeader.NumItems)
                {
                    if(Items[ct].Location==CARRIED && Items[ct].AutoGet && Items[ct].AutoGet[0]!='*')
                    {
                        no=WhichWord(Items[ct].AutoGet,Nouns);
                        disable_sysfunc=1;
                        PerformActions(vb,no);
                        disable_sysfunc=0;
                        Items[ct].Location=MyLoc;
                        Output(Items[ct].Text);
                        Output(": OK\n");
                        Redraw=1;
                        f=1;
                    }
                    ct++;
                }
                if(f==0)
                    Output("Nothing dropped.\n");
                return(0);
            }
            if(no==NOTFOUND)
            {
                Output("What?\n");
                return(0);
            }
            //i=MatchUpItem(NounText,CARRIED);
            i=MatchItem(no,CARRIED);
            if(i==-1)
            {
#ifdef YOUARE
                Output("It's beyond your power to do that.\n");
#else
                Output("It's beyond my power to do that.\n");
#endif
                return(0);
            }
            Items[i].Location=MyLoc;
            Output("OK");
            Redraw=1;
            return(0);
        }
    }
    return(fl);
}

void rungame()                                
{
    SaveBuf(startSave, sizeof(startSave));

    char yesno = CharInput("Do you want to restore a previously saved game? ");
    
    if (yesno == 'Y')
    {
        if (!LoadGame())
        {
            CharInput("Load Failed, Press ENTER to continue.");
        }
    }
    
    while(1)
    {
        uchar vb, no;
        
        if(Redraw!=0)
        {
            Look();
            Redraw=0;
        }
        PerformActions(0,0);
        if(Redraw!=0)
        {
            Look();
            Redraw=0;
        }
        GetInput(&vb,&no);
        switch(PerformActions(vb,no))
        {
            case -1:Output("I don't understand your command.\n");
                break;
            case -2:Output("I can't do that yet.\n");
                break;
        }
        /* Brian Howarth games seem to use -1 for forever */
        if(Items[LIGHT_SOURCE].Location/*==-1*/!=DESTROYED && GameHeader.LightTime!= -1)
        {
            GameHeader.LightTime--;
            if(GameHeader.LightTime<1)
            {
                //BitFlags|=(1L<<LIGHTOUTBIT);
                bfSet(LIGHTOUTBIT);
                if(Items[LIGHT_SOURCE].Location==CARRIED ||
                    Items[LIGHT_SOURCE].Location==MyLoc)
                {
#ifdef YOUARE
                    Output("Your light has run out.\n");
#else                    
                    Output("Light has run out!\n");
#endif

                }
                //if(Options&PREHISTORIC_LAMP) Items[LIGHT_SOURCE].Location=DESTROYED;
            }
            else if(GameHeader.LightTime<25)
            {
                if(Items[LIGHT_SOURCE].Location==CARRIED ||
                    Items[LIGHT_SOURCE].Location==MyLoc)
                {
            
#if 1 // SCOTTLIGHT
                    Output("Light runs out in ");
                    OutputNumber(GameHeader.LightTime);
                    Output(" turns.\n");
#else
                    if(GameHeader.LightTime%5==0) Output("Your light is growing dim.\n");
#endif
                }
            }
        }
    }    
}

void resetGame()
{
    LoadBuf(startSave);
    Redraw = 1;
}


