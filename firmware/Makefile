include makefile.conf

NAME=Didge

STARTUP_DEFS=

STARTUP=startup/startup_stm32f103.S

LDSCRIPTS=-L. -Lldscripts -T gcc.ld
LFLAGS=$(USE_NANO) $(USE_NOHOST) $(LDSCRIPTS) $(GC) $(MAP)

HPPFILES=$(wildcard *.hpp)
CPPFILES=retarget.cpp devices.cpp boot.cpp main.cpp

# Update paths below to reflect your file system
INCLUDES=-Ipath/to/Kvasir/Lib -I/path/to/boost/include/boost-1_xx

# Following Boost library definitions are not required if you use my
# modified rational implementation.
BOOST_FLAGS=-DBOOST_NO_EXCEPTIONS -DBOOST_EXCEPTION_DISABLE -DBOOST_NO_IOSTREAM

CXXFLAGS+=$(INCLUDES) -std=c++17 -fno-builtin -ggdb $(BOOST_FLAGS)

# Currently everything is compiled in one fell swoop
$(NAME).axf: $(STARTUP) $(CFILES) $(CPPFILES) $(HPPFILES)
	$(CXX) $(STARTUP) $(CFILES) $(CPPFILES) $(CXXFLAGS) $(LFLAGS) -o $@

bin: $(NAME).axf
	$(OBJCP) -O binary $(NAME).axf $(NAME).bin
	
clean: 
	rm -f $(NAME).axf *.map
