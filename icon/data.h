#ifndef ICON_DATA_H
#define ICON_DATA_H

typedef struct {
  unsigned int size;
  unsigned int cnt;
  const unsigned char *data;
} icon_data_t;

#define ICON_(s) { s, ARRLEN(icon_data_##s), icon_data_##s }

#include "data.gen.h"

#endif /* ICON_DATA_H */
