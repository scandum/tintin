/******************************************************************************
*   This file is part of TinTin++                                             *
*                                                                             *
*   Copyright 2004-2020 Igor van den Hoven                                    *
*                                                                             *
*   TinTin++ is free software; you can redistribute it and/or modify          *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 3 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with TinTin++.  If not, see https://www.gnu.org/licenses.           *
******************************************************************************/

/******************************************************************************
*                                                                             *
*              (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                   *
*                                                                             *
*                        coded by peter unold 1992                            *
*                       modified by Bill Reiss 1993                           *
*                    recoded by Igor van den Hoven 2004                       *
******************************************************************************/
	
#include <stdio.h>
#include <zlib.h>
#include <ctype.h>
#include <termios.h>
#include <pcre.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>

/******************************************************************************
*   Autoconf patching by David Hedbor                                         *
*******************************************************************************/

#include "config.h"

#if defined(HAVE_STRING_H)
#include <string.h>
#elif defined(HAVE_STRINGS_H)
#include <strings.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


#ifdef HAVE_TIME_H
#include <time.h>
#endif


#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#endif

/*
#ifdef SOCKS
#include <socks.h>
#endif
*/

#ifndef BADSIG
#define BADSIG (RETSIGTYPE (*)(int))-1
#endif


#ifdef HAVE_NET_ERRNO_H
#include <net/errno.h>
#endif


#ifdef HAVE_GNUTLS_H
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>
#else
#define gnutls_session_t int
#endif


#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if !defined(SO_PEERCRED)
	#define SO_PEERCRED 17
#endif

#ifndef __TINTIN_H__
#define __TINTIN_H__

/*
	A bunch of constants
*/

#define FLAG_NONE                        0

#define FALSE                            0
#define TRUE                             1

#define IS_LINE                          0
#define IS_PROMPT                        1

#define GET_ONE                          0 // stop at spaces
#define GET_ALL                          1 // stop at semicolon
#define GET_NST                          2 // nest square brackets
#define GET_VBT                          4 // ignore semicolon for verbatim mode

#define TEL_N                            0
#define TEL_Y                            1
#define TEL_I                            2

#define SCREEN_WIDTH                    80
#define SCREEN_HEIGHT                   24

#define SORT_PRIORITY                    0
#define SORT_ALPHA                       1
#define SORT_ALNUM                       2
#define SORT_APPEND                      3
#define SORT_DELAY                       4

#define DEFAULT_OPEN                   '{'
#define DEFAULT_CLOSE                  '}'

#define COMMAND_SEPARATOR              ';'

#define ASCII_NUL                        0
#define ASCII_ENQ                        5 // Ignore if possible
#define ASCII_HTML_AMP                   6 // Might conflict with MSDP
#define ASCII_BEL                        7
#define ASCII_BS                         8
#define ASCII_HTAB                       9
#define ASCII_LF                        10
#define ASCII_VTAB                      11
#define ASCII_FF                        12
#define ASCII_CR                        13
#define ASCII_ESC                       27
#define ASCII_HTML_OPEN                 28 // Also file separator, whatever that is
#define ASCII_HTML_CLOSE                30 // Also record separator, whatever that is

#define CTRL_A                           1
#define CTRL_B                           2
#define CTRL_C                           3
#define CTRL_D                           4
#define CTRL_E                           5
#define CTRL_F                           6
#define CTRL_G                           7
#define CTRL_H                           8
#define CTRL_I                           9
#define CTRL_J                          10
#define CTRL_K                          11
#define CTRL_L                          12
#define CTRL_M                          13
#define CTRL_N                          14
#define CTRL_O                          15
#define CTRL_P                          16
#define CTRL_Q                          17
#define CTRL_R                          18
#define CTRL_S                          19
#define CTRL_T                          20
#define CTRL_U                          21
#define CTRL_V                          22
#define CTRL_W                          23
#define CTRL_X                          24
#define CTRL_Y                          25
#define CTRL_Z                          26


#define ASCII_DEL                      127

#define DAEMON_DIR               "daemons"
#define TINTIN_DIR               ".tintin"

#define HISTORY_FILE         "history.txt"

#define BUFFER_SIZE                  40000
#define INPUT_SIZE                   10000
#define PATH_SIZE                     4096
#define STACK_SIZE                    1000
#define NAME_SIZE                      256
#define NUMBER_SIZE                    100
#define LEGEND_SIZE                     50
#define COLOR_SIZE                      50
#define CHAR_SIZE                        5
#define LIST_SIZE                        2

#define STRING_SIZE        2 * BUFFER_SIZE

#define CLIENT_NAME              "TinTin++"
#define CLIENT_VERSION           "2.02.04 "


#define XT_E                            0x27
#define XT_C                            0x5B
#define XT_O                            0x5D
#define XT_W                            0x74
#define XT_T                            0x07
#define XT_CS                           0x73
#define XT_S                            "\073"
#define XT_V                            "\000"

/*
	<faa> - red
	<afa> - green
	<ffa> - yellow
	<aaf> - blue
	<faf> - magenta
	<aff> - cyan
	<fff> - white
	<acf> - Azure
	<afc> - Jade
	<caf> - Violet
	<cfa> - Lime
	<fac> - Pink
	<fca> - Orange
*/

#define COLOR_BRACE         "\e[38;5;164m" // "<eae>" // magenta
#define COLOR_COMMAND       "\e[38;5;044m" // "<aee>" // cyan
#define COLOR_CONFIG        "\e[38;5;208m" // "<fca>" // orange
#define COLOR_RESET         "\e[0m"        // "<088>" // reset
#define COLOR_SEPARATOR     "\e[38;5;160m" // "<eaa>" // red
#define COLOR_STATEMENT     "\e[38;5;040m" // "<aea>" // green
#define COLOR_STRING        "\e[38;5;188m" // "<eee>" // white
//#define COLOR_TEXT          "\e[38;5;122m" // "<cfe>" // pale jade
#define COLOR_TEXT          "\e[0m" // "<cfe>" // pale jade
#define COLOR_TINTIN        "\e[38;5;184m" // "<eea>" // yellow
#define COLOR_REPEAT        "\e[38;5;33m"  // "<acf>" // azure

/*
	Index for lists used by tintin
*/

enum lists
{
	LIST_ACTION,
	LIST_ALIAS,
	LIST_BUTTON,
	LIST_CLASS,
	LIST_COMMAND,
	LIST_CONFIG,
	LIST_DELAY,
	LIST_EVENT,
	LIST_FUNCTION,
	LIST_GAG,
	LIST_HIGHLIGHT,
	LIST_HISTORY,
	LIST_LANDMARK,
	LIST_MACRO,
	LIST_PATH,
	LIST_PATHDIR,
	LIST_PROMPT,
	LIST_SUBSTITUTE,
	LIST_TAB,
	LIST_TERRAIN,
	LIST_TICKER,
	LIST_VARIABLE,
	LIST_MAX,
};
/*
#define LIST_ACTION                      0
#define LIST_ALIAS                       1
#define LIST_BUTTON                      2
#define LIST_CLASS                       3
#define LIST_COMMAND                     4
#define LIST_CONFIG                      5
#define LIST_DELAY                       6
#define LIST_EVENT                       7
#define LIST_FUNCTION                    8
#define LIST_GAG                         9
#define LIST_HIGHLIGHT                  10
#define LIST_HISTORY                    11
#define LIST_LANDMARK                   12
#define LIST_MACRO                      13
#define LIST_PATH                       14
#define LIST_PATHDIR                    15
#define LIST_PROMPT                     16
#define LIST_SUBSTITUTE                 17
#define LIST_TAB                        18
#define LIST_TERRAIN                    19
#define LIST_TICKER                     20
#define LIST_VARIABLE                   21
#define LIST_MAX                        22
*/
/*
	Command type
*/

enum operators
{
	TOKEN_TYPE_BREAK,
	TOKEN_TYPE_CASE,
	TOKEN_TYPE_COMMAND,
	TOKEN_TYPE_CONTINUE,
	TOKEN_TYPE_DEFAULT,
	TOKEN_TYPE_END,
	TOKEN_TYPE_ELSE,
	TOKEN_TYPE_ELSEIF,
	TOKEN_TYPE_FOREACH,
	TOKEN_TYPE_BROKEN_FOREACH,
	TOKEN_TYPE_IF,
	TOKEN_TYPE_LOOP,
	TOKEN_TYPE_BROKEN_LOOP,
	TOKEN_TYPE_PARSE,
	TOKEN_TYPE_BROKEN_PARSE,
	TOKEN_TYPE_REGEX,
	TOKEN_TYPE_RETURN,
	TOKEN_TYPE_SESSION,
	TOKEN_TYPE_STRING,
	TOKEN_TYPE_SWITCH,
	TOKEN_TYPE_WHILE,
	TOKEN_TYPE_BROKEN_WHILE,
	TOKEN_TYPE_STATEMENT,
	TOKEN_TYPE_CONFIG,
	TOKEN_TYPE_REPEAT
};

/*
	Various flags
*/
#define BV00 (0   <<  0)

#define BV01 (1   <<  0)
#define BV02 (1   <<  1)
#define BV03 (1   <<  2)
#define BV04 (1   <<  3)
#define BV05 (1   <<  4)
#define BV06 (1   <<  5)
#define BV07 (1   <<  6)
#define BV08 (1   <<  7)
#define BV09 (1   <<  8)
#define BV10 (1   <<  9)
#define BV11 (1   << 10)
#define BV12 (1   << 11)
#define BV13 (1   << 12)
#define BV14 (1   << 13)
#define BV15 (1   << 14)
#define BV16 (1   << 15)
#define BV17 (1   << 16)
#define BV18 (1   << 17)
#define BV19 (1   << 18)
#define BV20 (1   << 19)
#define BV21 (1   << 20)
#define BV22 (1   << 21)
#define BV23 (1   << 22)
#define BV24 (1   << 23)
#define BV25 (1   << 24)
#define BV26 (1   << 25)
#define BV27 (1   << 26)
#define BV28 (1   << 27)
#define BV29 (1   << 28)
#define BV30 (1   << 29)
#define BV31 (1   << 30)
#define BV32 (1LL << 31)
#define BV33 (1LL << 32)
#define BV34 (1LL << 33)
#define BV35 (1LL << 34)
#define BV36 (1LL << 35)
#define BV37 (1LL << 36)
#define BV38 (1LL << 37)
#define BV39 (1LL << 38)
#define BV40 (1LL << 39)


#define BUFFER_FLAG_GREP                  BV01


#define CHAR_FLAG_DIGIT                   BV01
#define CHAR_FLAG_ALPHA                   BV02
#define CHAR_FLAG_VAR                     BV03
#define CHAR_FLAG_SPACE                   BV04
#define CHAR_FLAG_HEX                     BV05
#define CHAR_FLAG_CSI                     BV06
#define CHAR_FLAG_PRINT                   BV07 // max


#define CHARSET_FLAG_UTF8                 BV01
#define CHARSET_FLAG_BIG5                 BV02
#define CHARSET_FLAG_GBK1                 BV03

#define CHARSET_FLAG_BIG5TOUTF8           BV04
#define CHARSET_FLAG_FANSITOUTF8          BV05
#define CHARSET_FLAG_GBK1TOUTF8           BV06
#define CHARSET_FLAG_ISO1TOUTF8           BV07
#define CHARSET_FLAG_ISO2TOUTF8           BV08
#define CHARSET_FLAG_KOI8TOUTF8           BV09
#define CHARSET_FLAG_CP1251TOUTF8         BV10

#define CHARSET_FLAG_EUC                  CHARSET_FLAG_BIG5|CHARSET_FLAG_GBK1
#define CHARSET_FLAG_ALL_TOUTF8           CHARSET_FLAG_BIG5TOUTF8|CHARSET_FLAG_CP1251TOUTF8|CHARSET_FLAG_FANSITOUTF8|CHARSET_FLAG_GBK1TOUTF8|CHARSET_FLAG_ISO1TOUTF8|CHARSET_FLAG_ISO2TOUTF8|CHARSET_FLAG_KOI8TOUTF8
#define CHARSET_FLAG_ALL                  CHARSET_FLAG_UTF8|CHARSET_FLAG_ALL_TOUTF8|CHARSET_FLAG_EUC


#define COL_BLD                       BV01
#define COL_ITA                       BV02
#define COL_UND                       BV03
#define COL_BLK                       BV04
#define COL_REV                       BV05

#define COL_XTF                       BV06
#define COL_XTF_5                     BV07
#define COL_XTF_R                     BV08
#define COL_XTB                       BV09
#define COL_XTB_5                     BV10
#define COL_XTB_R                     BV11

#define COL_TCF                       BV12
#define COL_TCF_2                     BV13
#define COL_TCF_R                     BV14
#define COL_TCB                       BV15
#define COL_TCB_2                     BV16
#define COL_TCB_R                     BV17

#define CHAT_NAME_CHANGE                 1
#define CHAT_REQUEST_CONNECTIONS         2
#define CHAT_CONNECTION_LIST             3
#define CHAT_TEXT_EVERYBODY              4
#define CHAT_TEXT_PERSONAL               5
#define CHAT_TEXT_GROUP                  6
#define CHAT_MESSAGE                     7
#define CHAT_DO_NOT_DISTURB              8
#define CHAT_SEND_ACTION                 9
#define CHAT_SEND_ALIAS                 10
#define CHAT_SEND_MACRO                 11
#define CHAT_SEND_VARIABLE              12
#define CHAT_SEND_EVENT                 13
#define CHAT_SEND_GAG                   14
#define CHAT_SEND_HIGHLIGHT             15
#define CHAT_SEND_LIST                  16
#define CHAT_SEND_ARRAY                 17
#define CHAT_SEND_BARITEM               18
#define CHAT_VERSION                    19
#define CHAT_FILE_START                 20
#define CHAT_FILE_DENY                  21
#define CHAT_FILE_BLOCK_REQUEST         22
#define CHAT_FILE_BLOCK                 23
#define CHAT_FILE_END                   24
#define CHAT_FILE_CANCEL                25
#define CHAT_PING_REQUEST               26
#define CHAT_PING_RESPONSE              27
#define CHAT_PEEK_CONNECTIONS           28
#define CHAT_PEEK_LIST                  29
#define CHAT_SNOOP_START                30
#define CHAT_SNOOP_DATA                 31

#define CHAT_END_OF_COMMAND            255

#define CHAT_FLAG_PRIVATE             BV01
#define CHAT_FLAG_REQUEST             BV02
#define CHAT_FLAG_SERVE               BV03
#define CHAT_FLAG_IGNORE              BV04
#define CHAT_FLAG_FORWARD             BV05
#define CHAT_FLAG_FORWARDBY           BV06
#define CHAT_FLAG_FORWARDALL          BV07
#define CHAT_FLAG_DND                 BV08
#define CHAT_FLAG_LINKLOST            BV09

#define DRAW_FLAG_NONE                   0
#define DRAW_FLAG_ASCII               BV01
#define DRAW_FLAG_BLANKED             BV02
#define DRAW_FLAG_BOT                 BV03
#define DRAW_FLAG_BOXED               BV04
#define DRAW_FLAG_BUMP                BV05
#define DRAW_FLAG_CIRCLED             BV06
#define DRAW_FLAG_COLOR1              BV07
#define DRAW_FLAG_COLOR2              BV08
#define DRAW_FLAG_CONVERT             BV09
#define DRAW_FLAG_CORNERED            BV10
#define DRAW_FLAG_CROSSED             BV11
#define DRAW_FLAG_FILLED              BV12
#define DRAW_FLAG_FOREGROUND          BV13
#define DRAW_FLAG_GRID                BV14
#define DRAW_FLAG_HOR                 BV15
#define DRAW_FLAG_HUGE                BV16
#define DRAW_FLAG_JEWELED             BV17
#define DRAW_FLAG_LEFT                BV18
//#define DRAW_FLAG_LINED               BV19 unused / obsolete
#define DRAW_FLAG_NUMBERED            BV20
#define DRAW_FLAG_PRUNED              BV21
#define DRAW_FLAG_RIGHT               BV22
#define DRAW_FLAG_ROUNDED             BV23
#define DRAW_FLAG_SCALED              BV24
#define DRAW_FLAG_SCROLL              BV25
#define DRAW_FLAG_SHADOWED            BV26
#define DRAW_FLAG_TEED                BV27
#define DRAW_FLAG_TOP                 BV28
#define DRAW_FLAG_TRACED              BV29
#define DRAW_FLAG_TUBED               BV30
#define DRAW_FLAG_UTF8                BV31
#define DRAW_FLAG_VER                 BV32

#define DRAW_FLAG_CURSIVE             BV33
#define DRAW_FLAG_FAT                 BV34
#define DRAW_FLAG_SANSSERIF           BV35
#define DRAW_FLAG_CALIGN              BV36
#define DRAW_FLAG_LALIGN              BV37
#define DRAW_FLAG_RALIGN              BV38
#define DRAW_FLAG_TALIGN              BV39
#define DRAW_FLAG_UALIGN              BV40

#define DRAW_FLAG_APPENDIX            DRAW_FLAG_CIRCLED|DRAW_FLAG_CORNERED|DRAW_FLAG_CROSSED|DRAW_FLAG_JEWELED|DRAW_FLAG_PRUNED|DRAW_FLAG_ROUNDED|DRAW_FLAG_TEED

#define INPUT_FLAG_EDIT               BV01
#define INPUT_FLAG_HISTORYBROWSE      BV02
#define INPUT_FLAG_HISTORYSEARCH      BV03
#define INPUT_FLAG_CONVERTMETACHAR    BV04

#define PORT_FLAG_PRIVATE             BV01
#define PORT_FLAG_REQUEST             BV02
#define PORT_FLAG_SERVE               BV03
#define PORT_FLAG_IGNORE              BV04
#define PORT_FLAG_FORWARD             BV05
#define PORT_FLAG_FORWARDBY           BV06
#define PORT_FLAG_FORWARDALL          BV07
#define PORT_FLAG_DND                 BV08
#define PORT_FLAG_LINKLOST            BV09

#define PORT_RANK_SPY                    0 // observer
#define PORT_RANK_DIPLOMAT               1 //
#define PORT_RANK_SCOUT                  2

#define RANK_FLAG_SCOUT               BV01

#define COMM_FLAG_DISCONNECT          BV01
#define COMM_FLAG_PASSWORD            BV02
#define COMM_FLAG_REMOTEECHO          BV03
#define COMM_FLAG_EOR                 BV04
#define COMM_FLAG_MSDPUPDATE          BV05
#define COMM_FLAG_256COLORS           BV06
#define COMM_FLAG_UTF8                BV07
#define COMM_FLAG_GMCP                BV08

#define MSDP_FLAG_COMMAND             BV01
#define MSDP_FLAG_LIST                BV02
#define MSDP_FLAG_SENDABLE            BV03
#define MSDP_FLAG_REPORTABLE          BV04
#define MSDP_FLAG_CONFIGURABLE        BV05
#define MSDP_FLAG_REPORTED            BV06
#define MSDP_FLAG_UPDATED             BV07

#define MTTS_FLAG_ANSI                BV01
#define MTTS_FLAG_VT100               BV02
#define MTTS_FLAG_UTF8                BV03
#define MTTS_FLAG_256COLORS           BV04
#define MTTS_FLAG_MOUSETRACKING       BV05
#define MTTS_FLAG_COLORPALETTE        BV06
#define MTTS_FLAG_SCREENREADER        BV07
#define MTTS_FLAG_PROXY               BV08
#define MTTS_FLAG_TRUECOLOR           BV09

#define SCAN_FLAG_NONE                0
#define SCAN_FLAG_FILE                BV01
#define SCAN_FLAG_SCAN                BV02

#define SCREEN_FLAG_CSIP              BV01
#define SCREEN_FLAG_OSCT              BV02
#define SCREEN_FLAG_OMIT              BV03
#define SCREEN_FLAG_GET_ONE           BV04
#define SCREEN_FLAG_GET_ALL           BV05
#define SCREEN_FLAG_GET_NONE          BV06

#define SCREEN_FLAG_SCROLLMODE        BV07
#define SCREEN_FLAG_SCROLLUPDATE      BV08

#define SCROLL_FLAG_RESIZE            BV01

#define STR_FLAG_STACK                BV01
#define STR_FLAG_LIST                 BV02
#define STR_FLAG_FREE                 BV03


// combines with event flags

#define SUB_NONE                      BV00

#define SUB_ARG                       BV01
#define SUB_SEC                       BV02
#define SUB_CMD                       BV03
#define SUB_VAR                       BV04
#define SUB_FUN                       BV05
#define SUB_COL                       BV06
#define SUB_ESC                       BV07
#define SUB_EOL                       BV08 // telnet
#define SUB_LNF                       BV09
#define SUB_SIL                       BV10 // silent
#define SUB_LIT                       BV11 // no soft escaping


/*
#define SUB_ARG                       BV01
#define SUB_SEC                       BV02
*/
#define EVENT_FLAG_CATCH              BV03
#define EVENT_FLAG_CLASS              BV04
#define EVENT_FLAG_GAG                BV05
#define EVENT_FLAG_INPUT              BV06
#define EVENT_FLAG_MAP                BV07
#define EVENT_FLAG_MOUSE              BV08
#define EVENT_FLAG_OUTPUT             BV09
#define EVENT_FLAG_PORT               BV10
#define EVENT_FLAG_SCAN               BV11
#define EVENT_FLAG_SCREEN             BV12
#define EVENT_FLAG_SESSION            BV13
#define EVENT_FLAG_SYSTEM             BV14
#define EVENT_FLAG_TELNET             BV15
#define EVENT_FLAG_TIME               BV16
#define EVENT_FLAG_UPDATE             BV17
#define EVENT_FLAG_VARIABLE           BV18
#define EVENT_FLAG_VT100              BV19


#define TAB_FLAG_FORWARD              BV01
#define TAB_FLAG_BACKWARD             BV02
#define TAB_FLAG_COMPLETE             BV03
#define TAB_FLAG_LIST                 BV04
#define TAB_FLAG_SCROLLBACK           BV05

#define REGEX_FLAG_NONE                  0
#define REGEX_FLAG_FIX                BV01
#define REGEX_FLAG_ARG                BV02
#define REGEX_FLAG_CMD                BV03



#define TINTIN_FLAG_GETNUMBER         BV01
#define TINTIN_FLAG_SESSIONUPDATE     BV02
#define TINTIN_FLAG_PROCESSINPUT      BV03
#define TINTIN_FLAG_INHERITANCE       BV04
#define TINTIN_FLAG_INSERTINPUT       BV05
#define TINTIN_FLAG_CHILDLOCK         BV06
#define TINTIN_FLAG_TERMINATE         BV06
#define TINTIN_FLAG_MOUSETRACKING     BV07
#define TINTIN_FLAG_DISPLAYUPDATE     BV08
#define TINTIN_FLAG_DAEMONIZE         BV09
#define TINTIN_FLAG_HIDDENCURSOR      BV10
#define TINTIN_FLAG_LOCAL             BV11
#define TINTIN_FLAG_PRESERVEMACRO     BV12


#define CONFIG_FLAG_AUTOPATCH         BV01
#define CONFIG_FLAG_AUTOPROMPT        BV02
#define CONFIG_FLAG_COLORPATCH        BV03
#define CONFIG_FLAG_CONVERTMETA       BV04
#define CONFIG_FLAG_ECHOCOMMAND       BV05
#define CONFIG_FLAG_MCCP              BV06
#define CONFIG_FLAG_MOUSEDEBUG        BV07
#define CONFIG_FLAG_MOUSEINFO         BV08
#define CONFIG_FLAG_MOUSEPIXELS       BV09
#define CONFIG_FLAG_MOUSETRACKING     BV10
#define CONFIG_FLAG_REPEATENTER       BV11
#define CONFIG_FLAG_SCREENREADER      BV12
#define CONFIG_FLAG_SCROLLLOCK        BV13
#define CONFIG_FLAG_SPEEDWALK         BV14
#define CONFIG_FLAG_TELNET            BV15
#define CONFIG_FLAG_VERBATIM          BV16
#define CONFIG_FLAG_VERBOSE           BV17
#define CONFIG_FLAG_WORDWRAP          BV18

#define SES_FLAG_BUFFERUPDATE         BV01
#define SES_FLAG_CLOSED               BV02
#define SES_FLAG_CONNECTED            BV03
#define SES_FLAG_GAG                  BV04
#define SES_FLAG_PATHMAPPING          BV05
#define SES_FLAG_PRINTBUFFER          BV06
#define SES_FLAG_PRINTLINE            BV07
#define SES_FLAG_READMUD              BV08
#define SES_FLAG_RUN                  BV09
#define SES_FLAG_SCANABORT            BV10
#define SES_FLAG_SCROLLSPLIT          BV11
#define SES_FLAG_SNOOP                BV12
#define SES_FLAG_SPLIT                BV13
#define SES_FLAG_UPDATEVTMAP          BV14


#define TELOPT_FLAG_TELNET            BV01
#define TELOPT_FLAG_SGA               BV02
#define TELOPT_FLAG_ECHO              BV03
#define TELOPT_FLAG_NAWS              BV04
#define TELOPT_FLAG_PROMPT            BV05
#define TELOPT_FLAG_DEBUG             BV06
#define TELOPT_FLAG_TSPEED            BV07
#define TELOPT_FLAG_TTYPE             BV08
#define TELOPT_FLAG_MTTS              BV09
#define TELOPT_FLAG_UPDATENAWS        BV10

#define LIST_FLAG_IGNORE              BV01
#define LIST_FLAG_PRIORITY            BV02
#define LIST_FLAG_MESSAGE             BV03
#define LIST_FLAG_DEBUG               BV04
#define LIST_FLAG_INFO                BV05
#define LIST_FLAG_LOG                 BV06
#define LIST_FLAG_CLASS               BV07
#define LIST_FLAG_READ                BV08
#define LIST_FLAG_WRITE               BV09
#define LIST_FLAG_HIDE                BV10
#define LIST_FLAG_INHERIT             BV11
#define LIST_FLAG_REGEX               BV12
#define LIST_FLAG_NEST                BV13
#define LIST_FLAG_DEFAULT             LIST_FLAG_MESSAGE

#define NODE_FLAG_ONESHOT             BV01

#define LOG_FLAG_NONE                    0
#define LOG_FLAG_LINEFEED             BV01
#define LOG_FLAG_OVERWRITE            BV02
#define LOG_FLAG_APPEND               BV03
#define LOG_FLAG_NEXT                 BV04
#define LOG_FLAG_LOW                  BV05

#define LOG_FLAG_HTML                 BV06
#define LOG_FLAG_PLAIN                BV07
#define LOG_FLAG_RAW                  BV08
#define LOG_FLAG_OLD_HTML             BV09
#define LOG_FLAG_OLD_PLAIN            BV10
#define LOG_FLAG_OLD_RAW              BV11


// Saved in map files, so don't swap around

// keep synced with exit flags

#define ROOM_FLAG_AVOID               BV01
#define ROOM_FLAG_HIDE                BV02
#define ROOM_FLAG_LEAVE               BV03
#define ROOM_FLAG_VOID                BV04
#define ROOM_FLAG_STATIC              BV05
#define ROOM_FLAG_CURVED              BV06
#define ROOM_FLAG_PATH                BV07
#define ROOM_FLAG_NOGLOBAL            BV08
#define ROOM_FLAG_INVIS               BV09

#define ROOM_FLAG_AVOID_TMP           BV10|ROOM_FLAG_AVOID // To realign exit and room flags in the future.
#define ROOM_FLAG_HIDE_TMP            BV11|ROOM_FLAG_HIDE
#define ROOM_FLAG_LEAVE_TMP           BV12|ROOM_FLAG_LEAVE
#define ROOM_FLAG_VOID_TMP            BV13|ROOM_FLAG_VOID
#define ROOM_FLAG_STATIC_TMP          BV13|ROOM_FLAG_STATIC
#define ROOM_FLAG_CURVED_TMP          BV14|ROOM_FLAG_CURVED
#define ROOM_FLAG_BLOCK               BV15
#define ROOM_FLAG_TERRAIN             BV20
#define ROOM_FLAG_FOG                 BV21

// keep synced with room flags

#define EXIT_FLAG_HIDE                BV01
#define EXIT_FLAG_AVOID               BV02
#define EXIT_FLAG_INVIS               BV03
#define EXIT_FLAG_BLOCK               BV04
#define EXIT_FLAG_ALL                 BV01|BV02|BV03|BV04

#define EXIT_GRID_0                     0
#define EXIT_GRID_N                     1
#define EXIT_GRID_E                     2
#define EXIT_GRID_S                     3
#define EXIT_GRID_W                     4
#define EXIT_GRID_U                     5
#define EXIT_GRID_D                     6
#define EXIT_GRID_NE                    7
#define EXIT_GRID_NW                    8
#define EXIT_GRID_SE                    9
#define EXIT_GRID_SW                   10

#define MAP_FLAG_STATIC               BV01
#define MAP_FLAG_VTMAP                BV02
#define MAP_FLAG_DIRECTION            BV03
#define MAP_FLAG_ASCIIGRAPHICS        BV04
#define MAP_FLAG_ASCIIVNUMS           BV05
#define MAP_FLAG_MUDFONT              BV06
#define MAP_FLAG_NOFOLLOW             BV07
#define MAP_FLAG_SYMBOLGRAPHICS       BV08
#define MAP_FLAG_UNICODEGRAPHICS      BV09
#define MAP_FLAG_BLOCKGRAPHICS        BV10
#define MAP_FLAG_RESIZE               BV11
#define MAP_FLAG_SYNC                 BV12
#define MAP_FLAG_ASCIILENGTH          BV13 // For debugging but might be useful
#define MAP_FLAG_TERRAIN              BV14
#define MAP_FLAG_UPDATETERRAIN        BV15
#define MAP_FLAG_DOUBLED              BV16
#define MAP_FLAG_QUIET                BV17
#define MAP_FLAG_READ                 BV18

#define MAP_SEARCH_NAME                0
#define MAP_SEARCH_EXITS               1
#define MAP_SEARCH_DESC                2
#define MAP_SEARCH_AREA                3
#define MAP_SEARCH_NOTE                4
#define MAP_SEARCH_TERRAIN             5
#define MAP_SEARCH_FLAG                6
#define MAP_SEARCH_ID                  7
#define MAP_SEARCH_MAX                 8

#define MAP_EXIT_N                     1
#define MAP_EXIT_E                     2
#define MAP_EXIT_S                     4
#define MAP_EXIT_W                     8
#define MAP_EXIT_U                    16
#define MAP_EXIT_D                    32

#define MAP_DIR_N                     (1LL << MAP_EXIT_N)
#define MAP_DIR_E                     (1LL << MAP_EXIT_E)
#define MAP_DIR_S                     (1LL << MAP_EXIT_S)
#define MAP_DIR_W                     (1LL << MAP_EXIT_W)
#define MAP_DIR_U                     (1LL << MAP_EXIT_U)
#define MAP_DIR_D                     (1LL << MAP_EXIT_D)

#define MAP_DIR_NE                    (1LL << (MAP_EXIT_N|MAP_EXIT_E))
#define MAP_DIR_NW                    (1LL << (MAP_EXIT_N|MAP_EXIT_W))
#define MAP_DIR_SE                    (1LL << (MAP_EXIT_S|MAP_EXIT_E))
#define MAP_DIR_SW                    (1LL << (MAP_EXIT_S|MAP_EXIT_W))

#define MAP_UNDO_MOVE                 (1 <<  0)
#define MAP_UNDO_CREATE               (1 <<  1)
#define MAP_UNDO_LINK                 (1 <<  2)
#define MAP_UNDO_INSERT               (1 <<  3)

#define TERRAIN_FLAG_DENSE            BV01
#define TERRAIN_FLAG_AMPLE            BV02
#define TERRAIN_FLAG_SPARSE           BV03
#define TERRAIN_FLAG_SCANT            BV04
#define TERRAIN_FLAG_NARROW           BV05
#define TERRAIN_FLAG_STANDARD         BV06
#define TERRAIN_FLAG_WIDE             BV07
#define TERRAIN_FLAG_VAST             BV08
#define TERRAIN_FLAG_FADEIN           BV09
#define TERRAIN_FLAG_FADEOUT          BV10
#define TERRAIN_FLAG_DOUBLE           BV11

#define MOUSE_FLAG_BUTTON_A              1
#define MOUSE_FLAG_BUTTON_B              2
#define MOUSE_FLAG_SHIFT                 4
#define MOUSE_FLAG_ALT                   8
#define MOUSE_FLAG_CTRL                 16
#define MOUSE_FLAG_MOTION               32
#define MOUSE_FLAG_WHEEL                64
#define MOUSE_FLAG_EXTRA               128
#define MOUSE_FLAG_RELEASE             256

#define CURSOR_FLAG_ALWAYS               1
//#define CURSOR_FLAG_NEVER               2
#define CURSOR_FLAG_GET_ONE              4
#define CURSOR_FLAG_GET_ALL              8

#define STARTUP_FLAG_NOGREETING          1
#define STARTUP_FLAG_SCREENREADER        2
#define STARTUP_FLAG_NORESET             4
#define STARTUP_FLAG_ARGUMENT            8
#define STARTUP_FLAG_NOTITLE            16
#define STARTUP_FLAG_VERBOSE            32

#define WRAP_FLAG_NONE                   0
#define WRAP_FLAG_DISPLAY             BV01
#define WRAP_FLAG_WORD                BV02
#define WRAP_FLAG_SPLIT               BV03
#define WRAP_FLAG_TAIL                BV04

#define LEGEND_ASCII                    0
#define LEGEND_ASCII_MISC              16
#define LEGEND_ASCII_CURVED            20
#define LEGEND_ASCII_DIRS              24
#define LEGEND_UNICODE                 32
#define LEGEND_UNICODE_MISC            48
#define LEGEND_UNICODE_CURVED          52
#define LEGEND_UNICODE_DIRS            56
#define LEGEND_MUDFONT                 64
#define LEGEND_MUDFONT_NWS             64
#define LEGEND_MUDFONT_NES             96
#define LEGEND_MUDFONT_CURVED         192
#define LEGEND_UNICODE_GRAPHICS       196
#define LEGEND_MAX                    230

#define UNICODE_DIR_SE                  1
#define UNICODE_DIR_NE                  2
#define UNICODE_DIR_SW                  4
#define UNICODE_DIR_NW                  8

#define UNICODE_DIR_D                  16
#define UNICODE_DIR_N                  17
#define UNICODE_DIR_S                  18
#define UNICODE_DIR_NS                 19
#define UNICODE_DIR_U                  20
#define UNICODE_DIR_E                  21
#define UNICODE_DIR_W                  22
#define UNICODE_DIR_EW                 23
#define UNICODE_DIR_RL                 24
#define UNICODE_DIR_RL_CURVED          25
#define UNICODE_DIR_RR                 26
#define UNICODE_DIR_RR_CURVED          27



#define MAP_COLOR_AVOID                 0
#define MAP_COLOR_BACK                  1
#define MAP_COLOR_BLOCK                 2
#define MAP_COLOR_EXIT                  3
#define MAP_COLOR_FOG                   4
#define MAP_COLOR_HIDE                  5
#define MAP_COLOR_INVIS                 6
#define MAP_COLOR_PATH                  7
#define MAP_COLOR_ROOM                  8
#define MAP_COLOR_SYMBOL                9
#define MAP_COLOR_USER                  10

#define MAP_COLOR_MAX                   11


/*
	Some macros to deal with double linked lists
*/

#define LINK(link, head, tail) \
{ \
	if ((head) == NULL) \
	{ \
		(head) = (link); \
	} \
	else \
	{ \
		(tail)->next = (link); \
	} \
	(link)->next = NULL; \
	(link)->prev = (tail); \
	(tail)				    = (link); \
}

/*
#define INSERT_LEFT(link, right, head) \
{ \
	(link)->prev = (right)->prev; \
	(right)->prev = (link); \
	(link)->next = (right); \
	\
	if ((link)->prev) \
	{ \
		(link)->prev->next = (link); \
	} \
	else \
	{ \
		(head) = (link); \
	} \
}


#define INSERT_RIGHT(link, left, tail) \
{ \
	(link)->next = (left)->next; \
	(left)->next = (link); \
	(link)->prev = (left); \
\
	if ((link)->next) \
	{ \
		(link)->next->prev = (link); \
	} \
	else \
	{ \
		(tail) = (link); \
	} \
}
*/

#define UNLINK(link, head, tail) \
{ \
	if (((link)->prev == NULL && (link) != head) \
	||  ((link)->next == NULL && (link) != tail)) \
	{ \
		tintin_printf2(NULL, "#UNLINK ERROR in file %s on line %d", __FILE__, __LINE__); \
		dump_stack(); \
	} \
	if ((link)->prev == NULL) \
	{ \
		(head)			   = (link)->next; \
	} \
	else \
	{ \
		(link)->prev->next	  = (link)->next; \
	} \
	if ((link)->next == NULL) \
	{ \
		(tail)			    = (link)->prev; \
	} \
	else \
	{ \
		(link)->next->prev	  = (link)->prev; \
	} \
	(link)->next = NULL; \
	(link)->prev = NULL; \
}

/*
	string allocation
*/

#define RESTRING(point, value) \
{ \
	if (point) \
	{ \
		free(point); \
	} \
	point = strdup((value)); \
}
/*
#define FREE(point) \
{ \
	free((point)); \
	point = NULL; \
}
*/
/*
	Bit operations
*/

#define HAS_BIT(bitvector, bit)   ((bitvector)  & (bit))
#define SET_BIT(bitvector, bit)   ((bitvector) |= (bit))
#define DEL_BIT(bitvector, bit)   ((bitvector) &= (~(bit)))
#define TOG_BIT(bitvector, bit)   ((bitvector) ^= (bit))
#define FFS_BIT(bitvector)        ((ffs(bitvector) - 1))

/*
	Generic
*/

#define URANGE(a, b, c)           ((b) < (a) ? (a) : (b) > (c) ? (c) : (b))
//#define URANGE(a, b, c)           ((b) <= (a) ? (a) : (c) >= (b) ? (b) : (c) < (b) ? (b) : (c))

#define UMAX(a, b)                ((a) > (b) ? (a) : (b))
#define UMIN(a, b)                ((a) < (b) ? (a) : (b))

#define next_arg(u)               (u < 99 ? u++ : u)

#define IS_SPLIT(ses)             (gtd->screen->rows != (ses)->split->bot_row)

#define SCROLL(ses)               ((ses)->cur_row == 0 || ((ses)->cur_row >= (ses)->split->top_row && (ses)->cur_row <= (ses)->split->bot_row) || ((ses)->cur_row >= ses->input->top_row && (ses)->cur_row <= ses->input->bot_row))

#define VERBATIM(ses)             (gtd->level->verbatim || (gtd->level->input == 0 && HAS_BIT((ses)->config_flags, CONFIG_FLAG_VERBATIM)) || HAS_BIT(gtd->flags, TINTIN_FLAG_CHILDLOCK))



/*
	Compatibility
*/


#define atoll(str) (strtoll(str, NULL, 10))

/************************ structures *********************/

struct listroot
{
	struct listnode      ** list;
	struct session        * ses;
	int                     size;
	int                     used;
	int                     update;
	short                   type;
	short                   flags;
};

struct listnode
{
	struct listroot       * root; // variable
	char                  * arg1;
	char                  * arg2;
	char                  * arg3;
	char                  * arg4;
	char                  * group;
	unsigned int            shots;
	union
	{
		pcre              * regex;      // act, alias, gag, highlight, substitute
		char              * data;       // class
		struct room_data  * room;       // terrain
		long long           val64;      // delay, tick, path
		short               val16[4];   // button
		int                 val32[2];   // landmark, event, pathdir
	};
};

/*
struct tableroot
{
	struct listtable     ** table;
	int                     size;
	int                     used;
};

struct tablenode
{
	struct tableroot      * root;
	char                  * arg1;
	char                  * arg2;
};
*/

struct scriptroot
{
	struct scriptnode    * next;
	struct scriptnode    * prev;
	struct session       * ses;
	struct listroot      * local;
	int                    list;
};

struct process_data
{
	pid_t                   pid;
	uid_t                   uid;
	gid_t                   gid;
};

struct tintin_data
{
	struct session        * ses;
	struct session        * update;
	struct session        * all;
	struct session        * dispose_next;
	struct session        * dispose_prev;
	struct listroot       * dispose_list;
	struct listroot       * banner_list;
	struct chat_data      * chat;
	struct screen_data    * screen;
	struct level_data     * level;
	struct memory_data    * memory;
	struct system_data    * system;
	struct termios          old_terminal;
	char                  * detach_file;
	int                     detach_port;
	struct process_data     detach_info;
	int                     detach_sock;
	char                 *  attach_file;
	int                     attach_pid;
	int                     attach_sock;
	int                     daemon;
	char                  * buf;
	char                  * out;
	char                  * mud_output_buf;
	int                     mud_output_max;
	int                     mud_output_len;
	unsigned char         * mccp_buf;
	int                     mccp_len;
	char                    macro_buf[BUFFER_SIZE];
	char                    is_result[NUMBER_SIZE];
	time_t                  time;
	time_t                  time_daemon;
	time_t                  time_input;
	time_t                  time_session;
	unsigned long long      utime;
	unsigned long long      utime_next_delay;
	unsigned long long      utime_next_tick;
	long long               total_io_exec;
	long long               total_io_delay;
	long long               convert_time;
	int                     history_size;
	unsigned short          command_ref[32];
	int                     msdp_table_size;
	int                     flags;
	int                     event_flags;
	struct scriptroot     * script_stack[STACK_SIZE];
	int                     script_index;
	char                    tintin_char;
	char                    verbatim_char;
	char                    repeat_char;
	char                  * vars[100];
	char                  * cmds[100];
	int                     args[100];
	char                    color_reset[COLOR_SIZE];
};

struct session
{
	struct session        * next;
	struct session        * prev;
	struct map_data       * map;
	struct port_data      * port;
	z_stream              * mccp2;
	z_stream              * mccp3;
	gnutls_session_t        ssl;
	struct termios          cur_terminal;
	struct scroll_data    * scroll;
	struct split_data     * split;
	struct input_data     * input;
	char                  * name;
	char                  * group;
	FILE                  * logfile;
	char                  * logname;
	int                     logmode;
	FILE                  * lognext_file;
	char                  * lognext_name;
	time_t                  lognext_time;
	FILE                  * logline_file;
	char                  * logline_name;
	time_t                  logline_time;
	char                  * line_capturefile;
	int                     line_captureindex;
	int                     gagline;
	struct listroot       * list[LIST_MAX];
	time_t                  created;
	int                     cur_row;
	int                     sav_row;
	int                     cur_col;
	int                     sav_col;
	int                     wrap;
	int                     fgc;
	int                     bgc;
	int                     vtc;
	int                     socket;
	int                     telopts;
	int                     telopt_flag[8];
	int                     event_flags;
        int                     config_flags;
	int                     flags;
	int                     charset;
	char                  * session_host;
	char                  * session_ip;
	char                  * session_port;
	char                  * cmd_color;
	unsigned char         * read_buf;
	int                     read_len;
	int                     read_max;
	unsigned long long      connect_retry;
	int                     connect_error;
	char                    more_output[BUFFER_SIZE];
	int                     color;
	char                    color_patch[100];
	unsigned long long      packet_patch;
	unsigned long long      check_output;
	int                     scrollback_tab;
	int                     tab_width;
	unsigned long long      rand;
	unsigned short          rkey;
	struct port_data      * proxy;
};

struct edit_data
{
	struct row_data      ** line;
	int                     used;
	int                     size;
	int                     update;
};

struct system_data
{
	char                  * tt_dir;
	char                  * home;
	char                  * lang;
	char                  * os;
	char                  * term;
	char                  * exec;
};

struct input_data
{
	struct edit_data      * line;
	struct edit_data      * edit;
	char                  * line_name;
	char                  * edit_name;

	int                     flags;

	int                     sav_top_row;
	int                     sav_top_col;
	int                     sav_bot_row;
	int                     sav_bot_col;

	int                     top_row;
	int                     top_col;
	int                     bot_row;
	int                     bot_col;

	int                     cur_row;

	int                     str_len;
	int                     raw_len;
	int                     str_pos;
	int                     raw_pos;

	int                     str_hid;
	int                     raw_hid;

	int                     str_off;

	char                   *buf;
	char                   *tmp;
	char                   *cut;
};

struct level_data
{
	unsigned int            background;
	unsigned int            convert;
	unsigned int            debug;
	unsigned int            grep;
	unsigned int            ignore;
	unsigned int            info;
	unsigned int            input;
	unsigned int            local;
	unsigned int            mshot;
	unsigned int            quiet;
	unsigned int            repeat;
	unsigned int            scan;
	unsigned int            scroll;
	unsigned int            shots;
	unsigned int            verbatim;
	unsigned int            verbose;
};

struct split_data
{
	int                     sav_top_row;
	int                     sav_top_col;
	int                     sav_bot_row;
	int                     sav_bot_col;
	int                     top_row;
	int                     top_col;
	int                     bot_row;
	int                     bot_col;
};


struct scroll_data
{
	struct buffer_data   ** buffer;
	int                     base;
	int                     line;
	int                     used;
	int                     size;
	int                     wrap;
	time_t                  time;
	char                  * input;
	int                     flags;
	int                     width;
};

struct buffer_data
{
	int                     width;
	int                     height;
	unsigned short          lines;
	unsigned short          flags;
	time_t                  time;
	char                  * str;
};

struct chat_data
{
	struct chat_data      * next;
	struct chat_data      * prev;
	struct chat_data      * update;
	char                  * name;
	char                  * ip;
	char                  * version;
	char                  * download;
	char                  * reply;
	char                  * prefix;
	char                  * paste_buf;
	char                  * color;
	char                  * group;
	int                     port;
	int                     fd;
	time_t                  timeout;
	int                     flags;
	unsigned long long      paste_time;
	FILE                  * file_pt;
	char                  * file_name;
	long long               file_size;
	int                     file_block_cnt;
	int                     file_block_tot;
	int                     file_block_patch;
	unsigned long long      file_start_time;
};

struct port_data
{
	struct port_data      * next;
	struct port_data      * prev;
	struct port_data      * update;
	char                  * name;
	char                  * ip;
	char                  * prefix;
	char                  * color;
	char                  * group;
	int                     port;
	int                     fd;
	int                     flags;
	int                     comm_flags;
	int                     mtts_flags;
	struct msdp_data     ** msdp_data;
	char                  * proxy;
	char                  * ttype;
	char                    telbuf[BUFFER_SIZE];
	int                     teltop;
	char                    inbuf[BUFFER_SIZE];
	int                     intop;
	int                     cols;
	int                     rows;
	int                     total;
	int                     rank;
	z_stream              * mccp2;
	z_stream              * mccp3;
	struct session        * ses;
};

struct link_data
{
	struct link_data     * next;
	struct link_data     * prev;
	char                 * str1;
	char                 * str2;
	char                 * str3;
};

struct map_data
{
	struct room_data     ** room_list;
	struct room_data     ** grid_rooms;
	int                   * grid_vnums;
	FILE                  * logfile;
	struct link_data      * undo_head;
	struct link_data      * undo_tail;
	struct search_data    * search;
	char                  * buf;
	char                  * out;
	char                    color[MAP_COLOR_MAX][COLOR_SIZE];
	char                    color_raw[MAP_COLOR_MAX][COLOR_SIZE];
	int                     center_x;
	int                     center_y;
	int                     center_z;
	int                     max_grid_x;
	int                     max_grid_y;
	int                     sav_top_row;
	int                     sav_top_col;
	int                     sav_bot_row;
	int                     sav_bot_col;
	int                     top_row;
	int                     top_col;
	int                     bot_row;
	int                     bot_col;
	int                     rows;
	int                     cols;
	int                     undo_size;
	int                     dir;
	int                     size;
	int                     flags;
	int                     in_room;
	int                     at_room;
	int                     last_room;
	int                     global_vnum;
	struct exit_data      * global_exit;
	int                     version;
	short                   display_stamp;
	int                     nofollow;
	char                    legend[LEGEND_MAX][LEGEND_SIZE];
	char                    legend_raw[LEGEND_MAX][LEGEND_SIZE];
};

struct room_data
{
	struct exit_data        * f_exit;
	struct exit_data        * l_exit;
	struct exit_data        * exit_grid[11];
	int                       vnum;
	long long                 exit_dirs;
	float                     length;
	float                     weight;
	unsigned short            exit_size;
	unsigned short            search_stamp;
	unsigned short            display_stamp;
	int                       flags;
	int                       w;
	int                       x;
	int                       y;
	int                       z;
	int                       terrain_index;
	short                     terrain_flags;
	char                    * area;
	char                    * color;
	char                    * data;
	char                    * desc;
	char                    * id;
	char                    * name;
	char                    * note;
	char                    * symbol;
	char                    * terrain;
};

struct exit_data
{
	struct exit_data        * next;
	struct exit_data        * prev;
	int                       vnum;
	int                       dir;
	int                       grid;
	int                       flags;
	float                     weight;
	float                     delay;
	char                    * name;
	char                    * cmd;
	char                    * color;
	char                    * data;
};

struct search_data
{
	int                     vnum;
	unsigned short          stamp;
	char                  * arg;
	pcre                  * name;
	int                     exit_size;
	long long               exit_dirs;
	char                  * exit_list;
	pcre                  * desc;
	pcre                  * area;
	pcre                  * note;
	pcre                  * terrain;
	long long               flag;
	char                  * id;
};

struct msdp_data
{
	char                 * value;
	int                    flags;
};

struct memory_data
{
	struct stack_data      ** debug;
	int                       debug_len;
	int                       debug_max;

	struct str_data        ** stack;
	int                       stack_len;
	int                       stack_cap;
	int                       stack_max;

	struct str_data        ** list;
	int                       list_len;
	int                       list_max;

	int                     * free;
	int                       free_len;
	int                       free_max;
};

struct row_data
{
	char                  * str;
};

struct screen_data
{
	struct row_data      ** line;
	struct row_data      ** grid;
	int                     flags;
	int                     rows;
	int                     cols;
	int                     height;
	int                     width;
	int                     tot_height;
	int                     tot_width;
	int                     pos_height;
	int                     pos_width;
	int                     minimized;
	int                     focus;
	int                     char_height;
	int                     char_width;
	int                     desk_rows;
	int                     desk_cols;
	int                     desk_height;
	int                     desk_width;
	int                     top_row;
	int                     bot_row;
	int                     cur_row;
	int                     cur_col;
	int                     max_row;
	int                     sav_lev;
	int                     sav_row[STACK_SIZE];
	int                     sav_col[STACK_SIZE];
};

struct stack_data
{
	char                    * name;
	int                       index;
};

struct str_data
{
	int                       index;
	int                       max;
	int                       len;
	short                     flags;
	short                     blank;
};


// unused

struct window_data
{
	char                    *name;

	int                     top_row;
	int                     top_col;
	int                     bot_row;
	int                     bot_col;

	int                     off_row;
	int                     off_col;

	int                     max_row;
	int                     max_col;

	int                     pos_row;
	int                     pos_col;

	struct input_data      **buffer;
};

#define DO_ARRAY(array)       struct session *array (struct session *ses, struct listnode *list, char *arg, char *var, char *arg1, char *arg2)
#define DO_BUFFER(buffer)               void buffer (struct session *ses, char *arg, char *arg1, char *arg2)
#define DO_CHAT(chat)                     void chat (char *arg1, char *arg2)
#define DO_CLASS(class)       struct session *class (struct session *ses, struct listnode *node, char *arg1, char *arg2)
#define DO_COMMAND(command) struct session *command (struct session *ses, char *arg, char *arg1, char *arg2, char *arg3, char *arg4)
#define DO_CONFIG(config)    struct session *config (struct session *ses, char *arg1, char *arg2, int index)
#define DO_CURSOR(cursor)               void cursor (struct session *ses, char *arg)
#define DO_DAEMON(daemon)               void daemon (struct session *ses, char *arg, char *arg1, char *arg2)
#define DO_EDIT(edit)          struct session *edit (struct session *ses, char *arg, char *arg1, char *arg2)
#define DO_HISTORY(history)            void history (struct session *ses, char *arg, char *arg1, char *arg2)
#define DO_LINE(line)          struct session *line (struct session *ses, char *arg, char *arg1, char *arg2, char *arg3)
#define DO_LOG(log)                        void log (struct session *ses, char *arg, char *arg1, char *arg2)
#define DO_MAP(map)                        void map (struct session *ses, char *arg, char *arg1, char *arg2)
#define DO_PATH(path)                     void path (struct session *ses, char *arg)
#define DO_PORT(port)          struct session *port (struct session *ses, char *arg1, char *arg2, char *arg)
#define DO_SCAN(scan)          struct session *scan (struct session *ses, FILE *fp, char *arg, char *arg1, char *arg2)
#define DO_SCREEN(screen)               void screen (struct session *ses, int ind, char *arg, char *arg1, char *arg2)
#define DO_DRAW(draw)                     void draw (struct session *ses, int top_row, int top_col, int bot_row, int bot_col, int rows, int cols, long long flags, char *color, char *arg, char *arg1, char *arg2, char *arg3)

/*
	Typedefs
*/

typedef int             CMPFUNC (const void *a, const void *b);

typedef struct session *ARRAY   (struct session *ses, struct listnode *list, char *arg, char *var, char *arg1, char *arg2);
typedef void            BUFFER  (struct session *ses, char *arg, char *arg1, char *arg2);
typedef void            CHAT    (char *arg1, char *arg2);
typedef struct session *CLASS   (struct session *ses, struct listnode *node, char *left, char *right);
typedef struct session *CONFIG  (struct session *ses, char *arg1, char *arg2, int index);
typedef struct session *COMMAND (struct session *ses, char *arg, char *arg1, char *arg2, char *arg3, char *arg4);
typedef void            CURSOR  (struct session *ses, char *arg);
typedef void            DAEMON  (struct session *ses, char *arg, char *arg1, char *arg2);
typedef void            DRAW    (struct session *ses, int top_row, int top_col, int bot_row, int bot_col, int rows, int cols, long long flags, char *color, char *arg, char *arg1, char *arg2, char *arg3);
typedef struct session *EDIT    (struct session *ses, char *arg, char *arg1, char *arg2);
typedef void            HISTORY (struct session *ses, char *arg, char *arg1, char *arg2);
typedef void            LOG     (struct session *ses, char *arg, char *arg1, char *arg2);
typedef struct session *LINE    (struct session *ses, char *arg, char *arg1, char *arg2, char *arg3);
typedef void            MAP     (struct session *ses, char *arg, char *arg1, char *arg2);
typedef void            MSDP    (struct session *ses, struct port_data *buddy, int index);
typedef void            PATH    (struct session *ses, char *arg);
typedef struct session *PORT    (struct session *ses, char *arg1, char *arg2, char *arg);
typedef struct session *SCAN    (struct session *ses, FILE *fp, char *arg, char *arg1, char *arg2);
typedef void            SCREEN  (struct session *ses, int ind, char *arg, char *arg1, char *arg2);




/*
	Structures for tables.c
*/

struct array_type
{
	char                  * name;
	ARRAY                 * fun;
	char                  * desc;
};

struct buffer_type
{
	char                  * name;
	BUFFER                * fun;
	char                  * desc;
};

struct chat_type
{
	char                  * name;
	CHAT                  * fun;
	int                     lval;
	int                     rval;
	char                  * desc;
};

struct class_type
{
	char                  * name;
	CLASS                 * fun;
};

struct color_type
{
	char                  * name;
	char                  * code;
	int                     len;
};

struct command_type
{
	char                  * name;
	COMMAND               * command;
	int                     args;
	int                     type;
};

struct config_type
{
	char                  * name;
	char                  * msg_on;
	char                  * msg_off;
	CONFIG                * config;
};

struct cursor_type
{
	char                  * name;
	char                  * desc;
	char                  * code;
	int                     flags;
	CURSOR                * fun;
	char                  * arg;
};

struct daemon_type
{
	char                  * name;
	DAEMON                * fun;
	char                  * desc;
};

struct draw_type
{
	char                  * name;
	char                  * desc;
	int                     flags;
	DRAW                  * fun;
};

struct edit_type
{
	char                  * name;
	EDIT                  * fun;
	char                  * desc;
};

struct event_type
{
	char                  * name;
	int                     level;
	int                     flags;
	char                  * group;
	char                  * desc;
};

struct history_type
{
	char                  * name;
	HISTORY               * fun;
	char                  * desc;
};

struct list_type
{
	char                  * name;
	char                  * name_multi;
	int                     mode;
	int                     args;
	int                     script_arg;
	int                     priority_arg;
	int                     flags;
};


struct line_type
{
	char                  * name;
	LINE                  * fun;
	char                  * desc;
};

struct log_type
{
	char                  * name;
	LOG                   * fun;
	char                  * desc;
};

struct map_type
{
	char                  * name;
	MAP                   * fun;
	int                     flags;
	int                     check;
	char                  * desc;
};

struct map_legend_type
{
	char                  * name;
	char                  * group;
	char                  * min;
	char                  * max;
};

struct map_legend_group_type
{
	char                  * name;
	char                  * group;
	int                     min_row;
	int                     min_col;
	int                     max_row;
	int                     max_col;
	int                     start;
	int                     end;
	char                  * reset;
};

struct msdp_type
{
	char                 * name;
	int                    flags;
	int                    rank;
	MSDP                 * fun;
};

struct path_type
{
	char                  * name;
	PATH                  * fun;
	char                  * desc;
};

struct port_type
{
	char                  * name;
	PORT                  * fun;
	int                     lval;
	int                     rval;
	char                  * desc;
};

struct pulse_type
{
	unsigned char           update_input;
	unsigned char           update_sessions;
	unsigned char           update_delays;
	unsigned char           update_daemon;
	unsigned char           update_chat;
	unsigned char           update_port;
	unsigned char           update_ticks;
	unsigned char           update_paths;
	unsigned char           update_packets;
	unsigned char           update_terminal;
	unsigned char           update_memory;
	unsigned char           update_time;
};


struct rank_type
{
	char                  * name;
	int                     flags;
};

struct scan_type
{
	char                  * name;
	SCAN                  * fun;
	int                   flags;
	char                  * desc;
};

struct screen_type
{
	char                  * name;
	char                  * desc;
	int                     get1;
	int                     get2;
	int                     flags;
	SCREEN                * fun;
};


struct stamp_type
{
	char                  * name;
	int                     length;
	char                  * desc;
};

struct substitution_type
{
	char                  * name;
	int                     bitvector;
};

struct timer_type
{
	char                  * name;
};

struct telopt_type
{
	char                  * name;
	int                     want;
	int                     flags;
};




/*
	Various structures
*/


#endif


/*
	Function declarations
*/



#ifndef __ARRAY_H__
#define __ARRAY_H__

extern DO_COMMAND(do_list);

extern DO_ARRAY(array_add);
extern DO_ARRAY(array_clear);
extern DO_ARRAY(array_collapse);
extern DO_ARRAY(array_create);
extern DO_ARRAY(array_delete);
extern DO_ARRAY(array_explode);
extern DO_ARRAY(array_find);
extern DO_ARRAY(array_get);
extern DO_ARRAY(array_index);
extern DO_ARRAY(array_insert);
extern DO_ARRAY(array_order);
extern DO_ARRAY(array_reverse);
extern DO_ARRAY(array_set);
extern DO_ARRAY(array_shuffle);
extern DO_ARRAY(array_simplify);
extern DO_ARRAY(array_size);
extern DO_ARRAY(array_sort);
extern DO_ARRAY(array_tokenize);

#endif


#ifndef __BANNER_H__
#define __BANNER_H__

extern DO_COMMAND(do_banner);

#endif


#ifndef __BASE_H__
#define __BASE_H__

extern int str_to_base64(char *in, char *out, size_t size);
extern int base64_to_str(char *in, char *out, size_t size);

extern void str_to_base64z(char *in, char *out, size_t size);
extern void base64z_to_str(char *in, char *out, size_t size);

extern int str_to_base252(char *in, char *out, size_t size);
extern int base252_to_str(char *in, char *out, size_t size);

extern void str_to_base252z(char *in, char *out, size_t size);
extern void base252z_to_str(char *in, char *out, size_t size);

#endif

#ifndef __BUFFER_H__
#define __BUFFER_H__

extern DO_COMMAND(do_buffer);
extern DO_COMMAND(do_grep);

extern void init_buffer(struct session *ses, int size);
extern void add_line_buffer(struct session *ses, char *line, int more_output);
extern int show_buffer(struct session *ses);

extern DO_BUFFER(buffer_up);
extern DO_BUFFER(buffer_clear);
extern DO_BUFFER(buffer_down);
extern DO_BUFFER(buffer_get);
extern DO_BUFFER(buffer_home);
extern DO_BUFFER(buffer_end);
extern DO_BUFFER(buffer_find);
extern DO_BUFFER(buffer_jump);
extern DO_BUFFER(buffer_lock);
extern DO_BUFFER(buffer_refresh);
extern DO_BUFFER(buffer_write);
extern DO_BUFFER(buffer_info);

#endif


#ifndef __CHAT_H__
#define __CHAT_H__

extern DO_COMMAND(do_chat);

extern void process_chat_connections(fd_set *read_set, fd_set *write_set, fd_set *exc_set);
extern void chat_socket_printf(struct chat_data *buddy, char *format, ...);
extern void close_chat(struct chat_data *buddy, int unlink);
extern void chat_forward_session(struct session *ses, char *linelog);

extern DO_CHAT(chat_accept);
extern DO_CHAT(chat_call);
extern DO_CHAT(chat_cancelfile);
extern DO_CHAT(chat_color);
extern DO_CHAT(chat_decline);
extern DO_CHAT(chat_dnd);
extern DO_CHAT(chat_downloaddir);
extern DO_CHAT(chat_emote);
extern DO_CHAT(chat_filestat);
extern DO_CHAT(chat_group);
extern DO_CHAT(chat_forward);
extern DO_CHAT(chat_forwardall);
extern DO_CHAT(chat_ignore);
extern DO_CHAT(chat_initialize);
extern DO_CHAT(chat_info);
extern DO_CHAT(chat_ip);
extern DO_CHAT(chat_message);
extern DO_CHAT(chat_name);
extern DO_CHAT(chat_paste);
extern DO_CHAT(chat_peek);
extern DO_CHAT(chat_ping);
extern DO_CHAT(chat_prefix);
extern DO_CHAT(chat_private);
extern DO_CHAT(chat_public);
extern DO_CHAT(chat_reply);
extern DO_CHAT(chat_request);
extern DO_CHAT(chat_send);
extern DO_CHAT(chat_sendfile);
extern DO_CHAT(chat_transfer);
extern DO_CHAT(chat_serve);
extern DO_CHAT(chat_uninitialize);
extern DO_CHAT(chat_who);
extern DO_CHAT(chat_zap);

#endif


#ifndef __CLASS_H__
#define __CLASS_H__

extern DO_COMMAND(do_class);

extern  int count_class(struct session *ses, struct listnode *group);
extern void parse_class(struct session *ses, char *input, struct listnode *group);

extern DO_CLASS(class_assign);
extern DO_CLASS(class_clear);
extern DO_CLASS(class_close);
extern DO_CLASS(class_kill);
extern DO_CLASS(class_list);
extern DO_CLASS(class_load);
extern DO_CLASS(class_open);
extern DO_CLASS(class_read);
extern DO_CLASS(class_save);
extern DO_CLASS(class_size);
extern DO_CLASS(class_write);

#endif

#ifndef __COMMAND_H__
#define __COMMAND_H__

extern DO_COMMAND(do_commands);

extern void init_commands(void);

#endif

#ifndef __CURSOR_H__
#define __CURSOR_H__

extern DO_COMMAND(do_cursor);

int inputline_cur_row(void);
int inputline_cur_col(void);
int inputline_max_row(void);
int inputline_editor(void);
int inputline_rows(struct session *ses);

void inputline_insert(char *arg, int str_pos);
void inputline_set(char *arg, int str_pos);

extern DO_CURSOR(cursor_backspace);
extern DO_CURSOR(cursor_brace);
extern DO_CURSOR(cursor_buffer_down);
extern DO_CURSOR(cursor_buffer_end);
extern DO_CURSOR(cursor_buffer_home);
extern DO_CURSOR(cursor_buffer_lock);
extern DO_CURSOR(cursor_buffer_up);
extern DO_CURSOR(cursor_check_line);
extern DO_CURSOR(cursor_check_line_modified);
extern DO_CURSOR(cursor_clear_left);
extern DO_CURSOR(cursor_clear_line);
extern DO_CURSOR(cursor_clear_right);
extern DO_CURSOR(cursor_convert_meta);
extern DO_CURSOR(cursor_delete);
extern DO_CURSOR(cursor_delete_or_exit);
extern DO_CURSOR(cursor_delete_word_left);
extern DO_CURSOR(cursor_delete_word_right);
extern DO_CURSOR(cursor_echo);
extern DO_CURSOR(cursor_end);
extern DO_CURSOR(cursor_enter);
extern DO_CURSOR(cursor_enter_finish);
extern DO_CURSOR(cursor_flag);
extern DO_CURSOR(cursor_get);
extern DO_CURSOR(cursor_history_find);
extern DO_CURSOR(cursor_history_next);
extern DO_CURSOR(cursor_history_prev);
extern DO_CURSOR(cursor_history_search);
extern DO_CURSOR(cursor_home);
extern DO_CURSOR(cursor_info);
extern DO_CURSOR(cursor_insert);
extern DO_CURSOR(cursor_macro);
extern DO_CURSOR(cursor_move_page_down);
extern DO_CURSOR(cursor_move_down);
extern DO_CURSOR(cursor_move_left);
extern DO_CURSOR(cursor_move_left_word);
extern DO_CURSOR(cursor_move_right);
extern DO_CURSOR(cursor_move_right_word);
extern DO_CURSOR(cursor_move_page_up);
extern DO_CURSOR(cursor_move_up);
extern DO_CURSOR(cursor_page);
extern DO_CURSOR(cursor_paste_buffer);
extern DO_CURSOR(cursor_position);
extern DO_CURSOR(cursor_redraw_input);
extern DO_CURSOR(cursor_redraw_line);
extern DO_CURSOR(cursor_redraw_singleline);
extern DO_CURSOR(cursor_redraw_multiline);
extern DO_CURSOR(cursor_redraw_edit);
extern DO_CURSOR(cursor_set);
extern DO_CURSOR(cursor_soft_enter);
extern DO_CURSOR(cursor_suspend);
extern DO_CURSOR(cursor_tab);
extern DO_CURSOR(cursor_tab_backward);
extern DO_CURSOR(cursor_tab_forward);

#endif


#ifndef __INPUT_H__
#define __INPUT_H__

extern void  process_input(void);
extern void  read_line(char *input, int len);
extern void  read_key(char *input, int len);
extern int   check_key(char *input, int len);
extern void  convert_meta(char *input, char *output, int eol);
extern char *str_convert_meta(char *input, int eol);
extern void  echo_command(struct session *ses, char *line);
extern void init_input(struct session *ses, int top_row, int top_col, int bot_row, int bot_col);
extern void free_input(struct session *ses);
extern void  input_printf(char *format, ...);
extern void  modified_input(void);

#endif


#ifndef __MAPPER_H__
#define __MAPPER_H__

extern DO_COMMAND(do_map);

extern void delete_room_data(struct room_data *room);
extern  int follow_map(struct session *ses, char *argument);
extern void show_vtmap(struct session *ses);
extern void map_mouse_handler(struct session *ses, char *left, char *right, int row, int col, int rev_row, int rev_col, int height, int width);
extern  int delete_map(struct session *ses);

extern DO_MAP(map_at);
extern DO_MAP(map_center);
extern DO_MAP(map_color);
extern DO_MAP(map_create);
extern DO_MAP(map_debug);
extern DO_MAP(map_delete);
extern DO_MAP(map_destroy);
extern DO_MAP(map_dig);
extern DO_MAP(map_entrance);
extern DO_MAP(map_exit);
extern DO_MAP(map_exitflag);
extern DO_MAP(map_explore);
extern DO_MAP(map_find);
extern DO_MAP(map_flag);
extern DO_MAP(map_get);
extern DO_MAP(map_global);
extern DO_MAP(map_goto);
extern DO_MAP(map_info);
extern DO_MAP(map_insert);
extern DO_MAP(map_jump);
extern DO_MAP(map_landmark);
extern DO_MAP(map_leave);
extern DO_MAP(map_legend);
extern DO_MAP(map_link);
extern DO_MAP(map_list);
extern DO_MAP(map_map);
extern DO_MAP(map_move);
extern DO_MAP(map_name);
extern DO_MAP(map_offset);
extern DO_MAP(map_read);
extern DO_MAP(map_resize);
extern DO_MAP(map_return);
extern DO_MAP(map_roomflag);
extern DO_MAP(map_run);
extern DO_MAP(map_set);
extern DO_MAP(map_sync);
extern DO_MAP(map_terrain);
extern DO_MAP(map_travel);
extern DO_MAP(map_undo);
extern DO_MAP(map_uninsert);
extern DO_MAP(map_unlandmark);
extern DO_MAP(map_unlink);
extern DO_MAP(map_unterrain);
extern DO_MAP(map_update);
extern DO_MAP(map_vnum);
extern DO_MAP(map_write);

#endif


#ifndef __TT_MATH_H__
#define __TT_MATH_H__

extern DO_COMMAND(do_math);

extern int is_math(struct session *ses, char *str);
extern int get_ellipsis(struct listroot *root, char *name, int *min, int *max);
extern long double get_number(struct session *ses, char *str);
extern unsigned long long get_ulong(struct session *ses, char *str);
extern long double get_double(struct session *ses, char *str);
extern void get_number_string(struct session *ses, char *str, char *result);
extern long double mathswitch(struct session *ses, char *left, char *right);
extern void mathexp(struct session *ses, char *str, char *result, int seed);
extern int mathexp_tokenize(struct session *ses, char *str, int seed, int debug);
extern void mathexp_level(struct session *ses, struct link_data *node);
extern void mathexp_compute(struct session *ses, struct link_data *node);
extern long double tinternary(char *arg1, char *arg2);
extern long double tintoi(char *str);
extern unsigned long long tintou(char *str);
extern long double tincmp(char *left, char *right);
extern long double tineval(struct session *ses, char *left, char *right);
extern long double tindice(struct session *ses, char *left, char *right);


#endif


#ifndef __CONFIG_H__
#define __CONFIG_H__

extern DO_COMMAND(do_configure);

extern DO_CONFIG(config_autotab);
extern DO_CONFIG(config_buffersize);
extern DO_CONFIG(config_charset);
extern DO_CONFIG(config_colormode);
extern DO_CONFIG(config_colorpatch);
extern DO_CONFIG(config_commandcolor);
extern DO_CONFIG(config_commandecho);
extern DO_CONFIG(config_connectretry);
extern DO_CONFIG(config_childlock);
extern DO_CONFIG(config_convertmeta);
extern DO_CONFIG(config_debugtelnet);
extern DO_CONFIG(config_historysize);
extern DO_CONFIG(config_inheritance);
extern DO_CONFIG(config_loglevel);
extern DO_CONFIG(config_logmode);
extern DO_CONFIG(config_mccp);
extern DO_CONFIG(config_mousetracking);
extern DO_CONFIG(config_packetpatch);
extern DO_CONFIG(config_randomseed);
extern DO_CONFIG(config_repeatchar);
extern DO_CONFIG(config_repeatenter);
extern DO_CONFIG(config_screenreader);
extern DO_CONFIG(config_scrolllock);
extern DO_CONFIG(config_speedwalk);
extern DO_CONFIG(config_tabwidth);
extern DO_CONFIG(config_telnet);
extern DO_CONFIG(config_tintinchar);
extern DO_CONFIG(config_verbatim);
extern DO_CONFIG(config_verbatimchar);
extern DO_CONFIG(config_verbose);
extern DO_CONFIG(config_wordwrap);

#endif



#ifndef __DAEMON_H__
#define __DAEMON_H__

extern DO_COMMAND(do_daemon);

extern DO_DAEMON(daemon_attach);
extern DO_DAEMON(daemon_detach);
extern DO_DAEMON(daemon_input);
extern DO_DAEMON(daemon_kill);
extern DO_DAEMON(daemon_list);

extern void reset_daemon(void);
extern void winch_daemon(void);

#endif

#ifndef __DATA_H__
#define __DATA_H__

extern DO_COMMAND(do_kill);
extern DO_COMMAND(do_killall);
extern DO_COMMAND(do_message);
extern DO_COMMAND(do_ignore);
extern DO_COMMAND(do_debug);


extern void kill_list(struct listroot *root);
extern void free_list(struct listroot *root);
extern  int show_node_with_wild(struct session *ses, char *cptr, struct listroot *root);
extern void show_node(struct listroot *root, struct listnode *node, int level);
extern void show_nest(struct listnode *node, char *result);
extern void show_list(struct listroot *root, int level);
extern void remove_node_list(struct session *ses, int type, struct listnode *node);
extern void remove_index_list(struct listroot *root, int index);
extern void dispose_node(struct listnode *node);
extern void delete_node_list(struct session *ses, int type, struct listnode *node);
extern  int delete_node_with_wild(struct session *ses, int index, char *string);
extern void delete_index_list(struct listroot *root, int index);
extern  int search_index_list(struct listroot *root, char *text, char *priority);
extern  int locate_index_list(struct listroot *root, char *text, char *priority);
extern  int bsearch_alpha_list(struct listroot *root, char *text, int seek);
extern  int bsearch_alnum_list(struct listroot *root, char *text, int seek);
extern  int bsearch_priority_list(struct listroot *root, char *text, char *priority, int seek);
extern  int nsearch_list(struct listroot *root, char *text);
extern struct listroot *init_list(struct session *ses, int type, int size);
extern struct listroot *copy_list(struct session *ses, struct listroot *sourcelist, int type);
extern struct listnode *create_node_list(struct listroot *root, char *arg1, char *arg2, char *arg3, char *arg4);
extern struct listnode *insert_node_list(struct listroot *root, struct listnode *node);
extern struct listnode *insert_index_list(struct listroot *root, struct listnode *node, int index);
extern struct listnode *update_node_list(struct listroot *root, char *arg1, char *arg2, char *arg3, char *arg4);
extern struct listnode *search_node_list(struct listroot *root, char *text);


#endif


#ifndef __DEBUG_H__
#define __DEBUG_H__

extern void push_call(char *format, ...);
extern  int push_call_printf(char *format, ...);
extern void pop_call(void);
extern void dump_stack(void);

#endif

#ifndef __DICT_H__
#define __DICT_H__

extern DO_COMMAND(do_dictionary);

extern int spellcheck_count(struct session *ses, char *in);

#endif

#ifndef __EDIT_H__
#define __EDIT_H__

DO_COMMAND(do_edit);

DO_EDIT(edit_create);
DO_EDIT(edit_load);
DO_EDIT(edit_read);
DO_EDIT(edit_resume);
DO_EDIT(edit_save);
DO_EDIT(edit_suspend);
DO_EDIT(edit_write);

extern struct edit_data *create_editor(void);

extern void enable_editor(struct edit_data *edit);
extern void delete_editor(struct edit_data *edit);
extern void resize_editor(struct edit_data *edit, int size);
extern void clear_editor(struct edit_data *edit);
extern  int str_save_editor(struct edit_data *edit, char **str);
extern  int var_save_editor(struct edit_data *edit, char **str);
extern void create_line(struct edit_data *edit, int index, char *str);
extern void delete_line(struct edit_data *edit, int index);
extern void insert_line(struct edit_data *edit, int index, char *str);
extern void remove_line(struct edit_data *edit, int index);
#endif

#ifndef __DRAW_H__
#define __DRAW_H__

extern DO_COMMAND(do_draw);

extern void scale_drawing(struct session *ses, int *top_row, int *top_col, int *bot_row, int *bot_col, int *rows, int *cols, int index, long long flags, char *arg);

extern DO_DRAW(draw_blank);
extern DO_DRAW(draw_bot_side);
extern DO_DRAW(draw_arg);
extern DO_DRAW(draw_box);
extern DO_DRAW(draw_buffer);
extern DO_DRAW(draw_corner);
extern DO_DRAW(draw_horizontal_line);
extern DO_DRAW(draw_left_side);
extern DO_DRAW(draw_line);
extern DO_DRAW(draw_map);
extern DO_DRAW(draw_right_side);
extern DO_DRAW(draw_side);
extern DO_DRAW(draw_square);
extern DO_DRAW(draw_rain);
extern DO_DRAW(draw_table_grid);
extern DO_DRAW(draw_text);
extern DO_DRAW(draw_top_side);
//extern DO_DRAW(draw_vertical_line);
extern DO_DRAW(draw_vertical_lines);

#endif


#ifndef __EVENT_H__
#define __EVENT_H__

extern DO_COMMAND(do_event);
extern DO_COMMAND(do_unevent);

extern  int check_all_events(struct session *ses, int flags, int args, int vars, char *fmt, ...);
extern void mouse_handler(struct session *ses, int val1, int val2, int val3);

#endif


#ifndef __FILES_H__
#define __FILES_H__

extern DO_COMMAND(do_read);
extern DO_COMMAND(do_write);

extern struct session *read_file(struct session *ses, FILE *fp, char *filename);
extern void write_node(struct session *ses, int mode, struct listnode *node, FILE *file);
extern char *fread_one_line(char **str, FILE *fp);

#endif


#ifndef __HELP_H__
#define __HELP_H__

extern DO_COMMAND(do_help);

#endif


#ifndef __HISTORY_H__
#define __HISTORY_H__

extern DO_COMMAND(do_history);
extern void add_line_history(struct session *ses, char *line);
extern void insert_line_history(struct session *ses, char *line);
extern struct session *repeat_history(struct session *ses, char *line);
extern int write_history(struct session *ses, char *filename);
extern int read_history(struct session *ses, char *filename);

DO_HISTORY(history_character);
DO_HISTORY(history_delete);
DO_HISTORY(history_get);
DO_HISTORY(history_insert);
DO_HISTORY(history_list);
DO_HISTORY(history_size);
DO_HISTORY(history_read);
DO_HISTORY(history_write);

#endif


#ifndef __LINE_H__
#define __LINE_H__

extern DO_COMMAND(do_line);
extern DO_LINE(line_background);
extern DO_LINE(line_benchmark);
extern DO_LINE(line_capture);
extern DO_LINE(line_convert);
extern DO_LINE(line_debug);
extern DO_LINE(line_gag);
extern DO_LINE(line_ignore);
extern DO_LINE(line_local);
extern DO_LINE(line_log);
extern DO_LINE(line_logmode);
extern DO_LINE(line_logverbatim);
extern DO_LINE(line_msdp);
extern DO_LINE(line_multishot);
extern DO_LINE(line_oneshot);
extern DO_LINE(line_quiet);
extern DO_LINE(line_strip);
extern DO_LINE(line_substitute);
extern DO_LINE(line_verbatim);
extern DO_LINE(line_verbose);

#endif


#ifndef __LOG_H__
#define __LOG_H__

extern DO_COMMAND(do_log);

DO_LOG(log_append);
DO_LOG(log_info);
DO_LOG(log_overwrite);
DO_LOG(log_off);
DO_LOG(log_remove);

extern void loginit(struct session *ses, FILE *file, int newline);
extern void logit(struct session *ses, char *txt, FILE *file, int newline);
extern void write_html_header(struct session *ses, FILE *fp);
extern void vt102_to_html(struct session *ses, char *txt, char *out);

#endif


#ifndef __MAIN_H__
#define __MAIN_H__

extern struct session *gts;
extern struct tintin_data *gtd;
extern void winch_handler(int signal);
extern void abort_handler(int signal);
extern void pipe_handler(int signal);
extern void suspend_handler(int signal);
extern void trap_handler(int signal);
extern  int main(int argc, char **argv);
extern void init_tintin(int greeting);
extern void quitmsg(char *message);
extern void syserr_fatal(int signal, char *msg);
extern void syserr_printf(struct session *ses, char *fmt, ...);

#endif

#ifndef __MCCP_H__
#define __MCCP_H__

void *zlib_alloc(void *opaque, unsigned int items, unsigned int size);
void  zlib_free(void *opaque, void *address);

#endif

#ifndef __MEMORY_H__
#define __MEMORY_H__

extern char *restring(char *point, char *string);
extern char *restringf(char *point, char *fmt, ...);

extern void init_memory(void);

extern struct str_data *get_str_ptr(char *str);
extern char *get_str_str(struct str_data *str_ptr);

extern  int str_len(char *str);
extern  int str_max(char *str);
extern  int str_fix(char *str);

extern char *str_alloc(int len);
extern void  str_free(char *ptr);

extern char *str_mim(char *original);
extern char *str_dup(char *original);
extern char *str_dup_clone(char *original);
extern char *str_dup_printf(char *fmt, ...);
extern char *str_ndup(char *original, int len);

extern char *str_resize(char **ptr, int add);
extern void  str_clone(char **clone, char *original);
extern char *str_cpy(char **ptr, char *str);
extern char *str_cpy_printf(char **ptr, char *fmt, ...);
extern char *str_ncpy(char **ptr, char *str, int len);
extern char *str_cat_len(char **str, char *arg, int len);
extern char *str_cat(char **str, char *arg);
extern char *str_cat_chr(char **ptr, char chr);
extern char *str_cat_printf(char **str, char *fmt, ...);
extern char *str_cap(char **str, int index, char *buf);
extern char *str_ins(char **str, int index, char *buf);
extern char *str_ins_printf(char **str, int index, char *fmt, ...);
extern char *str_mov(char **str, int dst, int src);

extern char *str_alloc_stack(int size);

#endif


#ifndef __MISC_H__
#define __MISC_H__

extern DO_COMMAND(do_all);
extern DO_COMMAND(do_bell);

extern DO_COMMAND(do_cr);
extern DO_COMMAND(do_echo);
extern DO_COMMAND(do_end);
extern DO_COMMAND(do_forall);
extern DO_COMMAND(do_info);
extern DO_COMMAND(do_nop);
extern DO_COMMAND(do_send);
extern DO_COMMAND(do_snoop);
extern DO_COMMAND(do_suspend);
extern DO_COMMAND(do_test);
extern DO_COMMAND(do_zap);

#endif

#ifndef __MSDP_H__
#define __MSDP_H__

extern void init_msdp_table(void);
extern  int msdp_find(char *var);
extern void arachnos_devel(struct session *ses, char *fmt, ...);
extern void arachnos_mudlist(struct session *ses, char *fmt, ...);
extern void msdp_update_all(char *var, char *fmt, ...);
extern void msdp_update_var(struct session *ses, struct port_data *buddy, char *var, char *str);
extern void msdp_update_varf(struct session *ses, struct port_data *buddy, char *var, char *fmt, ...);
extern void msdp_update_var_instant(struct session *ses, struct port_data *buddy, char *var, char *fmt, ...);
extern void msdp_send_update(struct session *ses, struct port_data *buddy);
extern char *msdp_get_var(struct session *ses, struct port_data *buddy, char *var);
extern void process_msdp_varval(struct session *ses, struct port_data *buddy, char *var, char *val );
extern void msdp_command_list(struct session *ses, struct port_data *buddy, int index);
extern void msdp_command_report(struct session *ses, struct port_data *buddy, int index);
extern void msdp_command_reset(struct session *ses, struct port_data *buddy, int index);
extern void msdp_command_send(struct session *ses, struct port_data *buddy, int index);
extern void msdp_command_unreport(struct session *ses, struct port_data *buddy, int index);
extern void msdp_configure_arachnos(struct session *ses, struct port_data *buddy, int index);
extern void write_msdp_to_descriptor(struct session *ses, struct port_data *buddy, char *src, int length);
extern  int msdp2json(unsigned char *src, int srclen, char *out);
extern  int json2msdp(unsigned char *src, int srclen, char *out);
extern  int tintin2msdp(char *src, char *out);
extern void arachnos_devel(struct session *ses, char *fmt, ...);
extern void arachnos_mudlist(struct session *ses, char *fmt, ...);
extern struct msdp_type msdp_table[];

#endif

#ifndef __NEST_H__
#define __NEST_H__

extern struct listroot *search_nest_base_ses(struct session *ses, char *arg);
extern struct listroot *search_nest_root(struct listroot *root, char *arg);
extern struct listnode *search_base_node(struct listroot *root, char *variable);
extern struct listnode *search_nest_node(struct listroot *root, char *variable);
extern struct listnode *search_nest_node_ses(struct session *ses, char *variable);
extern int search_nest_index(struct listroot *root, char *variable);
extern struct listroot *update_nest_root(struct listroot *root, char *arg);
extern void update_nest_node(struct listroot *root, char *arg);
extern int delete_nest_node(struct listroot *root, char *variable);
extern int get_nest_size_key(struct listroot *root, char *variable, char **result);
extern int get_nest_size_val(struct listroot *root, char *variable, char **result);
extern struct listnode *get_nest_node_key(struct listroot *root, char *variable, char **result, int def);
extern struct listnode *get_nest_node_val(struct listroot *root, char *variable, char **result, int def);
extern int get_nest_index(struct listroot *root, char *variable, char **result, int def);
extern void show_nest_node(struct listnode *node, char **result, int initialize);
extern void view_nest_node(struct listnode *node, char **str_result, int nest, int initialize);
extern struct listnode *set_nest_node_ses(struct session *ses, char *arg1, char *format, ...);
extern struct listnode *add_nest_node_ses(struct session *ses, char *arg1, char *format, ...);
extern struct listnode *set_nest_node(struct listroot *root, char *arg1, char *format, ...);
extern struct listnode *add_nest_node(struct listroot *root, char *arg1, char *format, ...);
extern void copy_nest_node(struct listroot *dst_root, struct listnode *dst, struct listnode *src);

#endif


#ifndef __NET_H__
#define __NET_H__

extern int connect_mud(struct session *ses, char *host, char *port);
extern void write_line_mud(struct session *ses, char *line, int size);
extern int read_buffer_mud(struct session *ses);
extern void readmud(struct session *ses);
extern void process_mud_output(struct session *ses, char *linebuf, int prompt);

#endif

#ifndef __PARSE_H__
#define __PARSE_H__

extern  int is_abbrev(char *str1, char *str2);
extern  int is_member(char *str1, char *str2);
extern  int is_vowel(char *str);
extern void filename_string(char *input, char *output);
extern struct session *execute(struct session *ses, char *format, ...);
extern struct session *command(struct session *ses, COMMAND *cmd, char *format, ...);
extern struct session *parse_input(struct session *ses, char *input);
extern struct session *parse_command(struct session *ses, char *input);
extern  int is_speedwalk(struct session *ses, char *input);
extern char *substitute_speedwalk(struct session *ses, char *input, char *output);
extern void process_speedwalk(struct session *ses, char *input);
extern struct session *parse_tintin_command(struct session *ses, char *input);
extern int cnt_arg_all(struct session *ses, char *string, int flag);
extern char *get_arg_all(struct session *ses, char *string, char *result, int verbatim);
extern char *sub_arg_all(struct session *ses, char *string, char *result, int verbatim, int sub);
extern char *get_arg_in_braces(struct session *ses, char *string, char *result, int flag);
extern char *sub_arg_in_braces(struct session *ses, char *string, char *result, int flag, int sub);
extern char *get_arg_with_spaces(struct session *ses, char *string, char *result, int flag);
extern char *get_arg_stop_spaces(struct session *ses, char *string, char *result, int flag);
extern char *sub_arg_stop_spaces(struct session *ses, char *string, char *result, int flag, int sub);

extern char *get_arg_stop_digits(struct session *ses, char *string, char *result, int flag);
extern char *space_out(char *string);
extern char *get_arg_to_brackets(struct session *ses, char *string, char *result);
extern char *get_arg_at_brackets(struct session *ses, char *string, char *result);
extern char *get_arg_in_brackets(struct session *ses, char *string, char *result);
extern char *get_char(struct session *ses, char *string, char *result);
extern void write_mud(struct session *ses, char *command, int flags);
extern void do_one_line(char *line, struct session *ses);

#endif


#ifndef __PATH_H__
#define __PATH_H__

extern DO_COMMAND(do_path);
extern DO_COMMAND(do_pathdir);
extern DO_COMMAND(do_unpathdir);

int is_pathdir(struct session *ses, char *dir);
int exit_to_dir(struct session *ses, char *name);
unsigned char pdir(struct listnode *node);
char *dir_to_exit(struct session *ses, int dir);

extern void check_append_path(struct session *ses, char *forward, char *backward, float delay, int follow);

extern DO_PATH(path_create);
extern DO_PATH(path_describe);
extern DO_PATH(path_delete);
extern DO_PATH(path_destroy);
extern DO_PATH(path_get);
extern DO_PATH(path_goto);
extern DO_PATH(path_insert);
extern DO_PATH(path_load);
extern DO_PATH(path_map);
extern DO_PATH(path_move);
extern DO_PATH(path_run);
extern DO_PATH(path_save);
extern DO_PATH(path_start);
extern DO_PATH(path_stop);
extern DO_PATH(path_swap);
extern DO_PATH(path_undo);
extern DO_PATH(path_unzip);
extern DO_PATH(path_walk);
extern DO_PATH(path_zip);

// old

extern DO_PATH(path_new);
extern DO_PATH(path_end);

#endif


#ifndef __PORT_H__
#define __PORT_H__


extern DO_COMMAND(do_port);
extern DO_PORT(port_call);
extern DO_PORT(port_color);
extern DO_PORT(port_flag);
extern DO_PORT(port_group);
extern DO_PORT(port_ignore);
extern DO_PORT(port_initialize);
extern DO_PORT(port_info);
extern DO_PORT(port_message);
extern DO_PORT(port_name);
extern DO_PORT(port_prefix);
extern DO_PORT(port_proxy);
extern DO_PORT(port_rank);
extern DO_PORT(port_send);
extern DO_PORT(port_uninitialize);
extern DO_PORT(port_who);
extern DO_PORT(port_zap);

extern  int port_new(struct session *ses, int s);
extern void close_port(struct session *ses, struct port_data *buddy, int unlink);
extern void process_port_connections(struct session *ses, fd_set *read_set, fd_set *write_set, fd_set *exc_set);
extern void port_forward_session(struct session *ses, char *linelog);
extern void port_socket_printf(struct session *ses, struct port_data *buddy, char *format, ...);
extern void port_telnet_printf(struct session *ses, struct port_data *buddy, size_t length, char *format, ...);
extern void port_log_printf(struct session *ses, struct port_data *buddy, char *format, ...);
extern void port_printf(struct session *ses, char *format, ...);
extern  int process_port_input(struct session *ses, struct port_data *buddy);
extern void get_port_commands(struct session *ses, struct port_data *buddy, char *buf, int len);
extern void port_name_change(struct session *ses, struct port_data *buddy, char *txt);
extern void port_receive_message(struct session *ses, struct port_data *buddy, char *txt);

extern void port_puts(struct session *ses, char *arg);
extern struct port_data *find_port_buddy(struct session *ses, char *arg);
extern struct port_data *find_port_group(struct session *ses, char *arg);

#endif

#ifndef __SCAN_H__
#define __SCAN_H__

extern DO_COMMAND(do_scan);

DO_SCAN(scan_abort);
DO_SCAN(scan_csv);
DO_SCAN(scan_dir);
DO_SCAN(scan_file);
DO_SCAN(scan_forward);
DO_SCAN(scan_tsv);
DO_SCAN(scan_txt);

#endif

#ifndef __SCREEN_H__
#define __SCREEN_H__

extern DO_COMMAND(do_screen);

extern DO_SCREEN(screen_blur);
extern DO_SCREEN(screen_clear);
extern DO_SCREEN(screen_cursor);
extern DO_SCREEN(screen_dump);
extern DO_SCREEN(screen_fill);
extern DO_SCREEN(screen_focus);
extern DO_SCREEN(screen_fullscreen);
extern DO_SCREEN(screen_get);
extern DO_SCREEN(screen_info);
extern DO_SCREEN(screen_inputregion);
extern DO_SCREEN(screen_load);
extern DO_SCREEN(screen_maximize);
extern DO_SCREEN(screen_minimize);
extern DO_SCREEN(screen_move);
extern DO_SCREEN(screen_print);
extern DO_SCREEN(screen_raise);
extern DO_SCREEN(screen_refresh);
extern DO_SCREEN(screen_resize);
extern DO_SCREEN(screen_rescale);
extern DO_SCREEN(screen_save);
extern DO_SCREEN(screen_scrollbar);
extern DO_SCREEN(screen_scrollregion);
extern DO_SCREEN(screen_set);
extern DO_SCREEN(screen_swap);

extern void init_inputregion(struct session *ses, int top_row, int top_col, int bot_row, int bot_col);
extern  int get_row_index(struct session *ses, int val);
extern  int get_col_index(struct session *ses, int val);
extern  int get_row_index_arg(struct session *ses, char *arg);
extern  int get_col_index_arg(struct session *ses, char *arg);
extern void csip_handler(int var1, int var2, int var3);
extern void csit_handler(int var1, int var2, int var3);
extern void rqlp_handler(int event, int button, int row, int col);
extern void osc_handler(char ind, char *arg);
extern void erase_scroll_region(struct session *ses);
extern void erase_input_region(struct session *ses);
extern void erase_split_region(struct session *ses);
extern void erase_bot_region(struct session *ses);
extern void erase_top_region(struct session *ses);
extern void erase_left_region(struct session *ses);
extern void erase_right_region(struct session *ses);
extern void erase_square(struct session *ses, int top_row, int top_col, int bot_row, int bot_col);
extern void fill_top_region(struct session *ses, char *arg);
extern void fill_bot_region(struct session *ses, char *arg);
extern void fill_left_region(struct session *ses, char *arg);
extern void fill_right_region(struct session *ses, char *arg);
extern void fill_split_region(struct session *ses, char *arg);
extern int inside_scroll_region(struct session *ses, int row, int col);
extern void add_row_index(struct row_data **row, int index);
extern void del_row_index(struct row_data **row, int index);

extern void print_scroll_region(struct session *ses);
extern void print_screen();
extern void init_screen(int rows, int cols, int pix_rows, int pix_cols);
extern void destroy_screen();
extern int inside_scroll_region(struct session *ses, int row, int col);
extern void set_grid_screen(struct session *ses, char *str, int row, int col);
extern void add_line_screen(struct session *ses, char *str, int row);

extern void set_line_screen(struct session *ses, char *ins, int row, int col);

extern void get_line_screen(char *str, int row);
extern  int get_link_screen(struct session *ses, char *var, char *val, int flags, int row, int col);
extern void get_word_screen(char *str, int row, int col);

#endif


#ifndef __SESSION_H__
#define __SESSION_H__

extern DO_COMMAND(do_session);
extern struct session *session_command(char *arg, struct session *ses);
extern void show_session(struct session *ses, struct session *ptr);
extern struct session *find_session(char *name);
extern struct session *newactive_session(void);
extern struct session *activate_session(struct session *ses);
extern struct session *new_session(struct session *ses, char *name, char *address, int desc, int ssl);
extern struct session *connect_session(struct session *ses);
extern void cleanup_session(struct session *ses);
extern void dispose_session(struct session *ses);

#endif


#ifndef __SHOW_H__
#define __SHOW_H__

extern DO_COMMAND(do_showme);

extern void show_message(struct session *ses, int index, char *format, ...);
extern void show_error(struct session *ses, int index, char *format, ...);
extern void show_debug(struct session *ses, int index, char *format, ...);
extern void show_info(struct session *ses, int index, char *format, ...);
extern void print_lines(struct session *ses, int flags, char *format, ...);
extern void show_lines(struct session *ses, char *str);
extern void tintin_header(struct session *ses, int width, char *format, ...);
extern void socket_printf(struct session *ses, size_t length, char *format, ...);
extern void telnet_printf(struct session *ses, int length, char *format, ...);

extern void tintin_printf2(struct session *ses, char *format, ...);
extern void tintin_printf(struct session *ses, char *format, ...);

extern void tintin_puts3(struct session *ses, char *string);
extern void tintin_puts2(struct session *ses, char *string);
extern void tintin_puts(struct session *ses, char *string);

#endif


#ifndef __SORT_H__
#define __SORT_H__

extern void quadsort(void *array, size_t nmemb, size_t size, CMPFUNC *cmp);
extern int cmp_int(const void * a, const void * b);
extern int cmp_str(const void * a, const void * b);
extern int cmp_num(const void * a, const void * b);

#endif

#ifndef __SPLIT_H__
#define __SPLIT_H__

extern DO_COMMAND(do_split);
extern DO_COMMAND(do_unsplit);

extern void init_split(struct session *ses, int top, int bot, int left, int right);
extern void reset_screen(struct session *ses);
extern void dirty_screen(struct session *ses);
extern void split_show(struct session *ses, char *prompt, char *row, char *col);

#endif


#ifndef __SSL_H__
#define __SSL_H__

extern DO_COMMAND(do_ssl);

extern gnutls_session_t ssl_negotiate(struct session *ses);

#endif

#ifndef __STRING_H__
#define __STRING_H__

extern int get_raw_len_str_range_str_width(struct session *ses, char *str, int start, int end, int *raw_width);
extern int get_raw_off_str_range_raw_width(struct session *ses, char *str, int start, int end, int *raw_width);

extern char *str_ins_str(struct session *ses, char **str, char *ins, int str_start, int str_end);

extern char *calign(struct session *ses, char *in, char *out, int width);
extern char *lalign(struct session *ses, char *in, char *out, int width);
extern char *ralign(struct session *ses, char *in, char *out, int width);
extern char *ualign(struct session *ses, char *in, char *out, int width);

extern char char_cmp(char left, char right);

extern char is_alnum(char input);
extern char is_alpha(char input);
extern char is_digit(char input);
extern char is_hex(char input);
extern char is_print(char input);
extern char is_space(char input);
extern char is_varchar(char input);
extern char is_csichar(char input);

#endif

#ifndef __SUBSTITUTE_H__
#define __SUBSTITUTE_H__

extern  int valid_escape(struct session *ses, char *arg);
extern char *fuzzy_color_code(struct session *ses, char *pti);
extern char *dim_color_code(struct session *ses, char *pti, int mod);
extern char *lit_color_code(struct session *ses, char *pti, int mod);
extern int is_color_code(char *str);
extern int is_color_name(char *str);
extern int substitute_color(char *input, char *output, int colors);

#endif

#ifndef __SYSTEM_H__
#define __SYSTEM_H__


extern DO_COMMAND(do_run);

extern DO_COMMAND(do_script);
extern DO_COMMAND(do_system);
extern DO_COMMAND(do_textin);

#endif


#ifndef __TABLES_H__
#define __TABLES_H__

extern struct array_type array_table[];
extern struct buffer_type buffer_table[];
extern struct chat_type chat_table[];
extern   char character_table[];
extern struct class_type class_table[];
extern struct color_type color_table[];
extern struct color_type map_color_table[];
//extern struct command_type command_table[];
extern struct config_type config_table[];
extern struct cursor_type cursor_table[];
extern struct daemon_type daemon_table[];
extern struct draw_type draw_table[];
extern struct edit_type edit_table[];
extern struct event_type event_table[];
extern struct history_type history_table[];
extern struct line_type line_table[];
extern struct list_type list_table[LIST_MAX];
extern struct log_type log_table[];
extern struct map_type map_table[];
extern struct path_type path_table[];
extern struct port_type port_table[];
extern struct rank_type rank_table[];
extern struct scan_type scan_table[];
extern struct stamp_type huge_stamp_table[];
extern struct substitution_type substitution_table[];
extern struct telopt_type telopt_table[];
extern   char *telcmds[];
extern struct timer_type timer_table[];
extern struct screen_type screen_table[];
extern struct map_legend_type map_legend_table[];
extern struct map_legend_group_type map_legend_group_table[];

#endif


#ifndef __TELOPT_H__
#define __TELOPT_H__

extern  int client_translate_telopts(struct session *ses, unsigned char *src, int cplen);
extern  int client_write_compressed(struct session *ses, char *txt, int length);
extern  int client_send_sb_naws(struct session *ses, int cplen, unsigned char *cpsrc);
extern void announce_support(struct session *ses, struct port_data *buddy);
extern  int server_translate_telopts(struct session *ses, struct port_data *buddy, unsigned char *src, int srclen, unsigned char *out, int outlen);
extern void write_mccp2(struct session *ses, struct port_data *buddy, char *txt, int length);
extern void client_end_mccp2(struct session *ses);
extern void end_mccp2(struct session *ses, struct port_data *buddy);
extern void client_end_mccp3(struct session *ses);
extern void end_mccp3(struct session *ses, struct port_data *buddy);
extern void init_msdp_table(void);

#endif


#ifndef __TERMINAL_H__
#define __TERMINAL_H__

extern void  init_terminal(struct session *ses);
extern void  reset_terminal(struct session *ses);
extern void  save_session_terminal(struct session *ses);
extern void  refresh_session_terminal(struct session *ses);
extern void  echo_on(struct session *ses);
extern void  echo_off(struct session *ses);
extern void  init_terminal_size(struct session *ses);
extern  int  get_scroll_rows(struct session *ses);
extern  int  get_scroll_cols(struct session *ses);
extern char *get_charset(struct session *ses);

#endif


#ifndef __TEXT_H__
#define __TEXT_H__

extern void print_line(struct session *ses, char **str, int isaprompt);
extern void print_stdout(int row, int col, char *format, ...);
extern  int word_wrap(struct session *ses, char *textin, char *textout, int display, int *height, int *width);
extern  int word_wrap_split(struct session *ses, char *textin, char *textout, int wrap, int start, int end, int flags, int *height, int *width);

#endif




#ifndef __TINEXP_H__
#define __TINEXP_H__

DO_COMMAND(do_regexp);

extern int substitute(struct session *ses, char *string, char *result, int flags);
extern int match(struct session *ses, char *str, char *exp, int flags);
extern int find(struct session *ses, char *str, char *exp, int sub, int flag);
extern int regexp_compare(struct session *ses, pcre *regex, char *str, char *exp, int option, int flag);
extern int check_one_regexp(struct session *ses, struct listnode *node, char *line, char *original, int option);
extern int tintin_regexp_check(struct session *ses, char *exp);
extern int tintin_regexp(struct session *ses, pcre *pcre, char *str, char *exp, int option, int flag);
extern pcre *regexp_compile(struct session *ses, char *exp, int option);
extern pcre *tintin_regexp_compile(struct session *ses, struct listnode *node, char *exp, int option);
extern void  tintin_macro_compile(char *input, char *output);

#endif


#ifndef __TOKENIZE_H__
#define __TOKENIZE_H__

extern void init_local(struct session *ses);
extern struct scriptroot *push_script_stack(struct session *ses, int list);
extern void pop_script_stack();
extern struct listroot *local_list(struct session *ses);
extern struct session *script_driver(struct session *ses, int list, char *str);
extern char *script_writer(struct session *ses, char *str);
extern char *script_viewer(struct session *ses, char *str);
#endif


#ifndef __TRIGGER_H__
#define __TRIGGER_H__


DO_COMMAND(do_action);
DO_COMMAND(do_unaction);
extern void check_all_actions(struct session *ses, char *original, char *line, char *buf);

DO_COMMAND(do_alias);
DO_COMMAND(do_unalias);
extern int check_all_aliases(struct session *ses, char *input);

DO_COMMAND(do_button);
DO_COMMAND(do_unbutton);
extern void check_all_buttons(struct session *ses, short row, short col, char *arg1, char *arg2, char *word, char *line);

extern DO_COMMAND(do_delay);
extern DO_COMMAND(do_undelay);

extern DO_COMMAND(do_function);
extern DO_COMMAND(do_unfunction);

extern DO_COMMAND(do_gag);
extern DO_COMMAND(do_ungag);
extern void check_all_gags(struct session *ses, char *original, char *line);

extern DO_COMMAND(do_highlight);
extern DO_COMMAND(do_unhighlight);
extern void check_all_highlights(struct session *ses, char *original, char *line);

extern DO_COMMAND(do_macro);
extern DO_COMMAND(do_unmacro);

extern DO_COMMAND(do_prompt);
extern DO_COMMAND(do_unprompt);
extern int check_all_prompts(struct session *ses, char *original, char *line);

extern DO_COMMAND(do_substitute);
extern DO_COMMAND(do_unsubstitute);
extern void check_all_substitutions(struct session *ses, char *original, char *line);

extern DO_COMMAND(do_tab);
extern DO_COMMAND(do_untab);

extern DO_COMMAND(do_tick);
extern DO_COMMAND(do_untick);

#endif

// update.c

extern void mainloop(void);
extern void show_cpu(struct session *ses);


#ifndef __UTILS_H__
#define __UTILS_H__



extern int is_number(char *str);
extern unsigned long long hex_number_64bit(char *str);
extern unsigned int hex_number_32bit(char *str);
extern int hex_number_8bit(char *str);
extern int oct_number(char *str);
extern int unicode_16_bit(char *str, char *out);
extern int unicode_21_bit(char *str, char *out);
extern unsigned long long utime(void);
extern time_t get_time(struct session *ses, char *str);
extern char *str_time(struct session *ses, char *format, time_t time);
extern unsigned long long generate_rand(struct session *ses);
extern void seed_rand(struct session *ses, unsigned long long seed);
extern char *capitalize(char *str);
extern char *ftos(float number);
extern char *ntos(long long number);
extern char *indent_one(int len);
extern char *indent(int len);
extern int cat_sprintf(char *dest, char *fmt, ...);
extern void ins_sprintf(char *dest, char *fmt, ...);
extern char *str_ins_printf(char **str, int index, char *fmt, ...);
extern int is_suffix(char *str1, char *str2);



#endif


#ifndef __UTF8_H__
#define __UTF8_H__

extern int get_ascii_width(char *str, int *width);

extern int is_utf8_head(char *str);
extern int is_utf8_tail(char *str);
extern int get_utf8_size(char *str);
extern int get_utf8_width(char *str, int *width);
extern int get_utf8_index(char *str, int *index);
extern int unicode_to_utf8(int index, char *out);
extern int utf8_strlen(char *str, int *width);

extern int utf8_to_all(struct session *ses, char *in, char *out);
extern int all_to_utf8(struct session *ses, char *in, char *out);
extern int iso1_to_utf8(char *input, char *output);
extern int utf8_to_iso1(char *input, char *output);
extern int iso2_to_utf8(char *input, char *output);
extern int utf8_to_iso2(char *input, char *output);
extern int koi8_to_utf8(char *input, char *output);
extern int utf8_to_koi8(char *input, char *output);
extern int fansi_to_utf8(char *input, char *output);
extern int is_euc_head(struct session *ses, char *str);
extern int get_euc_size(struct session *ses, char *str);
extern int get_euc_width(struct session *ses, char *str, int *width);
extern int is_big5(char *str);
extern int big5_to_utf8(char *input, char *output);
extern int utf8_to_big5(char *input, char *output);
extern int is_gbk1(char *str);
extern int gbk1_to_utf8(char *input, char *output);
extern int utf8_to_gbk1(char *input, char *output);

extern int cp1251_to_utf8(char *input, char *output);
extern int utf8_to_cp1251(char *input, char *output);

#endif

#ifndef __VARIABLE_H__
#define __VARIABLE_H__

extern DO_COMMAND(do_variable);
extern DO_COMMAND(do_unvariable);
extern DO_COMMAND(do_local);
extern DO_COMMAND(do_cat);
extern DO_COMMAND(do_format);
extern DO_COMMAND(do_replace);


extern  int valid_variable(struct session *ses, char *arg);
extern  int string_raw_str_len(struct session *ses, char *str, int start, int end);
extern  int string_str_raw_len(struct session *ses, char *str, int start, int end);
extern  int translate_color_names(struct session *ses, char *string, char *result);
extern  int get_color_names(struct session *ses, char *htype, char *result);
extern void lowerstring(char *str);
extern void upperstring(char *str);
extern void numbertocharacter(struct session *ses, char *str);
extern void charactertonumber(struct session *ses, char *str);
extern  int delete_variable(struct session *ses, char *variable);
extern void justify_string(struct session *ses, char *in, char *out, int align, int cut);
extern void format_string(struct session *ses, char *format, char *arg, char *out);
extern struct listnode *search_variable(struct session *ses, char *variable);
extern struct listnode *get_variable(struct session *ses, char *variable, char *result);
extern struct listnode *set_variable(struct session *ses, char *variable, char *format, ...);


#endif


#ifndef __VT102_H__
#define __VT102_H__

extern void init_pos(struct session *ses, int row, int col);
extern void hide_cursor(struct session *ses);
extern void show_cursor(struct session *ses);
extern void save_pos(struct session *ses);
extern void goto_pos(struct session *ses, int row, int col);
extern void restore_pos(struct session *ses);
extern void erase_cols(int cnt);
extern void erase_scroll_region(struct session *ses);
extern void reset(struct session *ses);
extern void scroll_region(struct session *ses, int top, int bottom);
extern void reset_scroll_region(struct session *ses);
extern int find_color_code(char *str);
extern int find_escaped_color_code(char *str);
extern int find_secure_color_code(char *str);
extern int get_vt102_width(struct session *ses, char *str, int *width);
extern int strip_vt102_width(struct session *ses, char *str, int *width);
extern int skip_vt102_codes(char *str);
extern int skip_vt102_codes_non_graph(char *str);
extern int strip_vt102_codes(char *str, char *buf);
extern void strip_vt102_codes_non_graph(char *str, char *buf);
extern void strip_non_vt102_codes(char *str, char *buf);
extern void get_color_codes(char *old, char *str, char *buf, int flags);
extern int strip_vt102_strlen(struct session *ses, char *str);
extern int strip_color_strlen(struct session *ses, char *str);
extern char *strip_vt102_strstr(char *str, char *buf, int *len);
extern int interpret_vt102_codes(struct session *ses, char *str, int real);
extern int catch_vt102_codes(struct session *ses, unsigned char *str, int cplen);

#endif
