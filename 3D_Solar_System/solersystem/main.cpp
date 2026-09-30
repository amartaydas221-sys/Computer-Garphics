#include <windows.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// ==========================================
// CONFIGURABLE SCENE & ANIMATION STATE
// ==========================================
int showStartScreen = 1;
int running = 1;
int showOrbits = 1;
int showHUD = 1;
int windowWidth = 1280;
int windowHeight = 720;

// Simulation time & time warp speed
float timeScale = 1.0f;
float simTime = 0.0f;
float sunGlowPhase = 0.0f;

// Camera state & smooth tracking
float camDist = 140.0f;
float targetCamDist = 140.0f;
float camYaw = 40.0f;
float camPitch = 25.0f;

float camLookX = 0.0f, camLookY = 0.0f, camLookZ = 0.0f;
float targetLookX = 0.0f, targetLookY = 0.0f, targetLookZ = 0.0f;

int mouseLeftDown = 0;
int mouseRightDown = 0;
int lastMouseX = 0;
int lastMouseY = 0;

// Focused celestial body: 0 = Overview, 1 = Sun, 2 = Mercury ... 9 = Neptune, 10 = Moon
int focusTarget = 0;

// ==========================================
// CELESTIAL BODY DATA STRUCTURE
// ==========================================
typedef struct {
    const char* name;
    float radius;
    float distance;
    float orbitSpeed;      // Relative orbital revolution speed
    float rotationSpeed;   // Axial rotation speed
    float axialTilt;       // Axial tilt in degrees
    float orbitAngle;      // Current orbital angle in degrees
    float rotationAngle;   // Current day/night spin angle
    float color[3];        // Base planet color
    float posX, posY, posZ;// Current 3D position in space
    const char* description;
    const char* orbitalPeriod;
    const char* dayLength;
} Planet;

#define PLANET_COUNT 9

// Physically proportional ratios adapted for visual clarity
Planet solarBodies[PLANET_COUNT] = {
    // Sun
    { "Sun",     14.0f,   0.0f,  0.0f,    0.4f,   7.25f, 0.0f, 0.0f, { 1.00f, 0.75f, 0.15f }, 0.0f,0.0f,0.0f, "G-type Main-Sequence Star", "N/A", "25 - 35 Earth Days" },
    // Mercury
    { "Mercury",  1.8f,  24.0f,  4.15f,   0.6f,   0.03f, 0.0f, 0.0f, { 0.72f, 0.68f, 0.65f }, 0.0f,0.0f,0.0f, "Smallest and innermost planet", "88 Earth Days", "58.6 Earth Days" },
    // Venus
    { "Venus",    3.4f,  38.0f,  1.62f,  -0.3f, 177.30f, 0.0f, 0.0f, { 0.92f, 0.78f, 0.45f }, 0.0f,0.0f,0.0f, "Dense sulfuric acid atmosphere", "225 Earth Days", "243 Earth Days (Retrograde)" },
    // Earth
    { "Earth",    3.6f,  54.0f,  1.00f,   1.8f,  23.44f, 0.0f, 0.0f, { 0.20f, 0.55f, 0.95f }, 0.0f,0.0f,0.0f, "Home planet with liquid water", "365.25 Days", "24 Hours" },
    // Mars
    { "Mars",     2.4f,  70.0f,  0.53f,   1.7f,  25.19f, 0.0f, 0.0f, { 0.88f, 0.35f, 0.18f }, 0.0f,0.0f,0.0f, "Red planet with rusty iron oxide", "687 Earth Days", "24.6 Hours" },
    // Jupiter
    { "Jupiter",  8.2f,  96.0f,  0.28f,   3.2f,   3.13f, 0.0f, 0.0f, { 0.85f, 0.65f, 0.45f }, 0.0f,0.0f,0.0f, "Gas giant with Great Red Spot storm", "11.86 Earth Years", "9.9 Hours" },
    // Saturn
    { "Saturn",   7.0f, 126.0f,  0.18f,   2.9f,  26.73f, 0.0f, 0.0f, { 0.92f, 0.82f, 0.55f }, 0.0f,0.0f,0.0f, "Adorned with iconic multi-banded rings", "29.45 Earth Years", "10.7 Hours" },
    // Uranus
    { "Uranus",   4.6f, 154.0f,  0.11f,  -1.5f,  97.77f, 0.0f, 0.0f, { 0.45f, 0.85f, 0.88f }, 0.0f,0.0f,0.0f, "Ice giant tilted completely on its side", "84 Earth Years", "17.2 Hours (Retrograde)" },
    // Neptune
    { "Neptune",  4.4f, 180.0f,  0.08f,   1.6f,  28.32f, 0.0f, 0.0f, { 0.15f, 0.35f, 0.95f }, 0.0f,0.0f,0.0f, "Vivid azure blue with supersonic winds", "164.8 Earth Years", "16.1 Hours" }
};

// Earth's Moon
float moonOrbitAngle = 0.0f;
float moonDistance = 7.0f;
float moonRadius = 0.95f;
float moonPosX = 0.0f, moonPosY = 0.0f, moonPosZ = 0.0f;

// Starfield background
#define STAR_COUNT 1500
typedef struct {
    float x, y, z;
    float r, g, b;
    float size;
    float twinkleSpeed;
    float phase;
} Star;

Star stars[STAR_COUNT];
GLUquadric* quadricObj = NULL;

// ==========================================
// FUNCTION PROTOTYPES
// ==========================================
void initStars();
void initOpenGL();
void drawStartScreen();
void drawText(float x, float y, const char* text);
void drawTextSmall(float x, float y, const char* text);
void drawCircle2D(float cx, float cy, float r);
void drawStarfield();
void drawOrbitLine(float distance);
void drawSun();
void drawSaturnRings(float innerR, float outerR);
void drawEarthAtmosphere(float radius);
void drawPlanetFeatures(int index, float radius);
void drawCelestialBody(int index);
void drawHUDOverlay();
void update(int value);
void display();
void reshape(int w, int h);
void handleKeypress(unsigned char key, int x, int y);
void handleSpecialKeys(int key, int x, int y);
void handleMouseClick(int button, int state, int x, int y);
void handleMouseMotion(int x, int y);

// ==========================================
// STARFIELD INITIALIZATION
// ==========================================
void initStars() {
    srand(4242);
    for (int i = 0; i < STAR_COUNT; i++) {
        float u = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        float theta = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
        float r = sqrtf(1.0f - u * u);
        float dist = 480.0f + ((float)rand() / RAND_MAX) * 80.0f;

        stars[i].x = r * cosf(theta) * dist;
        stars[i].y = u * dist;
        stars[i].z = r * sinf(theta) * dist;

        float cType = (float)rand() / RAND_MAX;
        if (cType < 0.25f) {
            stars[i].r = 0.70f; stars[i].g = 0.85f; stars[i].b = 1.00f; // Blue-white
        } else if (cType < 0.65f) {
            stars[i].r = 0.98f; stars[i].g = 0.98f; stars[i].b = 1.00f; // Pure white
        } else if (cType < 0.85f) {
            stars[i].r = 1.00f; stars[i].g = 0.92f; stars[i].b = 0.75f; // Solar yellow
        } else {
            stars[i].r = 1.00f; stars[i].g = 0.55f; stars[i].b = 0.40f; // Red giant
        }

        stars[i].size = 1.0f + ((float)rand() / RAND_MAX) * 2.2f;
        stars[i].twinkleSpeed = 0.5f + ((float)rand() / RAND_MAX) * 2.5f;
        stars[i].phase = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
    }
}

// ==========================================
// 2D PRIMITIVES & START SCREEN
// ==========================================
void drawCircle2D(float cx, float cy, float r) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < 40; i++) {
        float theta = (i * 2.0f * M_PI) / 40.0f;
        glVertex2f(cx + r * cosf(theta), cy + r * sinf(theta));
    }
    glEnd();
}

void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

void drawTextSmall(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, text[i]);
    }
}

void drawStartScreen() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glClearColor(0.04f, 0.05f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Glowing border frame
    glColor3f(0.20f, 0.65f, 0.95f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.92f, -0.92f);
    glVertex2f( 0.92f, -0.92f);
    glVertex2f( 0.92f,  0.92f);
    glVertex2f(-0.92f,  0.92f);
    glEnd();
    glLineWidth(1.0f);

    // Titles
    glColor3f(1.0f, 0.92f, 0.60f);
    drawText(-0.35f, 0.78f, "COMPUTER GRAPHICS & SIMULATION");

    glColor3f(0.30f, 0.85f, 1.0f);
    drawText(-0.42f, 0.68f, "REALISTIC 3D SOLAR SYSTEM ANIMATION");

    glColor3f(0.85f, 0.90f, 0.95f);
    drawText(-0.16f, 0.58f, "Section: U | Project");

    // Planetary graphic preview
    glColor3f(1.00f, 0.70f, 0.10f); drawCircle2D(-0.45f, 0.38f, 0.075f); // Sun
    glColor3f(0.70f, 0.70f, 0.70f); drawCircle2D(-0.32f, 0.38f, 0.018f); // Mercury
    glColor3f(0.90f, 0.78f, 0.40f); drawCircle2D(-0.25f, 0.38f, 0.026f); // Venus
    glColor3f(0.20f, 0.60f, 0.95f); drawCircle2D(-0.16f, 0.38f, 0.030f); // Earth
    glColor3f(0.90f, 0.35f, 0.20f); drawCircle2D(-0.07f, 0.38f, 0.022f); // Mars
    glColor3f(0.85f, 0.65f, 0.45f); drawCircle2D( 0.06f, 0.38f, 0.052f); // Jupiter
    glColor3f(0.92f, 0.82f, 0.55f); drawCircle2D( 0.22f, 0.38f, 0.044f); // Saturn
    glColor3f(0.45f, 0.85f, 0.88f); drawCircle2D( 0.36f, 0.38f, 0.032f); // Uranus
    glColor3f(0.18f, 0.40f, 0.95f); drawCircle2D( 0.46f, 0.38f, 0.030f); // Neptune

    // Features
    glColor3f(1.0f, 0.40f, 0.40f);
    drawText(-0.38f, 0.18f, "Solar System Physics & Rendering Engine");
    glColor3f(0.85f, 0.88f, 0.92f);
    drawText(-0.38f, 0.08f, "- Physically proportional orbital periods & Keplerian rates");
    drawText(-0.38f,-0.00f, "- True axial tilts & day/night rotation cycles");
    drawText(-0.38f,-0.08f, "- Saturn's translucent multi-banded rings & Cassini division");
    drawText(-0.38f,-0.16f, "- Dynamic omnidirectional solar lighting & starfield");

    // Controls manual
    glColor3f(1.0f, 0.85f, 0.30f);
    drawText(-0.20f, -0.30f, "INTERACTION CONTROLS");

    glColor3f(0.90f, 0.92f, 0.95f);
    drawText(-0.55f, -0.40f, "Mouse Left Drag : Orbit & rotate 3D camera view");
    drawText(-0.55f, -0.48f, "Mouse Right Drag: Zoom camera distance in / out");
    drawText(-0.55f, -0.56f, "Press 0 - 9     : Focus & track Sun, Mercury, Earth, Saturn...");
    drawText(-0.55f, -0.64f, "Press Spacebar  : Pause / Resume simulation");
    drawText(-0.55f, -0.72f, "Press + / -     : Speed up / slow down orbital time");
    drawText(-0.55f, -0.80f, "Press O / L / R : Toggle Orbits / Toggle HUD / Reset Camera");

    glColor3f(0.25f, 1.0f, 0.45f);
    drawText(-0.22f, -0.88f, "Press S to START SIMULATION");

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// ==========================================
// 3D RENDERING: STARFIELD & SUN
// ==========================================
void drawStarfield() {
    glPushAttrib(GL_ENABLE_BIT | GL_POINT_BIT);
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glBegin(GL_POINTS);
    for (int i = 0; i < STAR_COUNT; i++) {
        float twinkle = 0.70f + 0.30f * sinf(simTime * stars[i].twinkleSpeed + stars[i].phase);
        glColor4f(stars[i].r * twinkle, stars[i].g * twinkle, stars[i].b * twinkle, twinkle);
        glPointSize(stars[i].size);
        glVertex3f(stars[i].x, stars[i].y, stars[i].z);
    }
    glEnd();

    glDepthMask(GL_TRUE);
    glPopAttrib();
}

void drawOrbitLine(float distance) {
    if (distance <= 0.0f || !showOrbits) return;

    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(1.2f);

    glColor4f(0.35f, 0.55f, 0.85f, 0.30f);

    glBegin(GL_LINE_LOOP);
    const int segments = 160;
    for (int i = 0; i < segments; i++) {
        float theta = (i * 2.0f * M_PI) / segments;
        glVertex3f(cosf(theta) * distance, 0.0f, sinf(theta) * distance);
    }
    glEnd();

    glPopAttrib();
}

void drawSun() {
    Planet* sun = &solarBodies[0];

    glPushMatrix();
    glRotatef(sun->axialTilt, 0.0f, 0.0f, 1.0f);
    glRotatef(sun->rotationAngle, 0.0f, 1.0f, 0.0f);

    // Glowing core material
    GLfloat sunEmission[] = { 1.0f, 0.85f, 0.25f, 1.0f };
    GLfloat noEmit[]      = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, sunEmission);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, noEmit);
    glMaterialfv(GL_FRONT, GL_AMBIENT, noEmit);

    glColor3f(1.0f, 0.85f, 0.20f);
    gluSphere(quadricObj, sun->radius, 60, 60);

    // Pulsating solar corona flare shells
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_LIGHTING);

    float pulse1 = 1.0f + 0.035f * sinf(sunGlowPhase * 2.0f);
    glColor4f(1.0f, 0.65f, 0.10f, 0.35f);
    gluSphere(quadricObj, sun->radius * 1.08f * pulse1, 40, 40);

    float pulse2 = 1.0f + 0.055f * sinf(sunGlowPhase * 1.5f + 1.2f);
    glColor4f(1.0f, 0.35f, 0.05f, 0.20f);
    gluSphere(quadricObj, sun->radius * 1.18f * pulse2, 35, 35);

    glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
    glPopAttrib();

    glPopMatrix();
}

// ==========================================
// PLANETARY DETAILS, RINGS & ATMOSPHERES
// ==========================================
void drawSaturnRings(float innerR, float outerR) {
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    const int segments = 160;
    const int ringBands = 8;
    for (int b = 0; b < ringBands; b++) {
        float t0 = (float)b / ringBands;
        float t1 = (float)(b + 1) / ringBands;

        // Cassini Division Gap
        if (b == 5) continue;

        float rStart = innerR + (outerR - innerR) * t0;
        float rEnd   = innerR + (outerR - innerR) * t1;

        float alpha = (b == 0 || b == ringBands - 1) ? 0.35f : 0.82f;
        float shade = 0.70f + 0.30f * sinf(b * 1.4f);

        glColor4f(0.85f * shade, 0.78f * shade, 0.58f * shade, alpha);

        glBegin(GL_QUAD_STRIP);
        glNormal3f(0.0f, 1.0f, 0.0f);
        for (int i = 0; i <= segments; i++) {
            float theta = (i * 2.0f * M_PI) / segments;
            float ct = cosf(theta);
            float st = sinf(theta);

            glVertex3f(ct * rStart, 0.0f, st * rStart);
            glVertex3f(ct * rEnd,   0.0f, st * rEnd);
        }
        glEnd();
    }

    glPopAttrib();
}

void drawEarthAtmosphere(float radius) {
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glColor4f(0.20f, 0.60f, 1.0f, 0.28f);
    gluSphere(quadricObj, radius * 1.06f, 36, 36);

    glColor4f(0.50f, 0.80f, 1.0f, 0.12f);
    gluSphere(quadricObj, radius * 1.12f, 32, 32);

    glPopAttrib();
}

void drawPlanetFeatures(int index, float radius) {
    if (index == 3) {
        // Earth: Ice caps, continents, atmosphere
        glColor3f(0.95f, 0.98f, 1.0f);
        glPushMatrix();
        glTranslatef(0.0f, radius * 0.92f, 0.0f);
        glScalef(1.0f, 0.22f, 1.0f);
        gluSphere(quadricObj, radius * 0.45f, 20, 20);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.0f, -radius * 0.92f, 0.0f);
        glScalef(1.0f, 0.22f, 1.0f);
        gluSphere(quadricObj, radius * 0.45f, 20, 20);
        glPopMatrix();

        glColor3f(0.18f, 0.65f, 0.22f);
        glPushMatrix();
        glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
        glTranslatef(radius * 0.82f, radius * 0.25f, 0.0f);
        gluSphere(quadricObj, radius * 0.40f, 16, 16);
        glPopMatrix();

        glPushMatrix();
        glRotatef(-75.0f, 0.0f, 1.0f, 0.0f);
        glTranslatef(radius * 0.80f, -radius * 0.15f, 0.0f);
        gluSphere(quadricObj, radius * 0.45f, 16, 16);
        glPopMatrix();

        drawEarthAtmosphere(radius);
    }
    else if (index == 4) {
        // Mars: Ice caps and volcanic plains
        glColor3f(0.95f, 0.95f, 0.98f);
        glPushMatrix();
        glTranslatef(0.0f, radius * 0.92f, 0.0f);
        glScalef(1.0f, 0.25f, 1.0f);
        gluSphere(quadricObj, radius * 0.38f, 16, 16);
        glPopMatrix();

        glColor3f(0.55f, 0.20f, 0.12f);
        glPushMatrix();
        glTranslatef(radius * 0.85f, 0.0f, 0.0f);
        gluSphere(quadricObj, radius * 0.38f, 14, 14);
        glPopMatrix();
    }
    else if (index == 5) {
        // Jupiter: Cloud belts & Great Red Spot
        const int belts = 12;
        for (int b = 0; b < belts; b++) {
            float yPos = -radius + (2.0f * radius / belts) * (b + 0.5f);
            float rBand = sqrtf(fmaxf(0.01f, radius * radius - yPos * yPos));

            if (b % 2 == 0) glColor3f(0.70f, 0.45f, 0.30f);
            else            glColor3f(0.90f, 0.82f, 0.70f);

            glPushMatrix();
            glTranslatef(0.0f, yPos, 0.0f);
            glScalef(1.0f, 0.18f, 1.0f);
            gluSphere(quadricObj, rBand * 1.002f, 32, 8);
            glPopMatrix();
        }

        glColor3f(0.85f, 0.25f, 0.15f);
        glPushMatrix();
        glRotatef(50.0f, 0.0f, 1.0f, 0.0f);
        glTranslatef(radius * 0.94f, -radius * 0.25f, 0.0f);
        glScalef(1.4f, 0.9f, 0.4f);
        gluSphere(quadricObj, radius * 0.24f, 20, 20);
        glPopMatrix();
    }
    else if (index == 6) {
        // Saturn rings
        drawSaturnRings(radius * 1.35f, radius * 2.45f);
    }
}

void drawCelestialBody(int index) {
    Planet* p = &solarBodies[index];

    drawOrbitLine(p->distance);

    glPushMatrix();

    glRotatef(p->orbitAngle, 0.0f, 1.0f, 0.0f);
    glTranslatef(p->distance, 0.0f, 0.0f);

    float rad = p->orbitAngle * (M_PI / 180.0f);
    p->posX = cosf(rad) * p->distance;
    p->posY = 0.0f;
    p->posZ = -sinf(rad) * p->distance;

    // Earth's Moon
    if (index == 3) {
        glPushMatrix();
        glRotatef(moonOrbitAngle, 0.0f, 1.0f, 0.0f);
        glTranslatef(moonDistance, 0.0f, 0.0f);

        float moonRad = moonOrbitAngle * (M_PI / 180.0f);
        moonPosX = p->posX + cosf(moonRad) * moonDistance;
        moonPosY = 0.0f;
        moonPosZ = p->posZ - sinf(moonRad) * moonDistance;

        glColor3f(0.80f, 0.80f, 0.82f);
        gluSphere(quadricObj, moonRadius, 24, 24);

        glPopMatrix();
    }

    glRotatef(p->axialTilt, 0.0f, 0.0f, 1.0f);
    glRotatef(p->rotationAngle, 0.0f, 1.0f, 0.0f);

    glColor3fv(p->color);
    gluSphere(quadricObj, p->radius, 48, 48);

    drawPlanetFeatures(index, p->radius);

    glPopMatrix();
}

// ==========================================
// 2D HUD OVERLAY & TELEMETRY
// ==========================================
void drawHUDOverlay() {
    if (!showHUD) return;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Telemetry header panel
    glColor4f(0.04f, 0.08f, 0.16f, 0.80f);
    glBegin(GL_QUADS);
    glVertex2f(15, windowHeight - 165);
    glVertex2f(380, windowHeight - 165);
    glVertex2f(380, windowHeight - 15);
    glVertex2f(15, windowHeight - 15);
    glEnd();

    glColor3f(0.20f, 0.70f, 1.0f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(15, windowHeight - 165);
    glVertex2f(380, windowHeight - 165);
    glVertex2f(380, windowHeight - 15);
    glVertex2f(15, windowHeight - 15);
    glEnd();

    char buffer[128];
    glColor3f(1.0f, 0.85f, 0.25f);
    drawText(28, windowHeight - 40, "SOLAR SYSTEM TELEMETRY");

    Planet* cur = (focusTarget >= 1 && focusTarget <= 9) ? &solarBodies[focusTarget - 1] : NULL;
    if (focusTarget == 10) {
        glColor3f(0.95f, 0.95f, 1.0f);
        drawText(28, windowHeight - 65, "TARGET: Moon (Luna)");
        drawTextSmall(28, windowHeight - 90, "Orbits: Earth (Synchronous Tidal Lock)");
        drawTextSmall(28, windowHeight - 110, "Orbital Period: 27.3 Earth Days");
        drawTextSmall(28, windowHeight - 130, "Mean Distance: ~384,400 km");
    } else if (cur) {
        glColor3f(0.95f, 0.95f, 1.0f);
        sprintf(buffer, "TARGET: %s", cur->name);
        drawText(28, windowHeight - 65, buffer);

        drawTextSmall(28, windowHeight - 88, cur->description);

        sprintf(buffer, "Orbital Period: %s", cur->orbitalPeriod);
        drawTextSmall(28, windowHeight - 108, buffer);

        sprintf(buffer, "Day Length    : %s", cur->dayLength);
        drawTextSmall(28, windowHeight - 128, buffer);

        sprintf(buffer, "Axial Tilt    : %.2f deg", cur->axialTilt);
        drawTextSmall(28, windowHeight - 148, buffer);
    } else {
        glColor3f(0.35f, 0.95f, 0.55f);
        drawText(28, windowHeight - 65, "VIEW: Heliospheric System Overview");
        drawTextSmall(28, windowHeight - 90, "Tracking: Central Solar Barycenter");
        drawTextSmall(28, windowHeight - 110, "Planets: 8 Major Planets + 1 Moon");
        drawTextSmall(28, windowHeight - 130, "Press [1-9] to lock target onto any planet");
        drawTextSmall(28, windowHeight - 150, "Press [0] to return to Heliospheric Overview");
    }

    // Bottom Controls Bar
    glColor4f(0.04f, 0.08f, 0.16f, 0.80f);
    glBegin(GL_QUADS);
    glVertex2f(15, 15);
    glVertex2f(windowWidth - 15, 15);
    glVertex2f(windowWidth - 15, 55);
    glVertex2f(15, 55);
    glEnd();

    glColor3f(0.20f, 0.70f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(15, 15);
    glVertex2f(windowWidth - 15, 15);
    glVertex2f(windowWidth - 15, 55);
    glVertex2f(15, 55);
    glEnd();

    glColor3f(0.85f, 0.92f, 0.98f);
    sprintf(buffer, "Status: %s | Time Warp: %.2fx | [Space] Pause/Play | [+ / -] Speed | [0-9] Target | [O] Orbits | [R] Reset",
            running ? "RUNNING" : "PAUSED", timeScale);
    drawTextSmall(28, 28, buffer);

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// ==========================================
// SCENE DISPLAY & ANIMATION TIMERS
// ==========================================
void display() {
    if (showStartScreen) {
        drawStartScreen();
        glutSwapBuffers();
        return;
    }

    glClearColor(0.015f, 0.018f, 0.035f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)windowWidth / (float)windowHeight, 1.0f, 2500.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Spherical camera eye calculation
    float yawRad   = camYaw * (M_PI / 180.0f);
    float pitchRad = camPitch * (M_PI / 180.0f);
    float eyeX = camLookX + camDist * cosf(pitchRad) * sinf(yawRad);
    float eyeY = camLookY + camDist * sinf(pitchRad);
    float eyeZ = camLookZ + camDist * cosf(pitchRad) * cosf(yawRad);

    gluLookAt(eyeX, eyeY, eyeZ,
              camLookX, camLookY, camLookZ,
              0.0f, 1.0f, 0.0f);

    // 1. Draw Deep Space Starfield
    drawStarfield();

    // 2. Setup Sun as Point Light Source (GL_LIGHT0 at 0,0,0)
    GLfloat lightPos[]      = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat lightAmbient[]  = { 0.12f, 0.12f, 0.14f, 1.0f };
    GLfloat lightDiffuse[]  = { 1.20f, 1.15f, 1.05f, 1.0f };
    GLfloat lightSpecular[] = { 0.80f, 0.80f, 0.80f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    GLfloat matSpecular[] = { 0.35f, 0.35f, 0.35f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, 25.0f);

    // 3. Render Sun & Corona
    drawSun();

    // 4. Render All Planets & Moons
    for (int i = 1; i < PLANET_COUNT; i++) {
        drawCelestialBody(i);
    }

    // 5. Draw 2D Telemetry & Controls HUD
    drawHUDOverlay();

    glutSwapBuffers();
}

void update(int value) {
    if (!showStartScreen && running) {
        simTime += 0.016f * timeScale;
        sunGlowPhase += 0.035f;

        // Advance orbital revolutions & day/night spins
        for (int i = 0; i < PLANET_COUNT; i++) {
            solarBodies[i].orbitAngle += solarBodies[i].orbitSpeed * 0.45f * timeScale;
            if (solarBodies[i].orbitAngle >= 360.0f) solarBodies[i].orbitAngle -= 360.0f;

            solarBodies[i].rotationAngle += solarBodies[i].rotationSpeed * 0.80f * timeScale;
            if (solarBodies[i].rotationAngle >= 360.0f) solarBodies[i].rotationAngle -= 360.0f;
        }

        // Earth's moon orbit
        moonOrbitAngle += 2.8f * timeScale;
        if (moonOrbitAngle >= 360.0f) moonOrbitAngle -= 360.0f;

        // Smooth camera tracking interpolation
        if (focusTarget == 0) {
            targetLookX = 0.0f; targetLookY = 0.0f; targetLookZ = 0.0f;
        } else if (focusTarget == 10) {
            targetLookX = moonPosX; targetLookY = moonPosY; targetLookZ = moonPosZ;
        } else if (focusTarget >= 1 && focusTarget <= 9) {
            Planet* p = &solarBodies[focusTarget - 1];
            targetLookX = p->posX; targetLookY = p->posY; targetLookZ = p->posZ;
        }

        // Smooth glide lerp
        camLookX += (targetLookX - camLookX) * 0.08f;
        camLookY += (targetLookY - camLookY) * 0.08f;
        camLookZ += (targetLookZ - camLookZ) * 0.08f;
        camDist  += (targetCamDist - camDist) * 0.08f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // 60 FPS
}

// ==========================================
// INPUT CONTROLS & RESIZE
// ==========================================
void handleKeypress(unsigned char key, int x, int y) {
    if (showStartScreen) {
        if (key == 's' || key == 'S') {
            showStartScreen = 0;
        }
        glutPostRedisplay();
        return;
    }

    switch (key) {
        case ' ': // Pause / Resume
            running = !running;
            break;
        case '+':
        case '=': // Speed up
            timeScale *= 1.35f;
            if (timeScale > 50.0f) timeScale = 50.0f;
            break;
        case '-':
        case '_': // Slow down
            timeScale *= 0.74f;
            if (timeScale < 0.05f) timeScale = 0.05f;
            break;
        case 'o':
        case 'O': // Toggle orbit paths
            showOrbits = !showOrbits;
            break;
        case 'l':
        case 'L': // Toggle HUD
            showHUD = !showHUD;
            break;
        case 'r':
        case 'R': // Reset view
            focusTarget = 0;
            targetCamDist = 140.0f;
            camYaw = 40.0f;
            camPitch = 25.0f;
            break;

        // Quick Planet Focus
        case '0': focusTarget = 0; targetCamDist = 160.0f; break; // System Overview
        case '1': focusTarget = 1; targetCamDist = 38.0f;  break; // Sun
        case '2': focusTarget = 2; targetCamDist = 8.0f;   break; // Mercury
        case '3': focusTarget = 3; targetCamDist = 12.0f;  break; // Venus
        case '4': focusTarget = 4; targetCamDist = 14.0f;  break; // Earth
        case '5': focusTarget = 5; targetCamDist = 10.0f;  break; // Mars
        case '6': focusTarget = 6; targetCamDist = 28.0f;  break; // Jupiter
        case '7': focusTarget = 7; targetCamDist = 26.0f;  break; // Saturn
        case '8': focusTarget = 8; targetCamDist = 18.0f;  break; // Uranus
        case '9': focusTarget = 9; targetCamDist = 16.0f;  break; // Neptune
        case 'm':
        case 'M': focusTarget = 10; targetCamDist = 4.5f;  break; // Moon
        case 27:  exit(0); break;                                 // ESC
    }

    glutPostRedisplay();
}

void handleSpecialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_UP) {
        camPitch += 3.0f;
        if (camPitch > 88.0f) camPitch = 88.0f;
    } else if (key == GLUT_KEY_DOWN) {
        camPitch -= 3.0f;
        if (camPitch < -88.0f) camPitch = -88.0f;
    } else if (key == GLUT_KEY_LEFT) {
        camYaw -= 4.0f;
    } else if (key == GLUT_KEY_RIGHT) {
        camYaw += 4.0f;
    }
    glutPostRedisplay();
}

void handleMouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseLeftDown = 1;
            lastMouseX = x;
            lastMouseY = y;
        } else {
            mouseLeftDown = 0;
        }
    } else if (button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseRightDown = 1;
            lastMouseX = x;
            lastMouseY = y;
        } else {
            mouseRightDown = 0;
        }
    }
    else if (button == 3 && state == GLUT_DOWN) { // Wheel Up
        targetCamDist -= targetCamDist * 0.12f;
        if (targetCamDist < 3.0f) targetCamDist = 3.0f;
    } else if (button == 4 && state == GLUT_DOWN) { // Wheel Down
        targetCamDist += targetCamDist * 0.12f;
        if (targetCamDist > 650.0f) targetCamDist = 650.0f;
    }
}

void handleMouseMotion(int x, int y) {
    int dx = x - lastMouseX;
    int dy = y - lastMouseY;

    if (mouseLeftDown) {
        camYaw += dx * 0.40f;
        camPitch += dy * 0.40f;
        if (camPitch > 88.0f)  camPitch = 88.0f;
        if (camPitch < -88.0f) camPitch = -88.0f;
    } else if (mouseRightDown) {
        targetCamDist += dy * 0.45f * (camDist * 0.02f);
        if (targetCamDist < 3.0f)   targetCamDist = 3.0f;
        if (targetCamDist > 650.0f) targetCamDist = 650.0f;
    }

    lastMouseX = x;
    lastMouseY = y;
    glutPostRedisplay();
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
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    quadricObj = gluNewQuadric();
    gluQuadricNormals(quadricObj, GLU_SMOOTH);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(40, 30);
    glutCreateWindow("Realistic 3D Solar System Simulation - OpenGL/GLUT");

    initOpenGL();
    initStars();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(handleKeypress);
    glutSpecialFunc(handleSpecialKeys);
    glutMouseFunc(handleMouseClick);
    glutMotionFunc(handleMouseMotion);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
