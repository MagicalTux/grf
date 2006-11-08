#!make
OBJECTS=main.o zlib.o hash_tables.o grf.o euc_kr.o
GB_OBJECTS=$(patsubst grfbuilder/%.cpp,%.o,$(wildcard grfbuilder/*.cpp)) moc_qt_win.o
TARGET=libgrf.so
TARGET_WIN=grf.dll
GB_TARGET=grfbuilder.bin
GB_TARGET_WIN=grfbuilder.exe
GB_REQ_DLL=mingwm10.dll QtCore4.dll QtGui4.dll
BUILD=unknown
LDFLAGS=--shared
LDFLAGS_TEST=
INCLUDES=-Iincludes -Izlib
ifndef DEBUG
DEBUG=yes
endif
ifeq ($(DEBUG),no)
CFLAGS=-pipe -O3 -Wall --std=gnu99
CXXFLAGS=-pipe -O3 -Wall
else
CFLAGS=-pipe -g -ggdb -O0 -Wall --std=gnu99 -D__DEBUG
CXXFLAGS=-pipe -g -ggdb -O0 -Wall -D__DEBUG
endif

ZOBJS = adler32.o compress.o crc32.o gzio.o uncompr.o deflate.o trees.o \
        zutil.o inflate.o infback.o inftrees.o inffast.o

UNAME=$(shell uname -s  | sed -e 's/_.*$$//')
ifeq ($(UNAME),Linux)
# *****
# *** Linux config
# *****
# TODO: Put back gcc32/g++32
CC=gcc
CXX=g++
STRIP=strip
# /opt/xmingw/ for old gentoo, i586-mingw32msvc-gcc for debian
# and mingw32-gcc for crossdev gentoo
ifeq ($(shell which i586-mingw32msvc-gcc 2>/dev/null),)
ifeq ($(shell which mingw32-gcc 2>/dev/null),)
CC_WIN=/opt/xmingw/bin/i386-mingw32msvc-gcc
CXX_WIN=/opt/xmingw/bin/i386-mingw32msvc-g++
STRIP_WIN=/opt/xmingw/bin/i386-mingw32msvc-strip
else
CC_WIN=mingw32-gcc
CXX_WIN=mingw32-g++
STRIP_WIN=mingw32-strip
endif
else
CC_WIN=i586-mingw32msvc-gcc
CXX_WIN=i586-mingw32msvc-g++
STRIP_WIN=i586-mingw32msvc-strip
endif
BUILD=Linux
WINFLAGS=-D__WIN32 -mwindows
LINFLAGS=-fPIC -DPIC

else
ifeq ($(UNAME),CYGWIN)
# *****
# *** Cygwin config
# *****
CC=gcc
CC_WIN=gcc
STRIP=strip
STRIP_WIN=strip
BUILD=Cygwin
WINFLAGS=-mno-cygwin -mwindows -D__WIN32
LINFLAGS=
endif
endif
QT_WIN=../qt/4.2.1
QT_WIN_LIBS=-L$(QT_WIN)/lib -lQtGui4 -lQtCore4
QT_WIN_INCLUDE=-I$(QT_WIN)/include -I$(QT_WIN)/include/QtCore -I$(QT_WIN)/include/QtGui -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB
#g++ -c -pipe -O2 -Wall -W -D_REENTRANT  -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED -I/usr/share/qt4/mkspecs/linux-g++ -I. -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtGui -I/usr/include/qt4 -I. -I. -I. -o main.o main.cpp
#g++  -o grfbuilder main.o    -L/usr/lib64/qt4 -lQtGui -L/usr/lib64 -L/usr/lib64/qt4 -lpng -lSM -lICE -lXi -lXrender -lXrandr -lXcursor -lXinerama -lfreetype -lfontconfig -lXext -lX11 -lQtCore -lz -lm -ldl -lpthread
QT_LIN_LIBS=$(shell pkg-config --libs QtGui QtCore)
QT_LIN_LIBS+=-lpthread
QT_LIN_INCLUDE=-I/usr/share/qt4/mkspecs/linux-g++ $(shell pkg-config --cflags QtGui QtCore)
QT_LIN_INCLUDE+=-D_REENTRANT  -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB
GCC_VERSION=$(shell $(CC) -dumpversion | awk -F. '{ print $$1 }')
ifeq ($(GCC_VERSION),4)
CFLAGS+=-Wno-attributes
endif
GCC_WIN_VERSION=$(shell $(CC_WIN) -dumpversion | awk -F. '{ print $$1 }')
ifeq ($(GCC_WIN_VERSION),4)
WINFLAGS+=-Wno-attributes
endif

win32/%.o: %.c
	@echo -en "  CC\t$<           \015"
	@$(CC_WIN) $(CFLAGS) $(WINFLAGS) $(INCLUDES) -c -o $@ $<

linux/%.o: %.c
	@echo -en "  CC\t$<           \015"
	@$(CC) $(CFLAGS) $(LINFLAGS) $(INCLUDES) -c -o $@ $<

win32/%.o: zlib/%.c
	@echo -en "  CC\t$<           \015"
	@$(CC_WIN) $(CFLAGS) $(WINFLAGS) $(INCLUDES) -c -o $@ $<

linux/%.o: zlib/%.c
	@echo -en "  CC\t$<           \015"
	@$(CC) $(CFLAGS) $(LINFLAGS) $(INCLUDES) -c -o $@ $<

win32/gb_%.o: grfbuilder/%.cpp
	@echo -en " CXX\t$<           \015"
	@$(CXX_WIN) $(CXXFLAGS) -Igrfbuilder $(WINFLAGS) $(QT_WIN_INCLUDE) $(INCLUDES) -c -o $@ $<

linux/gb_%.o: grfbuilder/%.cpp
	@echo -en " CXX\t$<           \015"
	@$(CXX) $(CXXFLAGS) -Igrfbuilder $(LINFLAGS) $(QT_LIN_INCLUDE) $(INCLUDES) -c -o $@ $<

.PHONY: make_dirs test dist gb

ifeq ($(BUILD),unknown)
all: ;@echo "Unknown system $(UNAME) !"
else
all: make_dirs $(TARGET) grf_test_linux $(TARGET_WIN) grf_test_win.exe $(GB_TARGET_WIN)
endif

make_dirs:
	@mkdir win32 linux 2>/dev/null || true

$(TARGET_WIN): $(patsubst %.o,win32/%.o,$(ZOBJS) $(OBJECTS))
	@echo -e "  LD\t$@              "
	@$(CC_WIN) $(CFLAGS) $(WINFLAGS) $(LDFLAGS) -o $@ $^
ifeq ($(DEBUG),no)
	@echo -e " STRIP\t$@"
	@$(STRIP_WIN) $@
endif

$(TARGET): $(patsubst %.o,linux/%.o,$(ZOBJS) $(OBJECTS))
	@echo -e "  LD\t$@              "
	@$(CC) $(CFLAGS) $(LINFLAGS) $(LDFLAGS) -o $@ $^
ifeq ($(DEBUG),no)
	@echo -e " STRIP\t$@"
	@$(STRIP) $@
endif

$(GB_TARGET): $(patsubst %.o,linux/gb_%.o,$(GB_OBJECTS))
	@echo -e "  LD\t$@              "
	@$(CXX) $(CXXFLAGS) $(LINFLAGS) $(QT_LIN_INCLUDE) -o $@ $^ $(QT_LIN_LIBS) -L. -lgrf
ifeq ($(DEBUG),no)
	@echo -e " STRIP\t$@"
	@$(STRIP) $@
endif

$(GB_TARGET_WIN): $(patsubst %.o,win32/gb_%.o,$(GB_OBJECTS))
	@echo -e "  LD\t$@              "
	@$(CXX_WIN) $(CXXFLAGS) $(WINFLAGS) $(QT_WIN_INCLUDE) -o $@ $^ $(QT_WIN_LIBS) -L. -lgrf
ifeq ($(DEBUG),no)
	@echo -e " STRIP\t$@"
	@$(STRIP_WIN) $@
endif

version.sh: includes/grf.h
	cat $< | grep "define VERSION" | grep -E "MAJOR|MINOR|REVISION" | sed -e 's/^#define //;s/ /=/' >$@

libgrf-%.zip: $(TARGET) $(TARGET_WIN) includes/libgrf.h $(wildcard examples/*) README QtCore4.dll QtGui4.dll mingwm10.dll grfbuilder.exe
	$(RM) $@
	zip -9r $@ $^ -x .svn '*.o'

dist: make_dirs version.sh
	. version.sh; make -C . libgrf-$$VERSION_MAJOR.$$VERSION_MINOR.$$VERSION_REVISION.zip DEBUG=no

grf_test_win.exe: win32/test.o $(TARGET_WIN)
	@echo -e "  LD\t$@              "
	@$(CC_WIN) $(CFLAGS) $(WINFLAGS) $(LDFLAGS_TEST) -o $@ $< -L. -lgrf

grf_test_linux: linux/test.o $(TARGET)
	@echo -e "  LD\t$@              "
	@$(CC) $(CFLAGS) $(LINFLAGS) $(LDFLAGS_TEST) -o $@ $< -L. -lgrf

ifeq ($(UNAME),Linux)
test: make_dirs grf_test_linux
	@LD_LIBRARY_PATH="." ./grf_test_linux

leak: make_dirs grf_test_linux
	@LD_LIBRARY_PATH="." valgrind --show-reachable=yes --leak-check=full ./grf_test_linux

gdb: make_dirs grf_test_linux
	@LD_LIBRARY_PATH="." gdb ./grf_test_linux

gb: make_dirs libgrf.so grfbuilder.bin
	@LD_LIBRARY_PATH="." ./grfbuilder.bin

else
ifeq ($(UNAME),CYGWIN)
test: make_dirs grf_test_win.exe
	./grf_test_win.exe
else
test: ;@echo "No test available for your platform ($(UNAME))."
endif
endif

## SPECIFIC RULES
grfbuilder/moc_qt_win.cpp: grfbuilder/qt_win.h
	@moc $(QT_LIN_INCLUDE) $< -o $@
grfbuilder/main.cpp: grfbuilder/qt_win.h grfbuilder/ui_qt_win.h 
grfbuilder/qt_win.cpp: grfbuilder/qt_win.h grfbuilder/ui_qt_win.h
grfbuilder/qt_win.h: grfbuilder/ui_qt_win.h
grfbuilder/ui_qt_win.h: grfbuilder/qt_win.ui
	@uic $< | sed -f grfbuilder/qt_win.sed >$@
	@lrelease grfbuilder/grfbuilder_fr.ts
grfbuilder/grfbuilder_fr.ts: $(wildcard grfbuilder/*.cpp grfbuilder/*.h)
	@lupdate $^ -ts $@

clean:
	$(RM) -r linux $(TARGET) win32 $(TARGET_WIN) $(GB_TARGET) $(GB_TARGET_WIN) grf_test_win.exe grf_test_linux libgrf-*.zip version.sh
	$(RM) grfbuilder/ui_qt_win.h grfbuilder/moc_qt_win.cpp grfbuilder/grfbuilder_fr.qm

