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
*   You should have received a copy of the GNU General Public License         *
*   along with TinTin++.  If not, see https://www.gnu.org/licenses.           *
******************************************************************************/

/******************************************************************************
*                               T I N T I N + +                               *
*                                                                             *
*                      coded by Igor van den Hoven 2004                       *
******************************************************************************/

#include "tintin.h"

/*
	telnet protocol.
*/

#define     IAC     255
#define     DONT    254
#define     DO      253
#define     WONT    252
#define     WILL    251
#define     SB      250
#define     GA      249   /* Used for prompt marking */
#define     EL      248
#define     EC      247
#define     AYT     246
#define     AO      245
#define     IP      244
#define     BREAK   243
#define     DM      242
#define     NOP     241
#define     SE      240
#define     EOR     239   /* Used for prompt marking */
#define     ABORT   238
#define     SUSP    237
#define     xEOF    236

/*
	telnet options
*/

#define     TELOPT_BINARY         0
#define     TELOPT_ECHO           1  /* Echo */
#define     TELOPT_RCP            2
#define     TELOPT_SGA            3  /* Supress Go Ahead */
#define     TELOPT_NAMS           4
#define     TELOPT_STATUS         5
#define     TELOPT_TIMINGMARK     6
#define     TELOPT_RCTE           7
#define     TELOPT_NAOL           8
#define     TELOPT_NAOP           9
#define     TELOPT_NAOCRD        10
#define     TELOPT_NAOHTS        11
#define     TELOPT_NAOHTD        12
#define     TELOPT_NAOFFD        13
#define     TELOPT_NAOVTS        14
#define     TELOPT_NAOVTD        15
#define     TELOPT_NAOLFD        16
#define     TELOPT_XASCII        17
#define     TELOPT_LOGOUT        18
#define     TELOPT_BM            19
#define     TELOPT_DET           20
#define     TELOPT_SUPDUP        21
#define     TELOPT_SUPDUPOUTPUT  22
#define     TELOPT_SNDLOC        23
#define     TELOPT_TTYPE         24  /* Terminal Type */
#define     TELOPT_EOR           25  /* End of Record */
#define     TELOPT_TUID          26
#define     TELOPT_OUTMRK        27
#define     TELOPT_TTYLOC        28
#define     TELOPT_3270REGIME    29
#define     TELOPT_X3PAD         30
#define     TELOPT_NAWS          31  /* Negotiate About Window Size */
#define     TELOPT_TSPEED        32
#define     TELOPT_LFLOW         33
#define     TELOPT_LINEMODE      34
#define     TELOPT_XDISPLOC      35
#define     TELOPT_OLD_ENVIRON   36
#define     TELOPT_AUTH          37
#define     TELOPT_ENCRYPT       38
#define     TELOPT_NEW_ENVIRON   39
#define     TELOPT_CHARSET       42  /* Charset */
#define     TELOPT_STARTTLS      46
#define     TELOPT_MSDP          69  /* Mud Server Data Protocol */
#define     TELOPT_MSSP          70  /* Mud Server Status Protocol */
#define     TELOPT_MCCP1         85
#define     TELOPT_MCCP2         86  /* Mud Client Compression Protocol v2 */
#define     TELOPT_MCCP3         87  /* Mud Client Compression Protocol v3 */
#define     TELOPT_MSP           90  /* Mud Sound Protocol */
#define     TELOPT_MXP           91  /* Mud eXtention Protocol */
#define     TELOPT_ZMP           93  /* Zenith Mud Protocol */
#define     TELOPT_GMCP         201  /* Generic Mud Communication Protocol */
#define     TELOPT_EXOPL        255

#define     TELCMD_OK(c)     ((c) >= xEOF)
#define     TELCMD(c)        telcmds[(c)-xEOF]
#define     TELOPT(c)       (telopt_table[(unsigned char) (c)].name)

/*
        Sub negotiation
*/

#define	    ENV_IS                0
#define	    ENV_SEND              1
#define     ENV_INFO              2

#define     ENV_VAR               0
#define     ENV_VAL               1
#define     ENV_ESC               2 /* Not implemented in tintin */
#define     ENV_USR               3

#define     CHARSET_REQUEST       1
#define     CHARSET_ACCEPTED      2
#define     CHARSET_REJECTED      3
/*
 TTABLE-IS ..................04
 TTABLE-REJECTED ............05
 TTABLE-ACK .................06
 TTABLE-NAK .................07
*/

#define     MSSP_VAR              1
#define     MSSP_VAL              2

#define     MSDP_VAR              1
#define     MSDP_VAL              2
#define     MSDP_TABLE_OPEN       3
#define     MSDP_TABLE_CLOSE      4
#define     MSDP_ARRAY_OPEN       5
#define     MSDP_ARRAY_CLOSE      6

#define     ANNOUNCE_NONE         0
#define     ANNOUNCE_WILL         1
#define     ANNOUNCE_DO           2
