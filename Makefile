CC = cc
CFLAGS = -O2
PROG ?= producer_consumer
OBJ = $(PROG).o
SRC = $(PROG).c


.PHONY: greet clean build rebuild run 

greet:
	@echo "After running, press Ctrl+C to terminate the process"
	@echo "Terminating make - please specify target explicitly"
	@echo "build: fast build"
	@echo "rebuild: full rebuild"
	@echo "run after fast build"
	@echo "clean: perform full clean"
	
	
build: $(PROG)

rebuild: clean $(PROG)

run: build
	./$(PROG)


$(PROG): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^


$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $< -o $@


clean: 
	rm -f $(PROG) $(OBJ)

