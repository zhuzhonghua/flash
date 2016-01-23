#include "shape_character_def.h"
#include "context.h"
#include "enum.h"
#include "stream.h"

ShapeCharacterDefinition::ShapeCharacterDefinition(Context* ctx)
:CharacterDefinition(ctx)
{

}

ShapeCharacterDefinition::~ShapeCharacterDefinition()
{

}

const ShapeCharacterDefinition & ShapeCharacterDefinition::operator = (const ShapeCharacterDefinition & def)
{

}

void ShapeCharacterDefinition::read(Stream* in, int tagType, bool withStyle, MovieDefinition* m)
{
	this->readStyle(in, tagType, withStyle, m);
}

void ShapeCharacterDefinition::readStyle(Stream* in, int tagType, bool withStyle, MovieDefinition* m)
{
	if (withStyle)
	{
		_bound.read(in);
		if (SWF::DEFINESHAPE4 == tagType)
		{
			_edgeBound.read(in);
			UInt8 b = in->readUI8();
			Assert((b & 0xFC) == 0);

			_usesNonScalingStrokes = b & 0x02 ? true : false;
			_usesScalingStrokes = b & 0x01 ? true : false;

			readFillStyles(in, tagType, m);
			readLineStyles(in, tagType, m);
		}
	}
}

void ShapeCharacterDefinition::readFillStyles(Stream* in, int tagType, MovieDefinition* m)
{
	Assert(_fillStyles.size() == 0);

	int	fillStyleCount = in->readUI8();

	if (fillStyleCount == 0xFF)
	{
		fillStyleCount = in->readUI16();
	}

	// Read the styles.
	for (int i = 0; i < fillStyleCount; i++)
	{
		FillStyle fs;
		fs.read(in, tagType, m);
		_fillStyles.push_back(fs);
	}
}

void ShapeCharacterDefinition::readLineStyles(Stream* in, int tagType, MovieDefinition* m)
{
	// Get the count.
	int	line_style_count = in->readUI8();

	// @@ does the 0xFF flag apply to all tag types?
	// if (tag_type > 2)
	// {
	if (line_style_count == 0xFF)
	{
		line_style_count = in->readUI16();
	}
	// }

	// Read the styles.
	for (int i = 0; i < line_style_count; i++)
	{
		LineStyle ls;
		ls.read(in, tagType, m);
		_lineStyles.push_back(ls);
	}
}