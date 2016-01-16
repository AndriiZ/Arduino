
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "ownserverapi.h"

void postWeather(char* temperature, char* humidity, char* uuid)
{
    int portno =        80;
    char *host =        "192.168.1.20";
    char *message_fmt = "POST /weather/newmeasurement.php HTTP/1.0\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s\r\n";

    char message[1024];
    char data[255];
    sprintf(data, "temperature=%s&humidity=%s&uuid=%s", temperature, humidity, uuid);
    sprintf(message,message_fmt,strlen(data), data);

    postToServer(host, portno, message);
}



