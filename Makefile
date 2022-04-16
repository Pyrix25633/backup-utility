.RECIPEPREFIX = >
CC = g++
NAME = backup-utility
VERSION = 0.1.0
64OPTIONS = -std=c++2a -m64 -lstdc++fs -o
32OPTIONS = -std=c++2a -m32 -lstdc++fs -o
DEBUGOPTIONS = -g
RELEASEOPTIONS = -O3
RUNOPTIONS = -s test/src -d test/dst -r test/rmv -e extensions.txt -t 10

#debug-64
default:
> clear
> make build
> make run

win:
> clear
> make build-win
> make run-win

#debug-32
debug-32:
> clear
> make build-32
> make run-32

debug-win-32:
> clear
> make build-win-32
> make run-win-32

#release-64
release:
> clear
> make build-release
> make run-release

release-win:
> clear
> make build-release-win
> make run-release-win

#release-32
release-32:
> clear
> make build-release-32
> make run-release

release-win-32:
> clear
> make build-release-win-32
> make run-release-win-32

#debug-64
build:
> $(CC) src/main.cpp $(DEBUGOPTIONS) $(64OPTIONS) bin/debug/$(NAME)-$(VERSION)-x64

run:
> ./bin/debug/$(NAME)-$(VERSION)-x64 $(RUNOPTIONS) --unix

build-win:
> $(CC) src/main.cpp $(DEBUGOPTIONS) $(64OPTIONS) bin/debug/$(NAME)-$(VERSION)-x64.exe

run-win:
> ./bin/debug/$(NAME)-$(VERSION)-x64.exe $(RUNOPTIONS)

#debug-32
build-23:
> $(CC) src/main.cpp $(DEBUGOPTIONS) $(32OPTIONS) bin/debug/$(NAME)-$(VERSION)-x32

run-32:
> ./bin/debug/$(NAME)-$(VERSION)-x32 $(RUNOPTIONS) --unix

build-win-32:
> $(CC) src/main.cpp $(DEBUGOPTIONS) $(32OPTIONS) bin/debug/$(NAME)-$(VERSION)-x32.exe

run-win-32:
> ./bin/debug/$(NAME)-$(VERSION)-x32.exe $(RUNOPTIONS)

#release-64
build-release:
> $(CC) src/main.cpp $(RELEASEOPTIONS) $(64OPTIONS) bin/release/$(NAME)-$(VERSION)-x64

run-release:
> ./bin/release/$(NAME)-$(VERSION)-x64 $(RUNOPTIONS) --unix

build-release-win:
> $(CC) src/main.cpp $(RELEASEOPTIONS) $(64OPTIONS) bin/release/$(NAME)-$(VERSION)-x64.exe

run-release-win:
> ./bin/release/$(NAME)-$(VERSION)-x64.exe $(RUNOPTIONS)

#release-32
build-release-32:
> $(CC) src/main.cpp $(RELEASEOPTIONS) $(32OPTIONS) bin/release/$(NAME)-$(VERSION)-x32

run-release-32:
> ./bin/release/$(NAME)-$(VERSION)-x32 $(RUNOPTIONS) --unix

build-release-win-32:
> $(CC) src/main.cpp $(RELEASEOPTIONS) $(32OPTIONS) bin/release/$(NAME)-$(VERSION)-x32.exe

run-release-win-32:
> ./bin/release/$(NAME)-$(VERSION)-x32.exe $(RUNOPTIONS)