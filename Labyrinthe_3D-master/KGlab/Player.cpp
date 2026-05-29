#include "Player.h"

void Player::updateFPS(double dt, const Maze& maze)
{
    if (slowTimer > 0.0f) slowTimer -= (float)dt;

    float speed = (slowTimer > 0.0f) ? TRAP_SPEED : NORMAL_SPEED;

    float cy = cosf(yaw), sy = sinf(yaw);

    float dx = 0, dz = 0;

    if (OpenGL::isKeyPressed(VK_UP) || OpenGL::isKeyPressed('W') || OpenGL::isKeyPressed('Z'))
    {
        dx += cy; dz += sy;
    }
    if (OpenGL::isKeyPressed(VK_DOWN) || OpenGL::isKeyPressed('S'))
    {
        dx -= cy; dz -= sy;
    }

    if (OpenGL::isKeyPressed(VK_LEFT) || OpenGL::isKeyPressed('A') || OpenGL::isKeyPressed('Q'))
    {
        dx += sy; dz -= cy;
    }
    if (OpenGL::isKeyPressed(VK_RIGHT) || OpenGL::isKeyPressed('D'))
    {
        dx -= sy; dz += cy;
    }

    float len = sqrtf(dx * dx + dz * dz);
    if (len > 0.001f) { dx /= len; dz /= len; }

    float nx = x + dx * speed * (float)dt;
    float nz = z + dz * speed * (float)dt;

    auto blocked = [&](float cx_, float cz_) -> bool {
        int c1 = Maze::toGrid(cx_ - RADIUS), c2 = Maze::toGrid(cx_ + RADIUS);
        int r1 = Maze::toGrid(cz_ - RADIUS), r2 = Maze::toGrid(cz_ + RADIUS);
        return maze.isWall(c1, r1) || maze.isWall(c2, r1) ||
            maze.isWall(c1, r2) || maze.isWall(c2, r2);
        };

    if (!blocked(nx, z)) x = nx;
    if (!blocked(x, nz)) z = nz;

    if (maze.isTrap(col(), row()) && slowTimer <= 0.0f)
        slowTimer = TRAP_DURATION;
}

void Player::applyFPSCamera() const
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float cy = cosf(yaw), sy = sinf(yaw);
    float cp = cosf(pitch), sp = sinf(pitch);

    float lx = cy * cp;
    float ly = sp;
    float lz = sy * cp;

    gluLookAt(x, y, z, x + lx, y + ly, z + lz, 0, 1, 0);
}

void Player::draw() const
{
    glPushMatrix();
    glTranslatef(x, RADIUS, z);

    GLfloat amb[] = { 0.02f, 0.02f, 0.18f, 1.0f };
    GLfloat dif[] = { 0.18f, 0.40f, 0.95f, 1.0f };
    GLfloat spec[] = { 1.00f, 1.00f, 1.00f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT, GL_SHININESS, 96.0f);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);

    GLUquadric* q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);
    gluSphere(q, RADIUS, 32, 32);
    gluDeleteQuadric(q);

    glPopMatrix();
}