APP_NAME:= GBmu.app

EXECUTABLE:= build/gbmu

APP_FILES:= \
	Info.plist \
	MacOS/gbmu \
	Resources/gbmu.icns

APP_FILES:= $(addprefix GBmu.app/Contents/,$(APP_FILES))

APP_DIRS:= GBmu.app/Contents/MacOS GBmu.app/Contents/Resources

CXX_SOURCES:= \
	code/sources/cartridge_loader.c \
	code/sources/check_events.c \
	code/sources/cpu_specs.c \
	code/sources/disassembler.c \
	code/sources/draw_line.c \
	code/sources/file_read.c \
	code/sources/gameboy_core.c \
	code/sources/graphics.c \
	code/sources/launcher.c \
	code/sources/lcd_driver.c \
	code/sources/lcd_function.c \
	code/sources/main.c \
	code/sources/mbc_load.c \
	code/sources/mbc_swap.c \
	code/sources/memory_map.c \
	code/sources/processor.c \
	code/sources/settings.c \
	code/sources/timing.c \
	code/sources/update_display.c \
	code/sources/write.c \

CXX_OBJECTS:= $(patsubst code/sources/%.c,build/objects/%.o,$(CXX_SOURCES))
CXX_DEPENDS:= $(patsubst code/sources/%.c,build/objects/%.d,$(CXX_SOURCES))

COMPILE:= gcc
CXX_FLAGS:= -Wall -Wextra -Werror -O2 -c -MMD -I code/include

LIB_NAME:= /Users/mhouppin/.brew/lib/libSDL2.dylib

all: $(APP_NAME)

$(APP_NAME): $(APP_DIRS) $(APP_FILES)

$(APP_DIRS):
	mkdir -p $@

GBmu.app/Contents/MacOS/gbmu: $(EXECUTABLE)
	cp $< $@

$(EXECUTABLE): build $(CXX_OBJECTS)
	$(COMPILE) -o $@ $(CXX_OBJECTS) $(LIB_NAME)

build:
	mkdir -p $@

GBmu.app/Contents/Info.plist: misc/Darwin/Info.plist
	cp $< $@

GBmu.app/Contents/Resources/gbmu.icns: misc/Darwin/gbmu.icns
	cp $< $@

build/objects/%.o: code/sources/%.c build/objects
	$(COMPILE) $(CXX_FLAGS) $(shell sdl2-config --cflags) -o $@ $<

build/objects:
	mkdir -p $@

-include $(DEPENDS)

clean:
	rm -rf build

fclean: clean
	rm -rf $(APP_NAME)

re: fclean all

.PHONY: all clean fclean re
