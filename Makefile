PROG1 = spotifyconnect
OBJS1 = src/main.o

PROGS = $(PROG1)

PKGS = axparameter gio-2.0 glib-2.0

CFLAGS += $(shell pkg-config --cflags $(PKGS))
LDLIBS += $(shell pkg-config --libs $(PKGS)) -lpthread

all: $(PROGS)

$(PROG1): $(OBJS1)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm -f $(PROGS) src/*.o
