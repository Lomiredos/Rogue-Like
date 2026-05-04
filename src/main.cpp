
#include "GameManager.hpp"
#include "DungeonGenerator.hpp"

int main(){


    GameManager gm("Rogue Like", 900, 900);
    gm.run();

    return 0;
}