#pragma once

#include <global.h>

#include <stdio.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>

extern void* sender(void* args);
