#ifndef Z_CHARACTER_
#define Z_CHARACTER_

#include "as_object.h"
#include "type/rgba.h"
#include "type/cxform.h"
#include "type/matrix.h"

class Context;
class Character : public ASObject
{
public:
	enum { _class_id = AS_CHARACTER };
	virtual bool is(int class_id) const
	{
		if (_class_id == class_id) return true;
		else return ASObject::is(class_id);
	}

	Character(Context* ctx, Character* parent, int id);
	virtual ~Character();

	virtual float	getBackgroundAlpha() const { return 1.0f; }
	virtual void	setBackgroundAlpha(float alpha) {}

	virtual void	setBackgroundColor(const RGBA& color) {}

	virtual void	executeFrameTags(int frame, bool stateOnly = false) {}

	virtual void	advance(double dt) {}
	virtual void	display() {}

	Character*		getParent() const { return _parent; }
	void			setParent(Character* parent) { _parent = parent; }  // for extern movie

	virtual CXForm	getWorldCXForm() const
		// Get our concatenated color transform (all our ancestor transforms,
		// times our cxform).  Maps from our local space into normal color space.
	{
		CXForm	m;
		if (_parent != NULL)
		{
			m = _parent->getWorldCXForm();
		}
		m.concatenate(getCXForm());

		return m;
	}

	const CXForm&	getCXForm() const
	{
		return _colorTransform;
	}
	
	void			setCXForm(const CXForm& cx)
	{
		_colorTransform = cx;
	}
	const Matrix&	getMatrix() const { return _matrix; }
	void			setMatrix(const Matrix& m)
	{
		_matrix = m;
	}
	Matrix	getWorldMatrix() const
		// Get our concatenated matrix (all our ancestor transforms, times our matrix).	 Maps
		// from our local space into "world" space (i.e. root movie space).
	{
		Matrix	m;
		const Character * current = _parent;

		m = getMatrix();

		while (current != NULL)
		{
			Matrix new_m;

			new_m = current->getMatrix();
			new_m.concatenate(m);
			m = new_m;
			current = current->_parent;
		}

		return m;
	}
private:
	Character*			_parent;
	int					_id;
	CXForm				_colorTransform;
	Matrix				_matrix;
};

#endif