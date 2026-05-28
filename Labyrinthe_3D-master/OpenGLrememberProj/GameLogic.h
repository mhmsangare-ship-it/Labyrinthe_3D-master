#pragma once
#include <windows.h>
#include <GL/gl.h>
#include <sstream>
#include <iomanip>
#include "GUItextRectangle.h"
#include "Player.h"
#include "Maze.h"

enum class GameState { MENU, PLAYING, WIN, GAMEOVER };
enum class CameraMode { ISO, FPS };

struct Level { int cols; int rows; double timeLimit; const wchar_t* name; };
constexpr Level LEVELS[3] = {
    { 13, 13, 120.0, L"Level 1 - Small  (13x13)   2 min" },
    { 21, 21, 180.0, L"Level 2 - Medium (21x21)   3 min" },
    { 31, 31, 300.0, L"Level 3 - Large  (31x31)   5 min" },
};

class GameLogic
{
public:
    GameState  state = GameState::MENU;
    CameraMode camMode = CameraMode::ISO;
    int        levelIdx = 0;
    double     elapsed = 0.0;
    double     timeLimit = LEVELS[0].timeLimit;

    void init()
    {
        hud_.setSize(700, 56);
        menu_.setSize(560, 200);
        slow_.setSize(360, 40);
    }

    bool isFPS()  const { return camMode == CameraMode::FPS; }
    bool isMenu() const { return state == GameState::MENU; }

    void update(double dt, const Player& p, const Maze& m)
    {
        if (state != GameState::PLAYING) return;
        elapsed += dt;
        if (elapsed >= timeLimit)
        {
            state = GameState::GAMEOVER; return;
        }
        if (p.col() == m.exitCol && p.row() == m.exitRow)
            state = GameState::WIN;
    }

    // ── Menu screen ───────────────────────────────────────────────────
    void drawMenu(int winW, int winH)
    {
        std::wostringstream ss;
        ss << L"== MAZE 3D ==\n";
        for (int i = 0; i < 3; i++)
            ss << (i == levelIdx ? L">> " : L"   ")
            << (wchar_t)('1' + i) << L". " << LEVELS[i].name << L"\n";
        ss << L"\n[Tab] Camera: "
            << (camMode == CameraMode::FPS ? L"FPS" : L"Isometric")
            << L"   [Enter] Play";

        int mx = (winW - 560) / 2;
        int my = (winH - 200) / 2;
        menu_.setPosition(mx, my);
        menu_.setText(ss.str().c_str(), (char)255, (char)220, (char)80);
        menu_.Draw();
    }

    // ── In-game HUD ───────────────────────────────────────────────────
    void drawHUD(int winW, int winH, const Player& p)
    {
        double rem = (timeLimit - elapsed > 0.0) ? (timeLimit - elapsed) : 0.0;
        std::wostringstream ss;
        char r = (char)255, g = (char)255, b = (char)255;

        if (state == GameState::PLAYING)
        {
            int mi = (int)rem / 60, se = (int)rem % 60;
            ss << LEVELS[levelIdx].name
                << L"   Time: " << mi << L":"
                << std::setw(2) << std::setfill(L'0') << se
                << L"   [Tab] " << (camMode == CameraMode::FPS ? L"->ISO" : L"->FPS")
                << L"   [R] Menu";
            if (rem < 15) { r = (char)255; g = (char)50;  b = (char)50; }
            else if (rem < 30) { r = (char)255; g = (char)160; b = (char)0; }
        }
        else if (state == GameState::WIN)
        {
            int e = (int)elapsed;
            ss << L"WELL DONE! Exit found in "
                << e / 60 << L":"
                << std::setw(2) << std::setfill(L'0') << e % 60
                << L"   [R] Play again";
            r = (char)60; g = (char)220; b = (char)60;
        }
        else
        {
            ss << L"TIME'S UP!   [R] Menu";
            r = (char)220; g = (char)50; b = (char)50;
        }

        hud_.setPosition(10, winH - 66);
        hud_.setText(ss.str().c_str(), r, g, b);
        hud_.Draw();

        // Trap indicator
        if (p.isSlowed())
        {
            slow_.setPosition(winW / 2 - 180, winH / 2 - 20);
            slow_.setText(L"TRAP! Slowed down...", (char)255, (char)60, (char)60);
            slow_.Draw();
        }
    }

    void reset() { state = GameState::PLAYING; elapsed = 0.0; }

private:
    GuiTextRectangle hud_, menu_, slow_;
};