#ifndef Z_SHAPE_CHARACTER_DEF_H
#define Z_SHAPE_CHARACTER_DEF_H

#include "character_def.h"
#include "type/rect.h"
#include "type/fill_style.h"
#include "type/line_style.h"
#include "base/memory_state.h"

class Stream;
class MovieDefinition;

class ShapeCharacterDefinition : public CharacterDefinition, public MemoryState<ShapeCharacterDefinition>
{
public:
	ShapeCharacterDefinition(Context* ctx);
	virtual ~ShapeCharacterDefinition();

	const 	ShapeCharacterDefinition & operator =(const ShapeCharacterDefinition & def);

	void	read(Stream* in, int tagType, bool withStyle, MovieDefinition* m);
	void	readStyle(Stream* in, int tagType, bool withStyle, MovieDefinition* m);
	void	readFillStyles(Stream* in, int tagType, MovieDefinition* m);
	void	readLineStyles(Stream* in, int tagType, MovieDefinition* m);
private:
	RECT	_bound;
	RECT	_edgeBound;

	bool	_usesNonScalingStrokes;
	bool	_usesScalingStrokes;

	std::vector<FillStyle>		_fillStyles;
	std::vector<LineStyle>		_lineStyles;
};
#endif