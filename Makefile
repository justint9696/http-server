.PHONY: all clean tests runtime server

all: server runtime tests

tests:
	@make -f test/Makefile --no-print-directory all

server:
	@make -f http-server/Makefile --no-print-directory all

runtime:
	@make -f runtime/Makefile --no-print-directory all

run: server
	@bin/server --port 8000 --module public --verbose

clean:
	@make -f http-server/Makefile --no-print-directory clean
	@make -f runtime/Makefile --no-print-directory clean
	@make -f test/Makefile --no-print-directory clean
