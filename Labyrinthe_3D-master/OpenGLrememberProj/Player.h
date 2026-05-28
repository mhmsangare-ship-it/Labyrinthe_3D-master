#pragma once
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include "MyOGL.h"
#include "Maze.h"

class Player
{
public:
    // World position
    float x = Maze::toWorld(1);
    float z = Maze::toWorld(1);
    float y = 1.1f;             // eye height

    // FPS look angles (set by Render.cpp from mouse delta)
    float yaw = 0.0f;
    float pitch = 0.0f;

    // Trap slow-down
    float slowTimer = 0.0f;

    static constexpr float RADIUS = 0.38f;
    static constexpr float NORMAL_SPEED = 4.5f;
    static constexpr float TRAP_SPEED = 1.35f;
    static constexpr float TRAP_DURATION = 2.0f;

    void reset()
    {
        x = Maze::toWorld(1);
        z = Maze::toWorld(1);
        yaw = 0.0f;
        pitch = 0.0f;
        slowTimer = 0.0f;
    }

    // Movement only — mouse look is done in Render.cpp
    void updateFPS(double dt, const Maze& maze);

    // Sets gluLookAt from player eye position + yaw/pitch
    void applyFPSCamera() const;

    // Sphere for ISO mode
    void draw() const;

    bool isSlowed() const { return slowTimer > 0.0f; }

    int col() const { return Maze::toGrid(x); }
    int row() const { return Maze::toGrid(z); }
};