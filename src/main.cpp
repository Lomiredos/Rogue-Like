
#include <iostream>
#include "GameManager.hpp"
#include "DungeonGenerator.hpp"

int main(){


    try {
        GameManager gm("Rogue Like", 900, 900);
        gm.run();
    }
    catch (const std::exception& e) {
        std::cerr <<"Fatal: " <<  e.what() << "\n";
        return 1;
    }
    return 0;
}