#include "utils/utils.h"

const char* getMonthName(int month) {
  const char* months[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
  };
  if (month >= 1 && month <= 12) {
    return months[month - 1];
  }
  return "Unknown";
}

String formatDate(int day, int month, int year) {
  char buffer[32];
  sprintf(buffer, "%s %d, %d", getMonthName(month), day, year);
  return String(buffer);
}