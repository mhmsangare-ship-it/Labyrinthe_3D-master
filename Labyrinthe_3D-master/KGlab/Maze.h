#pragma once
#include <vector>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "Texture.h"
#include "MyShaders.h"

constexpr float CELL_SIZE = 2.0f;
constexpr float WALL_H = 2.5f;

constexpr int TRAP_CELL = 2;

class Maze
{
public:
    int cols = 13;
    int rows = 13;

    std::vector<int> grid;

    int exitCol = 0;
    int exitRow = 0;

    Maze() { resize(13, 13); }

    void resize(int c, int r);
    void generate();
    void loadTextures();
    void draw(Shader& wallSh, Shader& floorSh, Shader& trapSh);

    bool isWall(int col, int row) const;
    bool isTrap(int col, int row) const;

    int& cell(int col, int row) { return grid[row * cols + col]; }
    int  cell(int col, int row) const { return grid[row * cols + col]; }

    static int   toGrid(float w) { return static_cast<int>(w / CELL_SIZE); }
    static float toWorld(int g) { return g * CELL_SIZE + CELL_SIZE * 0.5f; }

private:
    Texture texWall, texFloor, texExit, texTrap;

    void dfs(int col, int row, std::vector<std::vector<bool>>& visited);
    void placeTrap(int col, int row);

    static void drawQuad(float x0, float y0, float z0,
        float x1, float y1, float z1,
        float x2, float y2, float z2,
        float x3, float y3, float z3,
        float nx, float ny, float nz);

    void drawWallCube(int col, int row);
    void drawFloorTile(int col, int row, float yOff, Texture& tex);
};