CC = g++
FLAGS  = -Wall -g -MMD 
LIBS   = -lpthread


SRCS    = $(wildcard *.cpp)
BINS    = test

all: $(BINS)


$(BINS): $(SRCS:.cpp=.o)
	$(CC)  $(FLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CC) $(FLAGS) $(DDEFINE) $(INCLUDE) -c -o $@ $<
	@-mv -f $*.d .dep.$@

clean:
	@-rm -f *.o *.oxx *.po *.so *.d .dep.* $(BINS)

-include /dev/null $(wildcard .dep.*)
