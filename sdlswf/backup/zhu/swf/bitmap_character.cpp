#include "global.h"
#include "bitmap_character.h"
#include "type/bitmap_info.h"
#include "type/rect.h"
#include "type/cxform.h"
#include "character.h"
#include "render.h"

bool BitmapCharacter::pointTestLocal(float x, float y)
{
	RECT coords;
	coords._x_min = 0.0f;
	coords._x_max = PIXELS_TO_TWIPS(_bitmapInfo->getWidth());
	coords._y_min = 0.0f;
	coords._y_max = PIXELS_TO_TWIPS(_bitmapInfo->getHeight());
	if (coords.pointTest(x, y))
	{
		return true;
	}
	return false;
}

void BitmapCharacter::getBound(RECT* bound)
{
	bound->_x_min = 0.0f;
	bound->_x_max = PIXELS_TO_TWIPS(_bitmapInfo->getWidth());
	bound->_y_min = 0.0f;
	bound->_y_max = PIXELS_TO_TWIPS(_bitmapInfo->getHeight());
}

void BitmapCharacter::display(Character* ch)
{
	RECT coords;
	coords._x_min = 0.0f;
	coords._x_max = PIXELS_TO_TWIPS(_bitmapInfo->getWidth());
	coords._y_min = 0.0f;
	coords._y_max = PIXELS_TO_TWIPS(_bitmapInfo->getHeight());

	// show whole picture
	RECT uv_coords;
	uv_coords._x_min = 0.0f;
	uv_coords._x_max = 1.0f;
	uv_coords._y_min = 0.0f;
	uv_coords._y_max = 1.0f;

	CXForm cx = ch->getWorldCXForm();
	RGBA color = cx.transform(RGBA());
	Matrix m = ch->getWorldMatrix();
	Render::drawBitmap(m, _bitmapInfo, coords, uv_coords, color);
}

BitmapInfo*	BitmapCharacter::getBitmapInfo()
{
	return _bitmapInfo;
}