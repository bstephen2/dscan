# dscan
# (c) 2019, Brian Stephenson
# brian@bstephen.me.uk
#
BITS		=	64
EXE			=	dscan.exe
CC			=	gcc
LD			=	gcc
RELEASE		=	-O3 -DCELLO_NGC -DNDEBUG
DEBUG		=	-g -O0 -DCELLO_NGC
CFLAGS		=	${RELEASE} -c -mtune=native -m${BITS} -Wall -Wno-unused-label -std=gnu99
LDFLAGS		=	-o${EXE} -L.
IND			=	astyle
INDOPTS		=	--style=kr --align-pointer=type --indent=tab=3 --indent=spaces \
				--pad-oper --unpad-paren --break-blocks --pad-header
CHDS		=	dscan.h
CMODS		=	main.c work.c screen.c db.c
COBJS		=	main.o work.o screen.o db.o
CELMODS		=	Alloc.c Array.c Assign.c Cmp.c Concat.c Doc.c Exception.c File.c Function.c GC.c Get.c Hash.c Iter.c \
				Len.c List.c Num.c Pointer.c Push.c Resize.c Show.c Start.c String.c Table.c Thread.c Tree.c Tuple.c Type.c
CELOBJS		=	Alloc.o Array.o Assign.o Cmp.o Concat.o Doc.o Exception.o File.o Function.o GC.o Get.o Hash.o Iter.o \
				Len.o List.o Num.o Pointer.o Push.o Resize.o Show.o Start.o String.o Table.o Thread.o Tree.o Tuple.o Type.o
CLIBS		=	-lCello${BITS} -lncursesw -lmariadb -lDbgHelp

dscan:	${COBJS}
	${LD}   ${LDFLAGS} ${COBJS} ${CLIBS}

main.o:	main.c ${CHDS}
	${CC} ${CFLAGS} main.c
	objconv -fnasm main.o
	
work.o:	work.c ${CHDS}
	${CC} ${CFLAGS} work.c
	objconv -fnasm work.o
	
screen.o:	screen.c ${CHDS}
	${CC} ${CFLAGS} screen.c
	objconv -fnasm screen.o
	
db.o:	db.c ${CHDS}
	${CC} ${CFLAGS} db.c
	objconv -fnasm db.o
	
cello: ${CELMODS}
	${CC} ${CFLAGS} ${CELMODS}
	objconv -fnasm Alloc.o
	objconv -fnasm Array.o
	objconv -fnasm Assign.o
	objconv -fnasm Cmp.o
	objconv -fnasm Concat.o
	objconv -fnasm Doc.o
	objconv -fnasm Exception.o
	objconv -fnasm File.o
	objconv -fnasm Function.o
	objconv -fnasm GC.o
	objconv -fnasm Get.o
	objconv -fnasm Hash.o
	objconv -fnasm Iter.o
	objconv -fnasm Len.o
	objconv -fnasm List.o
	objconv -fnasm Num.o
	objconv -fnasm Pointer.o
	objconv -fnasm Push.o
	objconv -fnasm Resize.o
	objconv -fnasm Show.o
	objconv -fnasm Start.o
	objconv -fnasm String.o
	objconv -fnasm Table.o
	objconv -fnasm Thread.o
	objconv -fnasm Tree.o
	objconv -fnasm Tuple.o
	objconv -fnasm Type.o
	ar r libCello${BITS}.a ${CELOBJS}
	ranlib libCello${BITS}.a
	
clean:
	del ${COBJS} ${EXE}.exe  *orig

tidy:
	${IND} ${INDOPTS} ${CMODS} ${CHDS}

count:
	wc -l ${CMODS} ${CHDS} | sort -b -n
touch:
	touch -m ${CMODS} ${CHDS}
	
