
# make all
all: 
	gcc -o bin/server main.c http.c routing.c server.c responses.c -Iincludes -lpthread -Wall -Wextra -Werror -Wpedantic