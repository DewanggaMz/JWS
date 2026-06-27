#include "datetime/hijriah.h"

const char* namaBulanHijriah[] = {
  "Muharram",
  "Safar",
  "Rabiul Awal",
  "Rabiul Akhir",
  "Jumadil Awal",
  "Jumadil Akhir",
  "Rajab",
  "Sya'ban",
  "Ramadan",
  "Syawal",
  "Zulkaidah",
  "Zulhijah"
};

namespace HijriModule {
  long gregorianToJDN(int year, int month, int day) {
    int a = (14 - month) / 12;
    int y = year + 4800 - a;
    int m = month + 12 * a - 3;

    long jdn = day
               + (153L * m + 2) / 5
               + 365L * y
               + y / 4
               - y / 100
               + y / 400
               - 32045;

    return jdn;
  }

  HijriDate jdnToHijri(long jdn) {
    HijriDate h;

    long l = jdn - 1948440 + 10632;
    long n = (l - 1) / 10631;

    l = l - 10631 * n + 354;

    long j = ((10985 - l) / 5316) * ((50 * l) / 17719)
           + (l / 5670) * ((43 * l) / 15238);

    l = l
        - ((30 - j) / 15) * ((17719 * j) / 50)
        - (j / 16) * ((15238 * j) / 43)
        + 29;

    h.month = (24 * l) / 709;
    h.day = l - (709 * h.month) / 24;
    h.year = 30 * n + j - 30;

    return h;
  }


  HijriDate getHijriDate(int year, int month, int day, int correction) {
    long jdn = gregorianToJDN(year, month, day);

    jdn += correction;

    return jdnToHijri(jdn);
  }
}

const char* getHijriMonthName(int month) {
  if (month < 1 || month > 12) {
    return "-";
  }

  return namaBulanHijriah[month - 1];
}