#include "sys.h"

static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *base64_encode(uint8_t *data, uint32_t input_length) {
  char *encoded_data;
  uint32_t i, j, output_length;

  output_length = 4 * ((input_length + 2) / 3);
  encoded_data = sys_malloc(output_length + 1);
  if (encoded_data == NULL) return NULL;

  for (i = 0, j = 0; i < input_length;) {
    uint32_t a = i < input_length ? data[i++] : 0;
    uint32_t b = i < input_length ? data[i++] : 0;
    uint32_t c = i < input_length ? data[i++] : 0;
    uint32_t triple = (a << 16) | (b << 8) | c;

    encoded_data[j++] = base64_table[(triple >> 18) & 0x3F];
    encoded_data[j++] = base64_table[(triple >> 12) & 0x3F];
    encoded_data[j++] = base64_table[(triple >>  6) & 0x3F];
    encoded_data[j++] = base64_table[ triple        & 0x3F];
  }

  if (input_length % 3 == 1) {
    encoded_data[output_length - 1] = '=';
    encoded_data[output_length - 2] = '=';
  } else if (input_length % 3 == 2) {
    encoded_data[output_length - 1] = '=';
  }
  encoded_data[output_length] = 0;

  return encoded_data;
}
