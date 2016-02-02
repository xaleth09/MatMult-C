PROGRAMS = matmult_p multiply matformatter matmult_t

.PHONY: all clean

all: $(PROGRAMS)

%: %.c
	gcc -Wall -O2 $^ -o $@

clean:
	rm -rf *~ $(PROGRAMS)
