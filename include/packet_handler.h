#pragma once

#include <global.h>

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>

extern void* packet_handler(void* args);
