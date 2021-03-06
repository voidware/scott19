/*
 *	Controlling block
 */

#define LIGHT_SOURCE	9		/* Always 9 how odd */
#define CARRIED		255		/* Carried */
#define DESTROYED	0		/* Destroyed */
#define DARKBIT		15
#define LIGHTOUTBIT	16		/* Light gone out */

typedef unsigned char uchar;
 
typedef struct
{
 	short Unknown;
 	short NumItems;
 	short NumActions;
 	uchar NumWords;		/* Smaller of verb/noun is padded to same size */
 	uchar NumRooms;
 	short MaxCarry;
 	uchar PlayerRoom;
 	uchar Treasures;
 	uchar WordLength;
 	short LightTime;
 	short NumMessages;
 	uchar TreasureRoom;
} Header;

typedef struct
{
	unsigned short Vocab;
	unsigned short Condition[5];
	unsigned short Action[2];
} Action;

typedef struct
{
	char *Text;
	uchar Exits[6];
} Room;

typedef struct
{
	char *Text;
	/* PORTABILITY WARNING: THESE TWO MUST BE 8 BIT VALUES. */
	unsigned char Location;
	unsigned char InitialLoc;
	char *AutoGet;
} Item;

typedef struct
{
	short Version;
	short AdventureNumber;
	short Unknown;
} Tail;



//#define YOUARE		1	/* You are not I am */
#define SCOTTLIGHT	2	/* Authentic Scott Adams light messages */
#define DEBUGGING	4	/* Info from database load */
#define TRS80_STYLE	8	/* Display in style used on TRS-80 */
#define PREHISTORIC_LAMP 16	/* Destroy the lamp (very old databases) */


extern Header GameHeader;

extern void resetGame();

#ifndef GAME
extern Item *Items;
extern Action* Actions;
extern Room *Rooms;
extern char **Verbs;
extern char **Nouns;
extern char **Messages;
#endif

extern char lastChar;
extern uchar pos;
extern uchar Width;


