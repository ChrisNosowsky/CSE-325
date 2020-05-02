# Make for Project 11
# Chris Nosowsky
#
# Project 11 Executable

proj11.client:	proj11.client.o
				g++ proj11.client.o -o proj11.client

proj11.client.o:	proj11.client.c
				g++ -Wall -c proj11.client.c
