#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <sys/stat.h>
#include <errno.h>

#include "ownserverapi.h"

int mkpath(char* file_path, mode_t mode) {
  assert(file_path && *file_path);
  char* p;
  for (p=strchr(file_path+1, '/'); p; p=strchr(p+1, '/')) {
    *p='\0';
    if (mkdir(file_path, mode)==-1) {
      if (errno!=EEXIST) { *p='/'; return -1; }
    }
    *p='/';
  }
  return 0;
}


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


void saveMeasurement(float temperature, float humidity, char* uuid)
{
     char data[255];
     //sprintf(data, "{\"temperature\":%.2lf,\"humidity\":%.2lf,\"guid\":\"%s\"}", temperature, humidity, uuid);
     sprintf(data, "temperature=%.2lf&humidity=%.2lf&uuid=\%s", temperature, humidity, uuid);

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char path[255];
    sprintf(path, "/tmp/arduino/measurements/%s/%d%d%d%d%d%d.dat", uuid, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    mkpath(path, 0775);
    FILE *f = fopen(path, "w");
   if (f == NULL)
   {
      printf("Error opening file %s!\n", path);
      return; 
   }
   fprintf(f, data);
   fclose(f);
}


