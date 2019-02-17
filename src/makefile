IDIR=../include
CC=gcc
CFLAGS=-I $(IDIR) -g

ODIR=../obj
LDIR=../lib

LIBS=-lm

_DEPS = myfuncs.h stringparser.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o myfuncs.o stringparser.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

TARGET=../bin/wic6774shell

$(TARGET): $(OBJ)
		$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
		rm -f $(TARGET) $(ODIR)/*.o *~ core $(INCDIR)/*~
