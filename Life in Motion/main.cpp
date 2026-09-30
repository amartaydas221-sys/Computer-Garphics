
#include <windows.h>
#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

// ==========================================
// CONFIGURABLE SCENE & ANIMATION STATE
// ==========================================
float cloudMove = 0.0f;
float dayValue = 1.0f;
float targetDayValue = 1.0f;

int rainOn = 0;
float rainIntensity = 0.0f;

// Wind State: 1 = Left to Right, -1 = Right to Left, 0 = Calm
int windDirection = 0;
float windWave = 0.0f;

int showStartScreen = 1;

const int RAIN_COUNT = 250;
float rainX[RAIN_COUNT];
float rainY[RAIN_COUNT];
float rainSpeed[RAIN_COUNT];

char lightState = 'G';
int running = 1;

float normalSpeed = 0.008f;
float slowSpeed = 0.003f;

// Vehicle positions
float car1X = -1.00f;
float car2X = -0.40f;
float car3X = 0.80f;
float car4X = 0.20f;
float car5X = -1.30f;
float car6X = -0.85f;
float car7X = 1.05f;
float car8X = 0.55f;

// Cyclist position & pedaling angle (Positioned on Footpath)
float cyclistX = -0.60f;
float cyclistY = -0.23f; // Positioned on footpath to prevent car overlap
float cyclistSpeed = 0.004f;
float pedalAngle = 0.0f;

// Airplane position
float planeX = 1.25f;
float planeY = 0.62f;
float planeSpeed = 0.0030f;

// Bird animation state (Added as requested)
float birdX = -1.20f;
float birdY = 0.72f;
float birdSpeed = 0.0030f;
float birdWingPhase = 0.0f;
float birdWingAngle = 0.0f;

const float carWidth = 0.26f;
const float carGap = 0.10f;
const float wrapRight = 1.30f;
const float wrapLeft = -1.50f;
const float zebraLeft = -0.06f;
const float zebraRight = 0.06f;
const float rightStopX = zebraLeft - 0.03f;
const float leftStopX = zebraRight + 0.03f;

// ==========================================
// FUNCTION PROTOTYPES
// ==========================================
void drawCircle(float cx, float cy, float r);
void drawText(float x, float y, const char *text);
void drawStartScreen();
void initRain();
void drawSky();
void drawStars();
void drawSunMoon();
void drawClouds();
void drawRainClouds();
void drawRain();
void drawPlane();
void drawBuildings();
void drawGround();
void drawWindEffect();
float getWindBend();
void drawWindTree(float tx, float baseY, float scale);
void drawTrees();
void drawSingleLampPost(float postX, float baseY, float armDir);
void drawLampPosts();
void drawRoadLampPost(float postX, float baseY, int isDoubleArm);
void drawRoadLampPosts();
void drawSingleFlag(float poleX, float baseY, float poleHeight, float flagW, float flagH);
void drawFlags();
void drawRoad();
void drawTrafficLight();
void drawSingleCar(float carX, float y, float r, float g, float b, int direction);
void drawCars();
void drawCyclist();
void drawCartoonBird(float x, float y, float scale, bool faceRight, float wingAngle, bool isFlying);
void display();
void update(int value);
void updateAnimation(int value);
void handleKeypress(unsigned char key, int x, int y);
void init();
void drawZebraLane(float topY, float bottomY);
void moveRightLane(float *frontCar, float *backCar, float speed);
void moveLeftLane(float *frontCar, float *backCar, float speed);
float getGreenOrYellowSpeed();

// ==========================================
// PRIMITIVE HELPERS
// ==========================================
void drawCircle(float cx, float cy, float r) {
    glBegin(GL_POLYGON);
    for(int i = 0; i < 100; i++) {
        float theta = (i * 2.0f * 3.1415926f) / 100.0f;
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
// START SCREEN
// ==========================================
void drawStartScreen() {
    glClearColor(0.06f, 0.09f, 0.16f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.30f, 0.70f, 0.90f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.92f, -0.92f);
    glVertex2f( 0.92f, -0.92f);
    glVertex2f( 0.92f,  0.92f);
    glVertex2f(-0.92f,  0.92f);
    glEnd();
    glLineWidth(1.0f);

    glColor3f(1.0f, 0.95f, 0.80f);
    drawText(-0.32f, 0.78f, "American International University - Bangladesh");

    glColor3f(1.0f, 0.80f, 0.20f);
    drawText(-0.16f, 0.68f, "Computer Graphics");

    glColor3f(0.80f, 0.90f, 1.0f);
    drawText(-0.08f, 0.58f, "Section: U");

    glColor3f(0.20f, 0.95f, 0.85f);
    drawText(-0.28f, 0.42f, "PROJECT ON: URBAN LIFE IN MOTION");

    glColor3f(1.0f, 0.40f, 0.40f);
    drawText(-0.35f, 0.22f, "Amartay Das [ID: 23-55068-3]");
    drawText(-0.35f, 0.12f, "MD.Woasi Afzal Tonmoy [ID: 23-53233-3]");
    drawText(-0.35f, 0.02f, "Raiyan Ahnaf [ID: 23-54227-3]");
    drawText(-0.35f,-0.08f, "Sazzad Hossain Rabby [ID: 23-54859-3]");

    glColor3f(1.0f, 0.85f, 0.30f);
    drawText(-0.18f, -0.25f, "Animation Controls");

    glColor3f(0.90f, 0.92f, 0.95f);
    drawText(-0.55f, -0.35f, "Press R, Y, G : Control traffic light");
    drawText(-0.55f, -0.43f, "Press D       : Day mode");
    drawText(-0.55f, -0.51f, "Press N       : Night mode");
    drawText(-0.55f, -0.59f, "Press T       : Rain on/off");
    drawText(-0.55f, -0.67f, "Press L       : Wind left to right");
    drawText(-0.55f, -0.75f, "Press K       : Wind right to left");
    drawText(-0.55f, -0.83f, "Press O       : Stop wind");

    glColor3f(0.25f, 1.0f, 0.45f);
    drawText(-0.20f, -0.95f, "Press S to START");

    glFlush();
}

// ==========================================
// ENVIRONMENT & ATMOSPHERE
// ==========================================
void initRain() {
    for(int i = 0; i < RAIN_COUNT; i++) {
        rainX[i] = -1.0f + (rand() % 200) / 100.0f;
        rainY[i] = -1.0f + (rand() % 200) / 100.0f;
        rainSpeed[i] = 0.025f + (rand() % 100) / 5000.0f;
    }
}

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
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_QUADS);
    glColor3f(r * 0.75f, g * 0.85f, b * 1.05f > 1.0f ? 1.0f : b * 1.05f);
    glVertex2f(-1.0f, 0.20f);
    glVertex2f( 1.0f, 0.20f);
    glColor3f(r, g, b);
    glVertex2f( 1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(r + 0.08f, g + 0.08f, b + 0.08f);
    glVertex2f(-1.0f, -0.10f);
    glVertex2f( 1.0f, -0.10f);
    glColor3f(r, g, b);
    glVertex2f( 1.0f, 0.20f);
    glVertex2f(-1.0f, 0.20f);
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
    glPointSize(2.0f);
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
    float windSlant = (windDirection == 1) ? 0.03f : ((windDirection == -1) ? -0.03f : 0.005f);

    glLineWidth(1.5f);
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
// DYNAMIC WIND, TREES & FLAGS
// ==========================================
float getWindBend() {
    if(windDirection == 1) {
        return 0.022f + 0.008f * sinf(windWave);   // Bend Right
    }
    if(windDirection == -1) {
        return -0.022f - 0.008f * sinf(windWave);  // Bend Left
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

void drawWindTree(float tx, float baseY, float scale) {
    float bend = getWindBend();
    float trunkW = 0.015f * scale;
    float trunkH = 0.12f * scale;

    glColor3f(0.48f, 0.28f, 0.12f);
    glBegin(GL_QUADS);
    glVertex2f(tx - trunkW, baseY);
    glVertex2f(tx + trunkW, baseY);
    glVertex2f(tx + trunkW + bend * 0.45f, baseY + trunkH);
    glVertex2f(tx - trunkW + bend * 0.45f, baseY + trunkH);
    glEnd();

    float flutter1 = 0.0f, flutter2 = 0.0f, flutter3 = 0.0f;
    if(windDirection != 0) {
        flutter1 = windDirection * 0.005f * sinf(windWave * 1.8f);
        flutter2 = windDirection * 0.006f * sinf(windWave * 1.8f + 1.2f);
        flutter3 = windDirection * 0.005f * sinf(windWave * 1.8f + 2.4f);
    }

    glColor3f(0.04f, 0.52f, 0.12f);
    drawCircle(tx + bend * 0.70f + flutter1, baseY + (0.17f * scale), 0.05f * scale);

    glColor3f(0.08f, 0.64f, 0.16f);
    drawCircle(tx - (0.03f * scale) + bend * 0.55f + flutter2, baseY + (0.15f * scale), 0.04f * scale);

    glColor3f(0.05f, 0.58f, 0.14f);
    drawCircle(tx + (0.03f * scale) + bend * 0.85f + flutter3, baseY + (0.15f * scale), 0.04f * scale);

    glColor3f(0.15f, 0.75f, 0.22f);
    drawCircle(tx + bend * 1.05f + flutter1 * 1.2f, baseY + (0.22f * scale), 0.028f * scale);
}

void drawTrees() {
    drawWindTree(-0.90f, -0.12f, 1.0f);
    drawWindTree(-0.70f, -0.12f, 1.0f);
    drawWindTree(-0.48f, -0.12f, 1.0f);
    drawWindTree(-0.20f, -0.12f, 1.0f);
    drawWindTree( 0.16f, -0.12f, 1.0f);
    drawWindTree( 0.46f, -0.12f, 1.0f);
    drawWindTree( 0.72f, -0.12f, 1.0f);
    drawWindTree( 0.92f, -0.12f, 1.0f);
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
// BACKGROUND LAMP POSTS (FOOTPATH LEVEL)
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
        glColor3f(1.0f, 0.90f, 0.40f);
        drawCircle(lampCenterX, lampCenterY, 0.030f);

        glColor3f(1.0f, 1.0f, 0.85f);
        drawCircle(lampCenterX, lampCenterY, 0.018f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.90f, 0.30f, 0.25f);
        glVertex2f(lampCenterX, lampCenterY);
        glColor4f(1.0f, 0.85f, 0.20f, 0.02f);
        glVertex2f(lampCenterX - 0.08f, baseY);
        glVertex2f(lampCenterX + 0.08f, baseY);
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
// STREET LAMP POSTS DIRECTLY ON THE ROAD
// ==========================================
void drawRoadLampPost(float postX, float baseY, int isDoubleArm) {
    float postW = 0.014f;
    float postH = 0.22f;
    float topY = baseY + postH;

    // Metallic Steel Pole
    glColor3f(0.28f, 0.32f, 0.38f);
    glBegin(GL_QUADS);
    glVertex2f(postX - postW * 0.5f, baseY);
    glVertex2f(postX + postW * 0.5f, baseY);
    glVertex2f(postX + postW * 0.5f, topY);
    glVertex2f(postX - postW * 0.5f, topY);
    glEnd();

    // Heavy Base on Road Asphalt
    glColor3f(0.18f, 0.20f, 0.24f);
    glBegin(GL_QUADS);
    glVertex2f(postX - postW * 1.2f, baseY);
    glVertex2f(postX + postW * 1.2f, baseY);
    glVertex2f(postX + postW * 0.8f, baseY + 0.025f);
    glVertex2f(postX - postW * 0.8f, baseY + 0.025f);
    glEnd();

    // Right Arm
    float rightArmEnd = postX + 0.045f;
    glColor3f(0.28f, 0.32f, 0.38f);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    glVertex2f(postX, topY - 0.01f);
    glVertex2f(rightArmEnd, topY + 0.015f);
    glEnd();
    glLineWidth(1.0f);

    float leftArmEnd = postX - 0.045f;
    if(isDoubleArm) {
        glLineWidth(2.5f);
        glBegin(GL_LINES);
        glVertex2f(postX, topY - 0.01f);
        glVertex2f(leftArmEnd, topY + 0.015f);
        glEnd();
        glLineWidth(1.0f);
    }

    int isNightOrRain = (dayValue <= 0.4f || rainOn == 1);

    float rLampX = rightArmEnd;
    float rLampY = topY + 0.01f;

    glColor3f(0.15f, 0.15f, 0.18f);
    drawCircle(rLampX, rLampY, 0.010f);

    if(isNightOrRain) {
        glColor3f(1.0f, 0.92f, 0.45f);
        drawCircle(rLampX, rLampY, 0.016f);
        glColor3f(1.0f, 1.0f, 0.90f);
        drawCircle(rLampX, rLampY, 0.010f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.92f, 0.45f, 0.28f);
        glVertex2f(rLampX, rLampY);
        glColor4f(1.0f, 0.85f, 0.25f, 0.03f);
        glVertex2f(rLampX - 0.09f, baseY - 0.12f);
        glVertex2f(rLampX + 0.09f, baseY - 0.12f);
        glEnd();
        glDisable(GL_BLEND);
    } else {
        glColor3f(0.90f, 0.90f, 0.75f);
        drawCircle(rLampX, rLampY, 0.008f);
    }

    if(isDoubleArm) {
        float lLampX = leftArmEnd;
        float lLampY = topY + 0.01f;

        glColor3f(0.15f, 0.15f, 0.18f);
        drawCircle(lLampX, lLampY, 0.010f);

        if(isNightOrRain) {
            glColor3f(1.0f, 0.92f, 0.45f);
            drawCircle(lLampX, lLampY, 0.016f);
            glColor3f(1.0f, 1.0f, 0.90f);
            drawCircle(lLampX, lLampY, 0.010f);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBegin(GL_TRIANGLES);
            glColor4f(1.0f, 0.92f, 0.45f, 0.28f);
            glVertex2f(lLampX, lLampY);
            glColor4f(1.0f, 0.85f, 0.25f, 0.03f);
            glVertex2f(lLampX - 0.09f, baseY + 0.12f);
            glVertex2f(lLampX + 0.09f, baseY + 0.12f);
            glEnd();
            glDisable(GL_BLEND);
        } else {
            glColor3f(0.90f, 0.90f, 0.75f);
            drawCircle(lLampX, lLampY, 0.008f);
        }
    }
}

void drawRoadLampPosts() {
    // Double-arm street lights on the central road divider (-0.62f)
    drawRoadLampPost(-0.75f, -0.62f, 1);
    drawRoadLampPost(-0.35f, -0.62f, 1);
    drawRoadLampPost( 0.15f, -0.62f, 1);
    drawRoadLampPost( 0.65f, -0.62f, 1);

    // Single-arm street lights along bottom road boundary (-0.95f)
    drawRoadLampPost(-0.55f, -0.95f, 0);
    drawRoadLampPost( 0.35f, -0.95f, 0);
}

// ==========================================
// AIRPLANE
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
// BUILDINGS & CITYSCAPE (RED, GREEN, BLUE & BLACK MIXED PALETTE)
// ==========================================
void drawBuildings() {
    float winR, winG, winB;
    if (rainIntensity > 0.4f || dayValue < 0.3f) {
        winR = 1.0f; winG = 0.85f; winB = 0.25f; // Warm glowing windows
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

    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(-0.87f, -0.15f); glVertex2f(-0.83f, -0.15f); glVertex2f(-0.83f, -0.05f); glVertex2f(-0.87f, -0.05f);
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

    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(-0.57f, -0.15f); glVertex2f(-0.53f, -0.15f); glVertex2f(-0.53f, -0.05f); glVertex2f(-0.57f, -0.05f);
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
    glVertex2f(-0.36f, 0.12f); glVertex2f(-0.32f, 0.12f); glVertex2f(-0.32f, 0.17f); glVertex2f(-0.36f, 0.17f);
    glVertex2f(-0.28f, 0.12f); glVertex2f(-0.24f, 0.12f); glVertex2f(-0.24f, 0.17f); glVertex2f(-0.28f, 0.17f);
    glVertex2f(-0.20f, 0.12f); glVertex2f(-0.16f, 0.12f); glVertex2f(-0.16f, 0.17f); glVertex2f(-0.20f, 0.17f);
    glEnd();

    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(-0.30f, -0.15f); glVertex2f(-0.26f, -0.15f); glVertex2f(-0.26f, -0.05f); glVertex2f(-0.30f, -0.05f);
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
    glVertex2f(-0.11f, 0.08f); glVertex2f(-0.07f, 0.08f); glVertex2f(-0.07f, 0.13f); glVertex2f(-0.11f, 0.13f);
    glVertex2f(-0.03f, 0.08f); glVertex2f( 0.01f, 0.08f); glVertex2f( 0.01f, 0.13f); glVertex2f(-0.03f, 0.13f);
    glVertex2f( 0.05f, 0.08f); glVertex2f( 0.09f, 0.08f); glVertex2f( 0.09f, 0.13f); glVertex2f( 0.05f, 0.13f);
    glEnd();

    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(-0.02f, -0.15f); glVertex2f(0.02f, -0.15f); glVertex2f(0.02f, -0.05f); glVertex2f(-0.02f, -0.05f);
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
    glVertex2f(0.14f, 0.11f); glVertex2f(0.18f, 0.11f); glVertex2f(0.18f, 0.16f); glVertex2f(0.14f, 0.16f);
    glVertex2f(0.22f, 0.11f); glVertex2f(0.26f, 0.11f); glVertex2f(0.26f, 0.16f); glVertex2f(0.22f, 0.16f);
    glVertex2f(0.30f, 0.11f); glVertex2f(0.34f, 0.11f); glVertex2f(0.34f, 0.16f); glVertex2f(0.30f, 0.16f);
    glEnd();

    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(0.22f, -0.15f); glVertex2f(0.26f, -0.15f); glVertex2f(0.26f, -0.05f); glVertex2f(0.22f, -0.05f);
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
    glVertex2f(0.44f, 0.13f); glVertex2f(0.48f, 0.13f); glVertex2f(0.48f, 0.18f); glVertex2f(0.44f, 0.18f);
    glVertex2f(0.52f, 0.13f); glVertex2f(0.56f, 0.13f); glVertex2f(0.56f, 0.18f); glVertex2f(0.52f, 0.18f);
    glVertex2f(0.60f, 0.13f); glVertex2f(0.64f, 0.13f); glVertex2f(0.64f, 0.18f); glVertex2f(0.60f, 0.18f);
    glEnd();

    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(0.52f, -0.15f); glVertex2f(0.56f, -0.15f); glVertex2f(0.56f, -0.05f); glVertex2f(0.52f, -0.05f);
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
    glVertex2f(0.74f, 0.07f); glVertex2f(0.78f, 0.07f); glVertex2f(0.78f, 0.12f); glVertex2f(0.74f, 0.12f);
    glVertex2f(0.82f, 0.07f); glVertex2f(0.86f, 0.07f); glVertex2f(0.86f, 0.12f); glVertex2f(0.82f, 0.12f);
    glVertex2f(0.90f, 0.07f); glVertex2f(0.94f, 0.07f); glVertex2f(0.94f, 0.12f); glVertex2f(0.90f, 0.12f);
    glEnd();

    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(0.82f, -0.15f); glVertex2f(0.86f, -0.15f); glVertex2f(0.86f, -0.05f); glVertex2f(0.82f, -0.05f);
    glEnd();

    // ================= Foreground Buildings =================
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
    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(-0.90f, -0.15f); glVertex2f(-0.86f, -0.15f); glVertex2f(-0.86f, -0.05f); glVertex2f(-0.90f, -0.05f);
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
    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(-0.65f, -0.15f); glVertex2f(-0.61f, -0.15f); glVertex2f(-0.61f, -0.05f); glVertex2f(-0.65f, -0.05f);
    glEnd();

    // HOSPITAL (Crisp White with Royal Blue Roof & Red Cross)
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
    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(-0.41f, -0.15f); glVertex2f(-0.35f, -0.15f); glVertex2f(-0.35f, -0.05f); glVertex2f(-0.41f, -0.05f);
    glEnd();

    // Building 3 (Foreground - Emerald Green with Deep Red Roof)
    glColor3f(0.08f, 0.55f, 0.24f);
    glBegin(GL_QUADS);
    glVertex2f(-0.23f, -0.15f); glVertex2f(-0.03f, -0.15f);
    glVertex2f(-0.03f,  0.30f); glVertex2f(-0.23f,  0.30f);
    glEnd();

    glColor3f(0.70f, 0.12f, 0.14f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.23f, 0.30f); glVertex2f(-0.03f, 0.30f); glVertex2f(-0.13f, 0.39f);
    glEnd();

    glColor3f(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(-0.19f, 0.18f); glVertex2f(-0.15f, 0.18f); glVertex2f(-0.15f, 0.23f); glVertex2f(-0.19f, 0.23f);
    glVertex2f(-0.11f, 0.18f); glVertex2f(-0.07f, 0.18f); glVertex2f(-0.07f, 0.23f); glVertex2f(-0.11f, 0.23f);
    glVertex2f(-0.19f, 0.06f); glVertex2f(-0.15f, 0.06f); glVertex2f(-0.15f, 0.11f); glVertex2f(-0.19f, 0.11f);
    glVertex2f(-0.11f, 0.06f); glVertex2f(-0.07f, 0.06f); glVertex2f(-0.07f, 0.11f); glVertex2f(-0.11f, 0.11f);
    glEnd();
    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(-0.15f, -0.15f); glVertex2f(-0.11f, -0.15f); glVertex2f(-0.11f, -0.05f); glVertex2f(-0.15f, -0.05f);
    glEnd();

    // SCHOOL (Foreground - Deep Royal Blue with Black Roof Trim)
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
    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(0.12f, -0.15f); glVertex2f(0.17f, -0.15f); glVertex2f(0.17f, -0.05f); glVertex2f(0.12f, -0.05f);
    glEnd();

    // FIRE SERVICE STATION (Foreground - Bright Scarlet Red with Black Roof)
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

    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(0.37f, -0.15f); glVertex2f(0.49f, -0.15f); glVertex2f(0.49f, 0.05f); glVertex2f(0.37f, 0.05f);
    glEnd();

    glColor3f(winR, winG, winB);
    glBegin(GL_QUADS);
    glVertex2f(0.35f, 0.12f); glVertex2f(0.39f, 0.12f); glVertex2f(0.39f, 0.16f); glVertex2f(0.35f, 0.16f);
    glVertex2f(0.47f, 0.12f); glVertex2f(0.51f, 0.12f); glVertex2f(0.51f, 0.16f); glVertex2f(0.47f, 0.16f);
    glEnd();

    // POLICE STATION (Foreground - Deep Navy Police Blue with Emerald Roof)
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
    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(0.68f, -0.15f); glVertex2f(0.72f, -0.15f); glVertex2f(0.72f, -0.05f); glVertex2f(0.68f, -0.05f);
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
    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(0.92f, -0.15f); glVertex2f(0.96f, -0.15f); glVertex2f(0.96f, -0.05f); glVertex2f(0.92f, -0.05f);
    glEnd();
}

void drawGround() {
    glColor3f(0.08f, 0.58f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -0.10f);
    glVertex2f( 1.0f, -0.10f);
    glVertex2f( 1.0f, -1.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();
}

// ==========================================
// ROAD, VEHICLES, CYCLIST & TRAFFIC
// ==========================================
void drawZebraLane(float topY, float bottomY) {
    float y = topY;
    glBegin(GL_QUADS);
    while(y > bottomY) {
        glVertex2f(zebraLeft, y);
        glVertex2f(zebraRight, y);
        glVertex2f(zebraRight, y - 0.025f);
        glVertex2f(zebraLeft, y - 0.025f);
        y -= 0.055f;
    }
    glEnd();
}

void drawRoad() {
    glColor3f(0.65f, 0.65f, 0.68f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -0.18f);
    glVertex2f( 1.0f, -0.18f);
    glVertex2f( 1.0f, -0.28f);
    glVertex2f(-1.0f, -0.28f);
    glEnd();

    glBegin(GL_QUADS);
    for(float cx = -1.0f; cx < 1.0f; cx += 0.10f) {
        if(((int)((cx + 1.0f) * 10.0f) % 2) == 0) glColor3f(0.85f, 0.15f, 0.15f);
        else glColor3f(0.95f, 0.95f, 0.95f);

        glVertex2f(cx, -0.27f);
        glVertex2f(cx + 0.10f, -0.27f);
        glVertex2f(cx + 0.10f, -0.28f);
        glVertex2f(cx, -0.28f);
    }
    glEnd();

    glColor3f(0.16f, 0.17f, 0.20f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -0.28f);
    glVertex2f( 1.0f, -0.28f);
    glVertex2f( 1.0f, -0.95f);
    glVertex2f(-1.0f, -0.95f);
    glEnd();

    glColor3f(0.95f, 0.95f, 0.95f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(-1.0f, -0.28f); glVertex2f( 1.0f, -0.28f);
    glVertex2f(-1.0f, -0.95f); glVertex2f( 1.0f, -0.95f);
    glEnd();
    glLineWidth(1.0f);

    glColor3f(0.98f, 0.85f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -0.61f);
    glVertex2f( 1.0f, -0.61f);
    glVertex2f( 1.0f, -0.63f);
    glVertex2f(-1.0f, -0.63f);
    glEnd();

    glColor3f(0.95f, 0.95f, 0.90f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    for(float x = -0.95f; x < 0.95f; x += 0.20f) {
        glVertex2f(x, -0.45f); glVertex2f(x + 0.10f, -0.45f);
        glVertex2f(x, -0.79f); glVertex2f(x + 0.10f, -0.79f);
    }
    glEnd();
    glLineWidth(1.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawZebraLane(-0.31f, -0.45f);
    drawZebraLane(-0.48f, -0.60f);
    drawZebraLane(-0.66f, -0.79f);
    drawZebraLane(-0.82f, -0.94f);
}

void drawTrafficLight() {
    glColor3f(0.20f, 0.20f, 0.22f);
    glBegin(GL_QUADS);
    glVertex2f(-0.01f, -0.28f);
    glVertex2f( 0.01f, -0.28f);
    glVertex2f( 0.01f,  0.12f);
    glVertex2f(-0.01f,  0.12f);
    glEnd();

    glColor3f(0.10f, 0.10f, 0.12f);
    glBegin(GL_QUADS);
    glVertex2f(-0.04f, 0.00f);
    glVertex2f( 0.04f, 0.00f);
    glVertex2f( 0.04f, 0.16f);
    glVertex2f(-0.04f, 0.16f);
    glEnd();

    if(lightState == 'R') glColor3f(1.0f, 0.0f, 0.0f);
    else glColor3f(0.35f, 0.0f, 0.0f);
    drawCircle(0.00f, 0.12f, 0.015f);

    if(lightState == 'Y') glColor3f(1.0f, 0.95f, 0.0f);
    else glColor3f(0.35f, 0.35f, 0.0f);
    drawCircle(0.00f, 0.08f, 0.015f);

    if(lightState == 'G') glColor3f(0.0f, 1.0f, 0.30f);
    else glColor3f(0.0f, 0.35f, 0.0f);
    drawCircle(0.00f, 0.04f, 0.015f);
}

void drawSingleCar(float carX, float y, float r, float g, float b, int direction) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(carX + 0.00f, y);
    glVertex2f(carX + 0.26f, y);
    glVertex2f(carX + 0.26f, y + 0.08f);
    glVertex2f(carX + 0.00f, y + 0.08f);
    glEnd();

    glColor3f(r * 0.88f, g * 0.88f, b * 0.88f);
    glBegin(GL_POLYGON);
    glVertex2f(carX + 0.04f, y + 0.08f);
    glVertex2f(carX + 0.22f, y + 0.08f);
    glVertex2f(carX + 0.19f, y + 0.15f);
    glVertex2f(carX + 0.07f, y + 0.15f);
    glEnd();

    glColor3f(0.70f, 0.90f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(carX + 0.07f, y + 0.09f); glVertex2f(carX + 0.12f, y + 0.09f);
    glVertex2f(carX + 0.11f, y + 0.14f); glVertex2f(carX + 0.08f, y + 0.14f);
    glVertex2f(carX + 0.14f, y + 0.09f); glVertex2f(carX + 0.20f, y + 0.09f);
    glVertex2f(carX + 0.18f, y + 0.14f); glVertex2f(carX + 0.13f, y + 0.14f);
    glEnd();

    int isNightOrRain = (dayValue <= 0.4f || rainOn == 1);
    glBegin(GL_QUADS);
    if(direction == 1) {
        glColor3f(1.0f, 1.0f, 0.70f);
        glVertex2f(carX + 0.24f, y + 0.02f); glVertex2f(carX + 0.26f, y + 0.02f);
        glVertex2f(carX + 0.26f, y + 0.06f); glVertex2f(carX + 0.24f, y + 0.06f);
        glColor3f(0.95f, 0.10f, 0.10f);
        glVertex2f(carX + 0.00f, y + 0.02f); glVertex2f(carX + 0.02f, y + 0.02f);
        glVertex2f(carX + 0.02f, y + 0.06f); glVertex2f(carX + 0.00f, y + 0.06f);
    } else {
        glColor3f(1.0f, 1.0f, 0.70f);
        glVertex2f(carX + 0.00f, y + 0.02f); glVertex2f(carX + 0.02f, y + 0.02f);
        glVertex2f(carX + 0.02f, y + 0.06f); glVertex2f(carX + 0.00f, y + 0.06f);
        glColor3f(0.95f, 0.10f, 0.10f);
        glVertex2f(carX + 0.24f, y + 0.02f); glVertex2f(carX + 0.26f, y + 0.02f);
        glVertex2f(carX + 0.26f, y + 0.06f); glVertex2f(carX + 0.24f, y + 0.06f);
    }
    glEnd();

    if(isNightOrRain) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_TRIANGLES);
        if(direction == 1) {
            glColor4f(1.0f, 1.0f, 0.70f, 0.35f);
            glVertex2f(carX + 0.26f, y + 0.04f);
            glColor4f(1.0f, 1.0f, 0.60f, 0.0f);
            glVertex2f(carX + 0.42f, y + 0.08f);
            glVertex2f(carX + 0.42f, y - 0.02f);
        } else {
            glColor4f(1.0f, 1.0f, 0.70f, 0.35f);
            glVertex2f(carX, y + 0.04f);
            glColor4f(1.0f, 1.0f, 0.60f, 0.0f);
            glVertex2f(carX - 0.16f, y + 0.08f);
            glVertex2f(carX - 0.16f, y - 0.02f);
        }
        glEnd();
        glDisable(GL_BLEND);
    }

    glColor3f(0.08f, 0.08f, 0.08f);
    drawCircle(carX + 0.06f, y, 0.030f);
    drawCircle(carX + 0.20f, y, 0.030f);

    glColor3f(0.75f, 0.78f, 0.82f);
    drawCircle(carX + 0.06f, y, 0.014f);
    drawCircle(carX + 0.20f, y, 0.014f);
}

void drawCars() {
    drawSingleCar(car1X, -0.42f, 0.12f, 0.55f, 0.95f,  1);
    drawSingleCar(car2X, -0.57f, 0.98f, 0.45f, 0.08f,  1);
    drawSingleCar(car3X, -0.77f, 0.10f, 0.78f, 0.35f, -1);
    drawSingleCar(car4X, -0.91f, 0.68f, 0.18f, 0.88f, -1);
    drawSingleCar(car5X, -0.42f, 0.92f, 0.12f, 0.15f,  1);
    drawSingleCar(car6X, -0.57f, 0.10f, 0.82f, 0.88f,  1);
    drawSingleCar(car7X, -0.77f, 0.98f, 0.85f, 0.08f, -1);
    drawSingleCar(car8X, -0.91f, 0.90f, 0.92f, 0.95f, -1);
}

// ==========================================
// ANIMATED CYCLIST
// ==========================================
void drawCyclist() {
    glPushMatrix();
    glTranslatef(cyclistX, cyclistY, 0.0f);
    glScalef(1.0f, 1.0f, 1.0f);

    float wheelR = 0.024f;
    float rearHubX = -0.045f;
    float frontHubX = 0.045f;
    float hubY = 0.024f;

    float crankX = 0.00f;
    float crankY = hubY;
    float crankR = 0.010f;
    float hipX = -0.020f;
    float hipY = 0.075f;

    float farFootX = crankX - crankR * cosf(pedalAngle);
    float farFootY = crankY - crankR * sinf(pedalAngle);
    float farKneeX = hipX + 0.022f - 0.008f * cosf(pedalAngle);
    float farKneeY = hipY - 0.022f - 0.010f * sinf(pedalAngle);

    glColor3f(0.12f, 0.20f, 0.38f);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    glVertex2f(hipX, hipY);
    glVertex2f(farKneeX, farKneeY);
    glVertex2f(farKneeX, farKneeY);
    glVertex2f(farFootX, farFootY);
    glEnd();

    glColor3f(0.15f, 0.15f, 0.15f);
    drawCircle(rearHubX, hubY, wheelR);
    glColor3f(0.75f, 0.78f, 0.82f);
    drawCircle(rearHubX, hubY, wheelR - 0.004f);

    glColor3f(0.15f, 0.15f, 0.15f);
    drawCircle(frontHubX, hubY, wheelR);
    glColor3f(0.75f, 0.78f, 0.82f);
    drawCircle(frontHubX, hubY, wheelR - 0.004f);

    glColor3f(0.40f, 0.40f, 0.45f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for(int i = 0; i < 4; i++) {
        float angle = pedalAngle * 2.0f + (i * 3.14159f / 4.0f);
        glVertex2f(rearHubX - (wheelR - 0.004f) * cosf(angle), hubY - (wheelR - 0.004f) * sinf(angle));
        glVertex2f(rearHubX + (wheelR - 0.004f) * cosf(angle), hubY + (wheelR - 0.004f) * sinf(angle));
        glVertex2f(frontHubX - (wheelR - 0.004f) * cosf(angle), hubY - (wheelR - 0.004f) * sinf(angle));
        glVertex2f(frontHubX + (wheelR - 0.004f) * cosf(angle), hubY + (wheelR - 0.004f) * sinf(angle));
    }
    glEnd();

    glColor3f(0.2f, 0.2f, 0.2f);
    drawCircle(rearHubX, hubY, 0.005f);
    drawCircle(frontHubX, hubY, 0.005f);

    float seatPostX = -0.015f;
    float seatPostY = 0.068f;
    float headTubeX = 0.032f;
    float headTubeY = 0.068f;
    float handleX = 0.035f;
    float handleY = 0.082f;

    glColor3f(0.20f, 0.85f, 0.25f);
    glLineWidth(2.2f);
    glBegin(GL_LINES);
    glVertex2f(rearHubX, hubY); glVertex2f(seatPostX, seatPostY);
    glVertex2f(seatPostX, seatPostY); glVertex2f(crankX, crankY);
    glVertex2f(crankX, crankY); glVertex2f(rearHubX, hubY);
    glVertex2f(crankX, crankY); glVertex2f(headTubeX, headTubeY);
    glVertex2f(seatPostX, seatPostY); glVertex2f(headTubeX, headTubeY);
    glVertex2f(headTubeX, headTubeY); glVertex2f(frontHubX, hubY);
    glVertex2f(headTubeX, headTubeY); glVertex2f(handleX, handleY);
    glEnd();

    glColor3f(0.1f, 0.1f, 0.1f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(handleX - 0.006f, handleY + 0.003f);
    glVertex2f(handleX + 0.006f, handleY - 0.002f);
    glEnd();

    glColor3f(0.15f, 0.15f, 0.15f);
    glBegin(GL_QUADS);
    glVertex2f(seatPostX - 0.014f, seatPostY + 0.004f);
    glVertex2f(seatPostX + 0.008f, seatPostY + 0.004f);
    glVertex2f(seatPostX + 0.008f, seatPostY);
    glVertex2f(seatPostX - 0.014f, seatPostY);
    glEnd();

    glColor3f(0.3f, 0.3f, 0.3f);
    drawCircle(crankX, crankY, 0.006f);

    float shoulderX = 0.012f;
    float shoulderY = 0.102f;

    glColor3f(0.00f, 0.80f, 0.95f);
    glLineWidth(4.5f);
    glBegin(GL_LINES);
    glVertex2f(hipX, hipY);
    glVertex2f(shoulderX, shoulderY);
    glEnd();

    glColor3f(0.00f, 0.70f, 0.88f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(shoulderX, shoulderY);
    glVertex2f(handleX, handleY);
    glEnd();

    float headCenterX = 0.020f;
    float headCenterY = 0.120f;
    glColor3f(0.98f, 0.80f, 0.65f);
    drawCircle(headCenterX, headCenterY, 0.011f);

    glColor3f(0.95f, 0.15f, 0.20f);
    drawCircle(headCenterX - 0.002f, headCenterY + 0.004f, 0.012f);
    glColor3f(0.75f, 0.08f, 0.12f);
    glBegin(GL_TRIANGLES);
    glVertex2f(headCenterX + 0.005f, headCenterY + 0.008f);
    glVertex2f(headCenterX + 0.018f, headCenterY + 0.003f);
    glVertex2f(headCenterX + 0.005f, headCenterY);
    glEnd();

    float nearFootX = crankX + crankR * cosf(pedalAngle);
    float nearFootY = crankY + crankR * sinf(pedalAngle);
    float nearKneeX = hipX + 0.022f + 0.008f * cosf(pedalAngle);
    float nearKneeY = hipY - 0.022f + 0.010f * sinf(pedalAngle);

    glColor3f(0.20f, 0.35f, 0.65f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(hipX, hipY);
    glVertex2f(nearKneeX, nearKneeY);
    glVertex2f(nearKneeX, nearKneeY);
    glVertex2f(nearFootX, nearFootY);
    glEnd();

    glColor3f(0.98f, 0.98f, 0.98f);
    drawCircle(nearFootX, nearFootY, 0.004f);

    glLineWidth(1.0f);
    glPopMatrix();
}

// ==========================================
// CARTOON BIRD
// ==========================================
void drawCartoonBird(float x, float y, float scale, bool faceRight, float wingAngle, bool isFlying)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    if (!faceRight) {
        glScalef(-scale, scale, 1.0f);
    } else {
        glScalef(scale, scale, 1.0f);
    }
    // Legs & Feet
    glColor3f(0.85f, 0.55f, 0.15f);
    glLineWidth(2.2f);
    glBegin(GL_LINES);
    if (!isFlying) {
        glVertex2f(-0.005f, 0.000f);
        glVertex2f(-0.008f, 0.035f);
        glVertex2f(0.012f, 0.000f);
        glVertex2f(0.008f, 0.035f);
        glVertex2f(-0.018f, 0.000f);
        glVertex2f(0.002f, 0.000f);
        glVertex2f(0.000f, 0.000f);
        glVertex2f(0.020f, 0.000f);
    } else {
        glVertex2f(-0.010f, 0.025f);
        glVertex2f(-0.025f, 0.035f);
        glVertex2f(0.000f, 0.025f);
        glVertex2f(-0.015f, 0.035f);
    }
    glEnd();
    // Tail
    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_POLYGON);
    glVertex2f(-0.015f, 0.035f);
    glVertex2f(-0.090f, 0.010f);
    glVertex2f(-0.080f, 0.025f);
    glVertex2f(-0.065f, 0.038f);
    glVertex2f(-0.095f, 0.030f);
    glVertex2f(-0.010f, 0.065f);
    glEnd();
    // Body
    glColor3f(0.12f, 0.12f, 0.14f);
    glBegin(GL_POLYGON);
    glVertex2f(-0.035f, 0.035f);
    glVertex2f(-0.030f, 0.075f);
    glVertex2f(0.000f, 0.095f);
    glVertex2f(0.035f, 0.085f);
    glVertex2f(0.045f, 0.045f);
    glVertex2f(0.020f, 0.025f);
    glVertex2f(-0.015f, 0.025f);
    glEnd();
    // Chest Patch
    glColor3f(0.20f, 0.20f, 0.24f);
    glBegin(GL_POLYGON);
    glVertex2f(0.010f, 0.030f);
    glVertex2f(0.042f, 0.050f);
    glVertex2f(0.032f, 0.080f);
    glVertex2f(0.010f, 0.075f);
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
    for (int i = 0; i < 30; i++)
    {
        float angle = (i * 2.0f * 3.1416f) / 30.0f;
        glVertex2f(0.028f + 0.026f * cos(angle), 0.095f + 0.026f * sin(angle));
    }
    glEnd();
    // Crest Feathers
    glColor3f(0.08f, 0.08f, 0.10f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.018f, 0.102f);
    glVertex2f(-0.025f, 0.128f);
    glVertex2f(0.012f, 0.118f);
    glVertex2f(0.012f, 0.115f);
    glVertex2f(-0.028f, 0.110f);
    glVertex2f(0.018f, 0.095f);
    glEnd();
    // Eye
    glColor3f(0.95f, 0.95f, 0.90f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 20; i++)
    {
        float angle = (i * 2.0f * 3.1416f) / 20.0f;
        glVertex2f(0.036f + 0.013f * cos(angle), 0.100f + 0.013f * sin(angle));
    }
    glEnd();
    glColor3f(0.05f, 0.05f, 0.05f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 20; i++)
    {
        float angle = (i * 2.0f * 3.1416f) / 20.0f;
        glVertex2f(0.037f + 0.007f * cos(angle), 0.100f + 0.007f * sin(angle));
    }
    glEnd();
    // Beak
    glColor3f(0.95f, 0.60f, 0.10f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.045f, 0.110f);
    glVertex2f(0.082f, 0.092f);
    glVertex2f(0.045f, 0.082f);
    glEnd();
    glPopMatrix();
}

float getGreenOrYellowSpeed() {
    if(lightState == 'G') return normalSpeed;
    if(lightState == 'Y') return slowSpeed;
    return normalSpeed;
}

void moveRightLane(float *frontCar, float *backCar, float speed) {
    float first = *frontCar;
    float second = *backCar;

    if(second > first) {
        float temp = first;
        first = second;
        second = temp;
    }

    if(lightState == 'R') {
        if(first + carWidth > rightStopX) {
            first += speed;
        } else {
            float stopX = rightStopX - carWidth;
            first += speed;
            if(first > stopX) first = stopX;
        }

        if(second + carWidth > rightStopX) {
            second += speed;
        } else {
            float stopX = first - carWidth - carGap;
            second += speed;
            if(second > stopX) second = stopX;
        }
    } else {
        first += speed;
        second += speed;

        if(second + carWidth + carGap > first && second < first) {
            second = first - carWidth - carGap;
        }
    }

    if(first > wrapRight) first = wrapLeft;
    if(second > wrapRight) second = wrapLeft - carWidth - carGap;

    *frontCar = first;
    *backCar = second;
}

void moveLeftLane(float *frontCar, float *backCar, float speed) {
    float first = *frontCar;
    float second = *backCar;

    if(second < first) {
        float temp = first;
        first = second;
        second = temp;
    }

    if(lightState == 'R') {
        if(first < leftStopX) {
            first -= speed;
        } else {
            float stopX = leftStopX;
            first -= speed;
            if(first < stopX) first = stopX;
        }

        if(second < leftStopX) {
            second -= speed;
        } else {
            float stopX = first + carWidth + carGap;
            second -= speed;
            if(second < stopX) second = stopX;
        }
    } else {
        first -= speed;
        second -= speed;

        if(first + carWidth + carGap > second && first < second) {
            second = first + carWidth + carGap;
        }
    }

    if(first < wrapLeft) first = wrapRight;
    if(second < wrapLeft) second = wrapRight + carWidth + carGap;

    *frontCar = first;
    *backCar = second;
}

// ==========================================
// RENDER & UPDATE LOOPS
// ==========================================
void display() {
    if(showStartScreen == 1) {
        drawStartScreen();
        return;
    }

    drawSky();
    drawStars();
    drawSunMoon();
    drawClouds();
    drawRainClouds();

    // ONLY SHOW BIRDS DURING THE DAY (Added as requested)
    if(dayValue > 0.4f) {
        drawCartoonBird(birdX,         birdY,         0.45f, true, birdWingAngle, true);
        drawCartoonBird(birdX - 0.18f, birdY + 0.10f, 0.35f, true, -birdWingAngle * 0.85f, true);
        drawCartoonBird(birdX - 0.34f, birdY - 0.07f, 0.28f, true, birdWingAngle * 0.70f, true);
    }

    drawBuildings();
    drawGround();
    drawTrees();
    drawLampPosts();      // Background footpath lamp posts
    drawFlags();
    drawPlane();
    drawRoad();
    drawCars();           // Vehicles rendered BEFORE road street lamp posts!
    drawRoadLampPosts();  // Street lamp posts stand IN FRONT of cars on the road median!
    drawTrafficLight();
    drawCyclist();
    drawRain();
    drawWindEffect();

    glFlush();
}

void update(int value) {
    if(showStartScreen == 1) {
        glutPostRedisplay();
        glutTimerFunc(20, update, 0);
        return;
    }

    if(windDirection == 1) {
        cloudMove += 0.004f;
    } else if(windDirection == -1) {
        cloudMove -= 0.004f;
    } else {
        cloudMove += 0.001f;
    }

    if(cloudMove > 1.80f) cloudMove = -1.80f;
    if(cloudMove < -1.80f) cloudMove = 1.80f;

    if(rainOn == 1) {
        if(rainIntensity < 1.0f) rainIntensity += 0.01f;
    } else {
        if(rainIntensity > 0.0f) rainIntensity -= 0.01f;
    }

    for(int i = 0; i < RAIN_COUNT; i++) {
        rainY[i] -= rainSpeed[i];
        rainX[i] += (windDirection == 1) ? 0.003f : ((windDirection == -1) ? -0.003f : 0.001f);

        if(rainY[i] < -1.0f || rainX[i] > 1.15f || rainX[i] < -1.15f) {
            rainY[i] = 1.05f;
            rainX[i] = -1.0f + (rand() % 200) / 100.0f;
        }
    }

    if(dayValue < targetDayValue) {
        dayValue += 0.005f;
        if(dayValue > targetDayValue) dayValue = targetDayValue;
    }
    if(dayValue > targetDayValue) {
        dayValue -= 0.005f;
        if(dayValue < targetDayValue) dayValue = targetDayValue;
    }

    if(running == 1) {
        float speed = getGreenOrYellowSpeed();
        moveRightLane(&car1X, &car5X, speed);
        moveRightLane(&car2X, &car6X, speed);
        moveLeftLane(&car3X, &car7X, speed);
        moveLeftLane(&car4X, &car8X, speed);

        int isCyclistMoving = 1;
        if(lightState == 'R') {
            float stopLine = rightStopX - 0.08f;
            if(cyclistX < stopLine) {
                cyclistX -= cyclistSpeed;
                if(cyclistX > stopLine) cyclistX = stopLine;
                if(cyclistX == stopLine) isCyclistMoving = 0;
            } else if(cyclistX > rightStopX) {
                cyclistX += cyclistSpeed;
            }
        } else {
            float currentSpeed = (lightState == 'Y') ? cyclistSpeed * 0.6f : cyclistSpeed;
            cyclistX += currentSpeed;
        }

        if(cyclistX > wrapRight) {
            cyclistX = wrapLeft;
        }

        if(isCyclistMoving) {
            pedalAngle -= 0.18f;
            if(pedalAngle < 0.0f) pedalAngle += 6.283185f;
        }
    }

    if(windDirection != 0) {
        windWave += 0.15f;
        if(windWave > 6.283185f) windWave -= 6.283185f;
    }

    glutPostRedisplay();
    glutTimerFunc(20, update, 0);
}

void updateAnimation(int value) {
    planeX -= planeSpeed;
    if (planeX <-1.25f) planeX = +1.25f;

    // BIRD ANIMATION LOGIC (Added as requested)
    birdX += birdSpeed;
    if (birdX > 1.35f) birdX = -1.35f;

    birdWingPhase += 0.22f; // Flap wings
    if (birdWingPhase > 6.283185f) birdWingPhase -= 6.283185f;
    birdWingAngle = 28.0f * sinf(birdWingPhase);
    birdY = 0.72f + 0.018f * sinf(birdWingPhase * 0.45f); // Natural bouncing motion

    glutPostRedisplay();
    glutTimerFunc(16, updateAnimation, 0);
}

void handleKeypress(unsigned char key, int x, int y) {
    if(showStartScreen == 1) {
        if(key == 's' || key == 'S') {
            showStartScreen = 0;
        }
        glutPostRedisplay();
        return;
    }

    switch(key) {
        case 'r':
        case 'R':
            lightState = 'R';
            break;
        case 'g':
        case 'G':
            lightState = 'G';
            break;
        case 'y':
        case 'Y':
            lightState = 'Y';
            break;
        case 13: // Enter key
            running = !running;
            break;
        case 'd':
        case 'D':
            targetDayValue = 1.0f;
            break;
        case 'n':
        case 'N':
            targetDayValue = 0.0f;
            break;
        case 't':
        case 'T':
            rainOn = !rainOn;
            break;
        case 'l':
        case 'L':
            windDirection = 1;  // Wind left to right
            break;
        case 'k':
        case 'K':
            windDirection = -1; // Wind right to left
            break;
        case 'o':
        case 'O':
            windDirection = 0;  // Wind calm/stop
            break;
    }

    glutPostRedisplay();
}

void init() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(40, 30);
    glutCreateWindow("Urban Life in Motion");

    init();
    initRain();

    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeypress);
    glutTimerFunc(20, update, 0);
    glutTimerFunc(16, updateAnimation, 0);
    glutMainLoop();

    return 0;
}

