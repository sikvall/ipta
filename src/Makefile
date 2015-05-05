######################################################################
# Makefile
#
# Anders "Ichimusai" Sikvall
# anders@sikvall.se
#
# This source file is part of the ipta package and is maintained by
# the package owner, see http://ichimusai.org/ipta/ for more info
# about this. Any changes, patches, diffs etc that you would like to
# offer should be sent by email to ichi@ichimusai.org for review
# before they will be applied to the main code base.
#
# As usual this software is offered "as is" and placed in the public
# domain. You are free to copy, modify, spread and make use of this
# software. For the terms and conditions for this software you should
# refer to the "LICENCE" file in the source directory or run a
# compiled binary with the "--licence" option which will display the
# licence.
# 
# Any modifications to this source MUST retain this header. You are
# however allowed to add below your own changes and redistribute, as
# long as you do not violate any terms and condition in the LICENCE.
######################################################################


# Variables here

objects = main.o import-syslog.o analyze.o gethostbyaddr.o	\
	  print_licence.o db_maintenance.o follow.o
target = ipta
headers = ipta.h
includes = /usr/include/mysql/
libs = /usr/local/mysql/lib/
link = mysqlclient

cflags = -O3 -Wall

# Actual targets here, main first, then all supporting objects please.

all: ipta

dns_cache: dns_cache.o
	gcc ${cflags} dns_cache.o -o dns_cache -l ${link}

ipta: ${objects}
	gcc ${cflags} ${objects} -o ${target} -l ${link}

main.o: main.c ipta.h
	gcc ${cflags} -c main.c -I ${includes}

dns_cache.o: dns_cache.c ipta.h
	gcc ${cflags} -c dns_cache.c -I ${includes}

import-syslog.o: import-syslog.c ipta.h
	gcc ${cflags} -c import-syslog.c -L ${libs} -I ${includes}	

analyze.o: analyze.c ipta.h
	gcc ${cflags} -c analyze.c -L ${libs} -I ${includes}

gethostbyaddr.o: ipta.h gethostbyaddr.c
	gcc ${cflags} -c gethostbyaddr.c -L ${libs} -I ${includes}

print_licence.o: print_licence.c
	gcc ${cflags} -c print_licence.c  -L ${libs} -I ${includes}

db_maintenance.o: db_maintenance.c ipta.h
	gcc ${cflags} -c db_maintenance.c -L ${libs} -I ${includes}

follow.o: follow.c ipta.h
	gcc ${cflags} -c follow.c -L ${libs} -I ${includes}



# Special targets here

clean:
	rm -rf *.o
	rm -rf *~
	rm ipta

checkout:
	co -l *.c *.h Makefile LICENSE

checkin:
	ci -u *.c *.h Makefile LICENSE

install:
	make strip
	sudo install ipta -m 755 -o root -g root /usr/bin/

distclean:
	make clean

strip:
	strip ipta
