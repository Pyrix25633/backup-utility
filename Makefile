.RECIPEPREFIX = >
CC = g++
NAME = backup-utility
VERSION = 0.0.1

default:
> clear
> make build
> make run

win:
> clear
> make build-win
> make run-win

build:
> $(CC) src/main.cpp -std=c++17 -o bin/$(NAME)-$(VERSION)

run:
> ./bin/$(NAME)-$(VERSION) -s test/src -d test/dst -r test/rmv -t 10 --unix

build-win:
> $(CC) src/main.cpp -std=c++2a -m64 -lstdc++fs -o bin/$(NAME)-$(VERSION).exe

run-win:
> ./bin/$(NAME)-$(VERSION).exe -s test\src -d test\dst -r test\rmv -t 10