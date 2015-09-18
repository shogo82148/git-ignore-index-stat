# Makefile for git-ignore-device
git-ignore-index-stat: git-ignore-index-stat.c
	gcc -lssl -D_POSIX_C_SOURCE=200809L -std=c99 -Wall -O2 -o git-ignore-index-stat git-ignore-index-stat.c
