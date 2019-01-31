#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <unistd.h>
#include <syslog.h>
#include "rs232.h"
#include "ownserverapi.h"
#include "helper.h"

//    sprintf(data, "temperature=%s&humidity=%s&uuid=%s", temperature, humidity,"AF993B68-0EF7-4842-8A36-8FD03A695456");

#define MILLISECOND 1000
#define SECOND 1000*MILLISECOND
#define MINUTE 60*SECOND
#define COUNTPERMINUTE 6
#define ARDUINOBUFFER 128

#define BUFFER_SIZE 16

int compile_regex (regex_t * r, const char * regex_text);
int match_regex (regex_t * r, const char * to_match, char* results[]);


int main(int argc, char **argv)
{
  int cport_nr=16;/* /dev/ttyUSB0 (COM1 on windows) */


  if (argc > 1)
  {
    char* comport = argv[1];
    //printf("%s\n", argv[1]);

    int idx;
    for (idx=1;idx<38;idx++)
    {
      printf("%s\n", comports[idx]);
      if(!strcmp(comports[idx], comport))
      {
        //printf("%s %d\n", comports[idx], idx);
        cport_nr = idx;
      }
    }
  }

  int i, n,
      bdrate=9600;       /* 9600 baud */

  unsigned char buf[ARDUINOBUFFER];
  char mode[]={'8','N','1',0};


  if(RS232_OpenComport(cport_nr, bdrate, mode))
  {
     char error[80];
     sprintf(error, "Can not open serial port %s", comports[cport_nr]);
    putError(error);
    return(0);
  }

  char* result[2];
  result[0] = malloc(BUFFER_SIZE);
  result[1] = malloc(BUFFER_SIZE);

  regex_t r;
  const char * regex_text;
  regex_text = "([[:digit:]]+)[^[:digit:]]+([[:digit:]]+)";
  int cr = compile_regex(& r, regex_text);
  if (cr != 0)
  {
     return cr;
  }

  float humidities[COUNTPERMINUTE];
  float temperatures[COUNTPERMINUTE];

  long attempts = 0; 
  char t[16], h[16];
  char currentMeasurement[64];

  while(1)
  {
    n = RS232_PollComport(cport_nr, buf, ARDUINOBUFFER-1);

    if(n > 0)
    {
      buf[n] = 0;   /* always put a "null" at the end of a string! */

      for(i=0; i < n; i++)
      {
        if(buf[i] < 32)  /* replace unreadable control-codes by dots */
        {
          buf[i] = '.';
        }
      }

     // printf("received %i bytes: %s\n", n, (char *)buf);
      int success =  match_regex(& r, (char*)buf, result);
    if (success == 0)
    {
       attempts++;
       sprintf(currentMeasurement, "Temperature %s Humidity %s", result[0], result[1]);

       //puts(currentMeasurement);
       syslog(LOG_INFO, currentMeasurement);

       int idx = attempts %  COUNTPERMINUTE;
	humidities[idx] = atof(result[1]);
	temperatures[idx] = atof(result[0]);

	saveMeasurement( temperatures[idx], humidities[idx], "AF993B68-0EF7-4842-8A36-8FD03A695456");

	if (idx == 0 && attempts>=COUNTPERMINUTE)
	{
	   float sumTemp, sumHum;
	   int u = 0;
	   for (u=0; u< COUNTPERMINUTE; u++)
	   {
	     sumTemp += temperatures[u];
	     sumHum += humidities[u];
	   }
           sprintf(t, "%.2lf",sumTemp/COUNTPERMINUTE);
	   sprintf(h, "%.2lf",sumHum/COUNTPERMINUTE);
	    printf("Avg Temperature %s Humidity %s\r\n", t, h);
	   //saveWeather(t,h,"AF993B68-0EF7-4842-8A36-8FD03A695456");
	   sumTemp = 0;
	   sumHum = 0;
	}
    }
     else
     {
       putError("Can not parse response");
       putError((char*)buf);
     }

    }
    usleep(100000);  /* sleep for 100 milliSeconds */
  }
  regfree (& r);
  return(0);
}

#define MAX_ERROR_MSG 0x1000

int compile_regex (regex_t * r, const char * regex_text)
{
    int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);
    if (status != 0) {
        char error_message[MAX_ERROR_MSG];
        regerror (status, r, error_message, MAX_ERROR_MSG);
        printf ("Regex error compiling '%s': %s\n",
                 regex_text, error_message);
        return 1;
    }
    return 0;
}


int match_regex (regex_t * r, const char * to_match, char* results[])
{
    /* "P" is a pointer into the string which points to the end of the
       previous match. */
    const char * p = to_match;
    /* "N_matches" is the maximum number of matches allowed. */
    const int n_matches = 10;
    /* "M" contains the matches found. */
    regmatch_t m[n_matches];

    int mIdx =0;
    while (1) {
        int i = 0;
        int nomatch = regexec (r, p, n_matches, m, 0);
        if (nomatch) {
            //printf ("No more matches.\n");
            if (mIdx > 0)
             return 0;
            return nomatch;
        }
        for (i = 0; i < n_matches; i++) {
            int start;
            int finish;
            if (m[i].rm_so == -1) {
                break;
            }
            start = m[i].rm_so + (p - to_match);
            finish = m[i].rm_eo + (p - to_match);
            if (i == 0) {
            //    printf ("$& is ");
            //}
            //else {
            //    printf ("$%d is ", i);
            //}
                sprintf (results[mIdx],"%.*s", (finish - start), to_match + start );
                mIdx++;
            }
        }
        p += m[0].rm_eo;
    }
    return 0;
}
