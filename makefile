.PHONY: all clean tests runtime server

all: server runtime tests

tests:
	@make -f test/makefile --no-print-directory all

server:
	@make -f http-server/makefile --no-print-directory all

runtime:
	@make -f src/makefile --no-print-directory all

run: server
	@echo "Starting web sever on port 8000"
	@bin/server --port 8000 --module public --verbose

clean:
	@make -f http-server/makefile --no-print-directory clean
	@make -f src/makefile --no-print-directory clean
	@make -f test/makefile --no-print-directory clean
