#include "fill_style.h"
#include "matrix.h"
#include "../movie_definition.h"

#include "system.h"
#include "global.h"
using namespace System;

#include "base/image.h"
#include "../render.h"
#include "../bitmap_character.h"

FillStyle::FillStyle()
{

}

FillStyle::~FillStyle()
{

}

void FillStyle::read(Stream* in, int tagType, MovieDefinition* m)
{
	_type = in->readUI8();

	// color
	if (_type == 0x00)
	{
		// 0x00: solid fill
		if (tagType <= SWF::DEFINESHAPE2) 
		{
			_color.readRGB(in);
		}
		else 
		{
			_color.readRGBA(in);
		}
	}
	// 
	else if (_type == 0x10 || _type == 0x12)
	{
		// 0x10: linear gradient fill
		// 0x12: radial gradient fill

		Matrix	input_matrix;
		input_matrix.read(in);

		if (_type == 0x10)
		{
			_gradientMatrix.setIdentity();
			_gradientMatrix.concatenateTranslation(128.f, 0.f);
			_gradientMatrix.concatenateScale(1.0f / 128.0f);
		}
		else
		{
			_gradientMatrix.setIdentity();
			_gradientMatrix.concatenateTranslation(32.f, 32.f);
			_gradientMatrix.concatenateScale(1.0f / 512.0f);
		}


		Matrix	m;
		m.setInverse(input_matrix);
		_gradientMatrix.concatenate(m);

		// GRADIENT

		// 'spread_mode' and 'interpolation_mode' are not used for now
		// and so they are commented out
		//			in->align();
		//			int spread_mode = in->read_uint(2);
		//			int interpolation_mode = in->read_uint(2);
		//			int num_gradients = in->read_uint(4);

		// SWF 8 and later supports up to 15 gradient control points
		int num_gradients = in->readUI8() & 0x0F;

		_gradients.resize(num_gradients);
		for (int i = 0; i < num_gradients; i++)
		{
			_gradients[i].read(in, tagType);
		}

		// @@ hack.
		//if (num_gradients > 0)
		//{
		//	m_color = m_gradients[0].m_color;
		//}
		//
		//if (md->get_create_bitmaps() == DO_LOAD_BITMAPS)
		//{
		//	m_gradient_bitmap_info = create_gradient_bitmap();
		//}
		//else
		//{
		//	m_gradient_bitmap_info = render::create_bitmap_info_empty();
		//}
		//
		//// Make sure our movie_def_impl knows about this bitmap.
		//md->add_bitmap_info(m_gradient_bitmap_info.get_ptr());
	}
	else if (_type == 0x13)
	{
		// TODO: different with the swf9 sepc page 143
		//assert(tag_type == 83);
		// focal gradient fill, Flash 8
		// parsed but not implemented yet

		Matrix	input_matrix;
		input_matrix.read(in);

		// 'spread_mode' and 'interpolation_mode' are not used for now
		// and so they are commented out
		//			in->align();
		//			int spread_mode = in->read_uint(2);
		//			int interpolation_mode = in->read_uint(2);
		//			int gradient_count = in->read_uint(4);

		int num_gradients = in->readUI8() & 0x0F;

		for (int i = 0; i < num_gradients; i++)
		{
			int ratio = in->readUI8();
			UNUSED(ratio);
			RGBA color;
			color.readRGBA(in);
		}

		in->readUI16(); //Fixed 8
	}
	else if (_type >= 0x40 && _type <= 0x43)
	{
		// 0x40: tiled bitmap fill
		// 0x41: clipped bitmap fill
		// 0x42: non-smoothed repeating bitmap, Flash 8, TODO
		// 0x43: non-smoothed clipped bitmap, Flash 8, TODO

		int	bitmap_char_id = in->readUI16();
		//IF_VERBOSE_PARSE(log_msg("  bitmap_char = %d\n", bitmap_char_id));

		// Look up the bitmap character.
		_bitmapChDef = m->getBitmapCharacterDef(bitmap_char_id);

		Matrix	m;
		m.read(in);

		// For some reason, it looks like they store the inverse of the
		// TWIPS-to-texcoords matrix.
		_bitmapMatrix.setInverse(m);
		//IF_VERBOSE_PARSE(m_bitmap_matrix.print());
	}
	else
	{
		Assert(0);
	}
}

RGBA FillStyle::sampleGradient(int ratio) const
{
	Assert(ratio >= 0 && ratio <= 255);
	Assert(_type == 0x10 || _type == 0x12);
	Assert(_gradients.size() > 0);

	if (ratio < _gradients[0]._ratio)
		return _gradients[0]._color;

	for (int i = 1; i < _gradients.size(); i++)
	{
		if (_gradients[i]._ratio >= ratio)
		{
			const GradientRecord& gr0 = _gradients[i - 1];
			const GradientRecord& gr1 = _gradients[i];
			float	f = 0.0f;
			if (gr0._ratio != gr1._ratio)
			{
				f = (ratio - gr0._ratio) / float(gr1._ratio - gr0._ratio);
			}

			RGBA	result;
			result.setLerp(_gradients[i - 1]._color, _gradients[i]._color, f);
			return result;
		}
	}
	return _gradients.back()._color;
}

BitmapInfo* FillStyle::createGradientBitmap() const
{
	Assert(_type == 0x10 || _type == 0x12);

	Image::RGBA* im = NULL;

	if (_type == 0x10)
	{
		// Linear gradient.
		im = Image::createRGBA(256, 1);

		for (int i = 0; i < im->_width; i++)
		{
			RGBA	sample = sampleGradient(i);
			im->setPixel(i, 0, sample._r, sample._g, sample._b, sample._a);
		}
	}
	else if (_type == 0x12)
	{
		// Radial gradient.
		im = Image::createRGBA(64, 64);

		for (int j = 0; j < im->_height; j++)
		{
			for (int i = 0; i < im->_width; i++)
			{
				float	radius = (im->_height - 1) / 2.0f;
				float	y = (j - radius) / radius;
				float	x = (i - radius) / radius;
				int	ratio = (int)floorf(255.5f * sqrt(x * x + y * y));
				if (ratio > 255)
				{
					ratio = 255;
				}
				RGBA	sample = sampleGradient(ratio);
				im->setPixel(i, j, sample._r, sample._g, sample._b, sample._a);
			}
		}
	}

	BitmapInfo*	bi = Render::createBitmapInfoRGBA(im);
	delete im;

	return bi;
}

void FillStyle::apply(int fillSide, float ratio, BitmapBlendMode bm) const
{
	UNUSED(ratio);
	if (_type == 0x00)
	{
		Config::get()->getRenderHandler()->fillStyleColor(fillSide, _color);		
	}
	else if (_type == 0x10 || _type == 0x12)
	{
		// 0x10: linear gradient fill
		// 0x12: radial gradient fill

		if (_gradientBitmapInfo == NULL)
		{
			// This can happen when morphing gradient styles.
			// assert(morphing???);
			// log an error?
			FillStyle*	this_non_const = const_cast<FillStyle*>(this);
			this_non_const->_gradientBitmapInfo = createGradientBitmap();
		}

		if (_gradientBitmapInfo != NULL)
		{
			Render::fillStyleBitmap(fillSide, _gradientBitmapInfo, _gradientMatrix, WRAP_CLAMP, bm);
		}
	}
	else if (_type >= 0x40 || _type <= 0x43)
	{
		// bitmap fill (either tiled or clipped)
		BitmapInfo*	bi = NULL;
		if (_bitmapChDef != NULL)
		{
			bi = _bitmapChDef->getBitmapInfo();

			if (bi != NULL)
			{
				switch (_type)
				{
				case 0x40:
				case 0x42:
					Render::fillStyleBitmap(fillSide, bi, _bitmapMatrix, WRAP_REPEAT, bm);
					break;

				case 0x41:
				case 0x43:
					Render::fillStyleBitmap(fillSide, bi, _bitmapMatrix, WRAP_CLAMP, bm);
					break;

				default:
					Assert(0);
				}
			}
		}
	}
	else
	{
		Assert(0);
	}
}

GradientRecord::GradientRecord():
	_ratio(0)
{

}

void GradientRecord::read(Stream* in, int tag_type)
{
	_ratio = in->readUI8();
	_color.read(in, tag_type);
}