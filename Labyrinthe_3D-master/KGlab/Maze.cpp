#include "Maze.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>

void Maze::resize(int c, int r)
{
    cols = c; rows = r;
    exitCol = cols - 2;
    exitRow = rows - 2;
    grid.assign(rows * cols, 1);
}

void Maze::generate()
{
    srand(static_cast<unsigned>(time(nullptr)));
    grid.assign(rows * cols, 1);
    exitCol = cols - 2;
    exitRow = rows - 2;

    std::vector<std::vector<bool>> vis(rows, std::vector<bool>(cols, false));
    cell(1, 1) = 0;
    dfs(1, 1, vis);

    cell(1, 0) = 0;
    cell(exitCol, exitRow) = 0;
    cell(cols - 1, exitRow) = 0;

    int trapCount = (cols * rows) / 18;
    int placed = 0, attempts = 0;
    while (placed < trapCount && attempts < 2000)
    {
        attempts++;
        int c = 1 + rand() % (cols - 2);
        int r = 1 + rand() % (rows - 2);
        if (cell(c, r) != 0) continue;
        if (c <= 3 && r <= 3) continue;
        if (abs(c - exitCol) <= 2 && abs(r - exitRow) <= 2) continue;
        cell(c, r) = TRAP_CELL;
        placed++;
    }
}

void Maze::dfs(int col, int row, std::vector<std::vector<bool>>& vis)
{
    vis[row][col] = true;
    int dc[] = { 2,-2, 0, 0 };
    int dr[] = { 0, 0, 2,-2 };
    int ord[] = { 0,1,2,3 };
    for (int i = 3; i > 0; i--) std::swap(ord[i], ord[rand() % (i + 1)]);
    for (int i = 0; i < 4; i++)
    {
        int nc = col + dc[ord[i]];
        int nr = row + dr[ord[i]];
        if (nc < 1 || nc >= cols - 1 || nr < 1 || nr >= rows - 1) continue;
        if (vis[nr][nc]) continue;
        cell(col + dc[ord[i]] / 2, row + dr[ord[i]] / 2) = 0;
        cell(nc, nr) = 0;
        dfs(nc, nr, vis);
    }
}

void Maze::loadTextures()
{
    texWall.LoadTexture("textures/wall.png");
    texFloor.LoadTexture("textures/floor.png");
    texExit.LoadTexture("textures/exit.png");
    texTrap.LoadTexture("textures/trap.png");
}

void Maze::drawQuad(float x0, float y0, float z0,
    float x1, float y1, float z1,
    float x2, float y2, float z2,
    float x3, float y3, float z3,
    float nx, float ny, float nz)
{
    glNormal3f(nx, ny, nz);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(x0, y0, z0);
    glTexCoord2f(1, 0); glVertex3f(x1, y1, z1);
    glTexCoord2f(1, 1); glVertex3f(x2, y2, z2);
    glTexCoord2f(0, 1); glVertex3f(x3, y3, z3);
    glEnd();
}

void Maze::drawWallCube(int col, int row)
{
    float x = col * CELL_SIZE, z = row * CELL_SIZE, c = CELL_SIZE, h = WALL_H;
    drawQuad(x, 0, z + c, x + c, 0, z + c, x + c, h, z + c, x, h, z + c, 0, 0, 1);
    drawQuad(x + c, 0, z, x, 0, z, x, h, z, x + c, h, z, 0, 0, -1);
    drawQuad(x + c, 0, z + c, x + c, 0, z, x + c, h, z, x + c, h, z + c, 1, 0, 0);
    drawQuad(x, 0, z, x, 0, z + c, x, h, z + c, x, h, z, -1, 0, 0);
    drawQuad(x, h, z + c, x + c, h, z + c, x + c, h, z, x, h, z, 0, 1, 0);
}

void Maze::drawFloorTile(int col, int row, float yOff, Texture& tex)
{
    tex.Bind();
    float x = col * CELL_SIZE, z = row * CELL_SIZE, c = CELL_SIZE;
    glNormal3f(0, 1, 0);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(x, yOff, z);
    glTexCoord2f(1, 0); glVertex3f(x + c, yOff, z);
    glTexCoord2f(1, 1); glVertex3f(x + c, yOff, z + c);
    glTexCoord2f(0, 1); glVertex3f(x, yOff, z + c);
    glEnd();
}

void Maze::draw(Shader& wallSh, Shader& floorSh, Shader& trapSh)
{
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);

    floorSh.UseShader();
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
        {
            int v = cell(c, r);
            if (v == 1) continue;
            if (v == TRAP_CELL)
                drawFloorTile(c, r, 0.0f, texTrap);
            else if (c == exitCol && r == exitRow)
                drawFloorTile(c, r, 0.02f, texExit);
            else
                drawFloorTile(c, r, 0.0f, texFloor);
        }

    wallSh.UseShader();
    texWall.Bind();
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            if (cell(c, r) == 1)
                drawWallCube(c, r);

    Shader::DontUseShaders();
}

bool Maze::isWall(int col, int row) const
{
    if (col < 0 || col >= cols || row < 0 || row >= rows) return true;
    return grid[row * cols + col] == 1;
}

bool Maze::isTrap(int col, int row) const
{
    if (col < 0 || col >= cols || row < 0 || row >= rows) return false;
    return grid[row * cols + col] == TRAP_CELL;
}