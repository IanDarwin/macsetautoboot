#	$OpenBSD$

PROG=	macsetautoboot

MAN=    macsetautoboot.8

install:	macsetautoboot
			install $? /usr/local/sbin

.include <bsd.prog.mk>
