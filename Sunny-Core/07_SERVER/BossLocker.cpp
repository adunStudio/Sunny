#include "BossLocker.h"

unordered_map<int, Player*> BossLocker::players;
Player* BossLocker::player = nullptr;
int BossLocker::id = 0;
vector<Entity*> BossLocker::player_bullets;