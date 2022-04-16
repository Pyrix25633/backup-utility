.RECIPEPREFIX = >
CC = g++
NAME = backup-utility
VERSION = 0.1.0

default:
> clear
> make build
> make run

win:
> clear
> make build-win
> make run-win

release:
> clear
> make build-release
> make run-release

win-release:
> clear
> make build-win-release
> make run-win-release

#debug
build:
> $(CC) src/main.cpp -std=c++2a -m64 -lstdc++fs -o bin/debug/$(NAME)-$(VERSION)

run:
> ./bin/debug/$(NAME)-$(VERSION) -s test/src -d test/dst -r test/rmv -e extensions.txt -t 10 --unix

build-win:
> $(CC) src/main.cpp -std=c++2a -m64 -lstdc++fs -o bin/debug/$(NAME)-$(VERSION).exe

run-win:
> ./bin/debug/$(NAME)-$(VERSION).exe -s test\src -d test\dst -r test\rmv -e extensions.txt -t 10

#release
build-release:
> $(CC) src/main.cpp -std=c++2a -m64 -O3 -lstdc++fs -o bin/release/$(NAME)-$(VERSION)

run-release:
> ./bin/release/$(NAME)-$(VERSION) -s test/src -d test/dst -r test/rmv -e extensions.txt -t 10 --unix

build-win-release:
> $(CC) src/main.cpp -std=c++2a -m64 -O3 -lstdc++fs -o bin/release/$(NAME)-$(VERSION).exe

run-win-release:
> ./bin/release/$(NAME)-$(VERSION).exe -s test\src -d test\dst -r test\rmv -e extensions.txt -t 10