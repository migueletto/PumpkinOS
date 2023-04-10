#ifndef PLUTOSVG_H
#define PLUTOSVG_H

#include <plutovg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plutovg_document_t plutovg_document_t;

plutovg_document_t *plutosvg_parse_from_memory(const char* data, int size);
void plutosvg_document_size(plutovg_document_t *document, double *width, double *height);
int plutosvg_document_prepare(plutovg_document_t *document, double dpi);
void plutosvg_document_freeze(plutovg_document_t *document);
void plutosvg_document_reset(plutovg_document_t *document);
void plutosvg_document_rotate(plutovg_document_t *document, double radians);
void plutosvg_document_scale(plutovg_document_t *document, double factor);
int plutosvg_document_render(plutovg_document_t *document, plutovg_surface_t *surface, plutovg_font_t *font, double dpi);
void plutosvg_document_destroy(plutovg_document_t *document);

/**
 * @brief Load the image from a file in memory
 * @param data - pointer to the file data in memory
 * @param size - size of the data to load, in bytes
 * @param font - font to use for text rendering
 * @param width - requested width, in pixels
 * @param height - requested height, in pixels
 * @param dpi - dots per inch to use for units conversion to pixels
 * @return pointer to surface object on success, otherwise NULL
 */
plutovg_surface_t* plutosvg_load_from_memory(const char* data, int size, plutovg_font_t* font, int width, int height, double dpi);

/**
 * @brief Load the image from a file on disk
 * @param filename - path of the image file to load
 * @param font - font to use for text rendering
 * @param width - requested width, in pixels
 * @param height - requested height, in pixels
 * @param dpi - dots per inch to use for units conversion to pixels
 * @return pointer to surface object on success, otherwise NULL
 */
plutovg_surface_t* plutosvg_load_from_file(const char* filename, plutovg_font_t* font, int width, int height, double dpi);

/**
 * @brief Get image dimensions from a file in memory
 * @param data - pointer to the file data in memory
 * @param size - size of the data to load, in bytes
 * @param font - font to use for text rendering
 * @param width - width of the image, in pixels
 * @param height - height of the image, in pixels
 * @param dpi - dots per inch to use for units conversion to pixels
 * @return true on success, otherwise false
 */
int plutosvg_dimensions_from_memory(const char* data, int size, plutovg_font_t* font, int* width, int* height, double dpi);

/**
 * @brief Get image dimensions from a file on disk
 * @param filename - path of the image file to load
 * @param font - font to use for text rendering
 * @param width - width of the image, in pixels
 * @param height - height of the image, in pixels
 * @param dpi - dots per inch to use for units conversion to pixels
 * @return true on success, otherwise false
 */
int plutosvg_dimensions_from_file(const char* filename, plutovg_font_t* font, int* width, int* height, double dpi);

#ifdef __cplusplus
}
#endif

#endif // PLUTOSVG_H
