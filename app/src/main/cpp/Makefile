export CC_USE_MATH = 1
export VKK_USE_UI  = 1
export VKK_USE_VG  = 1

TARGET  = glyph
CLASSES = glyph_engine glyph_object
SOURCE  = $(TARGET).c $(CLASSES:%=%.c)
OBJECTS = $(TARGET).o $(CLASSES:%=%.o)
HFILES  = $(CLASSES:%=%.h)
OPT     = -O2 -Wall -Wno-format-truncation
CFLAGS  = \
	$(OPT) -I.               \
	`sdl2-config --cflags`   \
	-I$(VULKAN_SDK)/include
LDFLAGS = \
	-Llibvkk -lvkk                  \
	-Llibbfs -lbfs                  \
	-Llibsqlite3 -lsqlite3          \
	-Llibcc -lcc                    \
	-Llibtess2/Source -ltess2       \
	-Llibxmlstream -lxmlstream      \
	-Llibexpat/expat/lib -lexpat    \
	-Ljsmn/wrapper -ljsmn           \
	-Ltexgz -ltexgz                 \
	-L$(VULKAN_SDK)/lib -lvulkan    \
	-L/usr/lib `sdl2-config --libs` \
	-ldl -lpthread -ljpeg -lz -lm
CCC = gcc

all: $(TARGET)

$(TARGET): $(OBJECTS) libcc libexpat libtess2 libvkk libbfs libsqlite3 libxmlstream jsmn texgz
	$(CCC) $(OPT) $(OBJECTS) -o $@ $(LDFLAGS)

.PHONY: libcc libexpat libtess2 libvkk libbfs libsqlite3 libxmlstream jsmn texgz

libcc:
	$(MAKE) -C libcc

libexpat:
	$(MAKE) -C libexpat/expat/lib

libtess2:
	$(MAKE) -C libtess2/Source

libvkk:
	$(MAKE) -C libvkk

libbfs:
	$(MAKE) -C libbfs

libsqlite3:
	$(MAKE) -C libsqlite3

libxmlstream:
	$(MAKE) -C libxmlstream

jsmn:
	$(MAKE) -C jsmn/wrapper

texgz:
	$(MAKE) -C texgz

clean:
	rm -f $(OBJECTS) *~ \#*\# $(TARGET)
	$(MAKE) -C libvkk clean
	$(MAKE) -C libcc clean
	$(MAKE) -C libexpat/expat/lib clean
	$(MAKE) -C libtess2/Source clean
	$(MAKE) -C libbfs clean
	$(MAKE) -C libsqlite3 clean
	$(MAKE) -C libxmlstream clean
	$(MAKE) -C jsmn/wrapper clean
	$(MAKE) -C texgz clean

$(OBJECTS): $(HFILES)
