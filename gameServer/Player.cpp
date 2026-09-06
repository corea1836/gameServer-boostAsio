#include "Player.h"

Player::~Player() {}

void Player::Bind(GameSessionRef session) { _session = session; }
