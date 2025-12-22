# src/Makefile.am - self-contained build

bin_PROGRAMS = tt++

tt___SOURCES = \
    advertise.c banner.c base.c buffer.c chat.c class.c command.c config.c \
    cursor.c daemon.c data.c debug.c dict.c draw.c edit.c event.c files.c \
    forkpty.c help.c history.c input.c line.c list.c log.c main.c mapper.c \
    math.c mccp.c memory.c misc.c msdp.c nest.c net.c parse.c path.c port.c \
    regex.c scan.c screen.c session.c show.c sort.c split.c ssl.c string.c \
    substitute.c system.c tables.c telopt_client.c telopt_server.c terminal.c \
    text.c tokenize.c trigger.c update.c utf8.c utils.c variable.c vt102.c

noinst_HEADERS = dict.h telnet.h tintin.h

CLEANFILES = tt++

clean-local:
	rm -f *.o *.lo *.slo

tt___LDADD = -lm -lgnutls -lpcre2-8 -lutil -lpthread -lz

AM_CFLAGS = -g -Wall -D_GNU_SOURCE -DHAVE_CONFIG_H

EXTRA_DIST = install-sh missing
