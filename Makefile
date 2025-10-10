CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lm
SRCDIR = src
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/shapes.c $(SRCDIR)/game.c \
		  $(SRCDIR)/render.c $(SRCDIR)/input.c $(SRCDIR)/splash.c
OBJECTS = $(SOURCES:.c=.o)
	TARGET = main

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRCDIR)/*.o $(TARGET)
