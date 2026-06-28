#include "datetime/pasaran.h"

#include "datetime/date_and_time.h"

const char* pasaran[] = {
  "Legi", "Pahing", "Pon", "Wage", "Kliwon"
};


int getPasaranIndex(int year, int month, int day) {
  long refJDN = gregorianToJDN(1945, 8, 17);
  long nowJDN = gregorianToJDN(year, month, day);

  long selisih = nowJDN - refJDN;

  int index = selisih % 5;
  if (index < 0) index += 5;

  return index;
}

String getPasaran(int day, int month, int year) {
  int index = getPasaranIndex(year, month, day);
  return String(pasaran[index]);
}