#include "Render.h"
#include "MyOGL.h"
#include "MyShaders.h"
#include "Texture.h"
#include "GUItextRectangle.h"
#include "Camera.h"
#include "Light.h"
#include "debout.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include "Maze.h"
#include "Player.h"
#include "GameLogic.h"

extern OpenGL gl;

static Camera    camera;
static Light     light;
static Shader    wallShader;
static Shader    floorShader;
static Shader    trapShader;
static Maze      maze;
static Player    player;
static GameLogic gameLogic;

static bool g_tabWasDown = false;
static bool g_enterWasDown = false;
static bool g_rWasDown = false;
static bool g_1WasDown = false;
static bool g_2WasDown = false;
static bool g_3WasDown = false;

static int  g_prevMouseX = -1;
static int  g_prevMouseY = -1;

static void mulVecMat4(const float v[4], const float m[16], float out[4])
{
    for (int i = 0; i < 4; i++) {
        out[i] = 0.0f;
        for (int j = 0; j < 4; j++) out[i] += v[j] * m[j * 4 + i];
    }
}

static void pushUniforms(const float lv[4])
{
    wallShader.UseShader();
    glUniform3fARB(glGetUniformLocationARB(wallShader.program, "light_pos_v"), lv[0], lv[1], lv[2]);
    glUniform3fARB(glGetUniformLocationARB(wallShader.program, "Ia"), 1, 1, 1);
    glUniform3fARB(glGetUniformLocationARB(wallShader.program, "Id"), 1, 1, 1);
    glUniform3fARB(glGetUniformLocationARB(wallShader.program, "Is"), 1, 1, 1);
    glUniform1iARB(glGetUniformLocationARB(wallShader.program, "tex"), 0);

    floorShader.UseShader();
    glUniform3fARB(glGetUniformLocationARB(floorShader.program, "light_pos_v"), lv[0], lv[1], lv[2]);
    glUniform3fARB(glGetUniformLocationARB(floorShader.program, "Ia"), 1, 1, 1);
    glUniform3fARB(glGetUniformLocationARB(floorShader.program, "Id"), 1, 1, 1);
    glUniform1iARB(glGetUniformLocationARB(floorShader.program, "tex"), 0);

    trapShader.UseShader();
    glUniform3fARB(glGetUniformLocationARB(trapShader.program, "light_pos_v"), lv[0], lv[1], lv[2]);
    glUniform3fARB(glGetUniformLocationARB(trapShader.program, "Ia"), 1, 1, 1);
    glUniform3fARB(glGetUniformLocationARB(trapShader.program, "Id"), 1, 1, 1);
    glUniform1iARB(glGetUniformLocationARB(trapShader.program, "tex"), 0);

    Shader::DontUseShaders();
}

static void beginOrtho(int w, int h)
{
    glActiveTexture(GL_TEXTURE0);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0, w - 1, 0, h - 1, 0, 1);
    glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
}

static void endOrtho()
{
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
}

static void applyISOCamera()
{
    float cx = (maze.cols / 2.f) * CELL_SIZE;
    float cz = (maze.rows / 2.f) * CELL_SIZE;

    float dist = (float)camera.distance();

    float ex = cx + dist * (float)cos(camera._fi2) * (float)cos(camera._fi1);
    float ey = dist * (float)sin(camera._fi2);
    float ez = cz + dist * (float)cos(camera._fi2) * (float)sin(camera._fi1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(ex, ey, ez, cx, 0, cz, 0, 1, 0);
}

static void setupISOCamera()
{
    camera._fi1 = 0.785;
    camera._fi2 = 0.70;

    float maxDim = (float)(maze.cols > maze.rows ? maze.cols : maze.rows);
    float dist = maxDim * CELL_SIZE * 0.85f;
    float cx = (maze.cols / 2.f) * CELL_SIZE;
    float cz = (maze.rows / 2.f) * CELL_SIZE;
    float ex = cx + dist * (float)cos(0.70) * (float)cos(0.785);
    float ey = dist * (float)sin(0.70);
    float ez = cz + dist * (float)cos(0.70) * (float)sin(0.785);

    camera.setPosition(ex - cx, ey, ez - cz);
    camera._fi1 = 0.785;
    camera._fi2 = 0.70;
    camera.caclulateCameraPos();

    light.SetPosition(cx, dist * 0.8f, cz);
}

static void startLevel(int idx)
{
    maze.resize(LEVELS[idx].cols, LEVELS[idx].rows);
    maze.generate();
    player.reset();
    gameLogic.reset();
    gameLogic.levelIdx = idx;
    gameLogic.timeLimit = LEVELS[idx].timeLimit;
    g_prevMouseX = -1;
    g_prevMouseY = -1;
    setupISOCamera();
}

void initRender()
{
    wallShader.VshaderFileName = "shaders/maze.vert";
    wallShader.FshaderFileName = "shaders/maze_wall.frag";
    wallShader.LoadShaderFromFile(); wallShader.Compile();

    floorShader.VshaderFileName = "shaders/maze.vert";
    floorShader.FshaderFileName = "shaders/maze_floor.frag";
    floorShader.LoadShaderFromFile(); floorShader.Compile();

    trapShader.VshaderFileName = "shaders/maze.vert";
    trapShader.FshaderFileName = "shaders/maze_trap.frag";
    trapShader.LoadShaderFromFile(); trapShader.Compile();

    maze.loadTextures();
    maze.generate();
    gameLogic.init();
    setupISOCamera();

    gl.WheelEvent.reaction(&camera, &Camera::Zoom);
    gl.MouseMovieEvent.reaction(&camera, &Camera::MouseMovie);
    gl.MouseLeaveEvent.reaction(&camera, &Camera::MouseLeave);
    gl.MouseLdownEvent.reaction(&camera, &Camera::MouseStartDrag);
    gl.MouseLupEvent.reaction(&camera, &Camera::MouseStopDrag);

    gl.MouseMovieEvent.reaction(&light, &Light::MoveLight);
    gl.KeyDownEvent.reaction(&light, &Light::StartDrug);
    gl.KeyUpEvent.reaction(&light, &Light::StopDrug);

    glClearColor(0.05f, 0.05f, 0.10f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
}

void Render(double dt)
{
    int  W = gl.getWidth(), H = gl.getHeight();
    bool tabDown = OpenGL::isKeyPressed(VK_TAB);
    bool enterDown = OpenGL::isKeyPressed(VK_RETURN) || OpenGL::isKeyPressed(VK_SPACE);
    bool rDown = OpenGL::isKeyPressed('R');
    bool k1 = OpenGL::isKeyPressed('1');
    bool k2 = OpenGL::isKeyPressed('2');
    bool k3 = OpenGL::isKeyPressed('3');

    if (gameLogic.state == GameState::MENU)
    {
        if (k1 && !g_1WasDown) { gameLogic.levelIdx = 0; maze.resize(LEVELS[0].cols, LEVELS[0].rows); maze.generate(); setupISOCamera(); }
        if (k2 && !g_2WasDown) { gameLogic.levelIdx = 1; maze.resize(LEVELS[1].cols, LEVELS[1].rows); maze.generate(); setupISOCamera(); }
        if (k3 && !g_3WasDown) { gameLogic.levelIdx = 2; maze.resize(LEVELS[2].cols, LEVELS[2].rows); maze.generate(); setupISOCamera(); }
        g_1WasDown = k1; g_2WasDown = k2; g_3WasDown = k3;

        if (tabDown && !g_tabWasDown)
            gameLogic.camMode = (gameLogic.camMode == CameraMode::ISO)
            ? CameraMode::FPS : CameraMode::ISO;
        g_tabWasDown = tabDown;

        if (enterDown && !g_enterWasDown)
        {
            startLevel(gameLogic.levelIdx);
            gameLogic.timeLimit = LEVELS[gameLogic.levelIdx].timeLimit;
            gameLogic.state = GameState::PLAYING;
            gameLogic.elapsed = 0.0;
            g_prevMouseX = -1;
            g_prevMouseY = -1;
        }
        g_enterWasDown = enterDown;

        applyISOCamera();
        float vm[16]; glGetFloatv(GL_MODELVIEW_MATRIX, vm);
        float lw[4] = { (float)light.x(),(float)light.y(),(float)light.z(),1.f };
        float lv[4]; mulVecMat4(lw, vm, lv);
        light.SetUpLight();
        pushUniforms(lv);
        glEnable(GL_TEXTURE_2D); glEnable(GL_LIGHTING);
        maze.draw(wallShader, floorShader, trapShader);
        Shader::DontUseShaders();

        beginOrtho(W, H);
        gameLogic.drawMenu(W, H);
        endOrtho();
        return;
    }

    if (rDown && !g_rWasDown)
    {
        gameLogic.state = GameState::MENU;
        g_prevMouseX = -1; g_prevMouseY = -1;
    }
    g_rWasDown = rDown;

    if (tabDown && !g_tabWasDown)
    {
        gameLogic.camMode = (gameLogic.camMode == CameraMode::ISO)
            ? CameraMode::FPS : CameraMode::ISO;
        g_prevMouseX = -1; g_prevMouseY = -1;
    }
    g_tabWasDown = tabDown;

    if (gameLogic.isFPS() && gameLogic.state == GameState::PLAYING)
    {
        POINT pt;
        GetCursorPos(&pt);

        if (g_prevMouseX >= 0)
        {
            int dx = pt.x - g_prevMouseX;
            int dy = pt.y - g_prevMouseY;
            player.yaw += dx * 0.0025f;
            player.pitch -= dy * 0.0025f;
            float minP = -1.2f, maxP = 1.2f;
            if (player.pitch < minP) player.pitch = minP;
            if (player.pitch > maxP) player.pitch = maxP;
        }
        g_prevMouseX = pt.x;
        g_prevMouseY = pt.y;

        HWND hw = GetForegroundWindow();
        RECT rc; GetClientRect(hw, &rc);
        POINT centre = { (rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2 };
        ClientToScreen(hw, &centre);
        if (abs(pt.x - centre.x) > 5 || abs(pt.y - centre.y) > 5)
        {
            SetCursorPos(centre.x, centre.y);
            g_prevMouseX = centre.x;
            g_prevMouseY = centre.y;
        }
    }

    if (gameLogic.state == GameState::PLAYING)
        player.updateFPS(dt, maze);

    gameLogic.update(dt, player, maze);

    if (gameLogic.isFPS())
    {
        player.applyFPSCamera();
    }
    else
    {
        applyISOCamera();
    }

    float vm[16]; glGetFloatv(GL_MODELVIEW_MATRIX, vm);
    float lw[4] = { (float)light.x(),(float)light.y(),(float)light.z(),1.f };
    float lv[4]; mulVecMat4(lw, vm, lv);
    light.SetUpLight();
    pushUniforms(lv);

    glEnable(GL_TEXTURE_2D); glEnable(GL_LIGHTING);
    maze.draw(wallShader, floorShader, trapShader);

    Shader::DontUseShaders();
    glDisable(GL_TEXTURE_2D); glEnable(GL_LIGHTING);
    if (!gameLogic.isFPS()) player.draw();

    if (gameLogic.isFPS())
    {
        beginOrtho(W, H);
        glDisable(GL_TEXTURE_2D);
        glLineWidth(2.0f);
        int cx2 = W / 2, cy2 = H / 2, cs = 10;
        glBegin(GL_LINES);
        glColor3f(1, 1, 1);
        glVertex2i(cx2 - cs, cy2); glVertex2i(cx2 + cs, cy2);
        glVertex2i(cx2, cy2 - cs); glVertex2i(cx2, cy2 + cs);
        glEnd();
        glColor3f(1, 1, 1);
        glLineWidth(1.0f);
        endOrtho();
    }

    beginOrtho(W, H);
    gameLogic.drawHUD(W, H, player);
    endOrtho();
}