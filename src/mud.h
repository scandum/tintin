#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <zlib.h>
#include <stdarg.h>


/*
	Utility macros.
*/

#define HAS_BIT(var, bit)       ((var)  & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define DEL_BIT(var, bit)       ((var) &= (~(bit)))
#define TOG_BIT(var, bit)       ((var) ^= (bit))

/*
	Update these to use whatever your MUD uses
*/

#define RESTRING(point, value) \
{ \
	STRFREE(point); \
	point = strdup(value); \
}

#define STRALLOC(point) \
{ \
	point = strdup(value); \
}

#define STRFREE(point) \
{ \
	free(point); \
	point = NULL; \
} 

/*
	Typedefs
*/

typedef struct mud_data           MUD_DATA;
typedef struct descriptor_data    DESCRIPTOR_DATA;

#define MUD_PORT                           4321
#define MAX_SKILL                           269
#define MAX_CLASS                             8
#define MAX_RACE                             16
#define MAX_LEVEL                            99

#define MAX_INPUT_LENGTH                   2000
#define MAX_STRING_LENGTH                 10000 // Must be at least 4 times larger than max input length.
#define COMPRESS_BUF_SIZE                 10000

#define FALSE                                 0
#define TRUE                                  1

#define BV00            (0   <<  0)
#define BV01            (1   <<  0)
#define BV02            (1   <<  1)
#define BV03            (1   <<  2)
#define BV04            (1   <<  3)
#define BV05            (1   <<  4)
#define BV06            (1   <<  5)
#define BV07            (1   <<  6)
#define BV08            (1   <<  7)
#define BV09            (1   <<  8)
#define BV10            (1   <<  9)

#define COMM_FLAG_DISCONNECT    BV01
#define COMM_FLAG_PASSWORD      BV02
#define COMM_FLAG_REMOTEECHO    BV03
#define COMM_FLAG_EOR           BV04
#define COMM_FLAG_MSDPUPDATE    BV05
#define COMM_FLAG_256COLORS     BV06
#define COMM_FLAG_UTF8          BV07
#define COMM_FLAG_GMCP          BV08

#define MSDP_FLAG_COMMAND       BV01
#define MSDP_FLAG_LIST          BV02
#define MSDP_FLAG_SENDABLE      BV03
#define MSDP_FLAG_REPORTABLE    BV04
#define MSDP_FLAG_CONFIGURABLE  BV05
#define MSDP_FLAG_REPORTED      BV06
#define MSDP_FLAG_UPDATED       BV07

// As per the MTTS standard

#define MTTS_FLAG_ANSI          BV01
#define MTTS_FLAG_VT100         BV02
#define MTTS_FLAG_UTF8          BV03
#define MTTS_FLAG_256COLORS     BV04
#define MTTS_FLAG_MOUSETRACKING BV05
#define MTTS_FLAG_COLORPALETTE  BV06
#define MTTS_FLAG_SCREENREADER  BV07
#define MTTS_FLAG_PROXY         BV08
#define MTTS_FLAG_TRUECOLOR     BV09

/*
	Mud data, structure containing global variables.
*/

struct mud_data
{
	DESCRIPTOR_DATA     * client;
	int                   server;
	int                   boot_time;
	int                   port;
	int                   total_plr;
	int                   top_area;
	int                   top_help;
	int                   top_mob_index;
	int                   top_obj_index;
	int                   top_room;
	int                   msdp_table_size;
	int                   mccp_len;
	unsigned char       * mccp_buf;
};

/*
	Descriptor (channel) partial structure.
*/

struct descriptor_data
{
	DESCRIPTOR_DATA   * next; 
	void              * character;
	char              * host;
	short               descriptor;
	struct msdp_data ** msdp_data;
	char              * proxy;
	char              * terminal_type;
	char                telbuf[MAX_INPUT_LENGTH];
	int                 teltop;
	long long           mtts;
	int                 comm_flags;
	short               cols;
	short               rows;
	z_stream          * mccp2;
	z_stream          * mccp3;
};

MUD_DATA *mud;

/*
	mud.c
*/

void log_printf(char *fmt, ...);
void log_descriptor_printf(DESCRIPTOR_DATA *d, char *fmt, ...);

int write_to_descriptor(DESCRIPTOR_DATA *d, char *txt, int length);

char *capitalize_all(char *str);

/*
	telopt.c
*/

int         translate_telopts        ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen, unsigned char *out, int outlen );
void        debug_telopts            ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );

int         process_do_eor           ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );
int         process_will_ttype       ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );
int         process_sb_ttype_is      ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );
int         process_sb_naws          ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );
int         process_will_new_environ ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );
int         process_sb_new_environ   ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );
int         process_do_charset       ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );
int         process_sb_charset       ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );
int         process_do_mssp          ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );
int         process_do_msdp          ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );
int         process_sb_msdp          ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );
int         process_do_gmcp          ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );
int         process_sb_gmcp          ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );

int         process_do_mccp2         ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );
int         process_dont_mccp2       ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );

int         skip_sb                  ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );
void        announce_support         ( DESCRIPTOR_DATA *d );

void        descriptor_printf        ( DESCRIPTOR_DATA *d, char *fmt, ...);


int         start_mccp2              ( DESCRIPTOR_DATA *d );
void        end_mccp2                ( DESCRIPTOR_DATA *d );
void        process_mccp2            ( DESCRIPTOR_DATA *d );
void        write_mccp2              ( DESCRIPTOR_DATA *d, char *txt, int length );

int         process_do_mccp3         ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );
int         process_sb_mccp3         ( DESCRIPTOR_DATA *d, unsigned char *src, int srclen );
void        end_mccp3                ( DESCRIPTOR_DATA *d );

void        send_echo_on             ( DESCRIPTOR_DATA *d );
void        send_echo_off            ( DESCRIPTOR_DATA *d );
void        send_ga                  ( DESCRIPTOR_DATA *d );
void        send_eor                 ( DESCRIPTOR_DATA *d );

/*
	utils.c
*/

int         cat_sprintf              ( char *dest, char *fmt, ... );
void        arachnos_devel           ( char *fmt, ... );
void        arachnos_mudlist         ( char *fmt, ... );

/*
	net.c
*/

int create_port(int port);
void mainloop(void);
void poll_port(void);
void process_port_connections(fd_set *read_set, fd_set *write_set, fd_set *exc_set);
void close_port_connection(void);
int process_port_input(void);
int port_new(int s);

/*
	client.c
*/

int         recv_sb_mssp             ( unsigned char *src, int srclen );

/*
	msdp.c
*/
void        init_msdp_table               ( void );
void        process_msdp_varval           ( DESCRIPTOR_DATA *d, char *var, char *val );
void        msdp_send_update              ( DESCRIPTOR_DATA *d );
void        msdp_update_var               ( DESCRIPTOR_DATA *d, char *var, char *fmt, ... );
void        msdp_update_var_instant       ( DESCRIPTOR_DATA *d, char *var, char *fmt, ... );

void        msdp_configure_arachnos       ( DESCRIPTOR_DATA *d, int index );
void        msdp_configure_pluginid       ( DESCRIPTOR_DATA *d, int index );

void        write_msdp_to_descriptor      ( DESCRIPTOR_DATA *d, char *src, int length );
int         msdp2json                     ( unsigned char *src, int srclen, char *out );
int         json2msdp                     ( unsigned char *src, int srclen, char *out );

/*
	tables.c
*/

#define ANNOUNCE_WILL   BV01
#define ANNOUNCE_DO     BV02

extern char *telcmds[];

struct telnet_type
{
	char      *name;
	int       flags;
};

extern struct telnet_type telnet_table[];

typedef void MSDP_FUN (struct descriptor_data *d, int index);

struct msdp_type
{
	char     *name;
	int       flags;
	MSDP_FUN *fun;
};

struct msdp_data
{
	char     *value;
	int      flags;
};

extern struct msdp_type msdp_table[];

