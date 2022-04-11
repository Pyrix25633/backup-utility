.RECIPEPREFIX = >
CC = g++
NAME = backup-utility
VERSION = 0.0.1

default:
> clear
> make build
> make run

build:
> $(CC) src/main.cpp -o bin/$(NAME)-$(VERSION)

run:
> ./bin/$(NAME)-$(VERSION)