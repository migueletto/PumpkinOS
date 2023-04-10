#include "plutovg-private.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

plutovg_surface_t* plutovg_surface_create(int width, int height)
{
    plutovg_surface_t* surface = malloc(sizeof(plutovg_surface_t));
    surface->ref = 1;
    surface->owndata = 1;
    surface->data = malloc((size_t)(width * height * 4));
    memset(surface->data, 0, (size_t)(width * height * 4));
    surface->width = width;
    surface->height = height;
    surface->stride = width * 4;
    return surface;
}

plutovg_surface_t* plutovg_surface_create_for_data(unsigned char* data, int width, int height, int stride)
{
    plutovg_surface_t* surface = malloc(sizeof(plutovg_surface_t));
    surface->ref = 1;
    surface->owndata = 0;
    surface->data = data;
    surface->width = width;
    surface->height = height;
    surface->stride = stride;
    return surface;
}

plutovg_surface_t* plutovg_surface_reference(plutovg_surface_t* surface)
{
    if(surface==NULL)
        return NULL;

    ++surface->ref;
    return surface;
}

void plutovg_surface_destroy(plutovg_surface_t* surface)
{
    if(surface==NULL)
        return;

    if(--surface->ref==0)
    {
        if(surface->owndata)
            free(surface->data);
        free(surface);
    }
}

int plutovg_surface_get_reference_count(const plutovg_surface_t* surface)
{
    if(surface==NULL)
        return 0;

    return surface->ref;
}

unsigned char* plutovg_surface_get_data(const plutovg_surface_t* surface)
{
    return surface->data;
}

int plutovg_surface_get_width(const plutovg_surface_t* surface)
{
    return surface->width;
}

int plutovg_surface_get_height(const plutovg_surface_t* surface)
{
    return surface->height;
}

int plutovg_surface_get_stride(const plutovg_surface_t* surface)
{
    return surface->stride;
}

void plutovg_surface_write_to_png(const plutovg_surface_t* surface, const char* filename)
{
    unsigned char* data = surface->data;
    int width = surface->width;
    int height = surface->height;
    int stride = surface->stride;
    unsigned char* image = malloc((size_t)(stride * height));
    for(int y = 0;y < height;y++)
    {
        const uint32_t* src = (uint32_t*)(data + stride * y);
        uint32_t* dst = (uint32_t*)(image + stride * y);
        for(int x = 0;x < width;x++)
        {
            uint32_t a = src[x] >> 24;
            if(a != 0)
            {
                uint32_t r = (((src[x] >> 16) & 0xff) * 255) / a;
                uint32_t g = (((src[x] >> 8) & 0xff) * 255) / a;
                uint32_t b = (((src[x] >> 0) & 0xff) * 255) / a;

                dst[x] = (a << 24) | (b << 16) | (g << 8) | r;
            }
            else
            {
                dst[x] = 0;
            }
        }
    }

    stbi_write_png(filename, width, height, 4, image, stride);
    free(image);
}

void plutovg_surface_write_to_memory(const plutovg_surface_t *surface,
    void (*set)(int x, int y, int r, int g, int b, int a, void *cdata), void *cdata) {
  unsigned char *data = surface->data;
  int width = surface->width;
  int height = surface->height;
  int stride = surface->stride;

  for (int y = 0; y < height; y++) {
    const uint32_t *src = (uint32_t *)(data + stride * y);

    for (int x = 0; x < width; x++) {
      uint32_t a = src[x] >> 24;
      uint32_t r = (src[x] >> 16) & 0xff;
      uint32_t g = (src[x] >> 8) & 0xff;
      uint32_t b = src[x] & 0xff;
      set(x, y, r, g, b, a, cdata);
    }
  }
}

plutovg_state_t* plutovg_state_create(void)
{
    plutovg_state_t* state = malloc(sizeof(plutovg_state_t));
    state->clippath = NULL;
    state->font = NULL;
    state->source = plutovg_paint_create_rgb(0, 0, 0);
    plutovg_matrix_init_identity(&state->matrix);
    state->winding = plutovg_fill_rule_non_zero;
    state->stroke.width = 1.0;
    state->stroke.miterlimit = 4.0;
    state->stroke.cap = plutovg_line_cap_butt;
    state->stroke.join = plutovg_line_join_miter;
    state->stroke.dash = NULL;
    state->op = plutovg_operator_src_over;
    state->fontsize = 12.0;
    state->opacity = 1.0;
    state->next = NULL;
    return state;
}

plutovg_state_t* plutovg_state_clone(const plutovg_state_t* state)
{
    plutovg_state_t* newstate = malloc(sizeof(plutovg_state_t));
    newstate->clippath = plutovg_rle_clone(state->clippath);
    newstate->font = plutovg_font_reference(state->font);
    newstate->source = plutovg_paint_reference(state->source); /** FIXME: clone!!?**/
    newstate->matrix = state->matrix;
    newstate->winding = state->winding;
    newstate->stroke.width = state->stroke.width;
    newstate->stroke.miterlimit = state->stroke.miterlimit;
    newstate->stroke.cap = state->stroke.cap;
    newstate->stroke.join = state->stroke.join;
    newstate->stroke.dash = plutovg_dash_clone(state->stroke.dash);
    newstate->op = state->op;
    newstate->fontsize = state->fontsize;
    newstate->opacity = state->opacity;
    newstate->next = NULL;
    return newstate;
}

void plutovg_state_destroy(plutovg_state_t* state)
{
    plutovg_rle_destroy(state->clippath);
    plutovg_font_destroy(state->font);
    plutovg_paint_destroy(state->source);
    plutovg_dash_destroy(state->stroke.dash);
    free(state);
}

plutovg_t* plutovg_create(plutovg_surface_t* surface)
{
    plutovg_t* pluto = malloc(sizeof(plutovg_t));
    pluto->ref = 1;
    pluto->surface = plutovg_surface_reference(surface);
    pluto->state = plutovg_state_create();
    pluto->path = plutovg_path_create();
    pluto->rle = plutovg_rle_create();
    pluto->clippath = NULL;
    pluto->clip.x = 0.0;
    pluto->clip.y = 0.0;
    pluto->clip.w = surface->width;
    pluto->clip.h = surface->height;
    return pluto;
}

plutovg_t* plutovg_reference(plutovg_t* pluto)
{
    if(pluto==NULL)
        return NULL;

    ++pluto->ref;
    return pluto;
}

void plutovg_destroy(plutovg_t* pluto)
{
    if(pluto==NULL)
        return;

    if(--pluto->ref==0)
    {
        while(pluto->state)
        {
            plutovg_state_t* state = pluto->state;
            pluto->state = state->next;
            plutovg_state_destroy(state);
        }

        plutovg_surface_destroy(pluto->surface);
        plutovg_path_destroy(pluto->path);
        plutovg_rle_destroy(pluto->rle);
        plutovg_rle_destroy(pluto->clippath);
        free(pluto);
    }
}

int plutovg_get_reference_count(const plutovg_t* pluto)
{
    if(pluto==NULL)
        return 0;

    return pluto->ref;
}

void plutovg_save(plutovg_t* pluto)
{
    plutovg_state_t* newstate = plutovg_state_clone(pluto->state);
    newstate->next = pluto->state;
    pluto->state = newstate;
}

void plutovg_restore(plutovg_t* pluto)
{
    plutovg_state_t* oldstate = pluto->state;
    pluto->state = oldstate->next;
    plutovg_state_destroy(oldstate);
}

void plutovg_set_source_rgb(plutovg_t* pluto, double r, double g, double b)
{
    plutovg_set_source_rgba(pluto, r, g, b, 1.0);
}

void plutovg_set_source_rgba(plutovg_t* pluto, double r, double g, double b, double a)
{
    plutovg_paint_t* source = plutovg_paint_create_rgba(r, g, b, a);
    plutovg_set_source(pluto, source);
    plutovg_paint_destroy(source);
}

void plutovg_set_source_surface(plutovg_t* pluto, plutovg_surface_t* surface, double x, double y)
{
    plutovg_paint_t* source = plutovg_paint_create_for_surface(surface);
    plutovg_texture_t* texture = plutovg_paint_get_texture(source);
    plutovg_matrix_t matrix;
    plutovg_matrix_init_translate(&matrix, x, y);
    plutovg_texture_set_matrix(texture, &matrix);
    plutovg_set_source(pluto, source);
    plutovg_paint_destroy(source);
}

void plutovg_set_source_color(plutovg_t* pluto, const plutovg_color_t* color)
{
    plutovg_set_source_rgba(pluto, color->r, color->g, color->b, color->a);
}

void plutovg_set_source_gradient(plutovg_t* pluto, plutovg_gradient_t* gradient)
{
    plutovg_paint_t* source = plutovg_paint_create_gradient(gradient);
    plutovg_set_source(pluto, source);
    plutovg_paint_destroy(source);
}

void plutovg_set_source_texture(plutovg_t* pluto, plutovg_texture_t* texture)
{
    plutovg_paint_t* source = plutovg_paint_create_texture(texture);
    plutovg_set_source(pluto, source);
    plutovg_paint_destroy(source);
}

void plutovg_set_source(plutovg_t* pluto, plutovg_paint_t* source)
{
    source = plutovg_paint_reference(source);
    plutovg_paint_destroy(pluto->state->source);
    pluto->state->source = source;
}

plutovg_paint_t* plutovg_get_source(const plutovg_t* pluto)
{
    return pluto->state->source;
}

void plutovg_set_operator(plutovg_t* pluto, plutovg_operator_t op)
{
    pluto->state->op = op;
}

void plutovg_set_opacity(plutovg_t* pluto, double opacity)
{
    pluto->state->opacity = CLAMP(opacity, 0.0, 1.0);
}

void plutovg_set_fill_rule(plutovg_t* pluto, plutovg_fill_rule_t winding)
{
    pluto->state->winding = winding;
}

plutovg_operator_t plutovg_get_operator(const plutovg_t* pluto)
{
    return pluto->state->op;
}

double plutovg_get_opacity(const plutovg_t* pluto)
{
    return pluto->state->opacity;
}

plutovg_fill_rule_t plutovg_get_fill_rule(const plutovg_t* pluto)
{
    return pluto->state->winding;
}

void plutovg_set_line_width(plutovg_t* pluto, double width)
{
    pluto->state->stroke.width = width;
}

void plutovg_set_line_cap(plutovg_t* pluto, plutovg_line_cap_t cap)
{
    pluto->state->stroke.cap = cap;
}

void plutovg_set_line_join(plutovg_t* pluto, plutovg_line_join_t join)
{
    pluto->state->stroke.join = join;
}

void plutovg_set_miter_limit(plutovg_t* pluto, double limit)
{
    pluto->state->stroke.miterlimit = limit;
}

void plutovg_set_dash(plutovg_t* pluto, double offset, const double* data, int size)
{
    plutovg_dash_destroy(pluto->state->stroke.dash);
    pluto->state->stroke.dash = plutovg_dash_create(offset, data, size);
}

double plutovg_get_line_width(const plutovg_t* pluto)
{
    return pluto->state->stroke.width;
}

plutovg_line_cap_t plutovg_get_line_cap(const plutovg_t* pluto)
{
    return pluto->state->stroke.cap;
}

plutovg_line_join_t plutovg_get_line_join(const plutovg_t* pluto)
{
    return pluto->state->stroke.join;
}

double plutovg_get_miter_limit(const plutovg_t* pluto)
{
    return pluto->state->stroke.miterlimit;
}

double plutovg_get_dash_offset(plutovg_t* pluto)
{
    plutovg_stroke_data_t* stroke = &pluto->state->stroke;
    return stroke->dash ? stroke->dash->offset : 0.0;
}

double* plutovg_get_dash_data(plutovg_t* pluto)
{
    plutovg_stroke_data_t* stroke = &pluto->state->stroke;
    return stroke->dash ? stroke->dash->data : NULL;
}

int plutovg_get_dash_count(plutovg_t* pluto)
{
    plutovg_stroke_data_t* stroke = &pluto->state->stroke;
    return stroke->dash ? stroke->dash->size : 0;
}

void plutovg_translate(plutovg_t* pluto, double x, double y)
{
    plutovg_matrix_translate(&pluto->state->matrix, x, y);
}

void plutovg_scale(plutovg_t* pluto, double x, double y)
{
    plutovg_matrix_scale(&pluto->state->matrix, x, y);
}

void plutovg_rotate(plutovg_t* pluto, double radians)
{
    plutovg_matrix_rotate(&pluto->state->matrix, radians);
}

void plutovg_transform(plutovg_t* pluto, const plutovg_matrix_t* matrix)
{
    plutovg_matrix_multiply(&pluto->state->matrix, matrix, &pluto->state->matrix);
}

void plutovg_set_matrix(plutovg_t* pluto, const plutovg_matrix_t* matrix)
{
    memcpy(&pluto->state->matrix, matrix, sizeof(plutovg_matrix_t));
}

void plutovg_identity_matrix(plutovg_t* pluto)
{
    plutovg_matrix_init_identity(&pluto->state->matrix);
}

void plutovg_get_matrix(const plutovg_t* pluto, plutovg_matrix_t* matrix)
{
    memcpy(matrix, &pluto->state->matrix, sizeof(plutovg_matrix_t));
}

void plutovg_move_to(plutovg_t* pluto, double x, double y)
{
    plutovg_path_move_to(pluto->path, x, y);
}

void plutovg_line_to(plutovg_t* pluto, double x, double y)
{
    plutovg_path_line_to(pluto->path, x, y);
}

void plutovg_quad_to(plutovg_t* pluto, double x1, double y1, double x2, double y2)
{
    plutovg_path_quad_to(pluto->path, x1, y1, x2, y2);
}

void plutovg_cubic_to(plutovg_t* pluto, double x1, double y1, double x2, double y2, double x3, double y3)
{
    plutovg_path_cubic_to(pluto->path, x1, y1, x2, y2, x3, y3);
}

void plutovg_rel_move_to(plutovg_t* pluto, double x, double y)
{
    plutovg_path_rel_move_to(pluto->path, x, y);
}

void plutovg_rel_line_to(plutovg_t* pluto, double x, double y)
{
    plutovg_path_rel_line_to(pluto->path, x, y);
}

void plutovg_rel_quad_to(plutovg_t* pluto, double x1, double y1, double x2, double y2)
{
    plutovg_path_rel_quad_to(pluto->path, x1, y1, x2, y2);
}

void plutovg_rel_cubic_to(plutovg_t* pluto, double x1, double y1, double x2, double y2, double x3, double y3)
{
    plutovg_path_rel_cubic_to(pluto->path, x1, y1, x2, y2, x3, y3);
}

void plutovg_rect(plutovg_t* pluto, double x, double y, double w, double h)
{
    plutovg_path_add_rect(pluto->path, x, y, w, h);
}

void plutovg_round_rect(plutovg_t* pluto, double x, double y, double w, double h, double rx, double ry)
{
    plutovg_path_add_round_rect(pluto->path, x, y, w, h, rx, ry);
}

void plutovg_ellipse(plutovg_t* pluto, double cx, double cy, double rx, double ry)
{
    plutovg_path_add_ellipse(pluto->path, cx, cy, rx, ry);
}

void plutovg_circle(plutovg_t* pluto, double cx, double cy, double r)
{
    plutovg_ellipse(pluto, cx, cy, r, r);
}

void plutovg_arc(plutovg_t* pluto, double cx, double cy, double r, double a0, double a1, int ccw)
{
    plutovg_path_add_arc(pluto->path, cx, cy, r, a0, a1, ccw);
}

void plutovg_add_path(plutovg_t* pluto, const plutovg_path_t* path)
{
    plutovg_path_add_path(pluto->path, path, NULL);
}

void plutovg_new_path(plutovg_t* pluto)
{
    plutovg_path_clear(pluto->path);
}

void plutovg_close_path(plutovg_t* pluto)
{
    plutovg_path_close(pluto->path);
}

void plutovg_get_current_point(const plutovg_t* pluto, double* x, double* y)
{
    plutovg_path_get_current_point(pluto->path, x, y);
}

plutovg_path_t* plutovg_get_path(const plutovg_t* pluto)
{
    return pluto->path;
}

void plutovg_set_font(plutovg_t* pluto, plutovg_font_t* font)
{
    font = plutovg_font_reference(font);
    plutovg_font_destroy(pluto->state->font);
    pluto->state->font = font;
}

void plutovg_set_font_size(plutovg_t* pluto, double size)
{
    pluto->state->fontsize = size;
}

plutovg_font_t* plutovg_get_font(const plutovg_t* pluto)
{
    return pluto->state->font;
}

double plutovg_get_font_size(const plutovg_t* pluto)
{
    return pluto->state->fontsize;
}

static inline int decode_utf8(const char** begin, const char* end, uint32_t* codepoint)
{
    static const int trailing[256] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
    };

    static const uint32_t offsets[6] =
    {
        0x00000000, 0x00003080, 0x000E2080, 0x03C82080, 0xFA082080, 0x82082080
    };

    const char* ptr = *begin;
    int trailing_bytes = trailing[(uint8_t)(*ptr)];
    if(ptr + trailing_bytes >= end)
        return 0;

    uint32_t output = 0;
    switch(trailing_bytes)
    {
    case 5: output += (uint8_t)(*ptr++); output <<= 6;
    case 4: output += (uint8_t)(*ptr++); output <<= 6;
    case 3: output += (uint8_t)(*ptr++); output <<= 6;
    case 2: output += (uint8_t)(*ptr++); output <<= 6;
    case 1: output += (uint8_t)(*ptr++); output <<= 6;
    case 0: output += (uint8_t)(*ptr++);
    }

    output -= offsets[trailing_bytes];
    *begin = ptr;
    *codepoint = output;
    return 1;
}

void plutovg_text(plutovg_t* pluto, const char* utf8, double x, double y)
{
    plutovg_state_t* state = pluto->state;
    if(state->font == NULL)
        return;

    const char* end = utf8 + strlen(utf8);
    uint32_t codepoint = 0;
    double scale = plutovg_font_get_scale(state->font, state->fontsize);
    plutovg_matrix_t matrix;
    while(utf8 < end)
    {
        if(!decode_utf8(&utf8, end, &codepoint))
            return;

        plutovg_matrix_init_translate(&matrix, x, y);
        plutovg_matrix_scale(&matrix, scale, -scale);

        const plutovg_glyph_t* glyph = plutovg_font_get_glyph(state->font, (int)codepoint);
        const plutovg_path_t* path = plutovg_glyph_get_path(glyph);
        plutovg_path_add_path(pluto->path, path, &matrix);

        x += plutovg_glyph_get_advance(glyph) * scale;
    }
}

void plutovg_char(plutovg_t* pluto, int ch, double x, double y)
{
    plutovg_state_t* state = pluto->state;
    if(state->font == NULL)
        return;

    double scale = plutovg_font_get_scale(state->font, state->fontsize);

    plutovg_matrix_t matrix;
    plutovg_matrix_init_translate(&matrix, x, y);
    plutovg_matrix_scale(&matrix, scale, -scale);

    const plutovg_glyph_t* glyph = plutovg_font_get_glyph(state->font, ch);
    const plutovg_path_t* path = plutovg_glyph_get_path(glyph);
    plutovg_path_add_path(pluto->path, path, &matrix);
}

void plutovg_text_extents(plutovg_t* pluto, const char* utf8, double* w, double* h)
{
    if(w) *w = 0;
    if(h) *h = 0;

    plutovg_state_t* state = pluto->state;
    if(state->font == NULL)
        return;

    const char* end = utf8 + strlen(utf8);
    uint32_t codepoint = 0;
    double scale = plutovg_font_get_scale(state->font, state->fontsize);
    double x = 0;
    while(utf8 < end)
    {
        if(!decode_utf8(&utf8, end, &codepoint))
            return;

        const plutovg_glyph_t* glyph = plutovg_font_get_glyph(state->font, (int)codepoint);
        x += plutovg_glyph_get_advance(glyph) * scale;
    }

    if(w) *w = x;
    if(h) *h = plutovg_font_get_leading(state->font) * scale;
}

void plutovg_fill(plutovg_t* pluto)
{
    plutovg_fill_preserve(pluto);
    plutovg_new_path(pluto);
}

void plutovg_stroke(plutovg_t* pluto)
{
    plutovg_stroke_preserve(pluto);
    plutovg_new_path(pluto);
}

void plutovg_clip(plutovg_t* pluto)
{
    plutovg_clip_preserve(pluto);
    plutovg_new_path(pluto);
}

void plutovg_paint(plutovg_t* pluto)
{
    plutovg_state_t* state = pluto->state;
    if(state->clippath==NULL && pluto->clippath==NULL)
    {
        plutovg_path_t* path = plutovg_path_create();
        plutovg_path_add_rect(path, pluto->clip.x, pluto->clip.y, pluto->clip.w, pluto->clip.h);
        plutovg_matrix_t matrix;
        plutovg_matrix_init_identity(&matrix);
        pluto->clippath = plutovg_rle_create();
        plutovg_rle_rasterize(pluto->clippath, path, &matrix, &pluto->clip, NULL, plutovg_fill_rule_non_zero);
        plutovg_path_destroy(path);
    }

    plutovg_rle_t* rle = state->clippath ? state->clippath : pluto->clippath;
    plutovg_blend(pluto, rle);
}

void plutovg_fill_preserve(plutovg_t* pluto)
{
    plutovg_state_t* state = pluto->state;
    plutovg_rle_clear(pluto->rle);
    plutovg_rle_rasterize(pluto->rle, pluto->path, &state->matrix, &pluto->clip, NULL, state->winding);
    plutovg_rle_clip_path(pluto->rle, state->clippath);
    plutovg_blend(pluto, pluto->rle);
}

void plutovg_stroke_preserve(plutovg_t* pluto)
{
    plutovg_state_t* state = pluto->state;
    plutovg_rle_clear(pluto->rle);
    plutovg_rle_rasterize(pluto->rle, pluto->path, &state->matrix, &pluto->clip, &state->stroke, plutovg_fill_rule_non_zero);
    plutovg_rle_clip_path(pluto->rle, state->clippath);
    plutovg_blend(pluto, pluto->rle);
}

void plutovg_clip_preserve(plutovg_t* pluto)
{
    plutovg_state_t* state = pluto->state;
    if(state->clippath)
    {
        plutovg_rle_clear(pluto->rle);
        plutovg_rle_rasterize(pluto->rle, pluto->path, &state->matrix, &pluto->clip, NULL, state->winding);
        plutovg_rle_clip_path(state->clippath, pluto->rle);
    }
    else
    {
        state->clippath = plutovg_rle_create();
        plutovg_rle_rasterize(state->clippath, pluto->path, &state->matrix, &pluto->clip, NULL, state->winding);
    }
}

void plutovg_fill_extents(plutovg_t* pluto, double* x, double* y, double* w, double* h)
{
    plutovg_state_t* state = pluto->state;
    plutovg_rle_clear(pluto->rle);
    plutovg_rle_rasterize(pluto->rle, pluto->path, &state->matrix, NULL, NULL, state->winding);
    if(x) *x = pluto->rle->x;
    if(y) *y = pluto->rle->y;
    if(w) *w = pluto->rle->w;
    if(h) *h = pluto->rle->h;
}

void plutovg_stroke_extents(plutovg_t* pluto, double* x, double* y, double* w, double* h)
{
    plutovg_state_t* state = pluto->state;
    plutovg_rle_clear(pluto->rle);
    plutovg_rle_rasterize(pluto->rle, pluto->path, &state->matrix, NULL, &state->stroke, plutovg_fill_rule_non_zero);
    if(x) *x = pluto->rle->x;
    if(y) *y = pluto->rle->y;
    if(w) *w = pluto->rle->w;
    if(h) *h = pluto->rle->h;
}

void plutovg_clip_extents(plutovg_t* pluto, double* x, double* y, double* w, double* h)
{
    plutovg_state_t* state = pluto->state;
    if(x) *x = state->clippath ? state->clippath->x : pluto->clip.x;
    if(y) *y = state->clippath ? state->clippath->y : pluto->clip.y;
    if(w) *w = state->clippath ? state->clippath->w : pluto->clip.w;
    if(h) *h = state->clippath ? state->clippath->h : pluto->clip.h;
}

void plutovg_reset_clip(plutovg_t* pluto)
{
    plutovg_rle_destroy(pluto->state->clippath);
    pluto->state->clippath = NULL;
}
