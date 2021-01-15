CC=gcc

all: wordgen wordsearch pager launcher

wordgen: wordgen.c
	$(CC) -std=gnu99  $^ -o $@
wordsearch: wordsearch.c
	$(CC) -std=gnu99  $^ -o $@
pager: pager.c
	$(CC) -std=gnu99 $^ -o $@
launcher: launcher.c
	$(CC) -std=gnu99 $^ -o $@

remove:
	@echo "removing..."
	@rm launcher pager wordgen wordsearch
	@echo "done!"
