#!/bin/bash
gcc -pthread connectionqueue.c connectionhandler.c stringutils.c main.c && ./a.out