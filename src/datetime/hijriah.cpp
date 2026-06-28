#include "datetime/hijriah.h"

#include "datetime/date_and_time.h"

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