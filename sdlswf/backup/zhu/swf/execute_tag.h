#ifndef Z_EXECUTE_TAG_
#define Z_EXECUTE_TAG_

class Character;

class ExecuteTag
{
public:
	virtual ~ExecuteTag() {}
	virtual void	execute(Character* m) {}
	virtual void	executeState(Character* m) {}
};

#endif
