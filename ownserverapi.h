#ifndef ownServerApi_INCLUDED
#define ownServerApi_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include "simplehttp.h"

void postWeather(char* temperature, char* humidity, char* uuid);

#ifdef __cplusplus
}
#endif
#endif
