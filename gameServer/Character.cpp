#include "Character.h"

Character::Character() {}

Character::~Character() {}

void Character::BindStats(string name, int32 level)
{
    _characterData.name = name;
    _characterData.level = level;
}
