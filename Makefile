# Makefile for git-ignore-index-stat
git-ignore-index-stat: git-ignore-index-stat.c
	gcc -std=c99 -Wall -O2 -o git-ignore-index-stat git-ignore-index-stat.c
