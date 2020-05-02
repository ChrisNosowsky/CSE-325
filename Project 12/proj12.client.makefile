# Make for Project 12
# Chris Nosowsky
#
# Project 12 Executable

proj12.client:	proj12.client.o
				g++ proj12.client.o -o proj12.client

proj12.client.o:	proj12.client.c
				g++ -Wall -c proj12.client.c
