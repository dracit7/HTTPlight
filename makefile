
BIN = server
SRC = lib/socket.c lib/http.c lib/logger.c lib/threads.c

RELEASE = main.c
TEST = test.c

run: server
	./server

test: $(TEST) $(SRC) always
	@gcc -o test -pthread $(SRC) $(TEST)
	@./test
	@rm test
	
server: $(RELEASE) $(SRC)
	gcc `pkg-config --cflags gtk+-3.0` -pthread -o $(BIN) $(SRC) $(RELEASE) `pkg-config --libs gtk+-3.0`

always: