#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "param.h"

int param_check(param_info_t pinfo[], char *name, char *value) {
  int32_t ivalue;
  float fvalue;
  int bvalue;

  for (int i = 0; pinfo[i].name; i++) {
    if (!strncasecmp(name, pinfo[i].name, strlen(pinfo[i].name))) {
      switch (pinfo[i].type) {
        case 'I':
          ivalue = atoi(value);
          if (ivalue >= pinfo[i].imin && ivalue <= pinfo[i].imax) {
            *(int32_t *)pinfo[i].p = ivalue;
            fprintf(stderr, "parameter %s set to %d\n", name, ivalue);
            return 1;
          }
          break;
        case 'F':
          fvalue = atof(value);
          if (fvalue >= pinfo[i].fmin && fvalue <= pinfo[i].fmax) {
            *(float *)pinfo[i].p = fvalue;
            fprintf(stderr, "parameter %s set to %.2f\n", name, fvalue);
            return 1;
          }
          break;
        case 'B':
          if (!strcmp(value, "true")) {
            bvalue = 1;
          } else if (!strcmp(value, "false")) {
            bvalue = 0;
          } else {
            return 0;
          }
          *(int *)pinfo[i].p = bvalue;
          fprintf(stderr, "parameter %s set to %s\n", name, bvalue ? "true" : "false");
          return 1;
      }
    }
  }

  return 0;
}

void param_usage(param_info_t pinfo[]) {
  for (int i = 0; pinfo[i].name; i++) {
    switch (pinfo[i].type) {
      case 'I':
        fprintf(stderr, "  -%-10s <integer>  (%s; default: %d)\n", pinfo[i].name, pinfo[i].help, *(int32_t *)pinfo[i].p);
        break;
      case 'F':
        fprintf(stderr, "  -%-10s <float>    (%s; default: %.02f)\n", pinfo[i].name, pinfo[i].help, *(float *)pinfo[i].p);
        break;
      case 'B':
        fprintf(stderr, "  -%-10s <boolean>  (%s; default: %s)\n", pinfo[i].name, pinfo[i].help, (*(int *)pinfo[i].p) ? "true" : "false");
        break;
    }
  }
}
