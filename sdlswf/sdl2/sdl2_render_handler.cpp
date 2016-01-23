#include "sdl2_render_handler.h"

#include "gameswf/gameswf.h"
#include "gameswf/gameswf_types.h"
#include "base/image.h"
#include "base/utility.h"

#include <vector>

SDL2RenderHandler* _renderInst;

fill_style::fill_style() :
m_mode(INVALID),
m_has_nonzero_bitmap_additive_color(false)
{
}

void fill_style::apply(/*const matrix& current_matrix*/) const
// Push our style into OpenGL.
{
	assert(m_mode != INVALID);

	if (m_mode == COLOR)
	{
		_renderInst->applyColor(m_color.m_r, m_color.m_g, m_color.m_b, m_color.m_a);
	}
	else
	if (m_mode == BITMAP_WRAP || m_mode == BITMAP_CLAMP)
	{
		assert(m_bitmap_info != NULL);
		_renderInst->applyColor(m_color.m_r, m_color.m_g, m_color.m_b, m_color.m_a);

		if (m_bitmap_info == NULL)
		{
		}
		else
		{
			m_bitmap_info->layout();
			GPU_Image* image = (GPU_Image*)m_bitmap_info->get_data();
			if (m_mode == BITMAP_CLAMP)
			{
				GPU_SetWrapMode(image, GPU_WRAP_NONE, GPU_WRAP_NONE);
			}
			else
			{
				assert(m_mode == BITMAP_WRAP);
				GPU_SetWrapMode(image, GPU_WRAP_REPEAT, GPU_WRAP_REPEAT);
			}

			// TODO:
			float x = TWIPS_TO_PIXELS(m_bitmap_matrix.m_[0][2]);
			float y = TWIPS_TO_PIXELS(m_bitmap_matrix.m_[1][2]);
			float rotation = m_bitmap_matrix.get_rotation()/3.14 * 180;
			float x_scale = m_bitmap_matrix.get_x_scale(); 
			float y_scale = m_bitmap_matrix.get_y_scale();	

			float width = m_bitmap_info->get_width()*x_scale;
			float height = m_bitmap_info->get_height()*y_scale;

			GPU_BlitTransform(image, NULL, _renderInst->getTarget(), 160, 200, rotation, 0.3, 0.3);
			//x' = x * ScaleX + y * RotateSkew1 + TranslateX
			//y' = x * RotateSkew0 + y * ScaleY + TranslateY
		}
	}
}

bool fill_style::needs_second_pass() const
// Return true if we need to do a second pass to make
// a valid color.  This is for cxforms with additive
// parts; this is the simplest way (that we know of)
// to implement an additive color with stock OpenGL.
{
	if (m_mode == BITMAP_WRAP
		|| m_mode == BITMAP_CLAMP)
	{
		return m_has_nonzero_bitmap_additive_color;
	}
	else
	{
		return false;
	}
}

void fill_style::apply_second_pass() const
// Set OpenGL state for a necessary second pass.
{
	//assert(needs_second_pass());

	//// The additive color also seems to be modulated by the texture. So,
	//// maybe we can fake this in one pass using using the mean value of 
	//// the colors: c0*t+c1*t = ((c0+c1)/2) * t*2
	//// I don't know what the alpha component of the color is for.
	////glDisable(GL_TEXTURE_2D);

	//glColor4f(
	//	m_bitmap_color_transform.m_[0][1] / 255.0f,
	//	m_bitmap_color_transform.m_[1][1] / 255.0f,
	//	m_bitmap_color_transform.m_[2][1] / 255.0f,
	//	m_bitmap_color_transform.m_[3][1] / 255.0f
	//	);

	//glBlendFunc(GL_ONE, GL_ONE);
}

void fill_style::cleanup_second_pass() const
{
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void fill_style::set_bitmap(gameswf::bitmap_info* bi, const gameswf::matrix& m, gameswf::render_handler::bitmap_wrap_mode wm, const gameswf::cxform& color_transform)
{
	m_mode = (wm == gameswf::render_handler::WRAP_REPEAT) ? BITMAP_WRAP : BITMAP_CLAMP;
	m_bitmap_info = bi;
	m_bitmap_matrix = m;
	m_bitmap_color_transform = color_transform;
	m_bitmap_color_transform.clamp();

	m_color = gameswf::rgba(
		Uint8(m_bitmap_color_transform.m_[0][0] * 255.0f),
		Uint8(m_bitmap_color_transform.m_[1][0] * 255.0f),
		Uint8(m_bitmap_color_transform.m_[2][0] * 255.0f),
		Uint8(m_bitmap_color_transform.m_[3][0] * 255.0f));

	if (m_bitmap_color_transform.m_[0][1] > 1.0f
		|| m_bitmap_color_transform.m_[1][1] > 1.0f
		|| m_bitmap_color_transform.m_[2][1] > 1.0f
		|| m_bitmap_color_transform.m_[3][1] > 1.0f)
	{
		m_has_nonzero_bitmap_additive_color = true;
	}
	else
	{
		m_has_nonzero_bitmap_additive_color = false;
	}
}

SDL2RenderHandler::SDL2RenderHandler(GPU_Target* r)
{
	_target = r;
	_renderInst = this;
}

SDL2RenderHandler::~SDL2RenderHandler()
{
	
}

// Your handler should return these with a ref-count of 0.  (@@ is that the right policy?)
gameswf::bitmap_info* SDL2RenderHandler::create_bitmap_info_empty()	// used when DO_NOT_LOAD_BITMAPS is set
{
	return NULL;
}

gameswf::bitmap_info* SDL2RenderHandler::create_bitmap_info_alpha(int w, int h, unsigned char* data)
{
	return new bitmap_info_sdl2(_target, w, h, data);
	//return NULL;
}

gameswf::bitmap_info* SDL2RenderHandler::create_bitmap_info_rgb(image::rgb* im)
{
	return new bitmap_info_sdl2(_target, im);
	//return NULL;
}

gameswf::bitmap_info* SDL2RenderHandler::create_bitmap_info_rgba(image::rgba* im)
{
	return new bitmap_info_sdl2(_target, im);
	//return NULL;
}

gameswf::video_handler*	SDL2RenderHandler::create_video_handler()
{
	return NULL;
}

// Bracket the displaying of a frame from a movie.
// Fill the background color, and set up default
// transforms, etc.
void SDL2RenderHandler::begin_display(
	gameswf::rgba bgc,
	int viewport_x0, int viewport_y0,
	int viewport_width, int viewport_height,
	float x0, float x1, float y0, float y1)
{
	GPU_Clear(_target);
	//SDL_SetRenderDrawColor(_render, bgc.m_r, bgc.m_g, bgc.m_b, bgc.m_a);
	//this->applyColor(bgc.m_r, bgc.m_g, bgc.m_b, bgc.m_a);

	SDL_Rect rect;
	rect.x = viewport_x0; rect.y = viewport_y0; rect.w = viewport_width; rect.h = viewport_height;

	//SDL_RenderFillRect(_render, &rect);
	SDL_Color sc = {bgc.m_r,bgc.m_g,bgc.m_b,bgc.m_a};
	GPU_RectangleFilled(_target, viewport_x0, viewport_y0, viewport_x0 + viewport_width, viewport_y0 + viewport_height, sc);
}

void SDL2RenderHandler::end_display()
{
	GPU_Flip(_target);
	//SDL_RenderPresent(_render);
}

// Geometric and color transforms for mesh and line_strip rendering.
void SDL2RenderHandler::set_matrix(const gameswf::matrix& m)
{
	_currentMatrix = m;
}

void SDL2RenderHandler::set_cxform(const gameswf::cxform& cx)
{
	_currentCXForm = cx;
}

void SDL2RenderHandler::draw_mesh_primitive(int primitive_type, const void* coords, int vertex_count)
{
	_current_styles[LEFT_STYLE].apply();
	SDL_Color sc = {0,0,0,0};
	GPU_Polygon(_target, vertex_count, (float*)coords, sc);
}
// Draw triangles using the current fill-style 0.
// Clears the style list after rendering.
//
// coords is a list of (x,y) coordinate pairs, in
// triangle-strip order.  The type of the array should
// be Sint16[vertex_count*2]
void SDL2RenderHandler::draw_mesh_strip(const void* coords, int vertex_count)
{
	//draw_mesh_primitive(GL_TRIANGLE_STRIP, coords, vertex_count);
	draw_mesh_primitive(0, coords, vertex_count);
}

// As above, but coords is in triangle list order.
void SDL2RenderHandler::draw_triangle_list(const void* coords, int vertex_count)
{
	//draw_mesh_primitive(GL_TRIANGLES, coords, vertex_count);
	draw_mesh_primitive(0, coords, vertex_count);
}

// Draw a line-strip using the current line style.
// Clear the style list after rendering.
//
// Coords is a list of (x,y) coordinate pairs, in
// sequence.  Each coord is a 16-bit signed integer.
void SDL2RenderHandler::draw_line_strip(const void* coords, int vertex_count)
{
	_current_styles[LINE_STYLE].apply();
	float scale = fabsf(_currentMatrix.get_x_scale()) + fabsf(_currentMatrix.get_y_scale());
	float w = _current_styles[LINE_STYLE].m_width * scale / 2.0f;
	w = TWIPS_TO_PIXELS(w);

	SDL_Color sc = { 0, 0, 0, 0 };
	GPU_Polygon(_target, vertex_count, (float*)coords, sc);
}

void SDL2RenderHandler::fill_style_disable(int fill_side)
{
	assert(fill_side >= 0 && fill_side < 2);

	_current_styles[fill_side].disable();
}

void SDL2RenderHandler::fill_style_color(int fill_side, const gameswf::rgba& color)
{
	assert(fill_side >= 0 && fill_side < 2);

	_current_styles[fill_side].set_color(_currentCXForm.transform(color));
}

void SDL2RenderHandler::fill_style_bitmap(int fill_side, gameswf::bitmap_info* bi, const gameswf::matrix& m,
	bitmap_wrap_mode wm, bitmap_blend_mode bm)
{
	assert(fill_side >= 0 && fill_side < 2);
	_current_styles[fill_side].set_bitmap(bi, m, wm, _currentCXForm);
}

void SDL2RenderHandler::line_style_disable()
{
	_current_styles[LINE_STYLE].disable();
}

void SDL2RenderHandler::line_style_color(gameswf::rgba color)
{
	_current_styles[LINE_STYLE].set_color(_currentCXForm.transform(color));
}

void SDL2RenderHandler::line_style_width(float width)
{
	_current_styles[LINE_STYLE].m_width = width;
}

// Special function to draw a rectangular bitmap;
// intended for textured glyph rendering.  Ignores
// current transforms.
void SDL2RenderHandler::draw_bitmap(
	const gameswf::matrix&		m,
	gameswf::bitmap_info*	bi,
	const gameswf::rect&		coords,
	const gameswf::rect&		uv_coords,
	gameswf::rgba			color)
{
	gameswf::point a, b, c, d;
	m.transform(&a, gameswf::point(coords.m_x_min, coords.m_y_min));
	m.transform(&b, gameswf::point(coords.m_x_max, coords.m_y_min));
	m.transform(&c, gameswf::point(coords.m_x_min, coords.m_y_max));
	d.m_x = b.m_x + c.m_x - a.m_x;
	d.m_y = b.m_y + c.m_y - a.m_y;

	bi->layout();
	GPU_Image* image = (GPU_Image*)bi->get_data();
	GPU_Blit(image, NULL, _target, 0, 0);
}

void SDL2RenderHandler::set_antialiased(bool enable)
{

}

bool SDL2RenderHandler::test_stencil_buffer(const gameswf::rect& bound, Uint8 pattern)
{
	return false;
}

void SDL2RenderHandler::begin_submit_mask()
{

}

void SDL2RenderHandler::end_submit_mask()
{

}

void SDL2RenderHandler::disable_mask()
{

}

bool SDL2RenderHandler::is_visible(const gameswf::rect& bound)
{
	return false;
}

void SDL2RenderHandler::open()
{

}

void SDL2RenderHandler::applyColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	SDL_Color cl;
	cl.r = r;
	cl.g = g;
	cl.b = b;
	cl.a = a;
	GPU_ClearColor(_target, cl);
	//SDL_SetRenderDrawColor(_render, c.m_r, c.m_g, c.m_b, c.m_a);
}

bitmap_info_sdl2::bitmap_info_sdl2(GPU_Target* renderer, image::rgb* im) :
m_texture(NULL),
m_width(im->m_width),
m_height(im->m_height),
m_renderer(renderer)
{
	assert(im);
	//m_suspended_image = image::create_rgb(im->m_width, im->m_height);
	//memcpy(m_suspended_image->m_data, im->m_data, im->m_pitch * im->m_height);
	
	//m_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB444, SDL_TEXTUREACCESS_TARGET, m_width, m_height);
	//SDL_UpdateTexture(m_texture, NULL, im->m_data, im->m_pitch);

	m_texture = GPU_CreateImage(m_width, m_height, GPU_FORMAT_RGB);
	GPU_UpdateImageBytes(m_texture, NULL, im->m_data, im->m_pitch);
}

bitmap_info_sdl2::bitmap_info_sdl2(GPU_Target* renderer, image::rgba* im) :
m_texture(NULL),
m_width(im->m_width),
m_height(im->m_height),
m_renderer(renderer)
{
	assert(im);
	//m_suspended_image = image::create_rgb(im->m_width, im->m_height);
	//memcpy(m_suspended_image->m_data, im->m_data, im->m_pitch * im->m_height);

	//m_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA4444, SDL_TEXTUREACCESS_TARGET, m_width, m_height);
	//SDL_UpdateTexture(m_texture, NULL, im->m_data, im->m_pitch);

	m_texture = GPU_CreateImage(m_width, m_height, GPU_FORMAT_RGBA);
	GPU_UpdateImageBytes(m_texture, NULL, im->m_data, im->m_pitch);
}

bitmap_info_sdl2::bitmap_info_sdl2(GPU_Target* renderer, int width, int height, Uint8* data) :
m_texture(NULL),
m_width(width),
m_height(height),
m_renderer(renderer)
{
	//assert(width > 0 && height > 0 && data);
	//m_suspended_image = image::create_alpha(width, height);
	//memcpy(m_suspended_image->m_data, data, m_suspended_image->m_pitch * m_suspended_image->m_height);

	//m_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA4444, SDL_TEXTUREACCESS_TARGET, m_width, m_height);
	//SDL_UpdateTexture(m_texture, NULL, data, width);

	m_texture = GPU_CreateImage(m_width, m_height, GPU_FORMAT_ALPHA);
	GPU_UpdateImageBytes(m_texture, NULL, data, width);
}

bitmap_info_sdl2::~bitmap_info_sdl2()
{
	GPU_FreeImage(m_texture);
	//SDL_DestroyTexture(m_texture);
}