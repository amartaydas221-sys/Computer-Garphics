#define _USE_MATH_DEFINES
#include <windows.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Fast math helpers
static inline float minF(float a, float b) { return (a < b) ? a : b; }
static inline float maxF(float a, float b) { return (a > b) ? a : b; }
static inline float clampF(float v, float lo, float hi) { return (v < lo) ? lo : ((v > hi) ? hi : v); }

// ==========================================
// GAME STATE & CONSTANTS
// ==========================================
#define STATE_START    0
#define STATE_PLAYING  1
#define STATE_CRASHED  2
#define STATE_FINISHED 3

int gameState = STATE_START;
int windowWidth = 1280;
int windowHeight = 720;

// Camera Modes:
// 0 = Authentic European Highway Chase Perspective (Looking down Highway to City Skyline)
// 1 = 3D Side Action Profile View
// 2 = Driver Cockpit / Hood Perspective (Looking over BMW Power-Dome)
int cameraMode = 0;
float camSmoothX = 0.0f;
float camSmoothY = 1.95f;
float camSmoothZ = 0.0f;

// Course Metrics
const float HIGHWAY_LENGTH = 1600.0f;
float currentDistance = 0.0f;
float bestDistance = 0.0f;
int coinsCount = 0;
float nitro = 100.0f;
const float MAX_NITRO = 100.0f;
const float CAR_SCALE = 0.65f;     // overall car size (1.0 = original, smaller = tinier)
const float MAX_SPEED = 26.0f;    // m/s (about 94 km/h)
float gameTime = 0.0f;

// Controls
int keyAccelerate = 0;
int keyBrake = 0;
int keySteerLeft = 0;
int keySteerRight = 0;
int keyJump = 0;

// ==========================================
// BACKEND ENVIRONMENT & ATMOSPHERE STATE
// (DAY/NIGHT, RAIN, WIND, CLOUDS, PLANE, BIRDS)
// ==========================================
float cloudMove = 0.0f;
float dayValue = 1.0f;
float targetDayValue = 1.0f;

int rainOn = 0;
float rainIntensity = 0.0f;

// Wind State: 1 = Left to Right, -1 = Right to Left, 0 = Calm
int windDirection = 0;
float windWave = 0.0f;

#define RAIN_COUNT 280
float rainX[RAIN_COUNT];
float rainY[RAIN_COUNT];
float rainSpeed[RAIN_COUNT];

// Airplane in Sky
float planeX = 1.25f;
float planeY = 0.62f;
float planeSpeed = 0.0015f;

// Cartoon Birds in Sky
float birdX = -1.20f;
float birdY = 0.72f;
float birdSpeed = 0.0030f;
float birdWingPhase = 0.0f;
float birdWingAngle = 0.0f;

// ==========================================
// 3D BMW M-SPORT COUPE DYNAMICS
// ==========================================
float carX = 14.0f;             // Forward position along highway
float carY = 0.34f;             // Elevation
float carZ = 1.25f;             // Lateral lane position

float carSpeed = 0.0f;          // Speed in m/s (up to 62 m/s ~ 223 km/h)
float carVy = 0.0f;             // Vertical velocity (jump)
float carYaw = 0.0f;            // Heading direction (smoothly self-centers)
float carPitch = 0.0f;          // Pitch angle
float carRoll = 0.0f;           // Body roll in cornering

float steerAngle = 0.0f;        // Front wheels steering angle
float wheelSpinAngle = 0.0f;    // Wheel revolution
float suspensionDive = 0.0f;    // Nose dive during hard braking
int isGrounded = 1;

// BMW Sports Coupe Proportions
const float CAR_LENGTH = 3.90f;
const float CAR_WIDTH  = 1.78f;
const float CAR_HEIGHT = 0.95f;
const float CAR_WHEEL_BASE = 2.30f;
const float CAR_WHEEL_TRACK = 1.48f;
const float CAR_WHEEL_RADIUS = 0.34f;
const float CAR_WHEEL_WIDTH  = 0.22f;

GLUquadric* quadric = NULL;

// ==========================================
// 4 HIGHWAY LANES (MATCHING PICTURE)
// ==========================================
const float LANE_Z[4] = { -3.35f, -1.25f, 1.25f, 3.35f };

// ==========================================
// GORGEOUS 3D GOLD COINS & SPARKLES
// ==========================================
#define COIN_COUNT 115
typedef struct {
    float x, z;
    int collected;
    float bobPhase;
    float spinSpeed;
} GorgeousCoin;
GorgeousCoin coins[COIN_COUNT];

#define MAX_SPARKLES 180
typedef struct {
    float x, y, z;
    float vx, vy, vz;
    float life;
    float maxLife;
    float size;
    float r, g, b;
} SparkleFX;
SparkleFX sparkles[MAX_SPARKLES];

// Exhaust Smoke / Dust Particle FX
#define MAX_EXHAUST 80
typedef struct {
    float x, y, z;
    float vx, vy, vz;
    float life, maxLife;
    float size;
} ExhaustFX;
ExhaustFX exhausts[MAX_EXHAUST];

// ==========================================
// FUNCTION PROTOTYPES
// ==========================================
void initGame();
void initRain();
void resetCar();
void updatePhysics(float dt);
void updateParticles(float dt);
void spawnCoinBurst(float x, float y, float z);
void spawnExhaust(float x, float y, float z);

// 2D Primitive Helpers for Background
void drawCircle(float cx, float cy, float r);
void drawText(float x, float y, const char *text);

// Backend Atmosphere & Cityscape Drawing
void drawSky();
void drawSunMoon();
void drawStars();
void drawClouds();
void drawRainClouds();
void drawRain();
void drawPlane();
void drawCartoonBird(float x, float y, float scale, bool faceRight, float wingAngle, bool isFlying);
void drawBuildings();
void drawGround();
void drawWindEffect();
float getWindBend();
void drawFlags();
void drawSingleFlag(float poleX, float baseY, float poleHeight, float flagW, float flagH);
void drawLampPosts();
void drawSingleLampPost(float postX, float baseY, float armDir);

// 3D Primitives & Objects
void drawCylinder3D(float rBase, float rTop, float height, int slices);
void drawDisk3D(float innerRadius, float outerRadius, int slices);
void drawBMWWheel3D(float radius, float width, float spinAngle);
void drawBMWEmblem(float size);
void drawKidneyGrille(float width, float height);
void drawBMWSportsCar();
void drawPitchBlackEuropeanRoad();
void drawOrganicRoadsideTreesAndHorizon();
void drawGorgeousCoins();
void drawParticles();
void drawDashboardHUD();
void drawStartScreen();

void display();
void reshape(int w, int h);
void update(int value);
void handleKeyDown(unsigned char key, int x, int y);
void handleKeyUp(unsigned char key, int x, int y);
void handleSpecialDown(int key, int x, int y);
void handleSpecialUp(int key, int x, int y);

// ==========================================
// GAME STATE INITIALIZATION
// ==========================================
void initRain() {
    for(int i = 0; i < RAIN_COUNT; i++) {
        rainX[i] = -1.0f + (rand() % 200) / 100.0f;
        rainY[i] = -1.0f + (rand() % 200) / 100.0f;
        rainSpeed[i] = 0.025f + (rand() % 100) / 5000.0f;
    }
}

void resetCar() {
    carX = 14.0f;
    carZ = LANE_Z[2]; // Start in Right Inner Lane
    carY = CAR_WHEEL_RADIUS;
    carSpeed = 0.0f;
    carVy = 0.0f;
    carYaw = 0.0f;
    carPitch = 0.0f;
    carRoll = 0.0f;
    steerAngle = 0.0f;
    wheelSpinAngle = 0.0f;
    suspensionDive = 0.0f;
    isGrounded = 1;
    nitro = MAX_NITRO;

    camSmoothX = carX - 5.4f;
    camSmoothY = 1.95f;
    camSmoothZ = carZ * 0.28f;
}

void initGame() {
    resetCar();
    initRain();
    currentDistance = 0.0f;
    coinsCount = 0;
    gameTime = 0.0f;

    // Distribute gorgeous coins across the 4 lanes
    for (int i = 0; i < COIN_COUNT; i++) {
        coins[i].x = 24.0f + (float)i * 13.2f;
        int lane = (i * 2 + (i / 3)) % 4;
        coins[i].z = LANE_Z[lane];
        coins[i].collected = 0;
        coins[i].bobPhase = ((float)rand() / (float)RAND_MAX) * 6.28f;
        coins[i].spinSpeed = 175.0f + ((rand() % 30) - 15.0f);
    }

    for (int i = 0; i < MAX_SPARKLES; i++) sparkles[i].life = 0.0f;
    for (int i = 0; i < MAX_EXHAUST; i++) exhausts[i].life = 0.0f;
}

// ==========================================
// 2D PRIMITIVE HELPERS
// ==========================================
void drawCircle(float cx, float cy, float r) {
    glBegin(GL_POLYGON);
    for(int i = 0; i < 64; i++) {
        float theta = (i * 2.0f * (float)M_PI) / 64.0f;
        glVertex2f(cx + r * cosf(theta), cy + r * sinf(theta));
    }
    glEnd();
}

void drawText(float x, float y, const char *text) {
    glRasterPos2f(x, y);
    for(int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

// ==========================================
// BACKEND: ATMOSPHERE, SKY, SUN, MOON, STARS
// ==========================================
void drawSky() {
    float r = 0.04f + dayValue * 0.32f;
    float g = 0.07f + dayValue * 0.65f;
    float b = 0.18f + dayValue * 0.78f;

    if(rainIntensity > 0.0f) {
        r = r * (1.0f - rainIntensity) + 0.28f * rainIntensity;
        g = g * (1.0f - rainIntensity) + 0.32f * rainIntensity;
        b = b * (1.0f - rainIntensity) + 0.40f * rainIntensity;
    }

    glClearColor(r, g, b, 1.0f);

    glBegin(GL_QUADS);
    glColor3f(r * 0.75f, g * 0.85f, b * 1.05f > 1.0f ? 1.0f : b * 1.05f);
    glVertex2f(-1.0f, -0.15f);
    glVertex2f( 1.0f, -0.15f);
    glColor3f(r, g, b);
    glVertex2f( 1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();
}

void drawSunMoon() {
    float sunY = 0.80f - (1.0f - dayValue) * 0.55f;

    if(dayValue > 0.05f && rainIntensity < 0.55f) {
        glColor3f(1.0f, 0.75f, 0.10f);
        drawCircle(0.75f, sunY, 0.085f);

        glColor3f(1.0f, 0.88f, 0.25f);
        drawCircle(0.75f, sunY, 0.060f);

        glColor3f(1.0f, 0.98f, 0.55f);
        drawCircle(0.75f, sunY, 0.035f);

        glColor3f(1.0f, 0.85f, 0.20f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(0.75f, sunY + 0.10f); glVertex2f(0.75f, sunY + 0.17f);
        glVertex2f(0.75f, sunY - 0.10f); glVertex2f(0.75f, sunY - 0.17f);
        glVertex2f(0.65f, sunY);         glVertex2f(0.58f, sunY);
        glVertex2f(0.85f, sunY);         glVertex2f(0.92f, sunY);
        glVertex2f(0.68f, sunY + 0.07f); glVertex2f(0.62f, sunY + 0.13f);
        glVertex2f(0.82f, sunY + 0.07f); glVertex2f(0.88f, sunY + 0.13f);
        glVertex2f(0.68f, sunY - 0.07f); glVertex2f(0.62f, sunY - 0.13f);
        glVertex2f(0.82f, sunY - 0.07f); glVertex2f(0.88f, sunY - 0.13f);
        glEnd();
        glLineWidth(1.0f);
    }

    if(dayValue < 0.30f && rainIntensity < 0.85f) {
        glColor3f(0.98f, 0.98f, 0.85f);
        drawCircle(-0.75f, 0.78f, 0.065f);

        float bgR = 0.04f + dayValue * 0.32f;
        float bgG = 0.07f + dayValue * 0.65f;
        float bgB = 0.18f + dayValue * 0.78f;
        glColor3f(bgR, bgG, bgB);
        drawCircle(-0.72f, 0.80f, 0.055f);
    }
}

void drawStars() {
    if(dayValue > 0.20f || rainIntensity > 0.5f) return;

    glColor3f(1.0f, 1.0f, 0.90f);
    glPointSize(2.2f);
    glBegin(GL_POINTS);
    glVertex2f(-0.90f, 0.90f);
    glVertex2f(-0.70f, 0.85f);
    glVertex2f(-0.45f, 0.92f);
    glVertex2f(-0.20f, 0.82f);
    glVertex2f( 0.05f, 0.88f);
    glVertex2f( 0.30f, 0.82f);
    glVertex2f( 0.55f, 0.90f);
    glVertex2f( 0.80f, 0.86f);
    glVertex2f(-0.85f, 0.72f);
    glVertex2f(-0.35f, 0.75f);
    glVertex2f( 0.15f, 0.76f);
    glVertex2f( 0.65f, 0.75f);
    glEnd();
    glPointSize(1.0f);
}

void drawClouds() {
    glPushMatrix();
    glTranslatef(cloudMove, 0.0f, 0.0f);

    float c = 0.85f + dayValue * 0.15f;
    c = c * (1.0f - rainIntensity) + 0.30f * rainIntensity;
    glColor3f(c, c, c + 0.04f);

    drawCircle(-0.75f, 0.78f, 0.055f + rainIntensity * 0.015f);
    drawCircle(-0.70f, 0.80f, 0.065f + rainIntensity * 0.015f);
    drawCircle(-0.64f, 0.78f, 0.055f + rainIntensity * 0.015f);

    drawCircle(-0.15f, 0.70f, 0.055f + rainIntensity * 0.015f);
    drawCircle(-0.10f, 0.72f, 0.068f + rainIntensity * 0.015f);
    drawCircle(-0.04f, 0.70f, 0.055f + rainIntensity * 0.015f);

    drawCircle(0.35f, 0.76f, 0.055f + rainIntensity * 0.015f);
    drawCircle(0.40f, 0.78f, 0.068f + rainIntensity * 0.015f);
    drawCircle(0.46f, 0.76f, 0.055f + rainIntensity * 0.015f);

    glPopMatrix();
}

void drawRainClouds() {
    if(rainIntensity <= 0.0f) return;

    glPushMatrix();
    glTranslatef(cloudMove, 0.0f, 0.0f);

    float c = 0.24f - rainIntensity * 0.05f;
    glColor3f(c, c + 0.02f, c + 0.05f);

    drawCircle(-0.95f, 0.92f, 0.12f);
    drawCircle(-0.75f, 0.94f, 0.14f);
    drawCircle(-0.55f, 0.92f, 0.13f);
    drawCircle(-0.32f, 0.94f, 0.15f);
    drawCircle(-0.08f, 0.91f, 0.13f);
    drawCircle( 0.18f, 0.94f, 0.15f);
    drawCircle( 0.43f, 0.91f, 0.13f);
    drawCircle( 0.68f, 0.94f, 0.15f);
    drawCircle( 0.92f, 0.91f, 0.13f);

    glColor3f(c + 0.05f, c + 0.07f, c + 0.10f);
    drawCircle(-0.80f, 0.78f, 0.09f);
    drawCircle(-0.55f, 0.76f, 0.10f);
    drawCircle(-0.25f, 0.78f, 0.11f);
    drawCircle( 0.08f, 0.76f, 0.10f);
    drawCircle( 0.38f, 0.78f, 0.11f);
    drawCircle( 0.68f, 0.76f, 0.10f);

    glPopMatrix();
}

void drawRain() {
    if(rainIntensity <= 0.0f) return;

    int dropsToDraw = (int)(RAIN_COUNT * rainIntensity);
    float windSlant = (windDirection == 1) ? 0.035f : ((windDirection == -1) ? -0.035f : 0.005f);

    glLineWidth(1.6f);
    glBegin(GL_LINES);

    for(int i = 0; i < dropsToDraw; i++) {
        float length = 0.08f + (i % 5) * 0.01f;
        float shade = 0.60f + (i % 4) * 0.10f;

        glColor3f(0.35f * shade, 0.65f * shade, 0.95f * shade);
        glVertex2f(rainX[i], rainY[i]);
        glVertex2f(rainX[i] + windSlant, rainY[i] - length);
    }

    glEnd();
    glLineWidth(1.0f);
}

// ==========================================
// BACKEND: DYNAMIC WIND & FLAGS
// ==========================================
float getWindBend() {
    if(windDirection == 1) {
        return 0.022f + 0.008f * sinf(windWave);
    }
    if(windDirection == -1) {
        return -0.022f - 0.008f * sinf(windWave);
    }
    return 0.0f;
}

void drawWindEffect() {
    if(windDirection == 0) return;

    glColor3f(0.90f, 0.95f, 1.0f);
    glLineWidth(1.2f);
    glBegin(GL_LINES);

    if(windDirection == 1) {
        glVertex2f(-0.90f, 0.65f); glVertex2f(-0.65f, 0.65f);
        glVertex2f(-0.70f, 0.55f); glVertex2f(-0.45f, 0.55f);
        glVertex2f(-0.20f, 0.68f); glVertex2f( 0.05f, 0.68f);
        glVertex2f( 0.25f, 0.58f); glVertex2f( 0.50f, 0.58f);
        glVertex2f( 0.60f, 0.70f); glVertex2f( 0.85f, 0.70f);
    } else {
        glVertex2f(-0.65f, 0.65f); glVertex2f(-0.90f, 0.65f);
        glVertex2f(-0.45f, 0.55f); glVertex2f(-0.70f, 0.55f);
        glVertex2f( 0.05f, 0.68f); glVertex2f(-0.20f, 0.68f);
        glVertex2f( 0.50f, 0.58f); glVertex2f( 0.25f, 0.58f);
        glVertex2f( 0.85f, 0.70f); glVertex2f( 0.60f, 0.70f);
    }

    glEnd();
    glLineWidth(1.0f);
}

void drawSingleFlag(float poleX, float baseY, float poleHeight, float flagW, float flagH) {
    float poleTopY = baseY + poleHeight;
    float poleWidth = 0.008f;

    glColor3f(0.70f, 0.75f, 0.80f);
    glBegin(GL_QUADS);
    glVertex2f(poleX - poleWidth * 0.5f, baseY);
    glVertex2f(poleX + poleWidth * 0.5f, baseY);
    glVertex2f(poleX + poleWidth * 0.5f, poleTopY);
    glVertex2f(poleX - poleWidth * 0.5f, poleTopY);
    glEnd();

    glColor3f(1.0f, 0.82f, 0.10f);
    drawCircle(poleX, poleTopY + 0.005f, 0.008f);

    float dir = (windDirection == -1) ? -1.0f : 1.0f;
    float windActive = (windDirection != 0) ? 1.0f : 0.0f;

    const int SEGMENTS = 14;
    float attachX = (dir > 0) ? (poleX + poleWidth * 0.5f) : (poleX - poleWidth * 0.5f);
    float flagTopY = poleTopY - 0.005f;
    float flagBottomY = flagTopY - flagH;

    glColor3f(0.00f, 0.48f, 0.22f);
    glBegin(GL_QUAD_STRIP);
    for(int i = 0; i <= SEGMENTS; i++) {
        float u = (float)i / SEGMENTS;
        float x = attachX + dir * (u * flagW);

        float wave = 0.0f;
        if(windActive > 0.0f) {
            wave = sinf(windWave * 2.5f - u * 4.2f) * (0.012f * u);
        } else {
            wave = -0.015f * u * u;
        }

        glVertex2f(x, flagTopY + wave);
        glVertex2f(x, flagBottomY + wave);
    }
    glEnd();

    float uCircle = 0.42f;
    float circleCenterX = attachX + dir * (uCircle * flagW);
    float circleWave = (windActive > 0.0f) ? sinf(windWave * 2.5f - uCircle * 4.2f) * (0.012f * uCircle) : (-0.015f * uCircle * uCircle);
    float circleCenterY = (flagTopY + flagBottomY) * 0.5f + circleWave;
    float circleRadius = flagH * 0.28f;

    glColor3f(0.95f, 0.12f, 0.12f);
    drawCircle(circleCenterX, circleCenterY, circleRadius);
}

void drawFlags() {
    drawSingleFlag(-0.095f, -0.18f, 0.36f, 0.11f, 0.08f);
    drawSingleFlag( 0.185f, -0.18f, 0.36f, 0.11f, 0.08f);
}

// ==========================================
// BACKEND: LAMP POSTS WITH LIGHT CONES
// ==========================================
void drawSingleLampPost(float postX, float baseY, float armDir) {
    float postW = 0.02f;
    float postH = 0.30f;
    float topY = baseY + postH;

    glColor3f(0.20f, 0.22f, 0.25f);
    glBegin(GL_QUADS);
    glVertex2f(postX, baseY);
    glVertex2f(postX + postW, baseY);
    glVertex2f(postX + postW, topY);
    glVertex2f(postX, topY);
    glEnd();

    float armEnd = postX + (armDir > 0 ? (postW + 0.05f) : -0.05f);
    glBegin(GL_QUADS);
    glVertex2f(postX, topY);
    glVertex2f(armEnd, topY);
    glVertex2f(armEnd, topY - 0.02f);
    glVertex2f(postX, topY - 0.02f);
    glEnd();

    float lampCenterX = armEnd;
    float lampCenterY = topY - 0.025f;

    if(dayValue <= 0.4f || rainOn == 1) {
        // FIXED: smaller bulb and a much subtler, narrower glow cone (no blur)
        glColor3f(1.0f, 0.90f, 0.40f);
        drawCircle(lampCenterX, lampCenterY, 0.022f);   // slightly smaller

        glColor3f(1.0f, 1.0f, 0.85f);
        drawCircle(lampCenterX, lampCenterY, 0.013f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.90f, 0.30f, 0.07f);           // was 0.25
        glVertex2f(lampCenterX, lampCenterY);
        glColor4f(1.0f, 0.85f, 0.20f, 0.0f);            // was 0.02
        glVertex2f(lampCenterX - 0.05f, baseY);         // narrower cone
        glVertex2f(lampCenterX + 0.05f, baseY);
        glEnd();
        glDisable(GL_BLEND);
    } else {
        glColor3f(0.85f, 0.85f, 0.70f);
        drawCircle(lampCenterX, lampCenterY, 0.015f);
    }
}

void drawLampPosts() {
    drawSingleLampPost(-0.82f, -0.18f,  1.0f);
    drawSingleLampPost(-0.55f, -0.18f,  1.0f);
    drawSingleLampPost( 0.52f, -0.18f,  1.0f);
    drawSingleLampPost( 0.80f, -0.18f,  1.0f);
}

// ==========================================
// BACKEND: AIRPLANE
// ==========================================
void drawPlane() {
    glPushMatrix();
    glTranslatef(planeX, planeY, 0.0f);
    glScalef(0.55f, 0.55f, 1.0f);

    glColor3f(0.96f, 0.96f, 0.98f);
    glBegin(GL_POLYGON);
    glVertex2f(-0.16f, 0.015f);
    glVertex2f(-0.12f, 0.035f);
    glVertex2f(0.09f, 0.035f);
    glVertex2f(0.15f, 0.025f);
    glVertex2f(0.20f, 0.015f);
    glVertex2f(0.15f, 0.005f);
    glVertex2f(0.09f, -0.005f);
    glVertex2f(-0.12f, -0.005f);
    glEnd();

    glColor3f(0.85f, 0.88f, 0.92f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.14f, 0.025f);
    glVertex2f(0.20f, 0.015f);
    glVertex2f(0.14f, 0.005f);
    glEnd();

    glColor3f(0.15f, 0.45f, 0.85f);
    glBegin(GL_POLYGON);
    glVertex2f(-0.025f, 0.025f);
    glVertex2f(0.045f, 0.025f);
    glVertex2f(0.090f, 0.125f);
    glVertex2f(0.045f, 0.115f);
    glVertex2f(-0.055f, 0.035f);
    glEnd();

    glColor3f(0.12f, 0.38f, 0.75f);
    glBegin(GL_POLYGON);
    glVertex2f(-0.025f, -0.005f);
    glVertex2f(0.045f, -0.005f);
    glVertex2f(0.090f, -0.090f);
    glVertex2f(0.045f, -0.080f);
    glVertex2f(-0.055f, -0.015f);
    glEnd();

    glColor3f(0.90f, 0.15f, 0.20f);
    glBegin(GL_POLYGON);
    glVertex2f(-0.13f, 0.025f);
    glVertex2f(-0.075f, 0.095f);
    glVertex2f(-0.045f, 0.035f);
    glEnd();

    glColor3f(0.82f, 0.85f, 0.90f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.12f, 0.015f);
    glVertex2f(-0.04f, 0.055f);
    glVertex2f(-0.045f, 0.010f);
    glEnd();

    glColor3f(0.15f, 0.75f, 0.95f);
    glBegin(GL_POLYGON);
    glVertex2f(-0.025f, 0.037f);
    glVertex2f(0.025f, 0.037f);
    glVertex2f(0.045f, 0.052f);
    glVertex2f(0.005f, 0.062f);
    glVertex2f(-0.025f, 0.052f);
    glEnd();

    glColor3f(0.80f, 0.95f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(-0.015f, 0.043f);
    glVertex2f(0.005f, 0.043f);
    glVertex2f(0.015f, 0.052f);
    glVertex2f(-0.005f, 0.052f);
    glEnd();

    glColor3f(0.10f, 0.25f, 0.50f);
    glBegin(GL_QUADS);
    glVertex2f(-0.075f, 0.015f);
    glVertex2f(-0.050f, 0.015f);
    glVertex2f(-0.050f, 0.027f);
    glVertex2f(-0.075f, 0.027f);
    glVertex2f(-0.042f, 0.015f);
    glVertex2f(-0.017f, 0.015f);
    glVertex2f(-0.017f, 0.027f);
    glVertex2f(-0.042f, 0.027f);
    glEnd();

    glColor3f(0.35f, 0.38f, 0.42f);
    glBegin(GL_POLYGON);
    glVertex2f(0.015f, -0.005f);
    glVertex2f(0.050f, -0.005f);
    glVertex2f(0.055f, -0.025f);
    glVertex2f(0.020f, -0.025f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(0.055f, -0.005f);
    glVertex2f(0.085f, -0.005f);
    glVertex2f(0.090f, -0.022f);
    glVertex2f(0.060f, -0.022f);
    glEnd();

    glPopMatrix();
}

// ==========================================
// BACKEND: CARTOON BIRDS
// ==========================================
void drawCartoonBird(float x, float y, float scale, bool faceRight, float wingAngle, bool isFlying) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    if (!faceRight) glScalef(-scale, scale, 1.0f);
    else            glScalef(scale, scale, 1.0f);

    // Legs & Feet
    glColor3f(0.85f, 0.55f, 0.15f);
    glLineWidth(2.2f);
    glBegin(GL_LINES);
    if (!isFlying) {
        glVertex2f(-0.005f, 0.000f); glVertex2f(-0.008f, 0.035f);
        glVertex2f(0.012f, 0.000f);  glVertex2f(0.008f, 0.035f);
        glVertex2f(-0.018f, 0.000f); glVertex2f(0.002f, 0.000f);
        glVertex2f(0.000f, 0.000f);  glVertex2f(0.020f, 0.000f);
    } else {
        glVertex2f(-0.010f, 0.025f); glVertex2f(-0.025f, 0.035f);
        glVertex2f(0.000f, 0.025f);  glVertex2f(-0.015f, 0.035f);
    }
    glEnd();

    // Tail
    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_POLYGON);
    glVertex2f(-0.015f, 0.035f); glVertex2f(-0.090f, 0.010f);
    glVertex2f(-0.080f, 0.025f); glVertex2f(-0.065f, 0.038f);
    glVertex2f(-0.095f, 0.030f); glVertex2f(-0.010f, 0.065f);
    glEnd();

    // Body
    glColor3f(0.12f, 0.12f, 0.14f);
    glBegin(GL_POLYGON);
    glVertex2f(-0.035f, 0.035f); glVertex2f(-0.030f, 0.075f);
    glVertex2f(0.000f, 0.095f);  glVertex2f(0.035f, 0.085f);
    glVertex2f(0.045f, 0.045f);  glVertex2f(0.020f, 0.025f);
    glVertex2f(-0.015f, 0.025f);
    glEnd();

    // Chest Patch
    glColor3f(0.20f, 0.20f, 0.24f);
    glBegin(GL_POLYGON);
    glVertex2f(0.010f, 0.030f); glVertex2f(0.042f, 0.050f);
    glVertex2f(0.032f, 0.080f); glVertex2f(0.010f, 0.075f);
    glVertex2f(-0.005f, 0.045f);
    glEnd();

    // Flapping Wing
    glPushMatrix();
    glTranslatef(-0.020f, 0.045f, 0.0f);
    glRotatef(wingAngle, 0.0f, 0.0f, 1.0f);
    glTranslatef(0.020f, -0.045f, 0.0f);
    glColor3f(0.16f, 0.16f, 0.18f);
    glBegin(GL_POLYGON);
    glVertex2f(-0.020f, 0.045f);
    glVertex2f(-0.060f, 0.040f);
    glVertex2f(-0.025f, 0.078f + (wingAngle * 0.001f));
    glVertex2f(0.015f, 0.082f);
    glVertex2f(0.005f, 0.060f);
    glEnd();
    glPopMatrix();

    // Head
    glColor3f(0.12f, 0.12f, 0.15f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 24; i++) {
        float angle = (i * 2.0f * (float)M_PI) / 24.0f;
        glVertex2f(0.028f + 0.026f * cosf(angle), 0.095f + 0.026f * sinf(angle));
    }
    glEnd();

    // Crest Feathers
    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.018f, 0.102f); glVertex2f(-0.025f, 0.128f); glVertex2f(0.012f, 0.118f);
    glVertex2f(0.012f, 0.115f); glVertex2f(-0.028f, 0.110f); glVertex2f(0.018f, 0.095f);
    glEnd();

    // Eye
    glColor3f(0.95f, 0.95f, 0.90f);
    drawCircle(0.036f, 0.100f, 0.013f);
    glColor3f(0.05f, 0.05f, 0.05f);
    drawCircle(0.037f, 0.100f, 0.007f);

    // Beak
    glColor3f(0.95f, 0.60f, 0.10f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.045f, 0.110f);
    glVertex2f(0.082f, 0.092f);
    glVertex2f(0.045f, 0.082f);
    glEnd();

    glPopMatrix();
}

// ==========================================
// BACKEND: CITY SKYLINE BUILDINGS
// ==========================================
void drawBuildings() {
    float winR, winG, winB;
    if (rainIntensity > 0.4f || dayValue < 0.35f) {
        winR = 1.0f; winG = 0.85f; winB = 0.25f; // Warm glowing windows at night/rain
    } else {
        winR = 0.70f; winG = 0.90f; winB = 1.0f; // Sky reflections
    }

    // ================= Background Buildings =================
    // Building 1 (Pitch Onyx Black Skyscraper with Crimson Peak)
    glColor3f(0.10f, 0.12f, 0.15f);
    glBegin(GL_QUADS);
    glVertex2f(-1.00f, -0.15f); glVertex2f(-0.70f, -0.15f);
    glVertex2f(-0.70f,  0.45f); glVertex2f(-1.00f,  0.45f);
    glEnd();

    glColor3f(0.65f, 0.10f, 0.12f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-1.00f, 0.45f); glVertex2f(-0.70f, 0.45f); glVertex2f(-0.85f, 0.54f);
    glEnd();

    glColor3f(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(-0.93f, 0.32f); glVertex2f(-0.89f, 0.32f); glVertex2f(-0.89f, 0.37f); glVertex2f(-0.93f, 0.37f);
    glVertex2f(-0.85f, 0.32f); glVertex2f(-0.81f, 0.32f); glVertex2f(-0.81f, 0.37f); glVertex2f(-0.85f, 0.37f);
    glVertex2f(-0.77f, 0.32f); glVertex2f(-0.73f, 0.32f); glVertex2f(-0.73f, 0.37f); glVertex2f(-0.77f, 0.37f);
    glVertex2f(-0.93f, 0.20f); glVertex2f(-0.89f, 0.20f); glVertex2f(-0.89f, 0.25f); glVertex2f(-0.93f, 0.25f);
    glVertex2f(-0.85f, 0.20f); glVertex2f(-0.81f, 0.20f); glVertex2f(-0.81f, 0.25f); glVertex2f(-0.85f, 0.25f);
    glVertex2f(-0.77f, 0.20f); glVertex2f(-0.73f, 0.20f); glVertex2f(-0.73f, 0.25f); glVertex2f(-0.77f, 0.25f);
    glVertex2f(-0.93f, 0.08f); glVertex2f(-0.89f, 0.08f); glVertex2f(-0.89f, 0.13f); glVertex2f(-0.93f, 0.13f);
    glVertex2f(-0.85f, 0.08f); glVertex2f(-0.81f, 0.08f); glVertex2f(-0.81f, 0.13f); glVertex2f(-0.85f, 0.13f);
    glVertex2f(-0.77f, 0.08f); glVertex2f(-0.73f, 0.08f); glVertex2f(-0.73f, 0.13f); glVertex2f(-0.77f, 0.13f);
    glEnd();

    // Building 2 (Vivid Crimson Red with Dark Black Trim)
    glColor3f(0.78f, 0.15f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(-0.70f, -0.15f); glVertex2f(-0.40f, -0.15f);
    glVertex2f(-0.40f,  0.42f); glVertex2f(-0.70f,  0.42f);
    glEnd();

    glColor3f(0.12f, 0.14f, 0.16f);
    glBegin(GL_QUADS);
    glVertex2f(-0.71f, 0.42f); glVertex2f(-0.39f, 0.42f); glVertex2f(-0.39f, 0.45f); glVertex2f(-0.71f, 0.45f);
    glEnd();

    glColor3f(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(-0.66f, 0.30f); glVertex2f(-0.62f, 0.30f); glVertex2f(-0.62f, 0.35f); glVertex2f(-0.66f, 0.35f);
    glVertex2f(-0.58f, 0.30f); glVertex2f(-0.54f, 0.30f); glVertex2f(-0.54f, 0.35f); glVertex2f(-0.58f, 0.35f);
    glVertex2f(-0.50f, 0.30f); glVertex2f(-0.46f, 0.30f); glVertex2f(-0.46f, 0.35f); glVertex2f(-0.50f, 0.35f);
    glVertex2f(-0.66f, 0.18f); glVertex2f(-0.62f, 0.18f); glVertex2f(-0.62f, 0.23f); glVertex2f(-0.66f, 0.23f);
    glVertex2f(-0.58f, 0.18f); glVertex2f(-0.54f, 0.18f); glVertex2f(-0.54f, 0.23f); glVertex2f(-0.58f, 0.23f);
    glVertex2f(-0.50f, 0.18f); glVertex2f(-0.46f, 0.18f); glVertex2f(-0.46f, 0.23f); glVertex2f(-0.50f, 0.23f);
    glVertex2f(-0.66f, 0.06f); glVertex2f(-0.62f, 0.06f); glVertex2f(-0.62f, 0.11f); glVertex2f(-0.66f, 0.11f);
    glVertex2f(-0.58f, 0.06f); glVertex2f(-0.54f, 0.06f); glVertex2f(-0.54f, 0.11f); glVertex2f(-0.58f, 0.11f);
    glVertex2f(-0.50f, 0.06f); glVertex2f(-0.46f, 0.06f); glVertex2f(-0.46f, 0.11f); glVertex2f(-0.50f, 0.11f);
    glEnd();

    // Building 3 (Deep Cobalt Blue with Emerald Green Roof)
    glColor3f(0.12f, 0.30f, 0.70f);
    glBegin(GL_QUADS);
    glVertex2f(-0.40f, -0.15f); glVertex2f(-0.15f, -0.15f);
    glVertex2f(-0.15f,  0.48f); glVertex2f(-0.40f,  0.48f);
    glEnd();

    glColor3f(0.08f, 0.50f, 0.22f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.40f, 0.48f); glVertex2f(-0.15f, 0.48f); glVertex2f(-0.275f, 0.58f);
    glEnd();

    glColor3f(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(-0.36f, 0.36f); glVertex2f(-0.32f, 0.36f); glVertex2f(-0.32f, 0.41f); glVertex2f(-0.36f, 0.41f);
    glVertex2f(-0.28f, 0.36f); glVertex2f(-0.24f, 0.36f); glVertex2f(-0.24f, 0.41f); glVertex2f(-0.28f, 0.41f);
    glVertex2f(-0.20f, 0.36f); glVertex2f(-0.16f, 0.36f); glVertex2f(-0.16f, 0.41f); glVertex2f(-0.20f, 0.41f);
    glVertex2f(-0.36f, 0.24f); glVertex2f(-0.32f, 0.24f); glVertex2f(-0.32f, 0.29f); glVertex2f(-0.36f, 0.29f);
    glVertex2f(-0.28f, 0.24f); glVertex2f(-0.24f, 0.24f); glVertex2f(-0.24f, 0.29f); glVertex2f(-0.28f, 0.29f);
    glVertex2f(-0.20f, 0.24f); glVertex2f(-0.16f, 0.24f); glVertex2f(-0.16f, 0.29f); glVertex2f(-0.20f, 0.29f);
    glEnd();

    // Building 4 (Rich Emerald Green with Midnight Black Roof)
    glColor3f(0.08f, 0.52f, 0.22f);
    glBegin(GL_QUADS);
    glVertex2f(-0.15f, -0.15f); glVertex2f( 0.10f, -0.15f);
    glVertex2f( 0.10f,  0.44f); glVertex2f(-0.15f,  0.44f);
    glEnd();

    glColor3f(0.12f, 0.12f, 0.15f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.15f, 0.44f); glVertex2f(0.10f, 0.44f); glVertex2f(-0.025f, 0.53f);
    glEnd();

    glColor3f(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(-0.11f, 0.32f); glVertex2f(-0.07f, 0.32f); glVertex2f(-0.07f, 0.37f); glVertex2f(-0.11f, 0.37f);
    glVertex2f(-0.03f, 0.32f); glVertex2f( 0.01f, 0.32f); glVertex2f( 0.01f, 0.37f); glVertex2f(-0.03f, 0.37f);
    glVertex2f( 0.05f, 0.32f); glVertex2f( 0.09f, 0.32f); glVertex2f( 0.09f, 0.37f); glVertex2f( 0.05f, 0.37f);
    glVertex2f(-0.11f, 0.20f); glVertex2f(-0.07f, 0.20f); glVertex2f(-0.07f, 0.25f); glVertex2f(-0.11f, 0.25f);
    glVertex2f(-0.03f, 0.20f); glVertex2f( 0.01f, 0.20f); glVertex2f( 0.01f, 0.25f); glVertex2f(-0.03f, 0.25f);
    glVertex2f( 0.05f, 0.20f); glVertex2f( 0.09f, 0.20f); glVertex2f( 0.09f, 0.25f); glVertex2f( 0.05f, 0.25f);
    glEnd();

    // Building 5 (Pitch Black Skyscraper with Vibrant Red Trim)
    glColor3f(0.14f, 0.14f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(0.10f, -0.15f); glVertex2f(0.40f, -0.15f);
    glVertex2f(0.40f,  0.47f); glVertex2f(0.10f,  0.47f);
    glEnd();

    glColor3f(0.85f, 0.15f, 0.15f);
    glBegin(GL_QUADS);
    glVertex2f(0.09f, 0.47f); glVertex2f(0.41f, 0.47f); glVertex2f(0.41f, 0.50f); glVertex2f(0.09f, 0.50f);
    glEnd();

    glColor3f(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(0.14f, 0.35f); glVertex2f(0.18f, 0.35f); glVertex2f(0.18f, 0.40f); glVertex2f(0.14f, 0.40f);
    glVertex2f(0.22f, 0.35f); glVertex2f(0.26f, 0.35f); glVertex2f(0.26f, 0.40f); glVertex2f(0.22f, 0.40f);
    glVertex2f(0.30f, 0.35f); glVertex2f(0.34f, 0.35f); glVertex2f(0.34f, 0.40f); glVertex2f(0.30f, 0.40f);
    glVertex2f(0.14f, 0.23f); glVertex2f(0.18f, 0.23f); glVertex2f(0.18f, 0.28f); glVertex2f(0.14f, 0.28f);
    glVertex2f(0.22f, 0.23f); glVertex2f(0.26f, 0.23f); glVertex2f(0.26f, 0.28f); glVertex2f(0.22f, 0.28f);
    glVertex2f(0.30f, 0.23f); glVertex2f(0.34f, 0.23f); glVertex2f(0.34f, 0.28f); glVertex2f(0.30f, 0.28f);
    glEnd();

    // Building 6 (Sapphire Blue with Dark Crimson Peak)
    glColor3f(0.15f, 0.35f, 0.80f);
    glBegin(GL_QUADS);
    glVertex2f(0.40f, -0.15f); glVertex2f(0.70f, -0.15f);
    glVertex2f(0.70f,  0.49f); glVertex2f(0.40f,  0.49f);
    glEnd();

    glColor3f(0.65f, 0.12f, 0.15f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.40f, 0.49f); glVertex2f(0.70f, 0.49f); glVertex2f(0.55f, 0.58f);
    glEnd();

    glColor3f(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(0.44f, 0.37f); glVertex2f(0.48f, 0.37f); glVertex2f(0.48f, 0.42f); glVertex2f(0.44f, 0.42f);
    glVertex2f(0.52f, 0.37f); glVertex2f(0.56f, 0.37f); glVertex2f(0.56f, 0.42f); glVertex2f(0.52f, 0.42f);
    glVertex2f(0.60f, 0.37f); glVertex2f(0.64f, 0.37f); glVertex2f(0.64f, 0.42f); glVertex2f(0.60f, 0.42f);
    glVertex2f(0.44f, 0.25f); glVertex2f(0.48f, 0.25f); glVertex2f(0.48f, 0.30f); glVertex2f(0.44f, 0.30f);
    glVertex2f(0.52f, 0.25f); glVertex2f(0.56f, 0.25f); glVertex2f(0.56f, 0.30f); glVertex2f(0.52f, 0.30f);
    glVertex2f(0.60f, 0.25f); glVertex2f(0.64f, 0.25f); glVertex2f(0.64f, 0.30f); glVertex2f(0.60f, 0.30f);
    glEnd();

    // Building 7 (Forest Emerald Green with Black Roof)
    glColor3f(0.06f, 0.45f, 0.20f);
    glBegin(GL_QUADS);
    glVertex2f(0.70f, -0.15f); glVertex2f(1.00f, -0.15f);
    glVertex2f(1.00f,  0.43f); glVertex2f(0.70f,  0.43f);
    glEnd();

    glColor3f(0.10f, 0.10f, 0.12f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.70f, 0.43f); glVertex2f(1.00f, 0.43f); glVertex2f(0.85f, 0.52f);
    glEnd();

    glColor3f(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(0.74f, 0.31f); glVertex2f(0.78f, 0.31f); glVertex2f(0.78f, 0.36f); glVertex2f(0.74f, 0.36f);
    glVertex2f(0.82f, 0.31f); glVertex2f(0.86f, 0.31f); glVertex2f(0.86f, 0.36f); glVertex2f(0.82f, 0.36f);
    glVertex2f(0.90f, 0.31f); glVertex2f(0.94f, 0.31f); glVertex2f(0.94f, 0.36f); glVertex2f(0.90f, 0.36f);
    glVertex2f(0.74f, 0.19f); glVertex2f(0.78f, 0.19f); glVertex2f(0.78f, 0.24f); glVertex2f(0.74f, 0.24f);
    glVertex2f(0.82f, 0.19f); glVertex2f(0.86f, 0.19f); glVertex2f(0.86f, 0.24f); glVertex2f(0.82f, 0.24f);
    glVertex2f(0.90f, 0.19f); glVertex2f(0.94f, 0.19f); glVertex2f(0.94f, 0.24f); glVertex2f(0.90f, 0.24f);
    glEnd();

    // ================= Foreground Specialty Buildings =================
    // Building 1 (Foreground Left - Crimson Red with Jet Black Roof)
    glColor3f(0.82f, 0.12f, 0.15f);
    glBegin(GL_QUADS);
    glVertex2f(-0.98f, -0.15f); glVertex2f(-0.78f, -0.15f);
    glVertex2f(-0.78f,  0.22f); glVertex2f(-0.98f,  0.22f);
    glEnd();

    glColor3f(0.12f, 0.12f, 0.14f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.98f, 0.22f); glVertex2f(-0.78f, 0.22f); glVertex2f(-0.88f, 0.31f);
    glEnd();

    glColor3f(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(-0.94f, 0.12f); glVertex2f(-0.90f, 0.12f); glVertex2f(-0.90f, 0.17f); glVertex2f(-0.94f, 0.17f);
    glVertex2f(-0.86f, 0.12f); glVertex2f(-0.82f, 0.12f); glVertex2f(-0.82f, 0.17f); glVertex2f(-0.86f, 0.17f);
    glVertex2f(-0.94f, 0.02f); glVertex2f(-0.90f, 0.02f); glVertex2f(-0.90f, 0.07f); glVertex2f(-0.94f, 0.07f);
    glVertex2f(-0.86f, 0.02f); glVertex2f(-0.82f, 0.02f); glVertex2f(-0.82f, 0.07f); glVertex2f(-0.86f, 0.07f);
    glEnd();

    // Building 2 (Foreground - Pitch Black with Blue Roof Peak)
    glColor3f(0.12f, 0.15f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(-0.73f, -0.15f); glVertex2f(-0.53f, -0.15f);
    glVertex2f(-0.53f,  0.32f); glVertex2f(-0.73f,  0.32f);
    glEnd();

    glColor3f(0.15f, 0.45f, 0.85f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.73f, 0.32f); glVertex2f(-0.53f, 0.32f); glVertex2f(-0.63f, 0.41f);
    glEnd();

    glColor3f(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(-0.69f, 0.20f); glVertex2f(-0.65f, 0.20f); glVertex2f(-0.65f, 0.25f); glVertex2f(-0.69f, 0.25f);
    glVertex2f(-0.61f, 0.20f); glVertex2f(-0.57f, 0.20f); glVertex2f(-0.57f, 0.25f); glVertex2f(-0.61f, 0.25f);
    glVertex2f(-0.69f, 0.08f); glVertex2f(-0.65f, 0.08f); glVertex2f(-0.65f, 0.13f); glVertex2f(-0.69f, 0.13f);
    glVertex2f(-0.61f, 0.08f); glVertex2f(-0.57f, 0.08f); glVertex2f(-0.57f, 0.13f); glVertex2f(-0.61f, 0.13f);
    glEnd();

    // ================= HOSPITAL (White with Blue Trim & Red Cross) =================
    glColor3f(0.95f, 0.96f, 0.98f);
    glBegin(GL_QUADS);
    glVertex2f(-0.48f, -0.15f); glVertex2f(-0.28f, -0.15f);
    glVertex2f(-0.28f,  0.35f); glVertex2f(-0.48f,  0.35f);
    glEnd();

    glColor3f(0.12f, 0.30f, 0.75f);
    glBegin(GL_QUADS);
    glVertex2f(-0.49f, 0.35f); glVertex2f(-0.27f, 0.35f); glVertex2f(-0.27f, 0.38f); glVertex2f(-0.49f, 0.38f);
    glEnd();

    // Red Cross Emblem
    glColor3f(0.95f, 0.10f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(-0.40f, 0.26f); glVertex2f(-0.36f, 0.26f); glVertex2f(-0.36f, 0.32f); glVertex2f(-0.40f, 0.32f);
    glVertex2f(-0.43f, 0.28f); glVertex2f(-0.33f, 0.28f); glVertex2f(-0.33f, 0.30f); glVertex2f(-0.43f, 0.30f);
    glEnd();

    glColor3f(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(-0.44f, 0.18f); glVertex2f(-0.40f, 0.18f); glVertex2f(-0.40f, 0.23f); glVertex2f(-0.44f, 0.23f);
    glVertex2f(-0.36f, 0.18f); glVertex2f(-0.32f, 0.18f); glVertex2f(-0.32f, 0.23f); glVertex2f(-0.36f, 0.23f);
    glVertex2f(-0.44f, 0.06f); glVertex2f(-0.40f, 0.06f); glVertex2f(-0.40f, 0.11f); glVertex2f(-0.44f, 0.11f);
    glVertex2f(-0.36f, 0.06f); glVertex2f(-0.32f, 0.06f); glVertex2f(-0.32f, 0.11f); glVertex2f(-0.36f, 0.11f);
    glEnd();

    // ================= SCHOOL (Royal Blue with Black Trim) =================
    glColor3f(0.15f, 0.32f, 0.72f);
    glBegin(GL_QUADS);
    glVertex2f(0.02f, -0.15f); glVertex2f(0.27f, -0.15f);
    glVertex2f(0.27f,  0.35f); glVertex2f(0.02f,  0.35f);
    glEnd();

    glColor3f(0.12f, 0.12f, 0.15f);
    glBegin(GL_QUADS);
    glVertex2f(0.01f, 0.35f); glVertex2f(0.28f, 0.35f); glVertex2f(0.28f, 0.38f); glVertex2f(0.01f, 0.38f);
    glEnd();

    glColor3f(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(0.05f, 0.25f); glVertex2f(0.09f, 0.25f); glVertex2f(0.09f, 0.30f); glVertex2f(0.05f, 0.30f);
    glVertex2f(0.12f, 0.25f); glVertex2f(0.16f, 0.25f); glVertex2f(0.16f, 0.30f); glVertex2f(0.12f, 0.30f);
    glVertex2f(0.19f, 0.25f); glVertex2f(0.23f, 0.25f); glVertex2f(0.23f, 0.30f); glVertex2f(0.19f, 0.30f);
    glVertex2f(0.05f, 0.12f); glVertex2f(0.09f, 0.12f); glVertex2f(0.09f, 0.17f); glVertex2f(0.05f, 0.17f);
    glVertex2f(0.12f, 0.12f); glVertex2f(0.16f, 0.12f); glVertex2f(0.16f, 0.17f); glVertex2f(0.12f, 0.17f);
    glVertex2f(0.19f, 0.12f); glVertex2f(0.23f, 0.12f); glVertex2f(0.23f, 0.17f); glVertex2f(0.19f, 0.17f);
    glEnd();

    // ================= FIRE SERVICE STATION (Scarlet Red with Garage) =================
    glColor3f(0.90f, 0.15f, 0.12f);
    glBegin(GL_QUADS);
    glVertex2f(0.32f, -0.15f); glVertex2f(0.54f, -0.15f);
    glVertex2f(0.54f,  0.28f); glVertex2f(0.32f,  0.28f);
    glEnd();

    glColor3f(0.10f, 0.10f, 0.12f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.31f, 0.28f); glVertex2f(0.55f, 0.28f); glVertex2f(0.43f, 0.37f);
    glEnd();

    glColor3f(0.12f, 0.30f, 0.75f);
    glBegin(GL_QUADS);
    glVertex2f(0.41f, 0.20f); glVertex2f(0.45f, 0.20f); glVertex2f(0.45f, 0.24f); glVertex2f(0.41f, 0.24f);
    glEnd();

    glColor3f(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(0.35f, 0.12f); glVertex2f(0.39f, 0.12f); glVertex2f(0.39f, 0.16f); glVertex2f(0.35f, 0.16f);
    glVertex2f(0.47f, 0.12f); glVertex2f(0.51f, 0.12f); glVertex2f(0.51f, 0.16f); glVertex2f(0.47f, 0.16f);
    glEnd();

    // ================= POLICE STATION (Deep Navy Police Blue with Emerald Roof) =================
    glColor3f(0.12f, 0.25f, 0.60f);
    glBegin(GL_QUADS);
    glVertex2f(0.58f, -0.15f); glVertex2f(0.82f, -0.15f);
    glVertex2f(0.82f,  0.33f); glVertex2f(0.58f,  0.33f);
    glEnd();

    glColor3f(0.08f, 0.50f, 0.22f);
    glBegin(GL_QUADS);
    glVertex2f(0.57f, 0.33f); glVertex2f(0.83f, 0.33f); glVertex2f(0.83f, 0.36f); glVertex2f(0.57f, 0.36f);
    glVertex2f(0.61f, 0.36f); glVertex2f(0.79f, 0.36f); glVertex2f(0.79f, 0.39f); glVertex2f(0.61f, 0.39f);
    glEnd();

    glColor3f(0.95f, 0.10f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(0.68f, 0.24f); glVertex2f(0.72f, 0.24f); glVertex2f(0.72f, 0.28f); glVertex2f(0.68f, 0.28f);
    glEnd();

    glColor3f(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(0.62f, 0.16f); glVertex2f(0.66f, 0.16f); glVertex2f(0.66f, 0.21f); glVertex2f(0.62f, 0.21f);
    glVertex2f(0.74f, 0.16f); glVertex2f(0.78f, 0.16f); glVertex2f(0.78f, 0.21f); glVertex2f(0.74f, 0.21f);
    glVertex2f(0.62f, 0.04f); glVertex2f(0.66f, 0.04f); glVertex2f(0.66f, 0.09f); glVertex2f(0.62f, 0.09f);
    glVertex2f(0.74f, 0.04f); glVertex2f(0.78f, 0.04f); glVertex2f(0.78f, 0.09f); glVertex2f(0.74f, 0.09f);
    glEnd();

    // Building 7 (Foreground Right - Midnight Black with Crimson Roof Peak)
    glColor3f(0.14f, 0.15f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(0.86f, -0.15f); glVertex2f(1.00f, -0.15f);
    glVertex2f(1.00f,  0.24f); glVertex2f(0.86f,  0.24f);
    glEnd();

    glColor3f(0.75f, 0.12f, 0.15f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.86f, 0.24f); glVertex2f(1.00f, 0.24f); glVertex2f(0.93f, 0.33f);
    glEnd();

    glColor3f(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(0.89f, 0.12f); glVertex2f(0.93f, 0.12f); glVertex2f(0.93f, 0.17f); glVertex2f(0.89f, 0.17f);
    glVertex2f(0.95f, 0.12f); glVertex2f(0.99f, 0.12f); glVertex2f(0.99f, 0.17f); glVertex2f(0.95f, 0.17f);
    glVertex2f(0.89f, 0.00f); glVertex2f(0.93f, 0.00f); glVertex2f(0.93f, 0.05f); glVertex2f(0.89f, 0.05f);
    glVertex2f(0.95f, 0.00f); glVertex2f(0.99f, 0.00f); glVertex2f(0.99f, 0.05f); glVertex2f(0.95f, 0.05f);
    glEnd();
}

void drawGround() {
    glColor3f(0.08f, 0.58f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -0.15f);
    glVertex2f( 1.0f, -0.15f);
    glVertex2f( 1.0f, -0.25f);
    glVertex2f(-1.0f, -0.25f);
    glEnd();
}

// ==========================================
// START SCREEN (AIUB PROJECT & STUDENT CREDITS)
// ==========================================
void drawStartScreen() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glClearColor(0.06f, 0.09f, 0.16f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Outer Decorative Frame
    glColor3f(0.30f, 0.70f, 0.90f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.94f, -0.94f);
    glVertex2f( 0.94f, -0.94f);
    glVertex2f( 0.94f,  0.94f);
    glVertex2f(-0.94f,  0.94f);
    glEnd();
    glLineWidth(1.0f);



    glColor3f(1.0f, 0.80f, 0.20f);
    drawText(-0.18f, 0.76f, "HEY GOOD PEOPLE,LETS PLAY");


    glColor3f(0.20f, 0.95f, 0.85f);
    drawText(-0.45f, 0.56f, "PROJECT ON:3D Car Racing and Collecting Coin");

    // Student Group Credits
    glColor3f(1.0f, 0.40f, 0.40f);
    drawText(-0.35f, 0.42f, " Made by: Amartay Das [ID: 23-55068-3]");

    // Driving Controls
    glColor3f(1.0f, 0.85f, 0.30f);
    drawText(-0.45f, 0.06f, "--- DRIVING & CAR HANDLING CONTROLS ---");

    glColor3f(0.92f, 0.95f, 0.98f);
    drawText(-0.58f, -0.02f, "[W] or [UP]        : Gas & Accelerate (Nitro Boost)");
    drawText(-0.58f, -0.09f, "[S] or [DOWN]      : ABS Brakes / Reverse");
    drawText(-0.58f, -0.16f, "[A] / [D] or LEFT/RIGHT : Smooth Lane Change Across 4 Lanes");
    drawText(-0.58f, -0.23f, "[SPACE]            : Suspension Hop / Jump");
    drawText(-0.58f, -0.30f, "[C]                : Toggle Camera View Modes");
    drawText(-0.58f, -0.37f, "[R]                : Restart / Drive Again");

    // Backend Atmosphere Controls
    glColor3f(0.30f, 0.90f, 1.0f);
    drawText(-0.45f, -0.48f, "--- BACKEND ATMOSPHERE & ENVIRONMENT CONTROLS ---");

    glColor3f(0.90f, 0.92f, 0.95f);
    drawText(-0.58f, -0.56f, "Press [1]          : Day Mode (Sunlight)");
    drawText(-0.58f, -0.63f, "Press [N] or [2]   : Night Mode (Moon, Stars, Headlights & Glow)");
    drawText(-0.58f, -0.70f, "Press [T]          : Rain Storm On / Off (Clouds & Drops)");
    drawText(-0.58f, -0.77f, "Press [L] / [K]    : Wind Left to Right / Right to Left");
    drawText(-0.58f, -0.84f, "Press [O]          : Stop Wind (Calm Breeze)");

    // Start Prompt
    glColor3f(0.25f, 1.0f, 0.45f);
    drawText(-0.22f, -0.91f, ">>> PRESS [S] TO START <<<");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glutSwapBuffers();
}

// ##########################################################################
// ADDED: all previously missing function implementations
// (physics, particles, 3D BMW car, road, trees, coins, HUD)
// ##########################################################################

// ---------- small helpers ----------
static float frand(float a, float b) {
    return a + (b - a) * ((float)rand() / (float)RAND_MAX);
}

static void drawBox(float x, float y, float z, float sx, float sy, float sz) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(sx, sy, sz);
    glutSolidCube(1.0f);
    glPopMatrix();
}

// ---------- particles ----------
void spawnExhaust(float x, float y, float z) {
    for (int i = 0; i < MAX_EXHAUST; i++) {
        if (exhausts[i].life <= 0.0f) {
            ExhaustFX* e = &exhausts[i];
            e->x = x; e->y = y; e->z = z;
            e->vx = frand(-2.0f, -0.5f);
            e->vy = frand(0.2f, 0.8f);
            e->vz = frand(-0.3f, 0.3f);
            e->maxLife = e->life = frand(0.4f, 0.9f);
            e->size = frand(6.0f, 12.0f);
            return;
        }
    }
}

void spawnCoinBurst(float x, float y, float z) {
    int spawned = 0;
    for (int i = 0; i < MAX_SPARKLES && spawned < 18; i++) {
        if (sparkles[i].life <= 0.0f) {
            SparkleFX* s = &sparkles[i];
            s->x = x; s->y = y; s->z = z;
            s->vx = frand(-3.0f, 3.0f);
            s->vy = frand(1.5f, 5.0f);
            s->vz = frand(-3.0f, 3.0f);
            s->maxLife = s->life = frand(0.4f, 0.9f);
            s->size = frand(4.0f, 8.0f);
            s->r = 1.0f; s->g = frand(0.75f, 0.95f); s->b = frand(0.1f, 0.4f);
            spawned++;
        }
    }
}

void updateParticles(float dt) {
    for (int i = 0; i < MAX_SPARKLES; i++) {
        SparkleFX* s = &sparkles[i];
        if (s->life <= 0.0f) continue;
        s->life -= dt;
        s->vy -= 6.0f * dt;
        s->x += s->vx * dt; s->y += s->vy * dt; s->z += s->vz * dt;
    }
    for (int i = 0; i < MAX_EXHAUST; i++) {
        ExhaustFX* e = &exhausts[i];
        if (e->life <= 0.0f) continue;
        e->life -= dt;
        e->x += e->vx * dt; e->y += e->vy * dt; e->z += e->vz * dt;
    }
}

// ---------- game physics ----------
void updatePhysics(float dt) {
    updateParticles(dt);
    if (gameState != STATE_PLAYING) return;

    gameTime += dt;

    // --- throttle / brake ---
    if (keyAccelerate) {
        float accel = 6.0f;                           // gentle acceleration
        if (nitro > 0.0f && carSpeed > 12.0f) {       // nitro boost
            accel += 4.0f;
            nitro -= 10.0f * dt;
        }
        carSpeed += accel * dt;
    } else {
        nitro = minF(MAX_NITRO, nitro + 3.0f * dt);
        if (!keyBrake) {                            // natural drag
            if (carSpeed > 0.0f) carSpeed = maxF(0.0f, carSpeed - 3.0f * dt);
            else                 carSpeed = minF(0.0f, carSpeed + 3.0f * dt);
        }
    }
    if (keyBrake && !keyAccelerate) carSpeed -= 18.0f * dt;
    if (nitro < 0.0f) nitro = 0.0f;
    carSpeed = clampF(carSpeed, -4.0f, MAX_SPEED);

    // --- steering (+Z is the driver's right) ---
    float input = (float)(keySteerRight - keySteerLeft);
    steerAngle += (input * 14.0f - steerAngle) * minF(1.0f, 4.0f * dt);
    float lat = sinf(steerAngle * (float)M_PI / 180.0f) * clampF(carSpeed, -6.0f, 14.0f) * 0.22f;
    carZ = clampF(carZ + lat * dt, -4.2f, 4.2f);

    float speedFrac = clampF(fabsf(carSpeed) / MAX_SPEED, 0.0f, 1.0f);
    carYaw  += (steerAngle * 0.4f - carYaw) * minF(1.0f, 6.0f * dt);
    carRoll += (-input * speedFrac * 3.0f - carRoll) * minF(1.0f, 5.0f * dt);

    float diveTarget = keyBrake ? speedFrac * 3.0f : (keyAccelerate ? -1.0f : 0.0f);
    suspensionDive += (diveTarget - suspensionDive) * minF(1.0f, 6.0f * dt);
    carPitch = -suspensionDive;

    // --- jump ---
    if (keyJump && isGrounded) { carVy = 5.0f; isGrounded = 0; }
    if (!isGrounded) {
        carVy -= 18.0f * dt;
        carY += carVy * dt;
        if (carY <= CAR_WHEEL_RADIUS) {
            carY = CAR_WHEEL_RADIUS; carVy = 0.0f; isGrounded = 1;
        }
    }

    // --- forward motion ---
    carX += carSpeed * dt;
    if (carX < 2.0f) carX = 2.0f;
    wheelSpinAngle = fmodf(wheelSpinAngle + carSpeed / CAR_WHEEL_RADIUS * dt * 57.29578f, 360.0f);

    currentDistance = carX - 14.0f;
    if (currentDistance > bestDistance) bestDistance = currentDistance;
    if (currentDistance >= HIGHWAY_LENGTH) {
        gameState = STATE_FINISHED;
        carSpeed = 0.0f;
    }

    // --- coin pickup ---
    for (int i = 0; i < COIN_COUNT; i++) {
        if (coins[i].collected) continue;
        if (fabsf(coins[i].x - carX) < 1.4f && fabsf(coins[i].z - carZ) < 1.1f && carY < 1.6f) {
            coins[i].collected = 1;
            coinsCount++;
            nitro = minF(MAX_NITRO, nitro + 5.0f);
            spawnCoinBurst(coins[i].x, 1.0f, coins[i].z);
        }
    }

    // --- exhaust smoke ---
    if (carSpeed > 2.0f && rand() % 3 == 0)
        spawnExhaust(carX - CAR_LENGTH * 0.5f * CAR_SCALE - 0.05f, carY - 0.15f * CAR_SCALE, carZ + (rand() % 2 ? 0.55f : -0.55f) * CAR_SCALE);

    // --- camera (X is locked, Y/Z are smoothed so there is no rubber-band lag) ---
    float tx, ty, tz;
    if (cameraMode == 0)      { tx = carX - 4.2f; ty = 1.55f;       tz = carZ * 0.28f; }
    else if (cameraMode == 1) { tx = carX;        ty = 1.3f;        tz = carZ - 6.0f;  }
    else                      { tx = carX + 0.25f * CAR_SCALE; ty = carY + 0.80f * CAR_SCALE; tz = carZ; }
    float k = (cameraMode == 2) ? 1.0f : minF(1.0f, 8.0f * dt);
    camSmoothX = tx;
    camSmoothY += (ty - camSmoothY) * k;
    camSmoothZ += (tz - camSmoothZ) * k;
}

// ---------- 3D primitives ----------
void drawCylinder3D(float rBase, float rTop, float height, int slices) {
    gluCylinder(quadric, rBase, rTop, height, slices, 1);
}

void drawDisk3D(float innerRadius, float outerRadius, int slices) {
    gluDisk(quadric, innerRadius, outerRadius, slices, 1);
}

void drawBMWWheel3D(float radius, float width, float spinAngle) {
    glPushMatrix();
    glRotatef(-spinAngle, 0, 0, 1);          // axle = Z axis
    glTranslatef(0, 0, -width * 0.5f);

    glColor3f(0.05f, 0.05f, 0.05f);          // tyre tread
    drawCylinder3D(radius, radius, width, 24);

    for (int side = 0; side < 2; side++) {   // side 0 = inner(-Z), side 1 = outer(+Z)
        float z = side ? width : 0.0f;
        glPushMatrix();
        glTranslatef(0, 0, z);
        if (!side) glRotatef(180, 0, 1, 0);  // make disk face outward

        glColor3f(0.08f, 0.08f, 0.08f);      // sidewall
        drawDisk3D(radius * 0.62f, radius, 24);

        glTranslatef(0, 0, 0.004f);
        glColor3f(0.78f, 0.80f, 0.84f);      // alloy rim
        drawDisk3D(0.0f, radius * 0.62f, 24);

        glColor3f(0.25f, 0.27f, 0.30f);      // 5 spokes
        for (int i = 0; i < 5; i++) {
            glPushMatrix();
            glRotatef(i * 72.0f, 0, 0, 1);
            glTranslatef(radius * 0.30f, 0, 0.004f);
            glScalef(radius * 0.60f, 0.05f, 0.02f);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
        glColor3f(0.05f, 0.25f, 0.85f);      // BMW blue centre cap
        drawDisk3D(0.0f, radius * 0.12f, 12);
        glPopMatrix();
    }
    glPopMatrix();
}

void drawBMWEmblem(float size) {
    glPushMatrix();
    glColor3f(0.05f, 0.05f, 0.05f);
    drawDisk3D(0.0f, size, 32);
    glTranslatef(0, 0, 0.003f);
    float r = size * 0.82f;
    glColor3f(0.10f, 0.40f, 0.90f);
    gluPartialDisk(quadric, 0, r, 16, 1, 0, 90);
    gluPartialDisk(quadric, 0, r, 16, 1, 180, 90);
    glColor3f(0.95f, 0.95f, 0.95f);
    gluPartialDisk(quadric, 0, r, 16, 1, 90, 90);
    gluPartialDisk(quadric, 0, r, 16, 1, 270, 90);
    glPopMatrix();
}

void drawKidneyGrille(float width, float height) {
    glPushMatrix();
    glColor3f(0.75f, 0.75f, 0.78f);                  // chrome frame
    drawBox(0, 0, 0, 0.05f, height, width);
    glColor3f(0.03f, 0.03f, 0.03f);                  // black mesh
    drawBox(0.03f, 0, 0, 0.03f, height * 0.8f, width * 0.78f);
    glColor3f(0.60f, 0.60f, 0.62f);                  // slats
    for (int i = -1; i <= 1; i++)
        drawBox(0.055f, i * height * 0.22f, 0, 0.01f, 0.012f, width * 0.74f);
    glPopMatrix();
}

// ---------- the car (local axes: +X forward, +Y up, +Z right) ----------
void drawBMWSportsCar() {
    // soft contact shadow
    glPushMatrix();
    glTranslatef(carX, 0.02f, carZ);
    glRotatef(-carYaw, 0, 1, 0);
    glScalef(CAR_SCALE, 1.0f, CAR_SCALE);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0, 0, 0, 0.40f);
    glBegin(GL_QUADS);
    glVertex3f(-2.0f, 0, -0.95f); glVertex3f(-2.0f, 0, 0.95f);
    glVertex3f( 2.0f, 0,  0.95f); glVertex3f( 2.0f, 0, -0.95f);
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(carX, carY - CAR_WHEEL_RADIUS * (1.0f - CAR_SCALE), carZ);
    glRotatef(-carYaw, 0, 1, 0);
    glRotatef(carPitch, 0, 0, 1);
    glRotatef(carRoll, 1, 0, 0);
    glScalef(CAR_SCALE, CAR_SCALE, CAR_SCALE);

    GLfloat spec[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 80.0f);

    // body
    glColor3f(0.05f, 0.25f, 0.75f);
    drawBox(0.00f, 0.08f, 0, CAR_LENGTH, 0.45f, CAR_WIDTH);      // lower body
    drawBox(1.15f, 0.36f, 0, 1.40f, 0.08f, 1.60f);               // bonnet / power dome
    drawBox(-1.45f, 0.34f, 0, 0.90f, 0.08f, 1.60f);              // boot
    drawBox(-0.15f, 0.56f, 0, 1.80f, 0.40f, 1.50f);              // cabin
    glColor3f(0.04f, 0.06f, 0.09f);                              // glass
    drawBox(-0.15f, 0.58f, 0, 1.60f, 0.26f, 1.52f);
    glColor3f(0.05f, 0.05f, 0.06f);                              // carbon roof + trim
    drawBox(-0.20f, 0.78f, 0, 1.30f, 0.04f, 1.40f);
    drawBox( 1.93f, -0.08f, 0, 0.10f, 0.14f, 1.70f);             // front splitter
    drawBox(-1.93f, -0.06f, 0, 0.10f, 0.16f, 1.70f);             // rear diffuser
    drawBox(0.0f, -0.12f,  0.87f, 2.2f, 0.08f, 0.06f);           // side skirts
    drawBox(0.0f, -0.12f, -0.87f, 2.2f, 0.08f, 0.06f);
    drawBox(0.55f, 0.50f,  0.88f, 0.12f, 0.08f, 0.15f);          // mirrors
    drawBox(0.55f, 0.50f, -0.88f, 0.12f, 0.08f, 0.15f);

    // kidney grille + roundel
    glPushMatrix();
    glTranslatef(1.96f, 0.12f, -0.17f); drawKidneyGrille(0.26f, 0.17f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1.96f, 0.12f,  0.17f); drawKidneyGrille(0.26f, 0.17f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1.96f, 0.30f, 0.0f);
    glRotatef(90, 0, 1, 0);
    drawBMWEmblem(0.07f);
    glPopMatrix();

    // lights (self-lit)
    int night = (dayValue < 0.4f || rainIntensity > 0.4f);
    glDisable(GL_LIGHTING);
    glColor3f(night ? 1.0f : 0.85f, night ? 1.0f : 0.85f, night ? 0.85f : 0.80f);
    drawBox(1.96f, 0.22f,  0.62f, 0.05f, 0.08f, 0.35f);
    drawBox(1.96f, 0.22f, -0.62f, 0.05f, 0.08f, 0.35f);
    glColor3f(keyBrake ? 1.0f : 0.65f, 0.02f, 0.02f);
    drawBox(-1.96f, 0.26f,  0.60f, 0.05f, 0.08f, 0.40f);
    drawBox(-1.96f, 0.26f, -0.60f, 0.05f, 0.08f, 0.40f);

    if (night) {                                                  // headlight beams on the road
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        glColor4f(1.0f, 0.95f, 0.70f, 0.20f);
        float y = -CAR_WHEEL_RADIUS + 0.03f;
        for (int s = -1; s <= 1; s += 2) {
            float zc = 0.62f * s;
            glBegin(GL_QUADS);
            glVertex3f(2.0f,  y, zc - 0.2f); glVertex3f(2.0f,  y, zc + 0.2f);
            glVertex3f(16.0f, y, zc + 2.2f); glVertex3f(16.0f, y, zc - 2.2f);
            glEnd();
        }
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
    glEnable(GL_LIGHTING);

    // wheels
    for (int sx = -1; sx <= 1; sx += 2) {
        for (int sz = -1; sz <= 1; sz += 2) {
            glPushMatrix();
            glTranslatef(sx * CAR_WHEEL_BASE * 0.5f, 0.0f, sz * CAR_WHEEL_TRACK * 0.5f);
            if (sx > 0) glRotatef(-steerAngle, 0, 1, 0);          // front wheels steer
            drawBMWWheel3D(CAR_WHEEL_RADIUS, CAR_WHEEL_WIDTH, wheelSpinAngle);
            glPopMatrix();
        }
    }

    GLfloat noSpec[] = { 0, 0, 0, 1 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, noSpec);
    glPopMatrix();
}

// ---------- road, trees, coins, particles ----------
void drawPitchBlackEuropeanRoad() {
    float x0 = carX - 25.0f, x1 = carX + 450.0f;
    const float RW = 4.6f;

    glNormal3f(0, 1, 0);
    glColor3f(0.10f, 0.45f, 0.14f);                      // grass
    glBegin(GL_QUADS);
    glVertex3f(x0, -0.02f, -250); glVertex3f(x0, -0.02f, 250);
    glVertex3f(x1, -0.02f,  250); glVertex3f(x1, -0.02f, -250);
    glEnd();

    glColor3f(0.05f, 0.05f, 0.06f);                      // asphalt
    glBegin(GL_QUADS);
    glVertex3f(x0, 0, -RW); glVertex3f(x0, 0, RW);
    glVertex3f(x1, 0,  RW); glVertex3f(x1, 0, -RW);
    glEnd();

    glColor3f(0.95f, 0.95f, 0.95f);                      // solid edge lines
    for (int s = -1; s <= 1; s += 2) {
        float z = 4.4f * s;
        glBegin(GL_QUADS);
        glVertex3f(x0, 0.01f, z - 0.07f); glVertex3f(x0, 0.01f, z + 0.07f);
        glVertex3f(x1, 0.01f, z + 0.07f); glVertex3f(x1, 0.01f, z - 0.07f);
        glEnd();
    }

    float zs[3] = { -2.3f, 0.0f, 2.3f };                 // dashed lane lines
    float start = floorf(x0 / 9.0f) * 9.0f;
    glBegin(GL_QUADS);
    for (int i = 0; i < 3; i++)
        for (float x = start; x < x1; x += 9.0f) {
            glVertex3f(x,        0.01f, zs[i] - 0.06f); glVertex3f(x,        0.01f, zs[i] + 0.06f);
            glVertex3f(x + 3.0f, 0.01f, zs[i] + 0.06f); glVertex3f(x + 3.0f, 0.01f, zs[i] - 0.06f);
        }
    glEnd();

    // Leitpfosten (roadside delineator posts)
    float px = floorf(x0 / 50.0f) * 50.0f;
    for (float x = px; x < carX + 300.0f; x += 50.0f)
        for (int s = -1; s <= 1; s += 2) {
            glColor3f(0.95f, 0.95f, 0.95f);
            drawBox(x, 0.45f, 4.9f * s, 0.12f, 0.9f, 0.12f);
            glColor3f(0.05f, 0.05f, 0.05f);
            drawBox(x, 0.70f, 4.9f * s + 0.0f, 0.125f, 0.15f, 0.125f);
        }
}

void drawOrganicRoadsideTreesAndHorizon() {
    // distant hills
    glColor3f(0.10f, 0.35f, 0.15f);
    for (int i = -1; i <= 1; i++) {
        glPushMatrix();
        glTranslatef(carX + 330.0f, 0.0f, i * 160.0f);
        glScalef(1.0f, 0.25f, 1.0f);
        glutSolidSphere(90.0f, 16, 12);
        glPopMatrix();
    }

    // trees (deterministic per world position so they do not flicker)
    float startX = floorf((carX - 15.0f) / 18.0f) * 18.0f;
    for (int k = 0; k < 26; k++) {
        float x = startX + k * 18.0f;
        int idx = (int)floorf(x / 18.0f);
        for (int side = -1; side <= 1; side += 2) {
            unsigned h = ((unsigned)(idx * 73856093) ^ (unsigned)(side * 19349663)) >> 3;
            float t = (float)(h % 1000) / 1000.0f;
            float s = 0.8f + (float)((h * 7u) % 100u) / 100.0f * 0.7f;
            float off = 7.0f + t * 10.0f;

            glPushMatrix();
            glTranslatef(x, 0.0f, side * off);
            glColor3f(0.35f, 0.22f, 0.10f);
            glPushMatrix();
            glRotatef(-90, 1, 0, 0);
            gluCylinder(quadric, 0.25f * s, 0.18f * s, 2.2f * s, 10, 1);
            glPopMatrix();
            glColor3f(0.05f + t * 0.10f, 0.40f + t * 0.15f, 0.12f);
            glTranslatef(0, 3.2f * s, 0);
            glutSolidSphere(1.4f * s, 12, 10);
            glTranslatef(0, 1.0f * s, 0);
            glutSolidSphere(0.9f * s, 12, 10);
            glPopMatrix();
        }
    }
}

void drawGorgeousCoins() {
    for (int i = 0; i < COIN_COUNT; i++) {
        GorgeousCoin* c = &coins[i];
        if (c->collected) continue;
        if (c->x < carX - 10.0f || c->x > carX + 260.0f) continue;

        float y = 0.95f + 0.12f * sinf(gameTime * 3.0f + c->bobPhase);
        glPushMatrix();
        glTranslatef(c->x, y, c->z);
        glRotatef(gameTime * c->spinSpeed, 0, 1, 0);
        glTranslatef(0, 0, -0.03f);

        glColor3f(1.0f, 0.80f, 0.10f);
        drawCylinder3D(0.38f, 0.38f, 0.06f, 24);

        glColor3f(1.0f, 0.90f, 0.30f);
        glPushMatrix();
        glRotatef(180, 0, 1, 0);
        drawDisk3D(0.0f, 0.38f, 24);
        glPopMatrix();

        glTranslatef(0, 0, 0.06f);
        drawDisk3D(0.0f, 0.38f, 24);
        glTranslatef(0, 0, 0.002f);
        glColor3f(0.85f, 0.62f, 0.05f);
        drawDisk3D(0.20f, 0.30f, 24);
        glPopMatrix();
    }
}

void drawParticles() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    for (int i = 0; i < MAX_EXHAUST; i++) {
        ExhaustFX* e = &exhausts[i];
        if (e->life <= 0.0f) continue;
        float a = e->life / e->maxLife;
        glPointSize(e->size * (1.5f - a));
        glColor4f(0.6f, 0.6f, 0.6f, 0.35f * a);
        glBegin(GL_POINTS);
        glVertex3f(e->x, e->y, e->z);
        glEnd();
    }
    for (int i = 0; i < MAX_SPARKLES; i++) {
        SparkleFX* s = &sparkles[i];
        if (s->life <= 0.0f) continue;
        float a = s->life / s->maxLife;
        glPointSize(s->size);
        glColor4f(s->r, s->g, s->b, a);
        glBegin(GL_POINTS);
        glVertex3f(s->x, s->y, s->z);
        glEnd();
    }

    glPointSize(1.0f);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// ---------- road lamp posts (light up at night / in rain) ----------
void drawRoadLampPosts() {
    int night = (dayValue < 0.4f || rainIntensity > 0.4f);
    const float POLE_H = 6.0f;
    const float ROAD_SIDE = 5.6f;     // distance of pole from road centre
    const float ARM_LEN = 2.0f;       // arm reaches toward the road

    float startX = floorf((carX - 20.0f) / 18.0f) * 18.0f;
    for (int k = 0; k < 18; k++) {
        float x = startX + k * 18.0f;
        int idx = (int)floorf(x / 18.0f);
        float side = (idx & 1) ? 1.0f : -1.0f;       // staggered left / right
        float poleZ = ROAD_SIDE * side;
        float lampZ = poleZ - side * ARM_LEN;

        // pole, base, arm, lamp housing (lit by the scene light)
        glColor3f(0.22f, 0.24f, 0.28f);
        glPushMatrix();
        glTranslatef(x, 0.0f, poleZ);
        drawBox(0.0f, 0.15f, 0.0f, 0.35f, 0.30f, 0.35f);
        glRotatef(-90, 1, 0, 0);
        gluCylinder(quadric, 0.10f, 0.07f, POLE_H, 10, 1);
        glPopMatrix();

        drawBox(x, POLE_H, poleZ - side * ARM_LEN * 0.5f, 0.08f, 0.08f, ARM_LEN);
        glColor3f(0.12f, 0.13f, 0.15f);
        drawBox(x, POLE_H - 0.06f, lampZ, 0.65f, 0.12f, 0.32f);

        // bulb: glows at night, dull in daytime
        glDisable(GL_LIGHTING);
        if (night) glColor3f(1.0f, 0.93f, 0.55f);
        else       glColor3f(0.70f, 0.70f, 0.65f);
        drawBox(x, POLE_H - 0.14f, lampZ, 0.50f, 0.05f, 0.24f);

        if (night) {
            // FIXED: small, subtle light pool (no big blurry halo)
            glDisable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);

            const float POOL_R = 3.2f;   // was 6.5 (smaller = less haze)
            glBegin(GL_TRIANGLE_FAN);
            glColor4f(1.0f, 0.90f, 0.55f, 0.13f);   // was 0.38 (much subtler)
            glVertex3f(x, 0.03f, lampZ);
            glColor4f(1.0f, 0.90f, 0.55f, 0.0f);
            for (int i = 0; i <= 24; i++) {
                float a = i * 6.2831853f / 24.0f;
                glVertex3f(x + POOL_R * cosf(a), 0.03f, lampZ + POOL_R * sinf(a));
            }
            glEnd();

            // NOTE: the big GL_POINTS halo (glPointSize 22) was removed on purpose,
            // it drew a blurry square around each bulb.

            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
            glEnable(GL_CULL_FACE);
        }
        glEnable(GL_LIGHTING);
    }
}

// ---------- HUD ----------
void drawDashboardHUD() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0, 0, 0, 0.45f);
    glBegin(GL_QUADS);
    glVertex2f(-0.98f, 0.70f); glVertex2f(-0.52f, 0.70f);
    glVertex2f(-0.52f, 0.98f); glVertex2f(-0.98f, 0.98f);
    glEnd();
    glDisable(GL_BLEND);

    char buf[128];
    glColor3f(1.0f, 0.95f, 0.6f);
    snprintf(buf, sizeof(buf), "Speed: %d km/h", (int)(fabsf(carSpeed) * 3.6f));
    drawText(-0.96f, 0.91f, buf);
    snprintf(buf, sizeof(buf), "Distance: %d / %d m", (int)currentDistance, (int)HIGHWAY_LENGTH);
    drawText(-0.96f, 0.85f, buf);
    snprintf(buf, sizeof(buf), "Coins: %d   Time: %.1f s", coinsCount, gameTime);
    drawText(-0.96f, 0.79f, buf);

    // nitro bar
    float frac = clampF(nitro / MAX_NITRO, 0.0f, 1.0f);
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(-0.96f, 0.72f); glVertex2f(-0.56f, 0.72f);
    glVertex2f(-0.56f, 0.76f); glVertex2f(-0.96f, 0.76f);
    glEnd();
    glColor3f(0.2f, 0.7f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(-0.96f, 0.72f); glVertex2f(-0.96f + 0.40f * frac, 0.72f);
    glVertex2f(-0.96f + 0.40f * frac, 0.76f); glVertex2f(-0.96f, 0.76f);
    glEnd();

    static const char* camNames[3] = { "Chase", "Side", "Cockpit" };
    glColor3f(0.8f, 0.9f, 1.0f);
    snprintf(buf, sizeof(buf), "Camera: %s [C]", camNames[cameraMode]);
    drawText(0.60f, 0.92f, buf);

    if (gameState == STATE_FINISHED) {
        glColor3f(0.3f, 1.0f, 0.5f);
        drawText(-0.25f, 0.10f, "FINISH! Press [R] to drive again");
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
}

// ==========================================
// MAIN DISPLAY CALLBACK
// ==========================================
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (gameState == STATE_START) {
        drawStartScreen();
        return;
    }

    // ----------------------------------------------------
    // 1. RENDER 2D BACKEND SKYLINE & ENVIRONMENT HORIZON
    // (SKY, SUN/MOON, STARS, CLOUDS, CITY BUILDINGS, FLAGS, PLANE, BIRDS)
    // ----------------------------------------------------
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);   // FIX: 2D shapes have mixed winding

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    drawSky();
    drawStars();
    drawSunMoon();
    drawClouds();
    drawRainClouds();

    // City Skyline Buildings (Hospital, School, Police, Fire, Skyscrapers)
    drawBuildings();
    drawGround();
    drawFlags();
    drawLampPosts();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    // ----------------------------------------------------
    // 2. RENDER 3D EUROPEAN HIGHWAY RACING SCENE
    // (PITCH-BLACK AUTOBAHN, LEITPFOSTEN, 3D TREES, COINS, BMW M-POWER CAR)
    // ----------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);    // FIX: back to culling for 3D

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(62.0, (double)windowWidth / (double)windowHeight, 0.2, 500.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (cameraMode == 0) {
        gluLookAt(camSmoothX, camSmoothY, camSmoothZ,
                  carX + 55.0f, 1.40f, carZ * 0.12f,
                  0.0f, 1.0f, 0.0f);
    } else if (cameraMode == 1) {
        gluLookAt(camSmoothX, camSmoothY, camSmoothZ,
                  carX, carY + 0.4f, carZ,
                  0.0f, 1.0f, 0.0f);
    } else {
        gluLookAt(camSmoothX, camSmoothY, camSmoothZ,
                  carX + 30.0f, 0.95f, carZ - (steerAngle * 2.5f),
                  0.0f, 1.0f, 0.0f);
    }

    // Dynamic Lighting adjusted by Day/Night Cycle
    float ambLevel = 0.15f + dayValue * 0.38f;
    float diffLevel = 0.25f + dayValue * 0.75f;
    if (rainIntensity > 0.0f) {
        ambLevel *= (1.0f - rainIntensity * 0.35f);
        diffLevel *= (1.0f - rainIntensity * 0.45f);
    }

    GLfloat lightPos[] = { carX + 35.0f, 85.0f, 32.0f, 1.0f };
    GLfloat lightAmb[] = { ambLevel, ambLevel, ambLevel * 1.1f, 1.0f };
    GLfloat lightDiff[] = { diffLevel, diffLevel * 0.98f, diffLevel * 0.94f, 1.0f };
    GLfloat lightSpec[] = { diffLevel * 0.85f, diffLevel * 0.85f, diffLevel * 0.85f, 1.0f };

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);

    // 3D Objects
    drawPitchBlackEuropeanRoad();
    drawOrganicRoadsideTreesAndHorizon();
    drawRoadLampPosts();
    drawGorgeousCoins();
    drawParticles();
    drawBMWSportsCar();

    // ----------------------------------------------------
    // 3. 2D OVERLAY: RAIN, WIND STREAKS & DASHBOARD HUD
    // ----------------------------------------------------
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    drawRain();
    drawWindEffect();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    // HUD Telemetry
    drawDashboardHUD();

    glutSwapBuffers();
}

// 60 FPS Fixed-Step Physics & Environment Update
void update(int value) {
    const float dt = 0.01667f;
    updatePhysics(dt);

    // Cloud Movement with Wind
    if(windDirection == 1) {
        cloudMove += 0.0035f;
    } else if(windDirection == -1) {
        cloudMove -= 0.0035f;
    } else {
        cloudMove += 0.0008f;
    }
    if(cloudMove > 1.80f) cloudMove = -1.80f;
    if(cloudMove < -1.80f) cloudMove = 1.80f;

    // Rain Intensity Ramp
    if(rainOn == 1) {
        if(rainIntensity < 1.0f) rainIntensity += 0.012f;
    } else {
        if(rainIntensity > 0.0f) rainIntensity -= 0.012f;
    }

    // Rain Streaks Falling
    for(int i = 0; i < RAIN_COUNT; i++) {
        rainY[i] -= rainSpeed[i];
        rainX[i] += (windDirection == 1) ? 0.003f : ((windDirection == -1) ? -0.003f : 0.001f);

        if(rainY[i] < -1.0f || rainX[i] > 1.15f || rainX[i] < -1.15f) {
            rainY[i] = 1.05f;
            rainX[i] = -1.0f + (rand() % 200) / 100.0f;
        }
    }

    // Day/Night Interpolation
    if(dayValue < targetDayValue) {
        dayValue += 0.006f;
        if(dayValue > targetDayValue) dayValue = targetDayValue;
    }
    if(dayValue > targetDayValue) {
        dayValue -= 0.006f;
        if(dayValue < targetDayValue) dayValue = targetDayValue;
    }

    // Wind Wave
    if(windDirection != 0) {
        windWave += 0.15f;
        if(windWave > 6.283185f) windWave -= 6.283185f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// ==========================================
// INPUT CONTROLS
// ==========================================
void handleKeyDown(unsigned char key, int x, int y) {
    if (gameState == STATE_START) {
        if (key == 's' || key == 'S') {
            gameState = STATE_PLAYING;
            initGame();
        }
        return;
    }

    switch (key) {
        // Driving Controls
        case 'w':
        case 'W':
            keyAccelerate = 1;
            break;
        case 's':
        case 'S':
            keyBrake = 1;
            break;
        case 'a':
        case 'A':
            keySteerLeft = 1;
            break;
        case 'd':
        case 'D':
            keySteerRight = 1;
            break;
        case ' ':
            keyJump = 1;
            break;
        case 'c':
        case 'C':
            cameraMode = (cameraMode + 1) % 3;
            break;
        case 'r':
        case 'R':
            initGame();
            gameState = STATE_PLAYING;
            break;

        // Backend Atmosphere Controls
        case '1':
            targetDayValue = 1.0f; // Day mode
            break;
        case 'n':
        case 'N':
        case '2':
            targetDayValue = 0.0f; // Night mode
            break;
        case 't':
        case 'T':
            rainOn = !rainOn;     // Toggle Rain
            break;
        case 'l':
        case 'L':
            windDirection = 1;    // Wind Left to Right
            break;
        case 'k':
        case 'K':
            windDirection = -1;   // Wind Right to Left
            break;
        case 'o':
        case 'O':
            windDirection = 0;    // Wind Calm
            break;
        case 27:
            exit(0);
            break;
    }
}

void handleKeyUp(unsigned char key, int x, int y) {
    switch (key) {
        case 'w':
        case 'W':
            keyAccelerate = 0;
            break;
        case 's':
        case 'S':
            keyBrake = 0;
            break;
        case 'a':
        case 'A':
            keySteerLeft = 0;
            break;
        case 'd':
        case 'D':
            keySteerRight = 0;
            break;
        case ' ':
            keyJump = 0;
            break;
    }
}

void handleSpecialDown(int key, int x, int y) {
    if (key == GLUT_KEY_UP)    keyAccelerate = 1;
    if (key == GLUT_KEY_DOWN)  keyBrake = 1;
    if (key == GLUT_KEY_LEFT)  keySteerLeft = 1;
    if (key == GLUT_KEY_RIGHT) keySteerRight = 1;
}

void handleSpecialUp(int key, int x, int y) {
    if (key == GLUT_KEY_UP)    keyAccelerate = 0;
    if (key == GLUT_KEY_DOWN)  keyBrake = 0;
    if (key == GLUT_KEY_LEFT)  keySteerLeft = 0;
    if (key == GLUT_KEY_RIGHT) keySteerRight = 0;
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
}

// ==========================================
// OPENGL INITIALIZATION & MAIN
// ==========================================
void initOpenGL() {
    glClearColor(0.06f, 0.09f, 0.16f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    quadric = gluNewQuadric();
    gluQuadricNormals(quadric, GLU_SMOOTH);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(40, 30);
    glutCreateWindow("AIUB Computer Graphics - 3D BMW M-Power European Highway Racing");

    initOpenGL();
    initGame();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKeyDown);
    glutKeyboardUpFunc(handleKeyUp);
    glutSpecialFunc(handleSpecialDown);
    glutSpecialUpFunc(handleSpecialUp);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}

