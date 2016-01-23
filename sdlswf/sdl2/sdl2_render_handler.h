#ifndef Z_SDL2_RENDER_HANDLER_H
#define Z_SDL2_RENDER_HANDLER_H

#include "SDL.h"
#include "SDL_gpu.h"
#include "gameswf/gameswf.h"
#include "gameswf/gameswf_types.h"
#include "base/image.h"
#include "base/utility.h"

// Style state.
enum style_index
{
	LEFT_STYLE = 0,
	RIGHT_STYLE,
	LINE_STYLE,

	STYLE_COUNT
};

struct fill_style
{
	enum mode
	{
		INVALID,
		COLOR,
		BITMAP_WRAP,
		BITMAP_CLAMP,
		LINEAR_GRADIENT,
		RADIAL_GRADIENT,
	};

	mode	m_mode;
	gameswf::rgba	m_color;
	gameswf::bitmap_info*	m_bitmap_info;
	gameswf::matrix	m_bitmap_matrix;
	gameswf::cxform	m_bitmap_color_transform;
	bool	m_has_nonzero_bitmap_additive_color;
	float m_width;	// for line style

	fill_style();

	void	apply(/*const matrix& current_matrix*/) const;


	bool	needs_second_pass() const;
	// Return true if we need to do a second pass to make
	// a valid color.  This is for cxforms with additive
	// parts; this is the simplest way (that we know of)
	// to implement an additive color with stock OpenGL.


	void	apply_second_pass() const;
	// Set OpenGL state for a necessary second pass.

	void	cleanup_second_pass() const;


	void	disable() { m_mode = INVALID; }
	void	set_color(gameswf::rgba color) { m_mode = COLOR; m_color = color; }
	void	set_bitmap(gameswf::bitmap_info* bi, const gameswf::matrix& m, gameswf::render_handler::bitmap_wrap_mode wm, const gameswf::cxform& color_transform);
	bool	is_valid() const { return m_mode != INVALID; }
};

class SDL2RenderHandler : public gameswf::render_handler
{
	
public:	
	SDL2RenderHandler(GPU_Target* r);
	~SDL2RenderHandler();

	// Your handler should return these with a ref-count of 0.  (@@ is that the right policy?)
	virtual gameswf::bitmap_info*	create_bitmap_info_empty();	// used when DO_NOT_LOAD_BITMAPS is set
	virtual gameswf::bitmap_info*	create_bitmap_info_alpha(int w, int h, unsigned char* data);
	virtual gameswf::bitmap_info*	create_bitmap_info_rgb(image::rgb* im);
	virtual gameswf::bitmap_info*	create_bitmap_info_rgba(image::rgba* im);
	virtual gameswf::video_handler*	create_video_handler();

	// Bracket the displaying of a frame from a movie.
	// Fill the background color, and set up default
	// transforms, etc.
	virtual void	begin_display(
		gameswf::rgba background_color,
		int viewport_x0, int viewport_y0,
		int viewport_width, int viewport_height,
		float x0, float x1, float y0, float y1);
	virtual void	end_display();

	// Geometric and color transforms for mesh and line_strip rendering.
	virtual void	set_matrix(const gameswf::matrix& m);
	virtual void	set_cxform(const gameswf::cxform& cx);

	void			draw_mesh_primitive(int primitive_type, const void* coords, int vertex_count);
	// Draw triangles using the current fill-style 0.
	// Clears the style list after rendering.
	//
	// coords is a list of (x,y) coordinate pairs, in
	// triangle-strip order.  The type of the array should
	// be Sint16[vertex_count*2]
	virtual void	draw_mesh_strip(const void* coords, int vertex_count);
	// As above, but coords is in triangle list order.
	virtual void	draw_triangle_list(const void* coords, int vertex_count);

	// Draw a line-strip using the current line style.
	// Clear the style list after rendering.
	//
	// Coords is a list of (x,y) coordinate pairs, in
	// sequence.  Each coord is a 16-bit signed integer.
	virtual void	draw_line_strip(const void* coords, int vertex_count);

	virtual void	fill_style_disable(int fill_side);
	virtual void	fill_style_color(int fill_side, const gameswf::rgba& color);
	virtual void	fill_style_bitmap(int fill_side, gameswf::bitmap_info* bi, const gameswf::matrix& m,
		bitmap_wrap_mode wm, bitmap_blend_mode bm);

	virtual void	line_style_disable();
	virtual void	line_style_color(gameswf::rgba color);
	virtual void	line_style_width(float width);

	// Special function to draw a rectangular bitmap;
	// intended for textured glyph rendering.  Ignores
	// current transforms.
	virtual void	draw_bitmap(
		const gameswf::matrix&		m,
		gameswf::bitmap_info*	bi,
		const gameswf::rect&		coords,
		const gameswf::rect&		uv_coords,
		gameswf::rgba			color);
	virtual void	set_antialiased(bool enable);

	virtual bool test_stencil_buffer(const gameswf::rect& bound, Uint8 pattern);
	virtual void begin_submit_mask();
	virtual void end_submit_mask();
	virtual void disable_mask();

	virtual bool is_visible(const gameswf::rect& bound);
	virtual void open();

	//
	void		 applyColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	GPU_Target*	 getTarget() { return _target; };
private:
	GPU_Target*			_target;

	gameswf::matrix		_currentMatrix;
	gameswf::cxform		_currentCXForm;

	fill_style			_current_styles[STYLE_COUNT];
};

// bitmap_info_ogl declaration
struct bitmap_info_sdl2 : public gameswf::bitmap_info
{
	int m_width;
	int m_height;
	GPU_Image*  m_texture;
	GPU_Target* m_renderer;
	image::image_base* m_suspended_image;

	//bitmap_info_sdl2();
	bitmap_info_sdl2(GPU_Target*, int width, int height, Uint8* data);
	bitmap_info_sdl2(GPU_Target*, image::rgb* im);
	bitmap_info_sdl2(GPU_Target*, image::rgba* im);

	//virtual void layout();

	//virtual void activate();

	~bitmap_info_sdl2();

	virtual unsigned char* get_data() const { return (unsigned char*)m_texture; }
	virtual int get_width() const { return m_width; }
	virtual int get_height() const { return m_height; }

};
#endif