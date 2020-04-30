#include "Command.h"

Command::Command() {
}

char* Command::split(char cs, char ce, char* data) {
  int n = 0;
  static char param[sizeof(data)];
  if (ce == '\0') {
    for (int i = indexof(cs, data)+1; i <= strlen(data); i++) {
      param[n] = data[i];
      n++;
    }
  } else {
    for (int i = indexof(cs, data); i <= indexof(ce, data); i++) {
      param[n] = data[i];
      n++;
    }
  }
  param[n] = '\0';
  return param;
}

bool Command::subchar(int ns, int ne, char* data, const char* comp) {
  char command[sizeof(data)];
  if (ns < 16 or ne < 16) {
    command[0] = '\0';
    int n = 0;
    for (int i = ns; i < ne; i++) {
      command[n] = data[i];
      n++;
    }
    command[n] = '\0';
    if (!strcmp(command, comp)) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

int Command::indexof(char c, char* data) {
  int ind;
  for (int i = 0; i < strlen(data); i++) {
    if (data[i] == c) {
      ind = i;
    }
  }
  return ind;
}

void Command::getList(char* data, int* list) {
  char* ptr = strtok(data, "[,]");
  byte i = 0;
  while (ptr != NULL) {
    list[i] = atoi(ptr);
    Serial.println(list[i]);
    ptr = strtok(NULL, "[,]");
    i++;
  }
  list[i] = -1;
}
