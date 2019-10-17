
BIN = server
SRC = lib/socket.c lib/http.c lib/logger.c

RELEASE = main.c
TEST = test.c

run: server
	./server

test: $(TEST) $(SRC) always
	@gcc `pkg-config --cflags gtk+-3.0` -o test $(SRC) $(TEST) `pkg-config --libs gtk+-3.0`
	@./test
	@rm test
	
server: $(RELEASE) $(SRC)
	gcc `pkg-config --cflags gtk+-3.0` -o $(BIN) $(SRC) $(RELEASE) `pkg-config --libs gtk+-3.0`

always: