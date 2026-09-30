#define NOMINMAX
#define _USE_MATH_DEFINES
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================
// 3D SKY COIN COLLECTOR & FLIGHT ADVENTURE (DELUXE EDITION)
//
// Refined Features:
// - Ultra-sleek, aerodynamic modern jetliner model:
//     * Curved multi-panel cockpit canopy with specular tint
//     * Dual-tone pearlescent livery with metallic gold cheatlines
//     * Glowing passenger cabin windows (port & starboard)
//     * Swept wings with blended winglets & flap track canoes
//     * Twin high-bypass turbofan engines with chrome intake rims & spinners
//     * Active navigation lights (red/green wingtips, pulsing strobes)
//     * Aerodynamic wingtip contrails / vapor ribbons
// - "Fine" atmospheric background & environment:
//     * Smooth multi-stop sky dome gradient (Cerulean -> Golden Horizon)
//     * Depth-cueing aerial perspective with OpenGL soft atmospheric fog
//     * Rolling mountain ridges and scenic patchwork terrain below
//     * Meandering blue river with specular glint
//     * Shimmering stars at dusk/night & luminous sun corona
// - Fluid, responsive arcade controls (Arrows & WASD)
// - Spinning 3D gold coins, Magnet, Nitro Boost, and Shield orbs
//
// American International University - Bangladesh (AIUB)
// Computer Graphics [Section: U]
// Student Project Team:
// - Amartay Das            [23-55068-3]
// - MD.Woasi Afzal Tonmoy   [23-53233-3]
// - Raiyan Ahnaf           [23-54227-3]
// - Sazzad Hossain Rabby   [23-54859-3]
// ============================================================

// ---------------- WINDOW CONFIGURATION ----------------
static int windowWidth  = 1280;
static int windowHeight = 720;

// ---------------- GAME STATE ----------------
enum GameState {
    STATE_PLAYING,
    STATE_GAME_OVER
};

static GameState gameState = STATE_PLAYING;
static bool showTeamInfo = false;

// ---------------- PLAYER SCORE & STATS ----------------
static int totalCoinsCollected = 0;
static int score = 0;
static float distanceTraveled = 0.0f;
static float coinPopTimer = 0.0f;
static char popupText[64] = "";

// Power-up States
static float magnetTimer = 0.0f;     // Active coin magnet duration
static float boostTimer  = 0.0f;     // Active nitro boost duration
static bool  hasShield   = true;     // 1-hit protection bubble

// ---------------- AIRCRAFT ATTRIBUTES ----------------
static float planeX       = 0.0f;    // Moves forward along +X
static float planeY       = 25.0f;   // Altitude (controlled by UP / DOWN)
static float planeZ       = 0.0f;    // Horizontal Lane (controlled by LEFT / RIGHT)

static float targetY      = 25.0f;
static float targetZ      = 0.0f;

static float planeRoll    = 0.0f;    // Dynamic bank angle
static float planePitch   = 0.0f;    // Dynamic pitch angle
static float baseSpeed    = 0.58f;   // Cruising forward speed
static float turbineSpin  = 0.0f;
static float strobePulse  = 0.0f;    // Beacon flash timer

// Limits
static const float MIN_ALT = 6.0f;
static const float MAX_ALT = 58.0f;
static const float MIN_Z   = -36.0f;
static const float MAX_Z   =  36.0f;

// ---------------- 3D COINS SYSTEM ----------------
#define MAX_COINS 65
struct CoinItem {
    float x, y, z;
    bool active;
    float rotAngle;
};

static CoinItem coins[MAX_COINS];

// ---------------- POWER-UPS & SPEED RINGS ----------------
#define MAX_POWERUPS 6
struct PowerUpItem {
    float x, y, z;
    int type; // 0 = Magnet, 1 = Speed Ring, 2 = Shield
    bool active;
};

static PowerUpItem powerUps[MAX_POWERUPS];

// ---------------- OBSTACLES: HOT AIR BALLOONS ----------------
#define MAX_BALLOONS 8
struct BalloonItem {
    float x, y, z;
    float r, g, b;
    bool active;
};

static BalloonItem balloons[MAX_BALLOONS];

// ---------------- CAMERA PERSPECTIVES ----------------
// 0 = Wide Overview, 1 = Chase Cam (Default), 2 = Cockpit View
static int cameraMode = 1;
static float camX = -35.0f, camY = 32.0f, camZ = 0.0f;
static float camTargetX = 20.0f, camTargetY = 25.0f, camTargetZ = 0.0f;

// ---------------- ENVIRONMENT & TIME ----------------
static float dayValue = 1.0f;         // 1.0 = Radiant Day, 0.0 = Starry Night
static float targetDayValue = 1.0f;
static float cloudMoveOffset = 0.0f;

// ============================================================
// PROCEDURAL MATH & OPENGL 1.1 DRAWING PRIMITIVES
// ============================================================

static float clampVal(float v, float minV, float maxV) {
    if (v < minV) return minV;
    if (v > maxV) return maxV;
    return v;
}

static void drawCube(float w, float h, float d) {
    glPushMatrix();
    glScalef(w, h, d);
    glutSolidCube(1.0);
    glPopMatrix();
}

// Procedural Smooth Cylinder along Z axis
static void drawCylinderZ(float radius, float length, int slices) {
    float step = 2.0f * (float)M_PI / (float)slices;
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; i++) {
        float a = i * step;
        float c = (float)cos(a);
        float s = (float)sin(a);
        glNormal3f(c, s, 0.0f);
        glVertex3f(radius * c, radius * s, 0.0f);
        glVertex3f(radius * c, radius * s, length);
    }
    glEnd();

    // End Caps
    glNormal3f(0.0f, 0.0f, -1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = slices; i >= 0; i--) {
        float a = i * step;
        glVertex3f(radius * (float)cos(a), radius * (float)sin(a), 0.0f);
    }
    glEnd();

    glNormal3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, length);
    for (int i = 0; i <= slices; i++) {
        float a = i * step;
        glVertex3f(radius * (float)cos(a), radius * (float)sin(a), length);
    }
    glEnd();
}

// Procedural Smooth Cone / Frustum along Z axis
static void drawConeZ(float r1, float r2, float length, int slices) {
    float step = 2.0f * (float)M_PI / (float)slices;
    float dr = r1 - r2;
    float l = (float)sqrt(dr * dr + length * length);
    float nz = dr / (l > 0.0001f ? l : 1.0f);
    float nr = length / (l > 0.0001f ? l : 1.0f);

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; i++) {
        float a = i * step;
        float c = (float)cos(a);
        float s = (float)sin(a);
        glNormal3f(c * nr, s * nr, nz);
        glVertex3f(r1 * c, r1 * s, 0.0f);
        glVertex3f(r2 * c, r2 * s, length);
    }
    glEnd();

    if (r1 > 0.001f) {
        glNormal3f(0.0f, 0.0f, -1.0f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.0f, 0.0f);
        for (int i = slices; i >= 0; i--) {
            float a = i * step;
            glVertex3f(r1 * (float)cos(a), r1 * (float)sin(a), 0.0f);
        }
        glEnd();
    }
    if (r2 > 0.001f) {
        glNormal3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.0f, length);
        for (int i = 0; i <= slices; i++) {
            float a = i * step;
            glVertex3f(r2 * (float)cos(a), r2 * (float)sin(a), length);
        }
        glEnd();
    }
}

// Tapered Elliptical Fuselage Section
static void drawFuselageSection(float r1x, float r1y, float r2x, float r2y, float length, int slices) {
    float step = 2.0f * (float)M_PI / (float)slices;
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; i++) {
        float a = i * step;
        float c = (float)cos(a);
        float s = (float)sin(a);
        glNormal3f(c, s * 0.9f, 0.1f);
        glVertex3f(r1x * c, r1y * s, 0.0f);
        glVertex3f(r2x * c, r2y * s, length);
    }
    glEnd();
}

// ---------------- 2D GUI PIPELINE ----------------
static void begin2D() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG); // Ensure HUD text is razor-sharp without fog
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void end2D() {
    glDisable(GL_BLEND);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

static void drawText(float x, float y, const char* str, void* font) {
    glRasterPos2f(x, y);
    while (*str) {
        glutBitmapCharacter(font, *str);
        str++;
    }
}

static void drawText(float x, float y, const char* str) {
    drawText(x, y, str, GLUT_BITMAP_HELVETICA_12);
}

static void drawPanel(float x, float y, float w, float h, float r, float g, float b, float a) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

// ============================================================
// 3D MODELS: REFINED AIRPLANE, COINS, POWER-UPS & BALLOONS
// ============================================================

// Shiny Rotating 3D Gold Coin with Star Emblem
static void draw3DGoldCoin(float rot) {
    glPushMatrix();
    glRotatef(rot, 0.0f, 1.0f, 0.0f); // Spin on Y axis

    // Glowing Gold Metallic Materials
    GLfloat goldMat[]  = { 1.0f, 0.82f, 0.12f, 1.0f };
    GLfloat goldSpec[] = { 1.0f, 0.96f, 0.65f, 1.0f };
    GLfloat starMat[]  = { 1.0f, 0.98f, 0.35f, 1.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, goldMat);
    glMaterialfv(GL_FRONT, GL_SPECULAR, goldSpec);
    glMaterialf(GL_FRONT, GL_SHININESS, 64.0f);

    // Thick Coin Rim Cylinder
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.2f);
    drawCylinderZ(1.35f, 0.4f, 20);
    glPopMatrix();

    // Raised Star Emblem on Coin Faces
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, starMat);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.22f);
    drawCube(0.7f, 0.7f, 0.06f);
    glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
    drawCube(0.7f, 0.7f, 0.06f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.22f);
    drawCube(0.7f, 0.7f, 0.06f);
    glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
    drawCube(0.7f, 0.7f, 0.06f);
    glPopMatrix();

    glPopMatrix();
}

// Power-Up Model (Magnet / Boost / Shield)
static void drawPowerUp(int type) {
    glPushMatrix();

    if (type == 0) {
        // Coin Magnet Orb (Neon Blue Horseshoe Ring)
        GLfloat blueGlow[] = { 0.12f, 0.65f, 1.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blueGlow);
        glutSolidSphere(1.4, 18, 18);

        // White Pole Caps
        GLfloat white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
        glPushMatrix();
        glTranslatef(0.0f, 1.25f, 0.0f);
        drawCube(0.6f, 0.4f, 0.6f);
        glPopMatrix();
    } else if (type == 1) {
        // Speed Boost Ring (Neon Green Torus)
        GLfloat neonGreen[] = { 0.15f, 1.0f, 0.35f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, neonGreen);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        drawCylinderZ(3.2f, 0.65f, 22);
    } else {
        // Protective Shield (Golden Star Prism)
        GLfloat goldGlow[] = { 1.0f, 0.88f, 0.18f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, goldGlow);
        glutSolidSphere(1.35, 16, 16);
    }

    glPopMatrix();
}

// Sky Obstacle: Hot Air Balloon
static void drawHotAirBalloon(float r, float g, float b) {
    glPushMatrix();

    GLfloat col1[] = { r, g, b, 1.0f };
    GLfloat col2[] = { 0.96f, 0.96f, 0.96f, 1.0f };
    GLfloat basketCol[] = { 0.58f, 0.38f, 0.18f, 1.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col1);
    glPushMatrix();
    glTranslatef(0.0f, 3.6f, 0.0f);
    glutSolidSphere(3.3, 20, 20);
    glPopMatrix();

    // Bottom Tapered Skirt
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col2);
    glPushMatrix();
    glTranslatef(0.0f, 1.8f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    drawConeZ(1.85f, 0.95f, 1.7f, 18);
    glPopMatrix();

    // Passenger Basket
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, basketCol);
    glPushMatrix();
    glTranslatef(0.0f, -0.4f, 0.0f);
    drawCube(1.4f, 1.2f, 1.4f);
    glPopMatrix();

    glPopMatrix();
}

// ============================================================
// HIGH-DETAIL LUXURY JETLINER MODEL (BEAUTIFUL 3D PLANE)
// ============================================================

static void drawAirplane() {
    glPushMatrix();

    // Refined High-End Materials
    GLfloat pearlWhite[]  = { 0.97f, 0.98f, 1.00f, 1.0f };
    GLfloat royalBlue[]   = { 0.05f, 0.28f, 0.72f, 1.0f };
    GLfloat metallicGold[]= { 0.96f, 0.78f, 0.16f, 1.0f };
    GLfloat chromeMetal[] = { 0.88f, 0.90f, 0.94f, 1.0f };
    GLfloat darkGraphite[]= { 0.12f, 0.14f, 0.16f, 1.0f };
    GLfloat glassCyan[]   = { 0.18f, 0.55f, 0.85f, 0.92f };
    GLfloat windowGlow[]  = { 0.95f, 0.92f, 0.75f, 1.0f };
    GLfloat lightRed[]    = { 1.00f, 0.15f, 0.15f, 1.0f };
    GLfloat lightGreen[]  = { 0.15f, 1.00f, 0.25f, 1.0f };
    GLfloat strobeWhite[] = { 1.00f, 1.00f, 1.00f, 1.0f };

    // Shiny Specular Highlights on Wings & Body
    GLfloat highSpec[] = { 0.9f, 0.9f, 0.95f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, highSpec);
    glMaterialf(GL_FRONT, GL_SHININESS, 60.0f);

    // 1. Aerodynamic Main Fuselage (Pearl White Upper + Smooth Contours)
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pearlWhite);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -5.6f);
    drawFuselageSection(1.28f, 1.34f, 1.28f, 1.34f, 11.2f, 22);
    glPopMatrix();

    // 2. Tapered Cockpit & Aerodynamic Nose Cone
    glPushMatrix();
    glTranslatef(0.0f, 0.05f, 5.6f);
    drawFuselageSection(1.28f, 1.34f, 0.40f, 0.42f, 3.4f, 22);

    // Sleek Radome / Nose Cap (Graphite Carbon)
    glTranslatef(0.0f, -0.04f, 3.4f);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, darkGraphite);
    glutSolidSphere(0.42, 16, 16);
    glPopMatrix();

    // 3. Multi-Panel Tinted Cockpit Windshield (Wrap-around glass)
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, glassCyan);
    glMaterialf(GL_FRONT, GL_SHININESS, 90.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.62f, 6.4f);
    glRotatef(22.0f, 1.0f, 0.0f, 0.0f);
    // Center windshield
    drawCube(1.15f, 0.38f, 0.92f);
    // Side cockpit quarter windows
    glTranslatef(-0.62f, -0.06f, -0.25f);
    glRotatef(18.0f, 0.0f, 1.0f, 0.0f);
    drawCube(0.25f, 0.34f, 0.70f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.62f, 0.56f, 6.15f);
    glRotatef(22.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(-18.0f, 0.0f, 1.0f, 0.0f);
    drawCube(0.25f, 0.34f, 0.70f);
    glPopMatrix();

    // 4. Elegant Luxury Livery (Royal Blue Underbelly + Gold Pinstripe)
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, royalBlue);
    glPushMatrix();
    glTranslatef(0.0f, -0.35f, -0.2f);
    drawCube(2.62f, 0.72f, 12.0f);
    glPopMatrix();

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, metallicGold);
    glPushMatrix();
    glTranslatef(0.0f, 0.04f, -0.2f);
    drawCube(2.64f, 0.10f, 12.2f);
    glPopMatrix();

    // 5. Glowing Passenger Cabin Windows (Left & Right Flanks)
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, windowGlow);
    for (int w = -7; w <= 6; w++) {
        float wz = (float)w * 0.75f - 0.2f;
        // Left Windows
        glPushMatrix();
        glTranslatef(-1.32f, 0.22f, wz);
        drawCube(0.08f, 0.22f, 0.32f);
        glPopMatrix();

        // Right Windows
        glPushMatrix();
        glTranslatef(1.32f, 0.22f, wz);
        drawCube(0.08f, 0.22f, 0.32f);
        glPopMatrix();
    }

    // 6. Sleek Swept Wings with Airfoil Shape & Winglets
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pearlWhite);

    // Left Wing (Port)
    glPushMatrix();
    glTranslatef(-7.0f, -0.25f, 0.5f);
    glRotatef(-13.0f, 0.0f, 1.0f, 0.0f); // Backward sweep
    glRotatef(-3.5f, 0.0f, 0.0f, 1.0f);  // Dihedral angle
    drawCube(11.8f, 0.22f, 2.7f);

    // Flap track canoes under left wing
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, darkGraphite);
    for (int f = 0; f < 3; f++) {
        glPushMatrix();
        glTranslatef(-2.2f + f * 2.8f, -0.18f, -1.2f);
        drawConeZ(0.14f, 0.02f, 1.4f, 10);
        glPopMatrix();
    }

    // Blended Winglet (Left)
    glTranslatef(-5.7f, 0.75f, -0.2f);
    glRotatef(72.0f, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, royalBlue);
    drawCube(1.5f, 0.12f, 1.1f);
    glTranslatef(0.0f, 0.35f, 0.0f);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, metallicGold);
    drawCube(0.8f, 0.14f, 0.5f);

    // Port Red Navigation Light
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, lightRed);
    glTranslatef(0.0f, 0.45f, 0.2f);
    glutSolidSphere(0.18, 12, 12);
    glPopMatrix();

    // Right Wing (Starboard)
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pearlWhite);
    glPushMatrix();
    glTranslatef(7.0f, -0.25f, 0.5f);
    glRotatef(13.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(3.5f, 0.0f, 0.0f, 1.0f);
    drawCube(11.8f, 0.22f, 2.7f);

    // Flap track canoes under right wing
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, darkGraphite);
    for (int f = 0; f < 3; f++) {
        glPushMatrix();
        glTranslatef(2.2f - f * 2.8f, -0.18f, -1.2f);
        drawConeZ(0.14f, 0.02f, 1.4f, 10);
        glPopMatrix();
    }

    // Blended Winglet (Right)
    glTranslatef(5.7f, 0.75f, -0.2f);
    glRotatef(-72.0f, 0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, royalBlue);
    drawCube(1.5f, 0.12f, 1.1f);
    glTranslatef(0.0f, 0.35f, 0.0f);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, metallicGold);
    drawCube(0.8f, 0.14f, 0.5f);

    // Starboard Green Navigation Light
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, lightGreen);
    glTranslatef(0.0f, 0.45f, 0.2f);
    glutSolidSphere(0.18, 12, 12);
    glPopMatrix();

    // Wingtip Aerodynamic Contrail Ribbons
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    GLfloat contrailCol[] = { 0.85f, 0.95f, 1.0f, (boostTimer > 0.0f) ? 0.65f : 0.25f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, contrailCol);
    glPushMatrix();
    glTranslatef(-12.8f, 0.35f, -8.0f);
    drawCube(0.15f, 0.15f, 14.0f);
    glTranslatef(25.6f, 0.0f, 0.0f);
    drawCube(0.15f, 0.15f, 14.0f);
    glPopMatrix();
    glDisable(GL_BLEND);

    // 7. Twin High-Bypass Turbofan Engines
    for (int s = -1; s <= 1; s += 2) {
        glPushMatrix();
        glTranslatef(s * 3.9f, -1.22f, 0.9f);

        // Aerodynamic Pylon
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, darkGraphite);
        glPushMatrix();
        glTranslatef(0.0f, 0.48f, 0.0f);
        drawCube(0.18f, 0.62f, 1.45f);
        glPopMatrix();

        // Nacelle Body (Pearl White & Blue Trim)
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pearlWhite);
        drawCylinderZ(0.68f, 2.8f, 18);

        // Chrome Intake Cowl Lip
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, chromeMetal);
        glMaterialfv(GL_FRONT, GL_SPECULAR, highSpec);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 2.8f);
        drawConeZ(0.68f, 0.62f, 0.32f, 18);

        // Spinning Turbine Fan Spinner & Spiral
        glRotatef(turbineSpin, 0.0f, 0.0f, 1.0f);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, darkGraphite);
        drawConeZ(0.24f, 0.02f, 0.50f, 14);

        // Turbofan Fan Blades
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, chromeMetal);
        for (int b = 0; b < 6; b++) {
            glPushMatrix();
            glRotatef(b * 60.0f, 0.0f, 0.0f, 1.0f);
            drawCube(0.08f, 0.46f, 0.06f);
            glPopMatrix();
        }
        glPopMatrix();

        // Engine Exhaust Cone & Afterburner Glow (when nitro boost is active)
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.0f);
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, darkGraphite);
        drawConeZ(0.65f, 0.42f, 0.45f, 16);

        if (boostTimer > 0.0f) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            GLfloat flameCol[] = { 0.2f, 0.75f, 1.0f, 0.75f };
            glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, flameCol);
            drawConeZ(0.38f, 0.05f, 2.2f, 14);
            glDisable(GL_BLEND);
        }
        glPopMatrix();

        glPopMatrix();
    }

    // 8. Tapered Tail Section & Swept Vertical Fin
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -5.6f);
    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pearlWhite);
    drawFuselageSection(1.28f, 1.34f, 0.35f, 0.38f, 4.4f, 20);
    glPopMatrix();

    // Swept Vertical Stabilizer with Royal Blue & Gold Crest
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, royalBlue);
    glPushMatrix();
    glTranslatef(0.0f, 2.95f, -7.5f);
    glRotatef(26.0f, 1.0f, 0.0f, 0.0f);
    drawCube(0.22f, 4.2f, 2.6f);

    // Gold Fin Tip Accent
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, metallicGold);
    glTranslatef(0.0f, 1.6f, -0.4f);
    drawCube(0.24f, 1.1f, 1.1f);

    // Pulsing Strobe Beacon on Tail
    float flash = (float)sin(strobePulse * 8.0f);
    if (flash > 0.3f) {
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, strobeWhite);
    } else {
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, lightRed);
    }
    glTranslatef(0.0f, 0.7f, 0.0f);
    glutSolidSphere(0.18, 12, 12);
    glPopMatrix();

    // Swept Horizontal Stabilizers (Elevators)
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, pearlWhite);
    glPushMatrix();
    glTranslatef(0.0f, 0.85f, -8.2f);
    drawCube(6.6f, 0.18f, 1.8f);
    // Gold stabilizer tips
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, metallicGold);
    glPushMatrix();
    glTranslatef(-3.25f, 0.0f, 0.0f);
    drawCube(0.4f, 0.20f, 1.7f);
    glTranslatef(6.5f, 0.0f, 0.0f);
    drawCube(0.4f, 0.20f, 1.7f);
    glPopMatrix();
    glPopMatrix();

    // Top Fuselage Anti-Collision Strobe
    if (flash > 0.4f) {
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, strobeWhite);
        glPushMatrix();
        glTranslatef(0.0f, 1.38f, 0.5f);
        glutSolidSphere(0.16, 10, 10);
        glPopMatrix();
    }

    // 9. Protective Shield Bubble
    if (hasShield) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        GLfloat shieldGlow[] = { 0.15f, 0.80f, 1.0f, 0.32f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, shieldGlow);
        glutSolidSphere(7.8, 22, 22);
        glDisable(GL_BLEND);
    }

    glPopMatrix();
}

// ============================================================
// "FINE" ATMOSPHERIC BACKGROUND, SKY DOME & SCENIC TERRAIN
// ============================================================

// Procedural Gradient Atmospheric Sky Dome & Sun/Moon
static void drawFineAtmosphere() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glPushMatrix();
    glTranslatef(planeX, 0.0f, 0.0f);

    // Multi-Stop Vertical Atmospheric Sky Curtain
    // Zenith Color (High Sky)
    float zR = 0.16f * dayValue + 0.01f * (1.0f - dayValue);
    float zG = 0.58f * dayValue + 0.02f * (1.0f - dayValue);
    float zB = 0.96f * dayValue + 0.12f * (1.0f - dayValue);

    // Mid Sky Color
    float mR = 0.30f * dayValue + 0.03f * (1.0f - dayValue);
    float mG = 0.72f * dayValue + 0.05f * (1.0f - dayValue);
    float mB = 1.00f * dayValue + 0.22f * (1.0f - dayValue);

    // Golden / Amber Horizon Glow
    float hR = 0.88f * dayValue + 0.08f * (1.0f - dayValue);
    float hG = 0.76f * dayValue + 0.09f * (1.0f - dayValue);
    float hB = 0.65f * dayValue + 0.18f * (1.0f - dayValue);

    // Ground Fog / Base
    float gR = 0.30f * dayValue + 0.03f * (1.0f - dayValue);
    float gG = 0.45f * dayValue + 0.04f * (1.0f - dayValue);
    float gB = 0.32f * dayValue + 0.08f * (1.0f - dayValue);

    float domeDist = 550.0f;
    int rings = 18;
    float step = 2.0f * (float)M_PI / (float)rings;

    // Upper Hemisphere: Zenith to Horizon
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= rings; i++) {
        float a = i * step;
        float c = (float)cos(a) * domeDist;
        float s = (float)sin(a) * domeDist;

        // Top Vertex (Zenith)
        glColor3f(zR, zG, zB);
        glVertex3f(c, 240.0f, s);

        // Mid Vertex
        glColor3f(mR, mG, mB);
        glVertex3f(c * 0.95f, 70.0f, s * 0.95f);
    }
    glEnd();

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= rings; i++) {
        float a = i * step;
        float c = (float)cos(a) * domeDist;
        float s = (float)sin(a) * domeDist;

        // Mid Vertex
        glColor3f(mR, mG, mB);
        glVertex3f(c * 0.95f, 70.0f, s * 0.95f);

        // Horizon Vertex
        glColor3f(hR, hG, hB);
        glVertex3f(c, 0.0f, s);
    }
    glEnd();

    // Lower Skirt: Horizon to Deep Below Ground
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= rings; i++) {
        float a = i * step;
        float c = (float)cos(a) * domeDist;
        float s = (float)sin(a) * domeDist;

        glColor3f(hR, hG, hB);
        glVertex3f(c, 0.0f, s);

        glColor3f(gR, gG, gB);
        glVertex3f(c * 0.85f, -80.0f, s * 0.85f);
    }
    glEnd();

    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    // Celestial Sun with Radiant Corona Glow (Day) or Silver Moon (Night)
    glPushMatrix();
    if (dayValue > 0.25f) {
        // Luminous Golden Sun
        GLfloat sunMat[] = { 1.0f, 0.96f, 0.70f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, sunMat);
        glTranslatef(planeX + 160.0f, 120.0f, -140.0f);
        glutSolidSphere(14.0, 24, 24);

        // Translucent Sun Halo / Corona
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        GLfloat haloMat[] = { 1.0f, 0.85f, 0.4f, 0.35f * dayValue };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, haloMat);
        glutSolidSphere(26.0, 20, 20);
        glDisable(GL_BLEND);
    } else {
        // Moon
        GLfloat moonMat[] = { 0.92f, 0.94f, 1.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, moonMat);
        glTranslatef(planeX + 160.0f, 120.0f, -140.0f);
        glutSolidSphere(10.0, 20, 20);
    }
    glPopMatrix();

    // Drifting Volumetric Cloud Clusters
    GLfloat cloudMat[] = { 0.98f, 0.98f, 1.0f, 0.88f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cloudMat);
    for (int c = 0; c < 7; c++) {
        glPushMatrix();
        float cx = planeX - 160.0f + c * 75.0f;
        float cy = 58.0f + (c % 3) * 7.0f;
        float cz = -110.0f + (c % 4) * 55.0f;
        glTranslatef(cx, cy, cz);

        glutSolidSphere(8.5, 14, 14);
        glTranslatef(5.5f, -1.2f, 2.0f);
        glutSolidSphere(6.8, 12, 12);
        glTranslatef(-10.5f, -1.0f, -1.5f);
        glutSolidSphere(6.2, 12, 12);
        glPopMatrix();
    }
}

// Scenic Rolling Mountain Ridges & Meandering River
static void drawScenicGround() {
    // ============================================================
    // DETAILED AIRPORT BACKGROUND
    // IMPORTANT: This function ONLY changes the scenery.
    // Plane, coins, power-ups, balloons, HUD, controls and gameplay
    // remain exactly as in the previous game.
    // ============================================================

    // ---------- SKY-SIDE DISTANT MOUNTAINS ----------
    GLfloat mountainFar[] = {0.16f, 0.30f, 0.43f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mountainFar);

    for (int side = -1; side <= 1; side += 2) {
        for (int i = 0; i < 9; i++) {
            float x = planeX - 300.0f + i * 75.0f;
            float z = side * 175.0f;

            glPushMatrix();
            glTranslatef(x, 0.0f, z);
            glRotatef(-90.0f, 1, 0, 0);
            drawConeZ(42.0f, 0.0f,
                      38.0f + (i % 4) * 12.0f, 8);
            glPopMatrix();
        }
    }

    // ---------- AIRPORT GRASS ----------
    GLfloat grass[] = {0.12f, 0.38f, 0.16f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, grass);

    glPushMatrix();
    glTranslatef(planeX, -0.30f, 0.0f);
    drawCube(900.0f, 0.5f, 390.0f);
    glPopMatrix();

    // ---------- LIGHT/DARK GRASS STRIPS ----------
    GLfloat grassLight[] = {0.18f, 0.48f, 0.20f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, grassLight);

    for (int i = -4; i <= 4; i++) {
        glPushMatrix();
        glTranslatef(planeX + i * 95.0f, -0.02f, 105.0f);
        drawCube(55.0f, 0.04f, 45.0f);
        glPopMatrix();
    }

    // ============================================================
    // MAIN BLACK RUNWAY
    // ============================================================
    GLfloat runwayBlack[] = {0.008f, 0.010f, 0.014f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, runwayBlack);

    glPushMatrix();
    glTranslatef(planeX + 190.0f, 0.02f, 0.0f);
    drawCube(570.0f, 0.12f, 72.0f);
    glPopMatrix();

    // runway threshold
    GLfloat runwayWhite[] = {0.94f, 0.95f, 0.98f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, runwayWhite);

    for (int i = -4; i <= 4; i++) {
        glPushMatrix();
        glTranslatef(planeX + 12.0f, 0.16f, i * 7.0f);
        drawCube(18.0f, 0.04f, 2.4f);
        glPopMatrix();
    }

    // runway center dashes
    for (int i = 0; i < 25; i++) {
        glPushMatrix();
        glTranslatef(planeX + 8.0f + i * 24.0f, 0.18f, 0.0f);
        drawCube(12.0f, 0.04f, 1.5f);
        glPopMatrix();
    }

    // runway side lines
    for (int side = -1; side <= 1; side += 2) {
        glPushMatrix();
        glTranslatef(planeX + 190.0f, 0.18f, side * 32.5f);
        drawCube(570.0f, 0.04f, 1.8f);
        glPopMatrix();
    }

    // ============================================================
    // RGB + CMY RUNWAY LIGHTS
    // ============================================================
    glDisable(GL_LIGHTING);

    const float rgbcmy[6][3] = {
        {1.0f,0.0f,0.0f}, // Red
        {0.0f,1.0f,0.0f}, // Green
        {0.0f,0.0f,1.0f}, // Blue
        {0.0f,1.0f,1.0f}, // Cyan
        {1.0f,0.0f,1.0f}, // Magenta
        {1.0f,1.0f,0.0f}  // Yellow
    };

    for (int i = 0; i < 30; i++) {
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(planeX + 5.0f + i * 18.0f,
                         0.55f,
                         side * 37.0f);

            int c = (i + (side == 1 ? 2 : 0)) % 6;
            glColor3f(rgbcmy[c][0], rgbcmy[c][1], rgbcmy[c][2]);
            glutSolidSphere(0.72, 10, 8);
            glPopMatrix();
        }
    }

    glEnable(GL_LIGHTING);

    // ============================================================
    // TAXIWAYS
    // ============================================================
    GLfloat taxiDark[] = {0.045f, 0.055f, 0.065f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, taxiDark);

    // left taxiway
    glPushMatrix();
    glTranslatef(planeX + 105.0f, 0.04f, -72.0f);
    drawCube(310.0f, 0.08f, 25.0f);
    glPopMatrix();

    // right taxiway
    glPushMatrix();
    glTranslatef(planeX + 105.0f, 0.04f, 72.0f);
    drawCube(310.0f, 0.08f, 25.0f);
    glPopMatrix();

    // yellow taxi center lines
    GLfloat taxiYellow[] = {1.0f,0.72f,0.02f,1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, taxiYellow);

    for (int side = -1; side <= 1; side += 2) {
        for (int i = 0; i < 12; i++) {
            glPushMatrix();
            glTranslatef(planeX - 20.0f + i*28.0f,
                         0.16f,
                         side*72.0f);
            drawCube(13.0f,0.04f,0.8f);
            glPopMatrix();
        }
    }

    // ============================================================
    // AIRPORT TERMINAL COMPLEX
    // ============================================================
    GLfloat terminal[] = {0.88f,0.91f,0.94f,1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, terminal);

    // main terminal
    glPushMatrix();
    glTranslatef(planeX + 95.0f, 19.0f, -112.0f);
    drawCube(190.0f, 38.0f, 42.0f);
    glPopMatrix();

    // terminal side wings
    glPushMatrix();
    glTranslatef(planeX - 25.0f, 13.0f, -112.0f);
    drawCube(70.0f, 26.0f, 32.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(planeX + 215.0f, 13.0f, -112.0f);
    drawCube(70.0f, 26.0f, 32.0f);
    glPopMatrix();

    // blue glass wall
    GLfloat glass[] = {0.03f,0.38f,0.72f,1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, glass);

    for (int i = 0; i < 12; i++) {
        glPushMatrix();
        glTranslatef(planeX - 5.0f + i*18.0f,
                     20.0f,
                     -90.5f);
        drawCube(10.0f, 22.0f, 0.8f);
        glPopMatrix();
    }

    // terminal roof
    GLfloat roof[] = {0.025f,0.08f,0.15f,1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, roof);

    glPushMatrix();
    glTranslatef(planeX + 95.0f, 41.0f, -112.0f);
    drawCube(205.0f, 4.0f, 48.0f);
    glPopMatrix();

    // RGB/CMY roof lighting
    glDisable(GL_LIGHTING);
    for (int i = 0; i < 12; i++) {
        int c = i % 6;
        glColor3f(rgbcmy[c][0],rgbcmy[c][1],rgbcmy[c][2]);
        glPushMatrix();
        glTranslatef(planeX + 0.0f + i*18.0f,
                     43.5f,
                     -136.0f);
        glutSolidSphere(0.9,10,8);
        glPopMatrix();
    }
    glEnable(GL_LIGHTING);

    // ============================================================
    // CONTROL TOWER
    // ============================================================
    GLfloat towerBody[] = {0.72f,0.76f,0.80f,1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, towerBody);

    glPushMatrix();
    glTranslatef(planeX + 205.0f, 47.0f, -78.0f);
    drawCube(15.0f, 86.0f, 15.0f);
    glPopMatrix();

    GLfloat towerGlass[] = {0.02f,0.35f,0.58f,1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, towerGlass);

    glPushMatrix();
    glTranslatef(planeX + 205.0f, 91.0f, -78.0f);
    drawCube(34.0f, 14.0f, 30.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(planeX + 205.0f, 100.0f, -78.0f);
    drawCube(40.0f, 3.0f, 34.0f);
    glPopMatrix();

    // tower antenna
    glDisable(GL_LIGHTING);
    glColor3f(1.0f,0.0f,0.0f);
    glPushMatrix();
    glTranslatef(planeX + 205.0f, 112.0f, -78.0f);
    glRotatef(-90.0f,1,0,0);
    glutSolidCone(1.2,8.0,12,8);
    glPopMatrix();
    glEnable(GL_LIGHTING);

    // ============================================================
    // HANGARS
    // ============================================================
    GLfloat hangar[] = {0.48f,0.53f,0.58f,1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, hangar);

    for (int i = 0; i < 3; i++) {
        float hx = planeX - 105.0f + i*58.0f;

        glPushMatrix();
        glTranslatef(hx, 15.0f, -158.0f);
        drawCube(48.0f,30.0f,35.0f);
        glPopMatrix();

        // hangar door
        GLfloat door[] = {0.04f,0.16f,0.25f,1.0f};
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, door);

        glPushMatrix();
        glTranslatef(hx, 13.0f, -139.5f);
        drawCube(34.0f,23.0f,0.8f);
        glPopMatrix();

        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, hangar);
    }

    // ============================================================
    // PARKING LOT + STREET
    // ============================================================
    GLfloat parking[] = {0.075f,0.085f,0.10f,1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, parking);

    glPushMatrix();
    glTranslatef(planeX + 80.0f, 0.03f, -185.0f);
    drawCube(300.0f,0.07f,48.0f);
    glPopMatrix();

    GLfloat parkingWhite[] = {0.95f,0.95f,0.95f,1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, parkingWhite);

    for (int i = 0; i < 14; i++) {
        glPushMatrix();
        glTranslatef(planeX - 55.0f + i*22.0f,
                     0.12f,
                     -185.0f);
        drawCube(1.0f,0.03f,18.0f);
        glPopMatrix();
    }

    // airport access road
    GLfloat road[] = {0.035f,0.04f,0.045f,1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, road);

    glPushMatrix();
    glTranslatef(planeX + 80.0f,0.02f,-220.0f);
    drawCube(420.0f,0.08f,25.0f);
    glPopMatrix();

    // road center line
    GLfloat roadYellow[] = {1.0f,0.70f,0.02f,1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, roadYellow);

    for (int i=0;i<15;i++) {
        glPushMatrix();
        glTranslatef(planeX - 100.0f+i*28.0f,0.14f,-220.0f);
        drawCube(14.0f,0.03f,0.8f);
        glPopMatrix();
    }

    // ============================================================
    // AIRPORT TREES
    // ============================================================
    GLfloat trunk[] = {0.25f,0.12f,0.05f,1.0f};
    GLfloat leaves[] = {0.04f,0.40f,0.12f,1.0f};

    for (int i=0;i<9;i++) {
        float tx = planeX - 230.0f + i*55.0f;

        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, trunk);
        glPushMatrix();
        glTranslatef(tx,5.0f,-235.0f);
        drawCube(2.5f,10.0f,2.5f);
        glPopMatrix();

        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, leaves);
        glPushMatrix();
        glTranslatef(tx,13.0f,-235.0f);
        glutSolidSphere(8.0,16,12);
        glPopMatrix();
    }

    // ============================================================
    // AIRPORT SERVICE VEHICLES
    // ============================================================
    GLfloat vehicleRed[] = {0.85f,0.025f,0.025f,1.0f};
    GLfloat vehicleWhite[] = {0.92f,0.94f,0.96f,1.0f};

    for (int i=0;i<4;i++) {
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, vehicleRed);

        glPushMatrix();
        glTranslatef(planeX + 10.0f+i*42.0f,2.0f,-72.0f);
        drawCube(10.0f,3.5f,5.5f);
        glPopMatrix();

        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, vehicleWhite);

        glPushMatrix();
        glTranslatef(planeX + 10.0f+i*42.0f,4.7f,-72.0f);
        drawCube(5.0f,2.0f,4.0f);
        glPopMatrix();
    }

    // ============================================================
    // BLUE WATER BEYOND THE AIRPORT
    // ============================================================
    GLfloat water[] = {0.02f,0.25f,0.70f,1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, water);

    glPushMatrix();
    glTranslatef(planeX, -0.10f, 245.0f);
    drawCube(900.0f,0.08f,80.0f);
    glPopMatrix();

    // small cyan water strips
    glDisable(GL_LIGHTING);
    for (int i=0;i<14;i++) {
        glColor3f(0.0f,0.85f,1.0f);
        glPushMatrix();
        glTranslatef(planeX-330.0f+i*52.0f,0.03f,245.0f+(i%3)*8.0f);
        drawCube(24.0f,0.02f,0.7f);
        glPopMatrix();
    }
    glEnable(GL_LIGHTING);
}
// ============================================================
// SLEEK ARCADE HUD & INTERFACE
// ============================================================

static void drawHUD() {
    begin2D();

    char buf[128];

    // 1. Top Header Banner
    drawPanel(15, windowHeight - 75, windowWidth - 30, 60, 0.05f, 0.09f, 0.16f, 0.88f);
    drawPanel(15, windowHeight - 17, windowWidth - 30, 3, 0.98f, 0.80f, 0.15f, 1.0f);

    glColor3f(0.96f, 0.80f, 0.20f); // Gold
    drawText(28, windowHeight - 35, "AIUB - Computer Graphics [Section: U] | 3D Sky Coin Collector Adventure", GLUT_BITMAP_HELVETICA_12);

    glColor3f(0.20f, 0.85f, 1.0f);
    drawText(28, windowHeight - 58, "CONTROLS: [ARROWS] or [W/A/S/D] to Steer & Collect Coins!", GLUT_BITMAP_HELVETICA_12);

    // Help cue & Day/Night toggle
    glColor3f(0.4f, 1.0f, 0.5f);
    drawText(windowWidth - 210, windowHeight - 35, "[H] Student Team Credits", GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.8f, 0.85f, 0.95f);
    drawText(windowWidth - 210, windowHeight - 58, "[L] Day / Night Mode", GLUT_BITMAP_HELVETICA_12);

    // 2. Shiny Gold Coin Counter Card (Top Left)
    drawPanel(20, windowHeight - 190, 200, 100, 0.05f, 0.09f, 0.16f, 0.88f);
    drawPanel(20, windowHeight - 93, 200, 3, 1.0f, 0.82f, 0.15f, 1.0f);

    // Golden Coin Icon Symbol
    glColor3f(1.0f, 0.85f, 0.15f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 16; i++) {
        float a = i * 2.0f * (float)M_PI / 16.0f;
        glVertex2f(45.0f + (float)cos(a) * 16.0f, windowHeight - 145.0f + (float)sin(a) * 16.0f);
    }
    glEnd();
    glColor3f(0.3f, 0.2f, 0.0f);
    drawText(40, windowHeight - 150, "$", GLUT_BITMAP_HELVETICA_18);

    sprintf(buf, "%d", totalCoinsCollected);
    glColor3f(1.0f, 0.95f, 0.3f);
    drawText(75, windowHeight - 152, buf, GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(0.9f, 0.95f, 1.0f);
    sprintf(buf, "SCORE: %d PTS", score);
    drawText(75, windowHeight - 175, buf, GLUT_BITMAP_HELVETICA_12);

    // 3. Flight Telemetry & Power-Up Card (Bottom Left)
    drawPanel(20, 15, 230, 120, 0.05f, 0.09f, 0.16f, 0.88f);
    glColor3f(0.2f, 0.85f, 1.0f);
    drawText(30, 115, "FLIGHT STATUS", GLUT_BITMAP_HELVETICA_12);

    sprintf(buf, "ALTITUDE:  %3.0f M", planeY);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(30, 95, buf, GLUT_BITMAP_HELVETICA_12);

    sprintf(buf, "DISTANCE:  %4.0f M", distanceTraveled);
    drawText(30, 75, buf, GLUT_BITMAP_HELVETICA_12);

    // Power-Up Status Indicators
    if (magnetTimer > 0.0f) {
        sprintf(buf, "MAGNET ACTIVE: %.1fs", magnetTimer);
        glColor3f(0.2f, 0.8f, 1.0f);
        drawText(30, 52, buf, GLUT_BITMAP_HELVETICA_12);
    } else if (boostTimer > 0.0f) {
        sprintf(buf, "NITRO BOOST: %.1fs", boostTimer);
        glColor3f(0.2f, 1.0f, 0.3f);
        drawText(30, 52, buf, GLUT_BITMAP_HELVETICA_12);
    } else {
        glColor3f(hasShield ? 0.3f : 0.8f, hasShield ? 1.0f : 0.4f, 0.4f);
        drawText(30, 52, hasShield ? "SHIELD: READY" : "SHIELD: BROKEN", GLUT_BITMAP_HELVETICA_12);
    }

    const char* cName = (cameraMode == 0) ? "Tower" : (cameraMode == 1 ? "Chase" : "Cockpit");
    sprintf(buf, "CAM: %s [1/2/3]", cName);
    glColor3f(0.9f, 0.8f, 0.3f);
    drawText(30, 30, buf, GLUT_BITMAP_HELVETICA_12);

    // 4. Coin Pickup Flash Animation Pop-Up
    if (coinPopTimer > 0.0f) {
        drawPanel(windowWidth * 0.5f - 110, windowHeight * 0.5f + 60, 220, 42, 1.0f, 0.85f, 0.15f, 0.92f);
        glColor3f(0.1f, 0.1f, 0.1f);
        drawText(windowWidth * 0.5f - 65, windowHeight * 0.5f + 73, popupText, GLUT_BITMAP_HELVETICA_18);
    }

    // 5. Student Project Team Dialog ([H] key)
    if (showTeamInfo) {
        drawPanel(windowWidth * 0.5f - 280, windowHeight * 0.5f - 160, 560, 320, 0.04f, 0.08f, 0.14f, 0.96f);
        drawPanel(windowWidth * 0.5f - 280, windowHeight * 0.5f + 157, 560, 3, 0.95f, 0.78f, 0.16f, 1.0f);

        glColor3f(0.96f, 0.80f, 0.20f);
        drawText(windowWidth * 0.5f - 240, windowHeight * 0.5f + 125, "AMERICAN INTERNATIONAL UNIVERSITY - BANGLADESH", GLUT_BITMAP_HELVETICA_18);
        glColor3f(0.20f, 0.85f, 1.0f);
        drawText(windowWidth * 0.5f - 150, windowHeight * 0.5f + 95, "Department of CSE | Computer Graphics [Section: U]", GLUT_BITMAP_HELVETICA_12);

        glColor3f(1.0f, 0.85f, 0.3f);
        drawText(windowWidth * 0.5f - 250, windowHeight * 0.5f + 65, "STUDENT PROJECT TEAM:", GLUT_BITMAP_HELVETICA_12);
        glColor3f(0.92f, 0.95f, 1.0f);
        drawText(windowWidth * 0.5f - 240, windowHeight * 0.5f + 40, "1. Amartay Das               [ID: 23-55068-3]", GLUT_BITMAP_HELVETICA_12);
        drawText(windowWidth * 0.5f - 240, windowHeight * 0.5f + 18, "2. MD.Woasi Afzal Tonmoy      [ID: 23-53233-3]", GLUT_BITMAP_HELVETICA_12);
        drawText(windowWidth * 0.5f - 240, windowHeight * 0.5f - 04, "3. Raiyan Ahnaf              [ID: 23-54227-3]", GLUT_BITMAP_HELVETICA_12);
        drawText(windowWidth * 0.5f - 240, windowHeight * 0.5f - 26, "4. Sazzad Hossain Rabby      [ID: 23-54859-3]", GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.2f, 0.95f, 0.4f);
        drawText(windowWidth * 0.5f - 250, windowHeight * 0.5f - 58, "GAME FEATURES & HOW TO PLAY:", GLUT_BITMAP_HELVETICA_12);
        glColor3f(0.85f, 0.90f, 0.95f);
        drawText(windowWidth * 0.5f - 240, windowHeight * 0.5f - 80, "- LEFT / RIGHT: Steer plane to collect streams of gold coins (+50 pts)", GLUT_BITMAP_HELVETICA_12);
        drawText(windowWidth * 0.5f - 240, windowHeight * 0.5f - 100, "- UP / DOWN: Climb or dive to reach higher/lower coin lines", GLUT_BITMAP_HELVETICA_12);
        drawText(windowWidth * 0.5f - 240, windowHeight * 0.5f - 120, "- Collect Blue Magnet Orbs to vacuum all nearby coins automatically!", GLUT_BITMAP_HELVETICA_12);

        glColor3f(1.0f, 0.85f, 0.2f);
        drawText(windowWidth * 0.5f - 110, windowHeight * 0.5f - 146, "Press [H] to Close", GLUT_BITMAP_HELVETICA_12);
    }

    // 6. Game Over Dialog
    if (gameState == STATE_GAME_OVER) {
        drawPanel(windowWidth * 0.5f - 220, windowHeight * 0.5f - 75, 440, 150, 0.35f, 0.05f, 0.05f, 0.95f);
        glColor3f(1.0f, 0.2f, 0.2f);
        drawText(windowWidth * 0.5f - 110, windowHeight * 0.5f + 35, "AIRCRAFT CRASHED!", GLUT_BITMAP_HELVETICA_18);
        glColor3f(1.0f, 0.95f, 0.95f);
        sprintf(buf, "Coins Collected: %d | Total Score: %d PTS", totalCoinsCollected, score);
        drawText(windowWidth * 0.5f - 150, windowHeight * 0.5f + 5, buf, GLUT_BITMAP_HELVETICA_12);
        drawText(windowWidth * 0.5f - 110, windowHeight * 0.5f - 30, "Press [X] to Play Again", GLUT_BITMAP_HELVETICA_12);
    }

    end2D();
}

// ============================================================
// SIMULATION, COIN SPAWN & COLLISION DETECTION
// ============================================================

static void spawnCoinsAhead(float startX) {
    for (int i = 0; i < MAX_COINS; i++) {
        if (!coins[i].active || coins[i].x < planeX - 50.0f) {
            coins[i].active = true;
            coins[i].x = startX + (i * 14.0f) + (float)(rand() % 8);
            // Height wave pattern
            coins[i].y = 18.0f + 14.0f * (float)sin(coins[i].x * 0.05f);
            // Lane pattern
            coins[i].z = 18.0f * (float)sin(coins[i].x * 0.03f + (i % 3));
            coins[i].rotAngle = (float)(rand() % 360);
        }
    }

    // Spawn Obstacles (Hot Air Balloons)
    for (int b = 0; b < MAX_BALLOONS; b++) {
        if (!balloons[b].active || balloons[b].x < planeX - 50.0f) {
            balloons[b].active = true;
            balloons[b].x = startX + 60.0f + b * 90.0f + (float)(rand() % 30);
            balloons[b].y = 15.0f + (float)(rand() % 30);
            balloons[b].z = -25.0f + (float)(rand() % 50);
            balloons[b].r = 0.4f + (float)(rand() % 60) / 100.0f;
            balloons[b].g = 0.2f + (float)(rand() % 60) / 100.0f;
            balloons[b].b = 0.5f + (float)(rand() % 50) / 100.0f;
        }
    }

    // Spawn Power-Ups (Magnets & Speed Rings)
    for (int p = 0; p < MAX_POWERUPS; p++) {
        if (!powerUps[p].active || powerUps[p].x < planeX - 50.0f) {
            powerUps[p].active = true;
            powerUps[p].x = startX + 100.0f + p * 140.0f;
            powerUps[p].y = 20.0f + (float)(rand() % 20);
            powerUps[p].z = -20.0f + (float)(rand() % 40);
            powerUps[p].type = p % 3; // Magnet, Boost, Shield
        }
    }
}

static void resetGame() {
    planeX        = 0.0f;
    planeY        = 25.0f;
    planeZ        = 0.0f;
    targetY       = 25.0f;
    targetZ       = 0.0f;
    planeRoll     = 0.0f;
    planePitch    = 0.0f;
    totalCoinsCollected = 0;
    score         = 0;
    distanceTraveled = 0.0f;
    magnetTimer   = 0.0f;
    boostTimer    = 0.0f;
    hasShield     = true;
    gameState     = STATE_PLAYING;

    for (int i = 0; i < MAX_COINS; i++) coins[i].active = false;
    for (int b = 0; b < MAX_BALLOONS; b++) balloons[b].active = false;
    for (int p = 0; p < MAX_POWERUPS; p++) powerUps[p].active = false;

    spawnCoinsAhead(50.0f);
}

static void updateSimulation(int value) {
    if (gameState == STATE_PLAYING) {
        turbineSpin += 24.0f;
        strobePulse += 0.05f;

        // Active Speed calculation (Boost increases forward velocity!)
        float currentSpeed = baseSpeed;
        if (boostTimer > 0.0f) {
            currentSpeed *= 1.6f;
            boostTimer -= 0.016f;
        }
        if (magnetTimer > 0.0f) {
            magnetTimer -= 0.016f;
        }
        if (coinPopTimer > 0.0f) {
            coinPopTimer -= 0.02f;
        }

        planeX += currentSpeed;
        distanceTraveled += currentSpeed * 2.0f;
        score += 1;

        // Smooth Interpolation towards target altitude and target lane
        float diffZ = targetZ - planeZ;
        planeZ += diffZ * 0.14f;
        planeRoll = -diffZ * 1.8f; // Dynamic banking roll

        float diffY = targetY - planeY;
        planeY += diffY * 0.12f;
        planePitch = diffY * 1.2f; // Dynamic pitch

        // Spawn more coins ahead seamlessly
        spawnCoinsAhead(planeX + 250.0f);

        // 1. Coin Collection & Magnet Attraction Physics
        for (int i = 0; i < MAX_COINS; i++) {
            if (!coins[i].active) continue;

            coins[i].rotAngle += 4.0f;
            if (coins[i].rotAngle > 360.0f) coins[i].rotAngle -= 360.0f;

            float dx = planeX - coins[i].x;
            float dy = planeY - coins[i].y;
            float dz = planeZ - coins[i].z;
            float dist = (float)sqrt(dx*dx + dy*dy + dz*dz);

            // Magnet Effect: Pull coins toward plane from far away!
            if (magnetTimer > 0.0f && dist < 34.0f) {
                coins[i].x += dx * 0.16f;
                coins[i].y += dy * 0.16f;
                coins[i].z += dz * 0.16f;
            }

            // Coin Touch / Collection Detection
            if (dist < 4.4f) {
                coins[i].active = false;
                totalCoinsCollected++;
                score += 50;
                coinPopTimer = 1.0f;
                strcpy(popupText, "+50 COIN!");
            }
        }

        // 2. Power-Up Pickup Detection
        for (int p = 0; p < MAX_POWERUPS; p++) {
            if (!powerUps[p].active) continue;
            float dx = planeX - powerUps[p].x;
            float dy = planeY - powerUps[p].y;
            float dz = planeZ - powerUps[p].z;
            float dist = (float)sqrt(dx*dx + dy*dy + dz*dz);

            if (dist < 5.0f) {
                powerUps[p].active = false;
                if (powerUps[p].type == 0) {
                    magnetTimer = 8.0f; // 8s coin magnet
                    strcpy(popupText, "MAGNET ACTIVATED!");
                } else if (powerUps[p].type == 1) {
                    boostTimer = 4.0f;  // 4s speed boost
                    strcpy(popupText, "NITRO BOOST!");
                } else {
                    hasShield = true;
                    strcpy(popupText, "SHIELD CHARGED!");
                }
                coinPopTimer = 1.2f;
                score += 100;
            }
        }

        // 3. Obstacle Collision Detection (Hot Air Balloons)
        for (int b = 0; b < MAX_BALLOONS; b++) {
            if (!balloons[b].active) continue;
            float dx = planeX - balloons[b].x;
            float dy = planeY - balloons[b].y;
            float dz = planeZ - balloons[b].z;
            float dist = (float)sqrt(dx*dx + dy*dy + dz*dz);

            if (dist < 4.8f) {
                if (hasShield) {
                    // Shield absorbs collision
                    hasShield = false;
                    balloons[b].active = false;
                    strcpy(popupText, "SHIELD BROKEN!");
                    coinPopTimer = 1.2f;
                } else {
                    // Crash
                    gameState = STATE_GAME_OVER;
                }
            }
        }
    }

    // Day/Night transition
    if (dayValue < targetDayValue) {
        dayValue += 0.01f;
        if (dayValue > targetDayValue) dayValue = targetDayValue;
    } else if (dayValue > targetDayValue) {
        dayValue -= 0.01f;
        if (dayValue < targetDayValue) dayValue = targetDayValue;
    }

    // ---------------- DYNAMIC CAMERA PERSPECTIVES ----------------
    if (cameraMode == 0) {
        // Tower Overview
        camX = planeX - 52.0f;
        camY = 48.0f;
        camZ = 62.0f;
        camTargetX = planeX + 25.0f;
        camTargetY = planeY;
        camTargetZ = planeZ;
    } else if (cameraMode == 1) {
        // High-Action Chase Camera (Behind & Above)
        camX = planeX - 34.0f;
        camY = planeY + 7.8f;
        camZ = planeZ * 0.75f;
        camTargetX = planeX + 24.0f;
        camTargetY = planeY + 0.8f;
        camTargetZ = planeZ;
    } else if (cameraMode == 2) {
        // First Person Cockpit View
        camX = planeX + 7.6f;
        camY = planeY + 0.85f;
        camZ = planeZ;
        camTargetX = planeX + 85.0f;
        camTargetY = planeY;
        camTargetZ = planeZ;
    }

    glutPostRedisplay();
    glutTimerFunc(16, updateSimulation, 0); // Smooth 60 FPS
}

// ============================================================
// OPENGL DISPLAY PIPELINE
// ============================================================

static void display() {
    // Horizon Fog & Clear Color calculation
    float fogR = 0.78f * dayValue + 0.05f * (1.0f - dayValue);
    float fogG = 0.84f * dayValue + 0.06f * (1.0f - dayValue);
    float fogB = 0.94f * dayValue + 0.16f * (1.0f - dayValue);

    glClearColor(fogR, fogG, fogB, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(65.0, (double)windowWidth / (double)windowHeight, 1.0, 950.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camX, camY, camZ, camTargetX, camTargetY, camTargetZ, 0.0, 1.0, 0.0);

    // Atmospheric Fog (Smooth Depth Cueing)
    glEnable(GL_FOG);
    GLfloat fogColor[4] = { fogR, fogG, fogB, 1.0f };
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 120.0f);
    glFogf(GL_FOG_END, 520.0f);

    // Directional Sunlight (Key Light)
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat lightPos[]  = { planeX - 50.0f, 120.0f, -60.0f, 1.0f };
    GLfloat lightDiff[] = { 0.98f * dayValue + 0.15f, 0.95f * dayValue + 0.15f, 0.90f * dayValue + 0.20f, 1.0f };
    GLfloat lightAmb[]  = { 0.38f * dayValue + 0.14f, 0.40f * dayValue + 0.14f, 0.46f * dayValue + 0.18f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);

    // Soft Fill Sky Light (Light1)
    glEnable(GL_LIGHT1);
    GLfloat fillPos[]  = { planeX + 50.0f, 30.0f, 80.0f, 1.0f };
    GLfloat fillDiff[] = { 0.32f * dayValue + 0.08f, 0.38f * dayValue + 0.08f, 0.50f * dayValue + 0.12f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, fillPos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, fillDiff);

    // 1. Fine Atmospheric Sky & Celestial Bodies
    drawFineAtmosphere();

    // 2. Rolling Ground & Scenic Terrain
    drawScenicGround();

    // 3. Draw 3D Spinning Gold Coins
    for (int i = 0; i < MAX_COINS; i++) {
        if (!coins[i].active) continue;
        glPushMatrix();
        glTranslatef(coins[i].x, coins[i].y, coins[i].z);
        draw3DGoldCoin(coins[i].rotAngle);
        glPopMatrix();
    }

    // 4. Draw Power-Ups (Magnets, Boost Rings, Shields)
    for (int p = 0; p < MAX_POWERUPS; p++) {
        if (!powerUps[p].active) continue;
        glPushMatrix();
        glTranslatef(powerUps[p].x, powerUps[p].y, powerUps[p].z);
        drawPowerUp(powerUps[p].type);
        glPopMatrix();
    }

    // 5. Draw Sky Obstacles (Hot Air Balloons)
    for (int b = 0; b < MAX_BALLOONS; b++) {
        if (!balloons[b].active) continue;
        glPushMatrix();
        glTranslatef(balloons[b].x, balloons[b].y, balloons[b].z);
        drawHotAirBalloon(balloons[b].r, balloons[b].g, balloons[b].b);
        glPopMatrix();
    }

    // 6. Draw Player High-Detail Jetliner
    glPushMatrix();
    glTranslatef(planeX, planeY, planeZ);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f); // Orient plane forward along +X
    glRotatef(planePitch, 1.0f, 0.0f, 0.0f);
    glRotatef(planeRoll, 0.0f, 0.0f, 1.0f);
    drawAirplane();
    glPopMatrix();

    // 7. 2D Interface HUD
    drawHUD();

    glutSwapBuffers();
}

// ============================================================
// WINDOW RESHAPE & KEYBOARD CONTROLS
// ============================================================

static void reshape(int w, int h) {
    windowWidth  = w;
    windowHeight = (h == 0) ? 1 : h;
    glViewport(0, 0, windowWidth, windowHeight);
}

// Left/Right/Up/Down Arrow and WASD Key handling
static void handleSteering(int dir) {
    // 0 = Left, 1 = Right, 2 = Up, 3 = Down
    if (gameState != STATE_PLAYING) return;

    if (dir == 0) {
        // GO LEFT
        targetZ -= 4.4f;
        targetZ = clampVal(targetZ, MIN_Z, MAX_Z);
    } else if (dir == 1) {
        // GO RIGHT
        targetZ += 4.4f;
        targetZ = clampVal(targetZ, MIN_Z, MAX_Z);
    } else if (dir == 2) {
        // GO UP
        targetY += 4.0f;
        targetY = clampVal(targetY, MIN_ALT, MAX_ALT);
    } else if (dir == 3) {
        // GO DOWN
        targetY -= 4.0f;
        targetY = clampVal(targetY, MIN_ALT, MAX_ALT);
    }
}

static void keyboard(unsigned char key, int x, int y) {
    if (key == 27) {
        exit(0);
    }

    switch (key) {
        case 'a':
        case 'A':
            handleSteering(0); // Left
            break;
        case 'd':
        case 'D':
            handleSteering(1); // Right
            break;
        case 'w':
        case 'W':
            handleSteering(2); // Up
            break;
        case 's':
        case 'S':
            handleSteering(3); // Down
            break;
        case '1':
            cameraMode = 0;
            break;
        case '2':
            cameraMode = 1;
            break;
        case '3':
            cameraMode = 2;
            break;
        case 'l':
        case 'L':
            targetDayValue = (targetDayValue > 0.5f) ? 0.0f : 1.0f;
            break;
        case 'h':
        case 'H':
            showTeamInfo = !showTeamInfo;
            break;
        case 'x':
        case 'X':
            resetGame();
            break;
    }
}

static void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            handleSteering(0); // Left arrow click -> GO LEFT
            break;
        case GLUT_KEY_RIGHT:
            handleSteering(1); // Right arrow click -> GO RIGHT
            break;
        case GLUT_KEY_UP:
            handleSteering(2); // Up arrow click -> GO UP
            break;
        case GLUT_KEY_DOWN:
            handleSteering(3); // Down arrow click -> GO DOWN
            break;
    }
}

// ============================================================
// INITIALIZATION
// ============================================================

static void initGraphics() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    srand((unsigned int)time(NULL));
    resetGame();
}

// ============================================================
// MAIN ENTRY POINT
// ============================================================

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(80, 45);
    glutCreateWindow("AIUB 3D Sky Coin Collector Adventure (Deluxe Edition)");

    initGraphics();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(16, updateSimulation, 0);

    glutMainLoop();
    return 0;
}
