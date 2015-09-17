# Makefile for git-ignore-device
git-ignore-device: git-ignore-device.c
	gcc -D_POSIX_C_SOURCE=200809L -std=c99 -Wall -O2 -o git-ignore-device git-ignore-device.c
