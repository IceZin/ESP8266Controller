#ifndef Command_h
#define Command_h

#include "Arduino.h"

class Command {
  public:
    Command();
    char* split(char cs, char ce, char* data);
    bool subchar(int ns, int ne, char* data, const char* comp);
    int indexof(char c, char* data);
    void getList(char* data, int* list);
  private:
};

#endif
