#include "Character.h"

Character::Character() {}

Character::~Character() {}

void Character::BindStats(string name, int32 level)
{
    _characterInfo.name = name;
    _characterInfo.level = level;
}
