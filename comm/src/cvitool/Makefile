#makefile:

#################################
CC := g++
AR := ar
CFLAGS	= -Wall -g -DLINUX32 -D_FILE_OFFSET_BITS=64 -Werror -DNDEBUG
CFLAGS  += -D__MY_BEGIN_DECLS__= -D__MY_END_DECLS__=




LIBS    = -lpthread
INCS    = -I../util

##test: sources := $(shell echo *.cpp)  or sources := $(shell ls *.cpp)
outputdir := ../../bin/cvitool/
output := $(outputdir)cvitool.e
sources :=  $(wildcard *.cpp *.c)
objects := $(subst .c,.o,$(subst .cpp,.o,$(sources)))
depends := $(subst .c,.d,$(subst .cpp,.d,$(sources)))
archives := ../util/util.a

################################
all: $(output)

%.o:%.cpp
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(INCS) -o $@ $<

$(output): $(objects) $(archives)
	-mkdir $(outputdir)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LIBS) -o $@ $^ $(STLIBS)
	
../util/util.a:
	(cd ../util;make)
	pwd	


#################################
#depend:
-include $(depends)
%.d: %.cpp
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$;                   \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;   \
	rm -f $@.$$$$
	
#################################
.PHONY : print clean move
print:
	echo $(sources)
	echo $(objects)
	echo $(depends)
	echo $(CFLAGS)
	echo $(CPPFLAGS)
	(cd ../util; make print)
clean:
	-rm *.d
	-rm *.o
	-rm $(output)
	(cd ../util; make clean)
##################################

