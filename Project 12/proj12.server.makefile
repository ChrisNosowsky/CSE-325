# Make for Project 12
# Chris Nosowsky
#
# Project 12 Server Executable

proj12.server:	proj12.server.o
				g++ proj12.server.o -o proj12.server

proj12.server.o:	proj12.server.c
				g++ -Wall -c proj12.server.c
