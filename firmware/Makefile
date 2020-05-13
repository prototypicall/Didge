include makefile.conf

NAME=lather

STARTUP_DEFS=

STARTUP=startup/startup_stm32f103.S

LDSCRIPTS=-L. -Lldscripts -T gcc.ld
#LFLAGS=$(USE_NANO) $(USE_SEMIHOST) $(LDSCRIPTS) $(GC) $(MAP)
LFLAGS=$(USE_NANO) $(USE_NOHOST) $(LDSCRIPTS) $(GC) $(MAP)

HPPFILES=$(wildcard *.hpp)
CPPFILES=retarget.cpp devices.cpp boot.cpp main.cpp

INCLUDES=-I../../Kvasir/Lib -I/c/source/lib/boost/include/boost-1_72

BOOST_FLAGS=-DBOOST_NO_EXCEPTIONS -DBOOST_EXCEPTION_DISABLE -DBOOST_NO_IOSTREAM

CXXFLAGS+=$(INCLUDES) -std=c++17 -fno-builtin -ggdb $(BOOST_FLAGS)

$(NAME).axf: $(STARTUP) $(CFILES) $(CPPFILES) $(HPPFILES)
	$(CXX) $(STARTUP) $(CFILES) $(CPPFILES) $(CXXFLAGS) $(LFLAGS) -o $@

bin: $(NAME).axf
	$(OBJCP) -O binary $(NAME).axf $(NAME).bin
	
clean: 
	rm -f $(NAME).axf *.map
