#pragma once
#include "math/Vector2.hpp"
#include "math/Rect.hpp"
#include "tmx/TmxParser.hpp"
#include "renderer/Renderer.hpp"

#include <random>



struct Camera {
    float x, y;

    ee::math::Vector2<float> getPosition() { return { x, y }; }
};

enum class Tile {
    EMPTY = 0,


    //Sol
    FLOOR_1 = 1, FLOOR_2, FLOOR_3, FLOOR_4,
    FLOOR_5, FLOOR_6, FLOOR_7, FLOOR_8,

    //Deco
    REDFLAG = -42, BLUEFLAG = -43,
    GREENFLAG = -44, YELLOWFLAG = -45,


    //sols avec des crane
    FLOOR_1B = 15, FLOOR_2B = 16, FLOOR_3B = 17,
    FLOOR_4B = 18, FLOOR_5B = 19, FLOOR_6B = 20,
    FLOOR_7B = 21, FLOOR_8B = 22,


    //fontaine
    WALL_MID_FOUNTAIN_W = -35,
    WALL_BOTTOM_FOUNTAIN_W = 36,

    WALL_MID_FOUNTAIN_L = -37,
    WALL_BOTTOM_FOUNTAIN_L = 38,

    //top
    WALL_TOP_FOUNTAIN_1 = -39,
    WALL_TOP_FOUNTAIN_2 = -40,
    WALL_TOP_FOUNTAIN_3 = -41,


    // Mur du haut (la face visible du mur vue de dessus)
    WALL_TOP_LEFT = -1,
    WALL_TOP_MID = -2,
    WALL_TOP_RIGHT = -3,

    // Mur du milieu (la tranche du mur)
    WALL_MID_LEFT = -4,
    WALL_MID = -5,
    WALL_MID_RIGHT = -6,

    // Coins extérieurs (ex: angle sortant d'une salle)
    WALL_OUTER_TOP_LEFT = -10,
    WALL_OUTER_TOP_RIGHT = -11,
    WALL_OUTER_MID_LEFT = -12,
    WALL_OUTER_MID_RIGHT = -13,
    WALL_OUTER_FRONT_LEFT = -14,
    WALL_OUTER_FRONT_RIGHT = -15,

    // Coins intérieurs (ex: angle rentrant quand deux couloirs se rejoignent)
    WALL_EDGE_TOP_LEFT = -20,
    WALL_EDGE_TOP_RIGHT = -21,
    WALL_EDGE_MID_LEFT = -22,
    WALL_EDGE_MID_RIGHT = -23,
    WALL_EDGE_MIDDOWN_LEFT = -24,
    WALL_EDGE_MIDDOWN_RIGHT = -25,
    WALL_EDGE_BOTTOM_LEFT = -26,
    WALL_EDGE_BOTTOM_RIGHT = -27,

    // Jonctions en T (quand un mur arrive sur un autre perpendiculairement)
    WALL_TSHAPE_BOTTOM_RIGHT = -31,
    WALL_TSHAPE_BOTTOM_LEFT = -32,
    WALL_TSHAPE_RIGHT = -33,
    WALL_TSHAPE_LEFT = -34,

    WALL_WITH_BOTTOM_MID = -46,
    WALL_WITH_BOTTOM_LEFT = -47,
    WALL_WITH_BOTTOM_RIGHT = -48,
};

inline bool isWalkable(Tile t) {
    return (int)t > 0;
}

inline bool isWall(Tile t) {
    return (int)t < 0;
}

struct Cell {
    Tile type = Tile::EMPTY;
};

inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}


inline std::mt19937& getRng()
{
    static std::mt19937 rng(std::random_device{}());
    return rng;
}

inline ee::math::Rect<float> tileIdToRect(const ee::tmx::TmxTileset& _tileset, int _id) {
    return { (float)(_id% _tileset.columns* _tileset.tileWidth), (float)(_id / _tileset.columns * _tileset.tileHeight), (float)(_tileset.tileWidth), (float)(_tileset.tileHeight) };
}

inline int rectToTileId(const ee::tmx::TmxTileset& _tileset, int x, int y) {
    return (y / _tileset.tileWidth) * _tileset.columns + (x / _tileset.tileHeight);
}