//============================================================================
// Name        : RushHour.cpp
// Author      : Dr. Sibt Ul Hussain 
// Version     : Optimized Movement and Fuel Logic
// Copyright   : (c) Reserved
// Description : Basic 2D game with continuous car movement, refuel logic, and dynamic car spawn
//============================================================================

#ifndef RushHour_CPP_
#define RushHour_CPP_
#include "util.h"
#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;

void DrawGrid();

void SetCanvas(int width, int height) 
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

class Car 
{
  public:
    int x, y;
    int col;
    int gas;
    bool active;
    Car() : x(0), y(0), col(0), gas(100), active(true) {}
};

class Player : public Car 
{
  public:
    int pts;
    int cash;
    int job;
    bool hasRider;
    bool hasBox;
    Player() : Car(), pts(0), cash(0), job(0), hasRider(false), hasBox(false) {}
};

class NPC : public Car 
{
  public:
    int dir;
    NPC() : Car(), dir(-1) {}
};

class Person 
{
  public:
    int x, y;
    int destX, destY;
    bool taken;
    Person() : x(0), y(0), destX(0), destY(0), taken(false) {}
};

class Package 
{
  public:
    int x, y;
    int destX, destY;
    bool taken;
    Package() : x(0), y(0), destX(0), destY(0), taken(false) {}
};

class Obstacle 
{
  public:
    int x, y;
    int kind;
    Obstacle() : x(0), y(0), kind(0) {}
};

class Station 
{
  public:
    int x, y;
    Station() : x(0), y(0) {}
};

class Map 
{
  public:
    int w, h;
    Map(int w, int h) : w(w), h(h) {}
};

const int W = 1020;
const int H = 840;
const int CELL = 40;
const int MAX_NPC = 5;
const int MAX_PEOPLE = 4;
const int MAX_PACKS = 4;
const int MAX_OBST = 8;
const int MAX_STAT = 3;

Map map(W, H);
Player me;
NPC npcs[MAX_NPC];
Person people[MAX_PEOPLE];
Package packs[MAX_PACKS];
Obstacle obst[MAX_OBST];
Station stations[MAX_STAT];

int npcCount = 3;
int peopleCount = 2;
int packCount = 2;
int obstCount = 6;
int statCount = 2;

const int ROAD = 0;
const int BUILDING = 1;
int grid[W / CELL][H / CELL];

bool IsRoad(int x, int y) 
{
    return grid[x][y] == ROAD;
}

void GetRoad(int &x, int &y) 
{
    int w = W / CELL;
    int h = H / CELL;
    do 
    {
        x = rand() % w;
        y = rand() % h;
    } while (!IsRoad(x, y));
}

void MakeMap() 
{
    int w = W / CELL;
    int h = H / CELL;
    for (int i = 0; i < w; ++i) 
    {
        for (int j = 0; j < h; ++j) 
        {
            grid[i][j] = ROAD;
        }
    }
    for (int i = 0; i < w; ++i) 
    {
        for (int j = 0; j < h; ++j) 
        {
            if (i % 3 == 0 || j % 3 == 0) 
            {
                grid[i][j] = ROAD;
            }
        }
    }
    int numBuildings = 15 + rand() % 2;
    for (int b = 0; b < numBuildings; ++b) 
    {
        int w = (rand() % 2) + 1;
        int h = (rand() % 2) + 1;
        int maxX = W/CELL - w - 1;
        int maxY = H/CELL - h - 1;
        int startX, startY;
        bool ok = false;
        int tries = 0;
        while (!ok && tries < 20) 
        {
            startX = 1 + rand() % maxX;
            startY = 1 + rand() % maxY;
            bool bad = false;
            for (int i = 0; i < w && !bad; ++i) 
            {
                for (int j = 0; j < h && !bad; ++j) 
                {
                    if (grid[startX + i][startY + j] != ROAD) 
                    {
                        bad = true;     
                    }
                    if ((startX + i == 0 && startY + j == 0) || (startX + i < 2 && startY + j < 2)) 
                    {
                        bad = true;
                    }
                }
            }
            if (!bad) 
            {
                for (int i = 0; i < w; ++i) 
                {
                    for (int j = 0; j < h; ++j) 
                    {
                        grid[startX + i][startY + j] = BUILDING;
                    }
                }
                ok = true;
            }
            tries++;
        }
    }
    grid[0][0] = ROAD;
}

bool IsTaken(int x, int y) 
{
    if (me.x / CELL == x && me.y / CELL == y)
    {
        return true;
    }
    for (int i = 0; i < npcCount; ++i) 
    {
        if (npcs[i].x / CELL == x && npcs[i].y / CELL == y) 
        {
            return true;
        }
    }
    for (int i = 0; i < obstCount; ++i) 
    {
        if (obst[i].x / CELL == x && obst[i].y / CELL == y) 
        {
            return true;
        }
    }
    for (int i = 0; i < peopleCount; ++i) 
    {
        if (people[i].x / CELL == x && people[i].y / CELL == y && !people[i].taken) 
        {
            return true;
        }
    }
    for (int i = 0; i < packCount; ++i) 
    {
        if (packs[i].x / CELL == x && packs[i].y / CELL == y && !packs[i].taken) 
        {
            return true;
        }
    }
    for (int i = 0; i < statCount; ++i) 
    {
        if (stations[i].x / CELL == x && stations[i].y / CELL == y) 
        {
            return true;
        }
    }
    return false;
}

#define TAXI_COLOR YELLOW
#define DELIVERY_COLOR CYAN
#define OTHER_CAR_COLOR1 GREEN
#define OTHER_CAR_COLOR2 BLUE
#define PLAYER_CAR_COLOR ORANGE

int playerJobCount = 0;
int carSpeed = 1;
const int dx[4] = {0, 0, -1, 1};
const int dy[4] = {1, -1, 0, 0};
int carMoveTick = 0;
const int CAR_MOVE_INTERVAL = 4;
int carMoveIdleTick = 0;
const int CAR_MOVE_IDLE_INTERVAL = 10;
const int STATUS_BAR_HEIGHT = 50;

int ClampGridY(int y) 
{
    int minY = STATUS_BAR_HEIGHT;
    int maxY = STATUS_BAR_HEIGHT + H - CELL;
    if (y < minY)
    {
        return minY;
    }
    if (y > maxY)
    {
        return maxY;
    }
    return y;
}

void InitGame() 
{
    MakeMap();
    int w = W / CELL;
    int h = H / CELL;
    me.x = 0;
    me.y = STATUS_BAR_HEIGHT;
    me.col = PLAYER_CAR_COLOR;
    me.gas = 100;
    me.pts = 0;
    me.cash = 0;
    me.hasRider = false;
    me.hasBox = false;
    playerJobCount = 0;
    carSpeed = 1;
    for (int i = 0; i < npcCount; ++i) 
    {
        int cx, cy;
        do 
        {
            GetRoad(cx, cy); 
        } while (IsTaken(cx, cy));
        
        npcs[i].x = cx * CELL;
        npcs[i].y = ClampGridY(cy * CELL + STATUS_BAR_HEIGHT);
        npcs[i].col = (i % 2 == 0) ? OTHER_CAR_COLOR1 : OTHER_CAR_COLOR2;
        npcs[i].dir = rand() % 4;
        npcs[i].gas = 100;
        npcs[i].active = true;
    }
    for (int i = 0; i < obstCount; ++i) 
    {
        int cx, cy;
        do 
        { 
            GetRoad(cx, cy); 
        } while (IsTaken(cx, cy));

        obst[i].x = cx * CELL;
        obst[i].y = ClampGridY(cy * CELL + STATUS_BAR_HEIGHT);
        obst[i].kind = (i % 2 == 0) ? 0 : 1;
    }

    for (int i = 0; i < peopleCount; ++i) 
    {
        int cx, cy;
        do 
        {
            GetRoad(cx, cy); 
        } while (IsTaken(cx, cy));
        
        people[i].x = cx * CELL;
        people[i].y = ClampGridY(cy * CELL + STATUS_BAR_HEIGHT);
        
        do 
        {
             GetRoad(cx, cy); 
        } while (IsTaken(cx, cy));
        
        people[i].destX = cx * CELL;
        people[i].destY = ClampGridY(cy * CELL + STATUS_BAR_HEIGHT);
        people[i].taken = false;
    }
    for (int i = 0; i < packCount; ++i) 
    {
        int cx, cy;
        do 
        { 
            GetRoad(cx, cy); 
        } while (IsTaken(cx, cy));
        packs[i].x = cx * CELL;
        packs[i].y = ClampGridY(cy * CELL + STATUS_BAR_HEIGHT);
        
        do 
        { 
            GetRoad(cx, cy); 
        } while (IsTaken(cx, cy));
        packs[i].destX = cx * CELL;
        packs[i].destY = ClampGridY(cy * CELL + STATUS_BAR_HEIGHT);
        packs[i].taken = false;
    }
    for (int i = 0; i < statCount; ++i) 
    {
        int cx, cy;
        do 
        { 
            GetRoad(cx, cy); 
        } while (IsTaken(cx, cy));
        stations[i].x = cx * CELL;
        stations[i].y = ClampGridY(cy * CELL + STATUS_BAR_HEIGHT);
    }
}

const int MENU = 0;
const int LEADERBOARD = 1;
const int ROLE_SELECT = 2;
const int NAME_ENTRY = 3;
const int RUNNING = 4;
const int GAME_OVER = 5;

int gameState = MENU;
int gameTicks = 0;
const int MAX_TICKS = 1800;

bool IsNear(int x, int y) 
{
    int px = me.x, py = me.y;
    return (abs(px - x) <= CELL && abs(py - y) <= CELL && !(px == x && py == y));
}

bool IsAt(int x, int y) 
{
    return (me.x == x && me.y == y);
}

void CheckHit() 
{
    for (int i = 0; i < obstCount; ++i) 
    {
        if (IsAt(obst[i].x, obst[i].y)) 
        {
            if (me.job == 0) {
                if (obst[i].kind == 0)
                {
                    me.pts -= 2;
                }
                else
                {
                    me.pts -= 2;
                }
            } 
            else 
            {
                me.pts -= 4;
            }
        }
    }
    for (int i = 0; i < npcCount; ++i) 
    {
        if (IsAt(npcs[i].x, npcs[i].y)) 
        {
            if (me.job == 0)
            {   
                me.pts -= 3;
            }
            else
            {
                me.pts -= 5;
            }
        }
    }
    for (int i = 0; i < peopleCount; ++i) 
    {
        if (!people[i].taken && IsAt(people[i].x, people[i].y)) 
        {
            if (me.job == 0)
            {
                me.pts -= 5;
            }
            else
            {
                me.pts -= 8;
            }
        }
    }
}
void TryGas() 
{
    for (int i = 0; i < statCount; ++i) 
    {
        if (IsAt(stations[i].x, stations[i].y)) 
        {
            if (me.cash >= 5 && me.gas < 100) 
            {
                me.cash -= 5;
                me.gas = 100;
            }
        }
    }
}

const int MAX_LEADERBOARD = 10;
struct HighScoreEntry 
{
    char name[20];
    int score;
};
HighScoreEntry leaderboard[MAX_LEADERBOARD];
int leaderboardCount = 0;

void LoadScores() 
{
    std::ifstream fin("highscores.txt", std::ios::binary);
    leaderboardCount = 0;
    if (fin) 
    {
        while (fin.read((char*)&leaderboard[leaderboardCount], sizeof(HighScoreEntry)) && leaderboardCount < MAX_LEADERBOARD) 
        {
            leaderboardCount++;
        }
        fin.close();
    }
}

void SaveScores() 
{
    std::ofstream fout("highscores.txt", std::ios::binary | std::ios::trunc);
    for (int i = 0; i < leaderboardCount; ++i) 
    {
        fout.write((char*)&leaderboard[i], sizeof(HighScoreEntry));
    }
    fout.close();
}

void AddScore(const char* name, int score) 
{
    int pos = leaderboardCount;
    for (int i = 0; i < leaderboardCount; ++i) 
    {
        if (score > leaderboard[i].score) 
        {
            pos = i;
            break;
        }
    }
    if (pos < MAX_LEADERBOARD) 
    {
        for (int i = std::min(MAX_LEADERBOARD-1, leaderboardCount); i > pos; --i) 
        {
            leaderboard[i] = leaderboard[i-1];
        }
        std::string nameStr(name);
        nameStr = nameStr.substr(0, 19);
        std::fill(std::begin(leaderboard[pos].name), std::end(leaderboard[pos].name), '\0');
        std::copy(nameStr.begin(), nameStr.end(), leaderboard[pos].name);
        leaderboard[pos].score = score;
        if (leaderboardCount < MAX_LEADERBOARD)
        {
            leaderboardCount++;
        }
        SaveScores();
    }
}

#define UI_TEXT_COLOR WHITE

void DrawScores() 
{
    int maxEntries = 8;
    int shown = std::min(leaderboardCount, maxEntries);
    int entrySpacing = 44;
    int boxW = 440;
    int boxH = 220 + shown * entrySpacing;
    int boxX = W/2 - boxW/2;
    int boxY = H/2 - boxH/2 + 40;
    float darkOverlay[4] = {0.1f, 0.1f, 0.1f, 0.85f};
    DrawRectangle(boxX, boxY, boxW, boxH, darkOverlay);
    int centerX = W/2;
    int y = boxY + 60;
    if (gameState == GAME_OVER) 
    {
        DrawString(centerX - 100, y, "GAME OVER!", colors[RED]);
        y += 44;
        DrawString(centerX - 160, y, "Press ENTER to return to menu", colors[RED]);
        y += 44;
        DrawString(centerX - 100, y, "LEADERBOARD", colors[RED]);
        y += 44;
        for (int i = 0; i < shown; ++i) 
        {
            std::ostringstream entryOss;
            const char* name = (leaderboard[i].name[0] != '\0') ? leaderboard[i].name : "Anonymous";
            entryOss << std::left << std::setw(15) << name << " " << std::setw(4) << leaderboard[i].score;
            DrawString(centerX - 100, y + i*entrySpacing, entryOss.str(), colors[RED]);
        }
        if (leaderboardCount == 0) 
        {
            DrawString(centerX - 100, y, "No scores yet!", colors[RED]);
        }
        y += shown * entrySpacing + 20;
        DrawString(centerX - 100, y, "Press ESC to return", colors[RED]);
    } 
    
    else 
    {
        y = boxY + 80;
        DrawString(centerX - 100, y, "LEADERBOARD", colors[RED]);
        y += 44;
        for (int i = 0; i < shown; ++i) 
        {
            std::ostringstream entryOss;
            const char* name = (leaderboard[i].name[0] != '\0') ? leaderboard[i].name : "Anonymous";
            entryOss << std::left << std::setw(15) << name << " " << std::setw(4) << leaderboard[i].score;
            DrawString(centerX - 100, y + i*entrySpacing, entryOss.str(), colors[RED]);
        }
        if (leaderboardCount == 0) 
        {
            DrawString(centerX - 100, y, "No scores yet!", colors[RED]);
        }
        y += shown * entrySpacing + 20;
        DrawString(centerX - 100, y, "Press ESC to return", colors[RED]);
        y += 44;
        DrawString(centerX - 160, y, "Press ENTER to return to menu", colors[RED]);
    }
}

int menuSelection = 0;
int roleSelection = 0;
char tempName[20] = "";
int tempNamePos = 0;

void ResetMenu() 
{
    menuSelection = 0;
    roleSelection = 0;
    tempName[0] = '\0';
    tempNamePos = 0;
}

void DrawBg() 
{
    int w = W / CELL;
    int h = H / CELL;
    float faintGray[3] = {0.25f, 0.25f, 0.25f};
    float faintYellow[3] = {0.4f, 0.4f, 0.1f};
    for (int i = 0; i <= w; ++i) 
    {
        int x = i * CELL;
        DrawRectangle(x, STATUS_BAR_HEIGHT, 2, H, faintGray);
        if (i % 3 == 0) 
        {
            DrawRectangle(x-1, STATUS_BAR_HEIGHT, 4, H, faintYellow);
        }
    }
    for (int j = 0; j <= h; ++j) 
    {
        int y = STATUS_BAR_HEIGHT + j * CELL;
        DrawRectangle(0, y, W, 2, faintGray);
        if (j % 3 == 0) 
        {
            DrawRectangle(0, y-1, W, 4, faintYellow);
        }
    }
    for (int i = 0; i <= w; i += 3) 
    {
        for (int j = 0; j <= h; j += 3) 
        {
            int x = i * CELL;
            int y = STATUS_BAR_HEIGHT + j * CELL;
            DrawCircle(x, y, 12, faintGray);
        }
    }
}

void DrawMenu() 
{
    DrawBg();
    DrawString(W/2 - 140, H/2 + 80, "Rush Hour Game", colors[YELLOW]);
    DrawString(W/2 - 120, H/2 + 30, menuSelection == 0 ? "> Start New Game" : "  Start New Game", colors[menuSelection == 0 ? GREEN : WHITE]);
    DrawString(W/2 - 120, H/2 - 10, menuSelection == 1 ? "> View Leaderboard" : "  View Leaderboard", colors[menuSelection == 1 ? GREEN : WHITE]);
    DrawString(W/2 - 120, H/2 - 70, "Use UP/DOWN and ENTER", colors[CYAN]);
}

void DrawRoles() 
{
    DrawBg();
    DrawString(W/2 - 140, H/2 + 60, "Choose Role:", colors[YELLOW]);
    DrawString(W/2 - 120, H/2 + 20, roleSelection == 0 ? "> Taxi Driver" : "  Taxi Driver", colors[roleSelection == 0 ? GREEN : WHITE]);
    DrawString(W/2 - 120, H/2 - 20, roleSelection == 1 ? "> Delivery Driver" : "  Delivery Driver", colors[roleSelection == 1 ? GREEN : WHITE]);
    DrawString(W/2 - 120, H/2 - 60, roleSelection == 2 ? "> Random Job" : "  Random Job", colors[roleSelection == 2 ? GREEN : WHITE]);
    DrawString(W/2 - 120, H/2 - 110, "Use UP/DOWN and ENTER", colors[CYAN]);
}

char playerName[20] = "";

void DrawName() 
{
    DrawBg();
    DrawString(W/2 - 140, H/2 + 60, "Enter your name:", colors[YELLOW]);
    DrawString(W/2 - 100, H/2 + 10, tempName, colors[GREEN]);
    DrawString(W/2 - 120, H/2 - 40, "(Press Enter to start)", colors[WHITE]);
}

void DrawGame() 
{
    glClearColor(0, 0, 0.0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    DrawRectangle(0, 0, W, STATUS_BAR_HEIGHT, colors[GRAY]);
    std::ostringstream oss;
    oss << "Score: " << me.pts;
    DrawString(30, STATUS_BAR_HEIGHT/2, oss.str(), colors[RED]);
    oss.str(""); oss.clear();
    oss << "Fuel: " << me.gas;
    DrawString(200, STATUS_BAR_HEIGHT/2, oss.str(), colors[RED]);
    oss.str(""); oss.clear();
    oss << "Cash: " << me.cash;
    DrawString(320, STATUS_BAR_HEIGHT/2, oss.str(), colors[RED]);
    oss.str(""); oss.clear();
    oss << "Role: " << (me.job == 0 ? "Taxi" : "Delivery");
    DrawString(470, STATUS_BAR_HEIGHT/2, oss.str(), colors[RED]);
    oss.str(""); oss.clear();
    int secondsLeft = (MAX_TICKS - gameTicks) / 10;
    oss << "Time: " << secondsLeft;
    DrawString(W - 200, STATUS_BAR_HEIGHT/2, oss.str(), colors[RED]);
    if (gameState == MENU || gameState == LEADERBOARD || gameState == ROLE_SELECT || gameState == NAME_ENTRY) 
    {
        DrawRectangle(0, STATUS_BAR_HEIGHT, W, H, colors[BLACK]);
        if (gameState == MENU) 
        {
            DrawMenu();
        } 
        else if (gameState == LEADERBOARD) 
        {
            DrawScores();
        } 
        else if (gameState == ROLE_SELECT) 
        {
            DrawRoles();
        } 
        else if (gameState == NAME_ENTRY) 
        {
            DrawName();
        }
        glutSwapBuffers();
        return;
    }
    if (gameState == GAME_OVER) 
    {
        DrawRectangle(0, STATUS_BAR_HEIGHT, W, H, colors[BLACK]);
        DrawScores();
        glutSwapBuffers();
        return;
    }
    DrawGrid();
    for (int i = 0; i < obstCount; ++i) 
    {
        if (obst[i].kind == 0)
        {
            DrawCircle(obst[i].x + CELL/2, obst[i].y + CELL/2, CELL/2 - 4, colors[DARK_GREEN]);
        }
        else
        {
            DrawSquare(obst[i].x, obst[i].y, CELL-4, colors[BROWN]);
        }
    }
    for (int i = 0; i < statCount; ++i) 
    {
        DrawRoundRect(stations[i].x, stations[i].y, CELL, CELL, colors[RED], 10);
    }
    if (me.job == 0) 
    {
        int pickedRiderIdx = -1;
        for (int i = 0; i < peopleCount; ++i) 
        {
            if (!people[i].taken) 
            {
                DrawCircle(people[i].x + CELL/2, people[i].y + CELL - 10, 8, colors[BLUE]);
                DrawLine(people[i].x + CELL/2, people[i].y + CELL - 18, people[i].x + CELL/2, people[i].y + 10, 2, colors[BLUE]);
            } 
            else if (me.hasRider) 
            {
                pickedRiderIdx = i;
            }
        }
        if (me.hasRider && pickedRiderIdx != -1) 
        {
            DrawSquare(people[pickedRiderIdx].destX, people[pickedRiderIdx].destY, CELL, colors[GREEN]);
        }
    }
    if (me.job == 1) 
    {
        for (int i = 0; i < packCount; ++i) 
        {
            if (!packs[i].taken) 
            {
                DrawSquare(packs[i].x + 8, packs[i].y + 8, CELL - 16, colors[BROWN]);
            }
        }
    }
    for (int i = 0; i < npcCount; ++i) 
    {
        DrawRoundRect(npcs[i].x, npcs[i].y, CELL, CELL/2, colors[npcs[i].col], 8);
        DrawCircle(npcs[i].x + 10, npcs[i].y, 6, colors[BLACK]);
        DrawCircle(npcs[i].x + CELL - 10, npcs[i].y, 6, colors[BLACK]);
    }
    DrawRoundRect(me.x, me.y, CELL, CELL/2, colors[me.col], 12);
    DrawCircle(me.x + 10, me.y, 6, colors[BLACK]);
    DrawCircle(me.x + CELL - 10, me.y, 6, colors[BLACK]);
    glutSwapBuffers();
}

void MoveKeys(int key, int x, int y) 
{
    if (gameState != RUNNING) 
    {
        return;
    }
    if (me.gas <= 0) 
    {
        return;
    }
    int moveStep = CELL;
    bool moved = false;
    int nextX = me.x, nextY = me.y;
    if (key == GLUT_KEY_LEFT) 
    {
        if (me.x - moveStep >= 0) 
        {
            nextX = me.x - moveStep;
        }
    } 
    else if (key == GLUT_KEY_RIGHT) 
    {
        if (me.x + moveStep <= W - CELL) 
        {
            nextX = me.x + moveStep;
        }
    } 
    else if (key == GLUT_KEY_UP) 
    {
        if (me.y + moveStep <= STATUS_BAR_HEIGHT + H - CELL) 
        {
            nextY = me.y + moveStep;
        }
    } 
    else if (key == GLUT_KEY_DOWN) 
    {
        if (me.y - moveStep >= STATUS_BAR_HEIGHT) 
        {
            nextY = me.y - moveStep;
        }
    }
    int cellX = nextX / CELL;
    int cellY = (nextY - STATUS_BAR_HEIGHT) / CELL;
    if (IsRoad(cellX, cellY)) 
    {
        if (nextX != me.x || nextY != me.y) 
        {
            me.x = nextX;
            me.y = nextY;
            moved = true;
            CheckHit();
        }
    }
    if (moved && me.gas > 0)
    {
        me.gas--;  
    }
    glutPostRedisplay();
}

bool AtGarage() 
{
    return (me.x == 0 && me.y == STATUS_BAR_HEIGHT);
}

void KeyPress(unsigned char key, int x, int y) 
{
    if (gameState == MENU) 
    {
        if (key == 13) 
        {
            if (menuSelection == 0) 
            {
                gameState = ROLE_SELECT;
            } 
            else 
            {
                LoadScores();
                gameState = LEADERBOARD;
            }
        } 
        else if (key == 27) 
        {
            exit(0);
        } 
        else if (key == 'w' || key == 'W' || key == 'k' || key == 'K') 
        {
            menuSelection = (menuSelection + 1) % 2;
            glutPostRedisplay();
        } 
        else if (key == 's' || key == 'S' || key == 'j' || key == 'J') 
        {
            menuSelection = (menuSelection + 1) % 2;
            glutPostRedisplay();
        }
        return;
    }
    if (gameState == LEADERBOARD) 
    {
        if (key == 27) 
        {
            ResetMenu();
            gameState = MENU;
            glutPostRedisplay();
        }
        return;
    }
    if (gameState == ROLE_SELECT) 
    {
        if (key == 13) 
        {
            if (roleSelection == 2) 
            {
                me.job = rand() % 2;
            } 
            else 
            {
                me.job = roleSelection;
            }
            tempName[0] = '\0';
            tempNamePos = 0;
            gameState = NAME_ENTRY;
            glutPostRedisplay();
        } 
        else if (key == 'w' || key == 'W' || key == 'k' || key == 'K') 
        {
            roleSelection = (roleSelection + 2) % 3;
            glutPostRedisplay();
        } 
        else if (key == 's' || key == 'S' || key == 'j' || key == 'J') 
        {
            roleSelection = (roleSelection + 1) % 3;
            glutPostRedisplay();
        } 
        else if (key == 27) {
            gameState = MENU;
            glutPostRedisplay();
        }
        return;
    }
    if (gameState == NAME_ENTRY) 
    {
        if (key == 13 && tempNamePos > 0) 
        {
            std::string tempNameStr(tempName);
            tempNameStr = tempNameStr.substr(0, 19);
            std::fill(std::begin(playerName), std::end(playerName), '\0');
            std::copy(tempNameStr.begin(), tempNameStr.end(), playerName);
            gameState = RUNNING;
            InitGame();
            glutPostRedisplay();
        } 
        else if ((key == 8 || key == 127) && tempNamePos > 0) 
        {
            tempName[--tempNamePos] = '\0';
            glutPostRedisplay();
        } 
        else if (tempNamePos < 19 && ((key >= 'A' && key <= 'Z') || (key >= 'a' && key <= 'z') || (key >= '0' && key <= '9') || key == ' ')) 
        {
            tempName[tempNamePos++] = key;
            tempName[tempNamePos] = '\0';
            glutPostRedisplay();
        } 
        else if (key == 27) 
        {
            gameState = MENU;
            glutPostRedisplay();
        }
        return;
    }
    if (key == 27) 
    {
        exit(1);
    }
    if (gameState == RUNNING && AtGarage() && (key == 'p' || key == 'P')) 
    {
        gameState = ROLE_SELECT;
        glutPostRedisplay();
        return;
    }
    if (key == 32) 
    {
        if (me.job == 0) 
        {
            if (!me.hasRider) 
            {
                for (int i = 0; i < peopleCount; ++i) 
                {
                    if (!people[i].taken && IsNear(people[i].x, people[i].y)) 
                    {
                        me.hasRider = true;
                        people[i].taken = true;
                        playerJobCount++;
                        break;
                    }
                }
            } 
            else 
            {
                for (int i = 0; i < peopleCount; ++i) 
                {
                    if (people[i].taken && IsAt(people[i].destX, people[i].destY)) 
                    {
                        me.hasRider = false;
                        me.pts += 10;
                        me.cash += 5;
                        int cx, cy;
                        do 
                        { 
                            GetRoad(cx, cy); 
                        } while (IsTaken(cx, cy));
                        people[i].x = cx * CELL;
                        people[i].y = ClampGridY(cy * CELL + STATUS_BAR_HEIGHT);
                        
                        do 
                        {
                            GetRoad(cx, cy);
                        } while (IsTaken(cx, cy) || grid[cx][cy] == BUILDING);
                        people[i].destX = cx * CELL;
                        people[i].destY = ClampGridY(cy * CELL + STATUS_BAR_HEIGHT);
                        people[i].taken = false;
                        break;
                    }
                }
            }
        } 
        else 
        {
            if (!me.hasBox) 
            {
                for (int i = 0; i < packCount; ++i) 
                {
                    if (!packs[i].taken && IsNear(packs[i].x, packs[i].y)) 
                    {
                        me.hasBox = true;
                        packs[i].taken = true;
                        playerJobCount++;
                        break;
                    }
                }
            } 
            else 
            {
                for (int i = 0; i < packCount; ++i) 
                {
                    if (packs[i].taken && IsAt(packs[i].destX, packs[i].destY)) 
                    {
                        me.hasBox = false;
                        me.pts += 20;
                        me.cash += 10;
                        int cx, cy;
                        
                        do 
                        {
                            GetRoad(cx, cy); 
                        } while (IsTaken(cx, cy));
                        packs[i].x = cx * CELL;
                        packs[i].y = ClampGridY(cy * CELL + STATUS_BAR_HEIGHT);
                        
                        do 
                        {
                            GetRoad(cx, cy);
                        } while (IsTaken(cx, cy) || grid[cx][cy] == BUILDING);
                        packs[i].destX = cx * CELL;
                        packs[i].destY = ClampGridY(cy * CELL + STATUS_BAR_HEIGHT);
                        packs[i].taken = false;
                        break;
                    }
                }
            }
        }
    }
    if (gameState == GAME_OVER) 
    {
        if (key == 13) 
        {
            ResetMenu();
            gameState = MENU;
            glutPostRedisplay();
        }
        return;
    }
    glutPostRedisplay();
}

bool CanMove(int cellX, int cellY) 
{
    int w = W / CELL;
    int h = H / CELL;
    if (cellX < 0 || cellX >= w || cellY < 0 || cellY >= h) 
    {
        return false;
    }
    if (!IsRoad(cellX, cellY)) 
    {
        return false;
    }
    for (int i = 0; i < npcCount; ++i)
    {
        if (npcs[i].x / CELL == cellX && npcs[i].y / CELL == cellY) 
        {
            return false;
        }
    }
    return true;
}

void MoveCars() 
{
    int w = W / CELL;
    int h = H / CELL;
    for (int i = 0; i < npcCount; ++i) 
    {
        if (!npcs[i].active)
        {
            continue;
        }
        int cx = npcs[i].x / CELL;
        int cy = (npcs[i].y - STATUS_BAR_HEIGHT) / CELL;
        int dirs[4] = {0, 1, 2, 3};
        
        for (int j = 3; j > 0; --j) 
        {
            int k = rand() % (j + 1);
            int temp = dirs[j];
            dirs[j] = dirs[k];
            dirs[k] = temp;
        }
        bool moved = false;
        
        for (int d = 0; d < 4; ++d) 
        {
            int dir = dirs[d];
            int nx = cx + dx[dir];
            int ny = cy + dy[dir];
            if (nx >= 0 && nx < w && ny >= 0 && ny < h && CanMove(nx, ny)) 
            {
                npcs[i].x = nx * CELL;
                npcs[i].y = ClampGridY(ny * CELL + STATUS_BAR_HEIGHT);
                npcs[i].dir = dir;
                moved = true;
                break;
            }
        }
        if (!moved) 
        {
            int attempts = 0;
            while (attempts < 10) 
            {
                int newX = rand() % w;
                int newY = rand() % h;
                if (CanMove(newX, newY)) 
                {
                    npcs[i].x = newX * CELL;
                    npcs[i].y = ClampGridY(newY * CELL + STATUS_BAR_HEIGHT);
                    npcs[i].dir = rand() % 4;
                    break;
                }
                attempts++;
            }
        }
    }
}

void GameLoop() 
{
    if (gameState == RUNNING) 
    {
        carMoveIdleTick++;
        if (carMoveIdleTick >= CAR_MOVE_IDLE_INTERVAL) 
        {
            for (int s = 0; s < carSpeed; ++s) 
            {
                MoveCars();
            }
            carMoveIdleTick = 0;
        }
    }
    glutPostRedisplay();
}

void GameTimer(int m) 
{
    if (gameState == GAME_OVER) 
    {
        glutTimerFunc(30, GameTimer, 0);
        return;
    }
    if (gameState == RUNNING) 
    {
        gameTicks++;
        TryGas();
        if (playerJobCount > 0 && playerJobCount % 2 == 0) 
        {
            if (npcCount < MAX_NPC) 
            {
                int cx, cy;
                do 
                { 
                    GetRoad(cx, cy); 
                } while (IsTaken(cx, cy));
                npcs[npcCount].x = cx * CELL;
                npcs[npcCount].y = ClampGridY(cy * CELL + STATUS_BAR_HEIGHT);
                npcs[npcCount].dir = rand() % 4;
                npcs[npcCount].gas = 100;
                npcs[npcCount].active = true;
                npcCount++;
                carSpeed++;
            }
            playerJobCount = 0;
        }
        if (me.gas <= 0 || me.pts < 0 || gameTicks >= MAX_TICKS) 
        {
            AddScore(playerName, me.pts);
            gameState = GAME_OVER;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(30, GameTimer, 0);
}

void MouseMove(int x, int y) 
{
    glutPostRedisplay();
}
void MouseHover(int x, int y) 
{
    glutPostRedisplay();
}
void MouseClick(int button, int state, int x, int y) 
{
    glutPostRedisplay();
}

void DrawGrid() 
{
    int w = W / CELL;
    int h = H / CELL;
    DrawRectangle(0, STATUS_BAR_HEIGHT, W, H, colors[DARK_GRAY]);
    for (int i = 0; i < w; ++i) 
    {
        for (int j = 0; j < h; ++j) 
        {
            int x = i * CELL;
            int y = STATUS_BAR_HEIGHT + j * CELL;
            if (grid[i][j] == ROAD) {
                DrawSquare(x, y, CELL, colors[GRAY]);
                if ((i % 3 == 0 || j % 3 == 0)) 
                {
                    for (int seg = 0; seg < CELL; seg += 12) 
                    {
                        DrawRectangle(x + CELL/2 - 2, y + seg, 4, 8, colors[YELLOW]);
                    }
                }
                DrawRectangle(x, y, 2, CELL, colors[WHITE]);
                DrawRectangle(x + CELL - 2, y, 2, CELL, colors[WHITE]);
                DrawRectangle(x, y, CELL, 2, colors[WHITE]);
                DrawRectangle(x, y + CELL - 2, CELL, 2, colors[WHITE]);
            } 
            else 
            {
                DrawSquare(x, y, CELL, colors[DARK_BLUE]);
            }
        }
    }
    DrawRectangle(0, STATUS_BAR_HEIGHT, CELL, CELL, colors[ORANGE]);
    if (me.job == 0 && me.hasRider) 
    {
        for (int i = 0; i < peopleCount; ++i) 
        {
            if (people[i].taken) 
            {
                DrawSquare(people[i].destX, people[i].destY, CELL, colors[GREEN]);
                break;
            }
        }
    } 
    else if (me.job == 1 && me.hasBox) 
    {
        for (int i = 0; i < packCount; ++i) 
        {
            if (packs[i].taken) 
            {
                DrawSquare(packs[i].destX, packs[i].destY, CELL, colors[GREEN]);
                break;
            }
        }
    }
}

void SwitchRole() 
{
    me.job = (me.job == 0) ? 1 : 0;
    InitGame();
}

int main(int argc, char*argv[]) 
{
    int width = W;
    int height = H + STATUS_BAR_HEIGHT;
    InitRandomizer();
    LoadScores();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(50, 50);
    glutInitWindowSize(width, height);
    glutCreateWindow("OOP Project");
    SetCanvas(width, height);
    glutDisplayFunc(DrawGame);
    glutSpecialFunc(MoveKeys);
    glutKeyboardFunc(KeyPress);
    glutTimerFunc(30, GameTimer, 0);
    glutMouseFunc(MouseClick);
    glutPassiveMotionFunc(MouseHover);
    glutMotionFunc(MouseMove);
    glutIdleFunc(GameLoop);
    ResetMenu();
    glutMainLoop();
    return 1;
}

#endif /* RushHour_CPP_ */
