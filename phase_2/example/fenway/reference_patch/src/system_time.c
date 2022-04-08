#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WINDOWS_EPOCH 1601


long double system_time(){

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  printf("Start Year: %d\n" , WINDOWS_EPOCH);
  printf("Current Year: %d\n\n", tm.tm_year + 1900);

  int difference = tm.tm_year + 1900 - WINDOWS_EPOCH;

  int i = 0, days = 0;
  for(i = WINDOWS_EPOCH; i < tm.tm_year + 1900; ++i){
    if(i % 4 == 0){
      if(i % 100 == 0 && i % 400 != 0){
        days+= 365;

      } else {
        days += 366;

      }

    } else {
      days += 365;
    }
  }

  printf("Days: %d\n", days);

  long long hours = days * 24 + tm.tm_hour;
  printf("Hours: %lld\n" , hours);

  long long minutes = (hours * 60) + tm.tm_min;
  printf("Minutes: %lld\n", minutes);

  long long seconds = minutes * 60 + tm.tm_sec;
  printf("Seconds: %lld\n" ,seconds);

  long double nano_seconds = ((long double)seconds * 1000000000) / 100;
  printf("100 Nano-Second Intervals: %.0Lf\n\n\n" , nano_seconds);

  return nano_seconds;
}
