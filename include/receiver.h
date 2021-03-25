#pragma once

#include <global.h>

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

extern void* receiver(void* args);
