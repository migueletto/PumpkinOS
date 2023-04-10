#ifndef PLUTOVG_H
#define PLUTOVG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @note plutovg_surface_t format is ARGB32_Premultiplied.
 */
typedef struct plutovg_surface plutovg_surface_t;

plutovg_surface_t* plutovg_surface_create(int width, int height);
plutovg_surface_t* plutovg_surface_create_for_data(unsigned char* data, int width, int height, int stride);
plutovg_surface_t* plutovg_surface_reference(plutovg_surface_t* surface);
void plutovg_surface_destroy(plutovg_surface_t* surface);
int plutovg_surface_get_reference_count(const plutovg_surface_t* surface);
unsigned char* plutovg_surface_get_data(const plutovg_surface_t* surface);
int plutovg_surface_get_width(const plutovg_surface_t* surface);
int plutovg_surface_get_height(const plutovg_surface_t* surface);
int plutovg_surface_get_stride(const plutovg_surface_t* surface);
void plutovg_surface_write_to_png(const plutovg_surface_t* surface, const char* filename);
void plutovg_surface_write_to_memory(const plutovg_surface_t *surface,
    void (*set)(int x, int y, int r, int g, int b, int a, void *data), void *data);

typedef struct {
    double x;
    double y;
} plutovg_point_t;

typedef struct {
    double x;
    double y;
    double w;
    double h;
} plutovg_rect_t;

typedef struct {
    double m00; double m10;
    double m01; double m11;
    double m02; double m12;
} plutovg_matrix_t;

void plutovg_matrix_init(plutovg_matrix_t* matrix, double m00, double m10, double m01, double m11, double m02, double m12);
void plutovg_matrix_init_identity(plutovg_matrix_t* matrix);
void plutovg_matrix_init_translate(plutovg_matrix_t* matrix, double x, double y);
void plutovg_matrix_init_scale(plutovg_matrix_t* matrix, double x, double y);
void plutovg_matrix_init_shear(plutovg_matrix_t* matrix, double x, double y);
void plutovg_matrix_init_rotate(plutovg_matrix_t* matrix, double radians);
void plutovg_matrix_init_rotate_translate(plutovg_matrix_t* matrix, double radians, double x, double y);
void plutovg_matrix_translate(plutovg_matrix_t* matrix, double x, double y);
void plutovg_matrix_scale(plutovg_matrix_t* matrix, double x, double y);
void plutovg_matrix_shear(plutovg_matrix_t* matrix, double x, double y);
void plutovg_matrix_rotate(plutovg_matrix_t* matrix, double radians);
void plutovg_matrix_rotate_translate(plutovg_matrix_t* matrix, double radians, double x, double y);
void plutovg_matrix_multiply(plutovg_matrix_t* matrix, const plutovg_matrix_t* a, const plutovg_matrix_t* b);
int plutovg_matrix_invert(plutovg_matrix_t* matrix);
void plutovg_matrix_map(const plutovg_matrix_t* matrix, double x, double y, double* _x, double* _y);
void plutovg_matrix_map_point(const plutovg_matrix_t* matrix, const plutovg_point_t* src, plutovg_point_t* dst);
void plutovg_matrix_map_rect(const plutovg_matrix_t* matrix, const plutovg_rect_t* src, plutovg_rect_t* dst);

typedef struct plutovg_path plutovg_path_t;

typedef enum {
    plutovg_path_element_move_to,
    plutovg_path_element_line_to,
    plutovg_path_element_cubic_to,
    plutovg_path_element_close
} plutovg_path_element_t;

plutovg_path_t* plutovg_path_create(void);
plutovg_path_t* plutovg_path_reference(plutovg_path_t* path);
void plutovg_path_destroy(plutovg_path_t* path);
int plutovg_path_get_reference_count(const plutovg_path_t* path);

void plutovg_path_move_to(plutovg_path_t* path, double x, double y);
void plutovg_path_line_to(plutovg_path_t* path, double x, double y);
void plutovg_path_quad_to(plutovg_path_t* path, double x1, double y1, double x2, double y2);
void plutovg_path_cubic_to(plutovg_path_t* path, double x1, double y1, double x2, double y2, double x3, double y3);
void plutovg_path_arc_to(plutovg_path_t* path, double x1, double y1, double x2, double y2, double radius);
void plutovg_path_close(plutovg_path_t* path);

void plutovg_path_rel_move_to(plutovg_path_t* path, double dx, double dy);
void plutovg_path_rel_line_to(plutovg_path_t* path, double dx, double dy);
void plutovg_path_rel_quad_to(plutovg_path_t* path, double dx1, double dy1, double dx2, double dy2);
void plutovg_path_rel_cubic_to(plutovg_path_t* path, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3);
void plutovg_path_rel_arc_to(plutovg_path_t* path, double dx1, double dy1, double dx2, double dy2, double radius);

void plutovg_path_add_rect(plutovg_path_t* path, double x, double y, double w, double h);
void plutovg_path_add_round_rect(plutovg_path_t* path, double x, double y, double w, double h, double rx, double ry);
void plutovg_path_add_ellipse(plutovg_path_t* path, double cx, double cy, double rx, double ry);
void plutovg_path_add_circle(plutovg_path_t* path, double cx, double cy, double r);
void plutovg_path_add_arc(plutovg_path_t* path, double cx, double cy, double r, double a0, double a1, int ccw);
void plutovg_path_add_path(plutovg_path_t* path, const plutovg_path_t* source, const plutovg_matrix_t* matrix);

void plutovg_path_transform(plutovg_path_t* path, const plutovg_matrix_t* matrix);
void plutovg_path_get_current_point(const plutovg_path_t* path, double* x, double* y);
int plutovg_path_get_element_count(const plutovg_path_t* path);
plutovg_path_element_t* plutovg_path_get_elements(const plutovg_path_t* path);
int plutovg_path_get_point_count(const plutovg_path_t* path);
plutovg_point_t* plutovg_path_get_points(const plutovg_path_t* path);
void plutovg_path_clear(plutovg_path_t* path);
int plutovg_path_empty(const plutovg_path_t* path);
plutovg_path_t* plutovg_path_clone(const plutovg_path_t* path);
plutovg_path_t* plutovg_path_clone_flat(const plutovg_path_t* path);

typedef struct {
    double r;
    double g;
    double b;
    double a;
} plutovg_color_t;

void plutovg_color_init_rgb(plutovg_color_t* color, double r, double g, double b);
void plutovg_color_init_rgba(plutovg_color_t* color, double r, double g, double b, double a);
void plutovg_color_init_rgb8(plutovg_color_t* color, unsigned char r, unsigned char g, unsigned char b);
void plutovg_color_init_rgba8(plutovg_color_t* color, unsigned char r, unsigned char g, unsigned char b, unsigned char a);

typedef enum {
    plutovg_spread_method_pad,
    plutovg_spread_method_reflect,
    plutovg_spread_method_repeat
} plutovg_spread_method_t;

typedef struct plutovg_gradient plutovg_gradient_t;

typedef enum {
    plutovg_gradient_type_linear,
    plutovg_gradient_type_radial
} plutovg_gradient_type_t;

typedef struct {
    double offset;
    plutovg_color_t color;
} plutovg_gradient_stop_t;

plutovg_gradient_t* plutovg_gradient_create_linear(double x1, double y1, double x2, double y2);
plutovg_gradient_t* plutovg_gradient_create_radial(double cx, double cy, double cr, double fx, double fy, double fr);
plutovg_gradient_t* plutovg_gradient_reference(plutovg_gradient_t* gradient);
void plutovg_gradient_destroy(plutovg_gradient_t* gradient);
int plutovg_gradient_get_reference_count(const plutovg_gradient_t* gradient);
void plutovg_gradient_set_spread(plutovg_gradient_t* gradient, plutovg_spread_method_t spread);
plutovg_spread_method_t plutovg_gradient_get_spread(const plutovg_gradient_t* gradient);
void plutovg_gradient_set_matrix(plutovg_gradient_t* gradient, const plutovg_matrix_t* matrix);
void plutovg_gradient_get_matrix(const plutovg_gradient_t* gradient, plutovg_matrix_t* matrix);
void plutovg_gradient_add_stop_rgb(plutovg_gradient_t* gradient, double offset, double r, double g, double b);
void plutovg_gradient_add_stop_rgba(plutovg_gradient_t* gradient, double offset, double r, double g, double b, double a);
void plutovg_gradient_add_stop_color(plutovg_gradient_t* gradient, double offset, const plutovg_color_t* color);
void plutovg_gradient_add_stop(plutovg_gradient_t* gradient, const plutovg_gradient_stop_t* stop);
void plutovg_gradient_clear_stops(plutovg_gradient_t* gradient);
int plutovg_gradient_get_stop_count(const plutovg_gradient_t* gradient);
plutovg_gradient_stop_t* plutovg_gradient_get_stops(const plutovg_gradient_t* gradient);
plutovg_gradient_type_t plutovg_gradient_get_type(const plutovg_gradient_t* gradient);
void plutovg_gradient_get_values_linear(const plutovg_gradient_t* gradient, double* x1, double* y1, double* x2, double* y2);
void plutovg_gradient_get_values_radial(const plutovg_gradient_t* gradient, double* cx, double* cy, double* cr, double* fx, double* fy, double* fr);
void plutovg_gradient_set_values_linear(plutovg_gradient_t* gradient, double x1, double y1, double x2, double y2);
void plutovg_gradient_set_values_radial(plutovg_gradient_t* gradient, double cx, double cy, double cr, double fx, double fy, double fr);
void plutovg_gradient_set_opacity(plutovg_gradient_t* paint, double opacity);
double plutovg_gradient_get_opacity(const plutovg_gradient_t* paint);

typedef struct plutovg_texture plutovg_texture_t;

typedef enum {
    plutovg_texture_type_plain,
    plutovg_texture_type_tiled
} plutovg_texture_type_t;

plutovg_texture_t* plutovg_texture_create(plutovg_surface_t* surface);
plutovg_texture_t* plutovg_texture_reference(plutovg_texture_t* texture);
void plutovg_texture_destroy(plutovg_texture_t* texture);
int plutovg_texture_get_reference_count(const plutovg_texture_t* texture);
void plutovg_texture_set_type(plutovg_texture_t* texture, plutovg_texture_type_t type);
plutovg_texture_type_t plutovg_texture_get_type(const plutovg_texture_t* texture);
void plutovg_texture_set_matrix(plutovg_texture_t* texture, const plutovg_matrix_t* matrix);
void plutovg_texture_get_matrix(const plutovg_texture_t* texture, plutovg_matrix_t* matrix);
void plutovg_texture_set_surface(plutovg_texture_t* texture, plutovg_surface_t* surface);
plutovg_surface_t* plutovg_texture_get_surface(const plutovg_texture_t* texture);
void plutovg_texture_set_opacity(plutovg_texture_t* texture, double opacity);
double plutovg_texture_get_opacity(const plutovg_texture_t* texture);

typedef struct plutovg_paint plutovg_paint_t;

typedef enum {
    plutovg_paint_type_color,
    plutovg_paint_type_gradient,
    plutovg_paint_type_texture
} plutovg_paint_type_t;

plutovg_paint_t* plutovg_paint_create_rgb(double r, double g, double b);
plutovg_paint_t* plutovg_paint_create_rgba(double r, double g, double b, double a);
plutovg_paint_t* plutovg_paint_create_linear(double x1, double y1, double x2, double y2);
plutovg_paint_t* plutovg_paint_create_radial(double cx, double cy, double cr, double fx, double fy, double fr);
plutovg_paint_t* plutovg_paint_create_for_surface(plutovg_surface_t* surface);
plutovg_paint_t* plutovg_paint_create_color(const plutovg_color_t* color);
plutovg_paint_t* plutovg_paint_create_gradient(plutovg_gradient_t* gradient);
plutovg_paint_t* plutovg_paint_create_texture(plutovg_texture_t* texture);
plutovg_paint_t* plutovg_paint_reference(plutovg_paint_t* paint);
void plutovg_paint_destroy(plutovg_paint_t* paint);
int plutovg_paint_get_reference_count(const plutovg_paint_t* paint);
plutovg_paint_type_t plutovg_paint_get_type(const plutovg_paint_t* paint);
plutovg_color_t* plutovg_paint_get_color(const plutovg_paint_t* paint);
plutovg_gradient_t* plutovg_paint_get_gradient(const plutovg_paint_t* paint);
plutovg_texture_t* plutovg_paint_get_texture(const plutovg_paint_t* paint);

typedef struct plutovg_glyph plutovg_glyph_t;

int plutovg_glyph_get_codepoint(const plutovg_glyph_t* glyph);
double plutovg_glyph_get_advance(const plutovg_glyph_t* glyph);
void plutovg_glyph_get_extents(const plutovg_glyph_t* glyph, double* x1, double* y1, double* x2, double* y2);
const plutovg_path_t* plutovg_glyph_get_path(const plutovg_glyph_t* glyph);

typedef struct plutovg_font plutovg_font_t;

plutovg_font_t* plutovg_font_load_from_memory(unsigned char* data, int owndata);
plutovg_font_t* plutovg_font_load_from_file(const char* filename);
plutovg_font_t* plutovg_font_reference(plutovg_font_t* font);
void plutovg_font_destroy(plutovg_font_t* font);
int plutovg_font_get_reference_count(const plutovg_font_t* font);
const plutovg_glyph_t* plutovg_font_get_glyph(plutovg_font_t* font, int codepoint);
double plutovg_font_get_scale(const plutovg_font_t* font, double size);
double plutovg_font_get_ascent(const plutovg_font_t* font);
double plutovg_font_get_descent(const plutovg_font_t* font);
double plutovg_font_get_line_gap(const plutovg_font_t* font);
double plutovg_font_get_leading(const plutovg_font_t* font);
void plutovg_font_get_extents(const plutovg_font_t* font, double* x1, double* y1, double* x2, double* y2);

typedef enum {
    plutovg_line_cap_butt,
    plutovg_line_cap_round,
    plutovg_line_cap_square
} plutovg_line_cap_t;

typedef enum {
    plutovg_line_join_miter,
    plutovg_line_join_round,
    plutovg_line_join_bevel
} plutovg_line_join_t;

typedef enum {
    plutovg_fill_rule_non_zero,
    plutovg_fill_rule_even_odd
} plutovg_fill_rule_t;

typedef enum {
    plutovg_operator_src,
    plutovg_operator_src_over,
    plutovg_operator_dst_in,
    plutovg_operator_dst_out
} plutovg_operator_t;

typedef struct plutovg plutovg_t;

/**
 * @brief Creates a new rendering context for rendering to surface target.
 * @param surface - a target surface for the context
 * @return Returns a newly allocated context with a reference count of 1.
 */
plutovg_t* plutovg_create(plutovg_surface_t* surface);

/**
 * @brief Increases the reference count of the context by 1.
 * @param pluto - a pluto context
 * @return Returns the referenced context.
 */
plutovg_t* plutovg_reference(plutovg_t* pluto);

/**
 * @brief Decrements the reference count for the context by 1.
 * If the reference count on the context falls to 0, the context is freed.
 * @param pluto - a pluto context
 */
void plutovg_destroy(plutovg_t* pluto);

/**
 * @brief Gets the reference count of the context.
 * @param pluto - a pluto context
 * @return Returns the reference count of the context.
 */
int plutovg_get_reference_count(const plutovg_t* pluto);

/**
 * @brief Saves the entire state of the context by pushing the current state onto a stack.
 * @param pluto - a pluto context
 */
void plutovg_save(plutovg_t* pluto);

/**
 * @brief Restores the most recently saved canvas state by popping the top entry in the drawing state stack.
 * @param pluto - a pluto context
 */
void plutovg_restore(plutovg_t* pluto);

/**
 * @brief Sets the source paint of the context to an opaque color.
 * The color components are floating point numbers in the range 0 to 1.
 * @param pluto - a pluto context
 * @param r - red component of color
 * @param g - green component of color
 * @param b - blue component of color
 */
void plutovg_set_source_rgb(plutovg_t* pluto, double r, double g, double b);

/**
 * @brief Sets the source paint of the context to a translucent color.
 * The color and alpha components are floating point numbers in the range 0 to 1.
 * @param pluto - a pluto context
 * @param r - red component of color
 * @param g - green component of color
 * @param b - blue component of color
 * @param a - alpha component of color
 */
void plutovg_set_source_rgba(plutovg_t* pluto, double r, double g, double b, double a);

/**
 * @brief Sets the source paint of the context to a texture.
 * @param pluto - a pluto context
 * @param surface - a surface to be used to set the source paint
 * @param x - user-space x coordinate for surface origin
 * @param y - user-space y coordinate for surface origin
 */
void plutovg_set_source_surface(plutovg_t* pluto, plutovg_surface_t* surface, double x, double y);

/**
 * @brief Sets the source paint of the context to a color.
 * @param pluto - a pluto context
 * @param color - a color to be used to set the source paint
 */
void plutovg_set_source_color(plutovg_t* pluto, const plutovg_color_t* color);

/**
 * @brief Sets the source paint of the context to a gradient.
 * @param pluto - a pluto context
 * @param gradient - a gradient to be used to set the source paint
 */
void plutovg_set_source_gradient(plutovg_t* pluto, plutovg_gradient_t* gradient);

/**
 * @brief Sets the source paint of the context to a texture.
 * @param pluto - a pluto context
 * @param texture - a texture to be used to set the source paint
 */
void plutovg_set_source_texture(plutovg_t* pluto, plutovg_texture_t* texture);

/**
 * @brief Sets the source paint of the context to a paint.
 * @param pluto - a pluto context
 * @param source - a paint to be used a the source for any subsequent drawing operation.
 */
void plutovg_set_source(plutovg_t* pluto, plutovg_paint_t* source);

/**
 * @brief Gets the current source paint.
 * @param pluto - a pluto context
 * @return The current source paint
 */
plutovg_paint_t* plutovg_get_source(const plutovg_t* pluto);

/**
 * @brief Sets the compositing operator to be used for drawing operations.
 * @param pluto - a pluto context
 * @param op - a compositing operator
 */
void plutovg_set_operator(plutovg_t* pluto, plutovg_operator_t op);

/**
 * @brief Sets the opacity value that is applied to paints before they are drawn onto the context.
 * @param pluto - a pluto context
 * @param opacity - an opacity value is a floating point number in the range 0 to 1
 */
void plutovg_set_opacity(plutovg_t* pluto, double opacity);

/**
 * @brief Sets the fill rule to be used for filling and clipping operations.
 * @param pluto - a pluto context
 * @param winding - a fill rule
 */
void plutovg_set_fill_rule(plutovg_t* pluto, plutovg_fill_rule_t winding);

/**
 * @brief Gets the current compositing operator.
 * @param pluto - a pluto context
 * @return Returns the current compositing operator.
 */
plutovg_operator_t plutovg_get_operator(const plutovg_t* pluto);

/**
 * @brief Gets the current opacity.
 * @param pluto - a pluto context
 * @return Returns the current opacity.
 */
double plutovg_get_opacity(const plutovg_t* pluto);

/**
 * @brief Gets the current fill rule.
 * @param pluto - a pluto context
 * @return Returns the current fill rule.
 */
plutovg_fill_rule_t plutovg_get_fill_rule(const plutovg_t* pluto);

/**
 * @brief Sets the current line width.
 * @param pluto - a pluto context
 * @param width - a line width
 */
void plutovg_set_line_width(plutovg_t* pluto, double width);

/**
 * @brief Sets the current line cap.
 * @param pluto - a pluto context
 * @param cap - a line cap style
 */
void plutovg_set_line_cap(plutovg_t* pluto, plutovg_line_cap_t cap);

/**
 * @brief Sets the current line join.
 * @param pluto - a pluto context
 * @param cap - a line join style
 */
void plutovg_set_line_join(plutovg_t* pluto, plutovg_line_join_t join);

/**
 * @brief Sets the current miter limit.
 * @param pluto - a pluto context
 * @param limit - a miter limit
 */
void plutovg_set_miter_limit(plutovg_t* pluto, double limit);

/**
 * @brief Sets the current dash pattern.
 * @param pluto - a pluto context
 * @param offset - an offset into the dash pattern at which the stroke start
 * @param data - an array specifying alternate lengths of lines and gaps which describe the pattern
 * @param size - the length of the data array
 */
void plutovg_set_dash(plutovg_t* pluto, double offset, const double* data, int size);

/**
* @brief Gets the current line width.
* @param pluto - a pluto context
* @return Returns the current line width.
 */
double plutovg_get_line_width(const plutovg_t* pluto);

/**
* @brief Gets the current line cap.
* @param pluto - a pluto context
* @return Returns the current line cap.
 */
plutovg_line_cap_t plutovg_get_line_cap(const plutovg_t* pluto);

/**
* @brief Gets the current line join.
* @param pluto - a pluto context
* @return Returns the current line join.
 */
plutovg_line_join_t plutovg_get_line_join(const plutovg_t* pluto);

/**
 * @brief Gets the current miter limit.
 * @param pluto - a pluto context
 * @return Returns the current miter limit.
*/
double plutovg_get_miter_limit(const plutovg_t* pluto);

double plutovg_get_dash_offset(plutovg_t* pluto);
double* plutovg_get_dash_data(plutovg_t* pluto);
int plutovg_get_dash_count(plutovg_t* pluto);

/**
 * @brief Adds a translation transformation to the current matrix.
 * @param pluto - a pluto context
 * @param x - amount to translate in the x direction
 * @param y - amount to translate in the y direction
 */
void plutovg_translate(plutovg_t* pluto, double x, double y);

/**
 * @brief Adds a scale transformation to the current matrix.
 * @param pluto - a pluto context
 * @param x - scale factor for x dimension
 * @param y - scale factor for y dimension
 */
void plutovg_scale(plutovg_t* pluto, double x, double y);

/**
 * @brief Adds a rotation transformation to the current matrix.
 * @param pluto - a pluto context
 * @param radians - angle in radians by which the matrix will be rotated.
 */
void plutovg_rotate(plutovg_t* pluto, double radians);

/**
 * @brief Adds a transformation matrix to the current matrix.
 * @param pluto - a pluto context
 * @param matrix - a transformation matrix
 */
void plutovg_transform(plutovg_t* pluto, const plutovg_matrix_t* matrix);

/**
 * @brief Sets the current matrix.
 * @param pluto - a pluto context
 * @param matrix - a transformation matrix
 */
void plutovg_set_matrix(plutovg_t* pluto, const plutovg_matrix_t* matrix);

/**
 * @brief Resets the current matrix to identity
 * @param pluto - a pluto context
 */
void plutovg_identity_matrix(plutovg_t* pluto);

/**
 * @brief Gets the current matrix.
 * @param pluto - a pluto context
 * @return Returns the current matrix.
 */
void plutovg_get_matrix(const plutovg_t* pluto, plutovg_matrix_t* matrix);

/**
 * @brief Begins a new sub-path at the point specified by the given coordinates.
 * @param pluto - a pluto context
 * @param x - the x coordinate of the new position
 * @param y - the y coordinate of the new position
 */
void plutovg_move_to(plutovg_t* pluto, double x, double y);

/**
 * @brief Adds a straight line to the current sub-path by connecting the sub-path's last point to the given coordinates.
 * @param pluto - a pluto context
 * @param x - the x coordinate of the line's end point
 * @param y - the y coordinate of the line's end point
 */
void plutovg_line_to(plutovg_t* pluto, double x, double y);

/**
 * @brief Adds a quadratic Bézier curve to the current sub-path.
 * @param pluto - a pluto context
 * @param x1 - the x coordinate of the control point
 * @param y1 - the y coordinate of the control point
 * @param x2 - the x coordinate of the end point
 * @param y2 - the y coordinate of the end point
 */
void plutovg_quad_to(plutovg_t* pluto, double x1, double y1, double x2, double y2);

/**
 * @brief Adds a cubic Bézier curve to the current sub-path.
 * @param pluto - a pluto context
 * @param x1 - the x coordinate of the first control point
 * @param y1 - the y coordinate of the first control point
 * @param x2 - the x coordinate of the second control point
 * @param y2 - the y coordinate of the second control point
 * @param x3 - the x coordinate of the end point
 * @param y3 - the y coordinate of the end point
 */
void plutovg_cubic_to(plutovg_t* pluto, double x1, double y1, double x2, double y2, double x3, double y3);

void plutovg_rel_move_to(plutovg_t* pluto, double dx, double dy);
void plutovg_rel_line_to(plutovg_t* pluto, double dx, double dy);
void plutovg_rel_quad_to(plutovg_t* pluto, double dx1, double dy1, double dx2, double dy2);
void plutovg_rel_cubic_to(plutovg_t* pluto, double dx1, double dy1, double dx2, double dy2, double dx3, double dy3);

void plutovg_rect(plutovg_t* pluto, double x, double y, double w, double h);
void plutovg_round_rect(plutovg_t* pluto, double x, double y, double w, double h, double rx, double ry);
void plutovg_ellipse(plutovg_t* pluto, double cx, double cy, double rx, double ry);
void plutovg_circle(plutovg_t* pluto, double cx, double cy, double r);
void plutovg_arc(plutovg_t* path, double cx, double cy, double r, double a0, double a1, int ccw);
void plutovg_add_path(plutovg_t* pluto, const plutovg_path_t* path);
void plutovg_new_path(plutovg_t* pluto);
void plutovg_close_path(plutovg_t* pluto);

void plutovg_get_current_point(const plutovg_t* pluto, double* x, double* y);
plutovg_path_t* plutovg_get_path(const plutovg_t* pluto);

void plutovg_set_font(plutovg_t* pluto, plutovg_font_t* font);
void plutovg_set_font_size(plutovg_t* pluto, double size);
plutovg_font_t* plutovg_get_font(const plutovg_t* pluto);
double plutovg_get_font_size(const plutovg_t* pluto);

void plutovg_text(plutovg_t* pluto, const char* utf8, double x, double y);
void plutovg_char(plutovg_t* pluto, int ch, double x, double y);
void plutovg_text_extents(plutovg_t* pluto, const char* utf8, double* w, double* h);

void plutovg_fill(plutovg_t* pluto);
void plutovg_stroke(plutovg_t* pluto);
void plutovg_clip(plutovg_t* pluto);
void plutovg_paint(plutovg_t* pluto);

void plutovg_fill_preserve(plutovg_t* pluto);
void plutovg_stroke_preserve(plutovg_t* pluto);
void plutovg_clip_preserve(plutovg_t* pluto);

/**
 * @brief Computes a bounding box in user coordinates covering the area inside the current fill.
 * @param pluto - a pluto context
 * @param x - the x coordinate of the bounding box
 * @param y - the y coordinate of the bounding box
 * @param w - the width of the bounding box
 * @param h - the height of the bounding box
 */
void plutovg_fill_extents(plutovg_t* pluto, double* x, double* y, double* w, double* h);

/**
 * @brief Computes a bounding box in user coordinates covering the area inside the current stroke.
 * @param pluto - a pluto context
 * @param x - the x coordinate of the bounding box
 * @param y - the y coordinate of the bounding box
 * @param w - the width of the bounding box
 * @param h - the height of the bounding box
 */
void plutovg_stroke_extents(plutovg_t* pluto, double* x, double* y, double* w, double* h);

/**
 * @brief Computes a bounding box in user coordinates covering the area inside the current clip.
 * @param pluto - a pluto context
 * @param x - the x coordinate of the bounding box
 * @param y - the y coordinate of the bounding box
 * @param w - the width of the bounding box
 * @param h - the height of the bounding box
 */
void plutovg_clip_extents(plutovg_t* pluto, double* x, double* y, double* w, double* h);

/**
 * @brief Reset the current clip region to its original, unrestricted state.
 * @param pluto - a pluto context
 */
void plutovg_reset_clip(plutovg_t* pluto);

#ifdef __cplusplus
}
#endif

#endif // PLUTOVG_H
