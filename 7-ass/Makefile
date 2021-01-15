CC=gcc

all: hello write_hello

hello: hello.S
	$(CC) -no-pie -o hello hello.S

write_hello: write_hello.S
	as --64 write_hello.S -o a.o
	ld -m elf_x86_64 a.o
