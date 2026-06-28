#pragma once
#include <Arduino.h>

struct HijriDate {
  int day;
  int month;
  int year;
};

namespace HijriModule {

  HijriDate jdnToHijri(long jdn);

  HijriDate getHijriDate(int year, int month, int day, int correction);

  
}

const char* getHijriMonthName(int month);

