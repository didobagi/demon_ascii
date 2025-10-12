CC = gcc
CFLAGS = -Wall -g -Iinclude
LDFLAGS = -lm
SRCDIR = src
BUILDDIR = build
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

#test_templates: build/test_templates.o build/map_template.o
#	$(CC) build/test_templates.o build/map_template.o -o test_templates

build/test_templates.o: src/test_templates.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean
