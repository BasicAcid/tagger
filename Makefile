build:
	gcc -O2 -Wall -Wextra -Wformat=2 -Wformat-overflow -Wformat-truncation -Wshadow -Wdouble-promotion -Wundef -fno-common -z noexecstack -Wconversion -o bin/tagger main.c
deploy:
	gcc -O2 -Wall -Wextra -Wformat=2 -Wformat-overflow -Wformat-truncation -Wshadow -Wdouble-promotion -Wundef -fno-common -z noexecstack -Wconversion -o ~/.local/bin/tagger main.c
run:
	./bin/main
