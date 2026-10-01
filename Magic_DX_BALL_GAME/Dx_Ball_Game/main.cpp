#include <windows.h>
#include <GL/glut.h>
#include <math.h>

int screenWidth = 800;
int screenHeight = 600;

int startWindow;
int menuWindow;

// Increases whenever we leave or enter a level.
// Old GLUT timer callbacks use the old number and immediately stop.
int timerGeneration = 0;


int getTextWidth(void* font, const char* text)
{
    int width = 0;
    for (int i = 0; text[i]; i++)
        width += glutBitmapWidth(font, text[i]);
    return width;
}

void drawText(float x, float y, void* font, const char* text)
{
    glRasterPos2f(x, y);
    for (int i = 0; text[i]; i++)
        glutBitmapCharacter(font, text[i]);
}

//--------------------------------------------------
// COLOUR / DRAWING HELPERS (visual only)
//--------------------------------------------------
void fillRect(float x, float y, float w, float h)
{
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
    glEnd();
}

void outlineRect(float x, float y, float w, float h)
{
   glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
    glEnd();
}

// Brick palette for LIGHT backgrounds (Level 1 and Level 2):
// deep, saturated colours so they stand out against sky and clouds.
void setBrickColorLight(int c)
{
    if (c == 0)      glColor3f(0.80f, 0.10f, 0.10f); // red
    else if (c == 1) glColor3f(0.00f, 0.50f, 0.20f); // dark green
    else if (c == 2) glColor3f(0.10f, 0.25f, 0.75f); // royal blue
    else if (c == 3) glColor3f(0.95f, 0.55f, 0.00f); // orange
    else             glColor3f(0.55f, 0.10f, 0.60f); // purple
}

// Brick palette for DARK backgrounds (Level 3):
// bright, glowing colours so they stand out against the night scene.
void setBrickColorDark(int c)
{
    if (c == 0)      glColor3f(1.00f, 0.90f, 0.20f); // yellow
    else if (c == 1) glColor3f(0.20f, 0.95f, 1.00f); // cyan
    else if (c == 2) glColor3f(0.30f, 1.00f, 0.30f); // lime
    else if (c == 3) glColor3f(0.45f, 0.60f, 1.00f); // light blue
    else             glColor3f(1.00f, 0.40f, 0.90f); // pink
}


void drawTitleAndWelcome()
{
    const char* title    = "MAGIC BALL GAME";
    const char* welcome  = "Gear up. Lock in. Let the games begin.";
    const char* creators = "Created by: Amartay Das";

    int titleWidth      = getTextWidth(GLUT_BITMAP_TIMES_ROMAN_24, title);
    int welcomeWidth    = getTextWidth(GLUT_BITMAP_HELVETICA_18, welcome);
    int creatorWidth    = getTextWidth(GLUT_BITMAP_8_BY_13, creators);
   // int instructorWidth = getTextWidth(GLUT_BITMAP_8_BY_13, instructor);

    //TITLE (gold)
    glColor3f(1.0f, 0.85f, 0.2f);
    drawText(
        (screenWidth - titleWidth) / 2,
        screenHeight / 2 + 120,
        GLUT_BITMAP_TIMES_ROMAN_24,
        title
    );

    //WELCOME (light cyan)
    glColor3f(0.65f, 0.92f, 1.0f);
    drawText(
        (screenWidth - welcomeWidth) / 2,
        screenHeight / 2 + 80,
        GLUT_BITMAP_HELVETICA_18,
        welcome
    );

    //CREATORS (White)
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(
        (screenWidth - creatorWidth) / 2,
        screenHeight / 2 - 260,
        GLUT_BITMAP_8_BY_13,
        creators
    );



}

//START BUTTON
void drawStartButton()
{
    int btnWidth  = 160;
    int btnHeight = 50;

    int btnX = screenWidth / 2 - btnWidth / 2;
    int btnY = screenHeight / 2 - btnHeight / 2;

    // Gold button
    glColor3f(1.0f, 0.80f, 0.10f);
    fillRect(btnX, btnY, btnWidth, btnHeight);

    // White border
    glColor3f(1, 1, 1);
    outlineRect(btnX, btnY, btnWidth, btnHeight);

    // Button text (dark on gold = very high contrast)
    const char* text = "START GAME";
    int textWidth = getTextWidth(GLUT_BITMAP_8_BY_13, text);

    int textX = btnX + (btnWidth - textWidth) / 2;
    int textY = btnY + btnHeight / 2 - 5;

    glColor3f(0.05f, 0.05f, 0.15f);
    drawText(textX, textY, GLUT_BITMAP_8_BY_13, text);
}

//START WINDOW
void startDisplay()
{
    glClear(GL_COLOR_BUFFER_BIT);
    drawTitleAndWelcome();
    drawStartButton();
    glFlush();
}

void menuDisplay();
void menuMouse(int, int, int, int);

//Helper: destroy the current window and return to the level-select menu
void goToMenu()
{
    // Stop old level timer callbacks from continuing after changing screens.
    timerGeneration++;

    int currentWindow = glutGetWindow();
    glutDestroyWindow(currentWindow);

    menuWindow = glutCreateWindow("DX Ball - Menu");
    glutReshapeWindow(screenWidth, screenHeight);

    glClearColor(0.02f, 0.03f, 0.12f, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, screenWidth, 0, screenHeight);

    glutDisplayFunc(menuDisplay);
    glutMouseFunc(menuMouse);
}

//--------------------------------------------------
// BACK TO MENU BUTTON
// Drawn top-left in every level; clicking it (or
// hitting Esc) returns to the level-select menu
// from anywhere in the game.
//--------------------------------------------------
const float backBtnW = 80;
const float backBtnH = 28;

void drawBackButton()
{
    float bx = 10;
    float by = screenHeight - 95;

    // Button background (deep blue)
    glColor3f(0.05f, 0.15f, 0.45f);
    fillRect(bx, by, backBtnW, backBtnH);

    // Button border
    glColor3f(1, 1, 1);
    outlineRect(bx, by, backBtnW, backBtnH);

    // Button label
    const char* text = "< MENU";
    glColor3f(1, 1, 1);
    glRasterPos2f(bx + 8, by + 9);
    for (int i = 0; text[i]; i++)
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, text[i]);
}

bool isBackButtonClicked(int x, int y)
{
    float bx = 10;
    float by = screenHeight - 95;

    int my = screenHeight - y;

    return (x >= bx && x <= bx + backBtnW &&
            my >= by && my <= by + backBtnH);
}

//--------------------------------------------------
// Esc key also returns to the menu, from any level
//--------------------------------------------------
void backKeyHandler(unsigned char key, int, int)
{
    if (key == 27) // Esc
        goToMenu();
}

void startMouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        int my = screenHeight - y;

        if (x >= screenWidth/2 - 80 && x <= screenWidth/2 + 80 &&
            my >= screenHeight/2 - 25 && my <= screenHeight/2 + 25)
        {
            glutDestroyWindow(startWindow);

            menuWindow = glutCreateWindow("DX Ball - Menu");
            glutReshapeWindow(screenWidth, screenHeight);

            glClearColor(0.02f, 0.03f, 0.12f, 1);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluOrtho2D(0, screenWidth, 0, screenHeight);

            glutDisplayFunc(menuDisplay);
            glutMouseFunc(menuMouse);
        }
    }
}



float paddleX = 350;
float paddleY = 50;
float paddleWidth = 100;
float paddleHeight = 15;

float ballX = 400;
float ballY = 200;
float ballRadius = 8;
float ballDX = 4;
float ballDY = 4;

bool gameStarted = false;

int score = 0;
int level = 1;
int lives = 3;

//--------------------------------------------------
// Shared paddle / ball shapes (fill + outline colours)
//--------------------------------------------------
void drawPaddleShape(float fr, float fg, float fb, float orr, float og, float ob)
{
    glColor3f(fr, fg, fb);
    fillRect(paddleX, paddleY, paddleWidth, paddleHeight);

    glColor3f(orr, og, ob);
    outlineRect(paddleX, paddleY, paddleWidth, paddleHeight);
}

void drawBallShape(float fr, float fg, float fb, float orr, float og, float ob)
{
    glColor3f(fr, fg, fb);
    glBegin(GL_POLYGON);
    glVertex2f(ballX, ballY + ballRadius);
    glVertex2f(ballX + ballRadius * 0.7f, ballY + ballRadius * 0.7f);
    glVertex2f(ballX + ballRadius, ballY);
    glVertex2f(ballX + ballRadius * 0.7f, ballY - ballRadius * 0.7f);
    glVertex2f(ballX, ballY - ballRadius);
    glVertex2f(ballX - ballRadius * 0.7f, ballY - ballRadius * 0.7f);
    glVertex2f(ballX - ballRadius, ballY);
    glVertex2f(ballX - ballRadius * 0.7f, ballY + ballRadius * 0.7f);
    glEnd();

    glColor3f(orr, og, ob);
    glBegin(GL_LINE_LOOP);
    glVertex2f(ballX, ballY + ballRadius);
    glVertex2f(ballX + ballRadius * 0.7f, ballY + ballRadius * 0.7f);
    glVertex2f(ballX + ballRadius, ballY);
    glVertex2f(ballX + ballRadius * 0.7f, ballY - ballRadius * 0.7f);
    glVertex2f(ballX, ballY - ballRadius);
    glVertex2f(ballX - ballRadius * 0.7f, ballY - ballRadius * 0.7f);
    glVertex2f(ballX - ballRadius, ballY);
    glVertex2f(ballX - ballRadius * 0.7f, ballY + ballRadius * 0.7f);
    glEnd();
}

void resetBall()
{
    ballX = paddleX + paddleWidth / 2;
    ballY = paddleY + paddleHeight + ballRadius;

    ballDX = 4;
    ballDY = 4;

    gameStarted = false;
}


//LEVEL 1

const int brickRows1 = 5;
const int brickCols1 = 10;
float brickWidth = 70;
float brickHeight = 20;
bool bricks1[brickRows1][brickCols1];


void drawText(float x, float y, const char* text)
{
    glColor3f(0, 0, 0);
    glRasterPos2f(x, y);
    for (int i = 0; text[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
}

void drawNumber(int number, float x, float y)
{
    if (number == 0)
    {
        glRasterPos2f(x, y);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, '0');
        return;
    }

    char digits[12];
    int i = 0;

    while (number > 0)
    {
        digits[i++] = (number % 10) + '0';
        number /= 10;
    }

    glRasterPos2f(x, y);
    for (int j = i - 1; j >= 0; j--)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, digits[j]);
}


bool allBricksBroken1()
{
    for (int i = 0; i < brickRows1; i++)
        for (int j = 0; j < brickCols1; j++)
            if (bricks1[i][j])
                return false;
    return true;
}


// BACKGROUND
void drawBackground()
{
    // Sky (light blue, so dark text and deep brick colours stand out)
    glColor3f(0.60f, 0.85f, 1.00f);
    glBegin(GL_QUADS);
    glVertex2f(0, screenHeight);
    glVertex2f(screenWidth, screenHeight);
    glVertex2f(screenWidth, screenHeight / 2);
    glVertex2f(0, screenHeight / 2);
    glEnd();

    // Ground (lighter green than the hills so hills stay distinct)
    glColor3f(0.35f, 0.75f, 0.30f);
    glBegin(GL_QUADS);
    glVertex2f(0, screenHeight / 2);
    glVertex2f(screenWidth, screenHeight / 2);
    glVertex2f(screenWidth, 0);
    glVertex2f(0, 0);
    glEnd();

    // Clouds
    glColor3f(1, 1, 1);

    float cloudX[][4] = {
        {100, 135, 170, 205},
        {300, 335, 370, 405},
        {520, 555, 590, 625}
    };
    float cloudY[] = {530, 560, 540};

    for (int c = 0; c < 3; c++)
    {
        for (int i = 0; i < 4; i++)
        {
            float rx = 22;
            float ry = (i == 1 || i == 2) ? 34 : 22;

            glBegin(GL_POLYGON);
            for (int a = 0; a < 360; a += 15)
            {
                float ang = a * 3.1416f / 180.0f;
                glVertex2f(
                    cloudX[c][i] + cos(ang) * rx,
                    cloudY[c] + sin(ang) * ry
                );
            }
            glEnd();
        }
    }

    //HILLS (darker green)
    glColor3f(0.05f, 0.42f, 0.12f);
    glBegin(GL_TRIANGLES);

    // Hill 1
    glVertex2f(0,   screenHeight / 2);
    glVertex2f(350, screenHeight / 2);
    glVertex2f(175, 420);

    // Hill 2
    glVertex2f(200, screenHeight / 2);
    glVertex2f(650, screenHeight / 2);
    glVertex2f(425, 440);

    // Hill 3
    glVertex2f(450, screenHeight / 2);
    glVertex2f(900, screenHeight / 2);
    glVertex2f(675, 410);

    glEnd();
}



void initBricks1()
{
    int centerRow = brickRows1 / 2;
    int centerCol = brickCols1 / 2;

    for (int i = 0; i < brickRows1; i++)
    {
        for (int j = 0; j < brickCols1; j++)
        {
            int dr = abs(i - centerRow);
            int dc = abs(j - centerCol);
            bricks1[i][j] = (dr + dc <= 4);
        }
    }
}

void drawPaddle()
{
    // Navy paddle with white outline
    drawPaddleShape(0.05f, 0.10f, 0.35f, 1, 1, 1);
}

void drawBall()
{
    // Bright orange-red ball with black outline
    drawBallShape(1.0f, 0.35f, 0.0f, 0, 0, 0);
}

void drawBricks()
{
    for (int i = 0; i < brickRows1; i++)
    {
        for (int j = 0; j < brickCols1; j++)
        {
            if (bricks1[i][j])
            {
                int color = (i + j) % 5;
                setBrickColorLight(color);

                float x = j * brickWidth + 35;
                float y = screenHeight - (i + 3) * brickHeight;

                fillRect(x, y, brickWidth, brickHeight);

                glColor3f(0, 0, 0);
                outlineRect(x, y, brickWidth, brickHeight);
            }
        }
    }
}

void update(int value)
{
    // If this timer belongs to an old level/window, stop it permanently.
    if (value != timerGeneration)
        return;

    if (gameStarted)
    {
        ballX += ballDX;
        ballY += ballDY;
    }
    else
    {
        ballX = paddleX + paddleWidth / 2;
        ballY = paddleY + paddleHeight + ballRadius;
    }

    if (ballX - ballRadius <= 0 || ballX + ballRadius >= screenWidth)
        ballDX = -ballDX;

    if (ballY + ballRadius >= screenHeight)
        ballDY = -ballDY;

    if (ballDY < 0 &&
        ballY - ballRadius <= paddleY + paddleHeight &&
        ballX >= paddleX &&
        ballX <= paddleX + paddleWidth)
    {
        ballDY = -ballDY;
    }

    //BRICK COLLISION
    for (int i = 0; i < brickRows1; i++)
    {
        for (int j = 0; j < brickCols1; j++)
        {
            if (bricks1[i][j])
            {
                float bx = j * brickWidth + 35;
                float by = screenHeight - (i + 3) * brickHeight;

                if (ballX + ballRadius > bx &&
                    ballX - ballRadius < bx + brickWidth &&
                    ballY + ballRadius > by &&
                    ballY - ballRadius < by + brickHeight)
                {
                    bricks1[i][j] = false;
                    score += 5;
                    ballDY = -ballDY;
                    goto done;
                }
            }
        }
    }
done:

    if (allBricksBroken1())
    {
        char msg[150];
        wsprintf(msg, "Congratulations! You cleared Level 1!\nYour Score: %d\n\nPlay this level again?\n(Yes = Play Again, No = Return to Menu)", score);
        int choice = MessageBox(0, msg, "Level Complete!", MB_YESNO | MB_ICONINFORMATION);

        score = 0;
        lives = 3;
        gameStarted = false;

        if (choice == IDYES)
        {
            initBricks1();
            resetBall();
        }
        else
        {
            goToMenu();
            return;
        }
    }

    if (ballY - ballRadius < 0)
    {
        lives--;

        if (lives > 0)
        {
            char msg[100];
            wsprintf(msg, "You lost a life!\nLives remaining: %d", lives);
            MessageBox(0, msg, "DX Ball", MB_OK | MB_ICONWARNING);

            resetBall();
        }
        else
        {
            char msg[100];
            wsprintf(msg, "Game Over!\nYour Score: %d", score);
            MessageBox(0, msg, "DX Ball", MB_OK | MB_ICONWARNING);

            score = 0;
            lives = 3;
            gameStarted = false;
            goToMenu();
            return;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, timerGeneration);
}


void mouseMotion(int x, int y)
{
    paddleX = x - paddleWidth / 2;
    if (paddleX < 0) paddleX = 0;
    if (paddleX + paddleWidth > screenWidth)
        paddleX = screenWidth - paddleWidth;
}

void mouseButton(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if (isBackButtonClicked(x, y))
        {
            goToMenu();
            return;
        }

        if (!gameStarted)
        {
            gameStarted = true;
            if (ballDY < 0) ballDY = -ballDY;
        }
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    drawBackground();
    drawPaddle();
    drawBall();
    drawBricks();


    drawText(10, screenHeight - 30, "Score:");
    drawNumber(score, 80, screenHeight - 30);

    drawText(10, screenHeight - 60, "Lives:");
    drawNumber(lives, 80, screenHeight - 60);

    drawText(screenWidth - 140, screenHeight - 30, "Level:");
    drawNumber(level, screenWidth - 70, screenHeight - 30);

    drawBackButton();

    glFlush();
}

void init()
{
    glClearColor(0, 0, 0, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, screenWidth, 0, screenHeight);
    initBricks1();
}

void openLevel1()
{
    // New level session: all previous level timers become invalid.
    timerGeneration++;

    glutDestroyWindow(menuWindow);

    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("DX Ball - Level 1");


    score = 0;
    lives = 3;
    level = 1;
    gameStarted = false;

    init();
    resetBall();

    glutDisplayFunc(display);
    glutPassiveMotionFunc(mouseMotion);
    glutMouseFunc(mouseButton);
    glutKeyboardFunc(backKeyHandler);
    glutTimerFunc(16, update, timerGeneration);
}


//LEVEL 2




const int brickRows = 8;
const int brickCols = 10;
float brickWidth2 = 70;
float brickHeight2 = 20;

bool bricks[brickRows][brickCols];
bool wallBricks[brickRows][brickCols];
int brickColor[brickRows][brickCols];




int W = 900, H = 600;

//CIRCLE
void circle(float cx, float cy, float r)
{
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i <= 360; i++)
    {
        float a = i * 3.1416f / 180;
        glVertex2f(cx + cos(a) * r, cy + sin(a) * r);
    }
    glEnd();
}




void drawSky()
{
    glBegin(GL_QUADS);
    // Lighter near the horizon, slightly deeper toward the top
    glColor3f(0.70f, 0.90f, 1.00f);
    glVertex2f(0, 300);
    glVertex2f(W, 300);
    glColor3f(0.30f, 0.68f, 0.95f);
    glVertex2f(W, H);
    glVertex2f(0, H);
    glEnd();
}


void cloud(float x, float y)
{
    glColor3f(1,1,1);
    circle(x, y, 18);
    circle(x+20, y+5, 22);
    circle(x+40, y, 18);
}


void drawSea()
{
    // Deeper blue so white sails and the light sky stand out
    glColor3f(0.00f, 0.35f, 0.60f);
    glBegin(GL_QUADS);
    glVertex2f(0, 180);
    glVertex2f(W, 180);
    glVertex2f(W, 300);
    glVertex2f(0, 300);
    glEnd();
}


void drawSand()
{
    glColor3f(0.96f, 0.87f, 0.60f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(W, 0);
    glVertex2f(W, 180);
    glVertex2f(0, 180);
    glEnd();
}




void palmTree(float x, float y)
{
    glColor3f(0.40f, 0.20f, 0.05f);
    glBegin(GL_QUADS);
    glVertex2f(x-5, y);
    glVertex2f(x+5, y);
    glVertex2f(x+10, y+80);
    glVertex2f(x, y+80);
    glEnd();
}


void bush(float x, float y)
{
    glColor3f(0.0f, 0.35f, 0.15f);
    circle(x, y, 20);
    circle(x+20, y+10, 25);
    circle(x+40, y, 20);
}

// --------------------------------------------------
// LEVEL 2 BOAT ANIMATION VARIABLES
// --------------------------------------------------
float boat1X = 50;
float boat2X = 400;
float boat3X = 700;
float boat4X = 250;

float boatSpeed1 = 1.5f;
float boatSpeed2 = 1.0f;
float boatSpeed3 = 1.8f;
float boatSpeed4 = 1.2f;


void boat(float x, float y)
{

    glColor3f(0.55f, 0.27f, 0.07f);
    glBegin(GL_POLYGON);
    glVertex2f(x - 40, y);
    glVertex2f(x + 40, y);
    glVertex2f(x + 25, y - 15);
    glVertex2f(x - 25, y - 15);
    glEnd();


    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(x - 2, y);
    glVertex2f(x + 2, y);
    glVertex2f(x + 2, y + 35);
    glVertex2f(x - 2, y + 35);
    glEnd();


    glColor3f(1, 1, 1);
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y + 35);
    glVertex2f(x, y);
    glVertex2f(x + 30, y + 15);
    glEnd();
}



void initBricksLevel2()
{
    for (int i = 0; i < brickRows; i++)
        for (int j = 0; j < brickCols; j++)
        {
            bricks[i][j] = false;
            wallBricks[i][j] = false;
        }

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < brickCols; j++)
            bricks[i][j] = true;

    for (int i = 3; i < brickRows; i++)
    {
        wallBricks[i][0] = true;
        wallBricks[i][brickCols - 1] = true;
        wallBricks[i][3] = true;
        wallBricks[i][6] = true;
    }

    int bottom = brickRows - 1;
    for (int j = 0; j < brickCols; j++)
    {
        if (j == 2 || j == 4 || j == 7)
            wallBricks[bottom][j] = false;
        else
            wallBricks[bottom][j] = true;
    }

    bricks[4][2] = true;
    bricks[4][4] = true;
    bricks[4][5] = true;
    bricks[4][7] = true;
    bricks[4][8] = true;

    for (int i = 0; i < brickRows; i++)
        for (int j = 0; j < brickCols; j++)
            if (bricks[i][j])
                brickColor[i][j] = (i * 3 + j) % 5;
}

void drawPaddleLevel2()
{
    // Navy paddle with white outline
    drawPaddleShape(0.05f, 0.10f, 0.35f, 1, 1, 1);
}

void drawBallLevel2()
{
    // Bright orange-red ball with black outline
    drawBallShape(1.0f, 0.35f, 0.0f, 0, 0, 0);
}

void drawBricksLevel2()
{
    for (int i = 0; i < brickRows; i++)
        for (int j = 0; j < brickCols; j++)
        {
            if (!bricks[i][j] && !wallBricks[i][j]) continue;

            if (wallBricks[i][j])
                glColor3f(0.33f, 0.36f, 0.42f); // dark steel gray
            else
                setBrickColorLight(brickColor[i][j]);

            float x = j * brickWidth + 35;
            float y = screenHeight - (i + 3) * brickHeight;

            fillRect(x, y, brickWidth, brickHeight);

            glColor3f(0, 0, 0);
            outlineRect(x, y, brickWidth, brickHeight);


        }
}

void drawText2(float x, float y, const char* text)
{
    glColor3f(0, 0, 0);
    glRasterPos2f(x, y);
    for (int i = 0; text[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
}

void drawNumber2(int number, float x, float y)
{
    if (number == 0)
    {
        glRasterPos2f(x, y);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, '0');
        return;
    }

    char digits[12];
    int i = 0;

    while (number > 0)
    {
        digits[i++] = (number % 10) + '0';
        number /= 10;
    }

    glRasterPos2f(x, y);
    for (int j = i - 1; j >= 0; j--)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, digits[j]);
}


bool allBricksBrokenLevel2()
{
    for (int i = 0; i < brickRows; i++)
        for (int j = 0; j < brickCols; j++)
            if (bricks[i][j])
                return false;
    return true;
}


// --------------------------------------------------
// LEVEL 2 BOAT MOVEMENT
// --------------------------------------------------
void updateBoatsLevel2()
{
    // Move all boats from left to right.
    boat1X += boatSpeed1;
    boat2X += boatSpeed2;
    boat3X += boatSpeed3;
    boat4X += boatSpeed4;

    // When a boat leaves the right side, bring it back
    // to the left side so the movement continues in a loop.
    if (boat1X - 40 > screenWidth) boat1X = -40;
    if (boat2X - 40 > screenWidth) boat2X = -40;
    if (boat3X - 40 > screenWidth) boat3X = -40;
    if (boat4X - 40 > screenWidth) boat4X = -40;
}


void updateLevel2(int value)
{
    // If this timer belongs to an old level/window, stop it permanently.
    if (value != timerGeneration)
        return;

    if (gameStarted)
    {
        updateBoatsLevel2();
        ballX += ballDX;
        ballY += ballDY;
    }
    else
    {
        ballX = paddleX + paddleWidth / 2;
        ballY = paddleY + paddleHeight + ballRadius;
    }

    if (ballX - ballRadius <= 0)
    {
        ballX = ballRadius;
        ballDX = -ballDX;
    }
    else if (ballX + ballRadius >= screenWidth)
    {
        ballX = screenWidth - ballRadius;
        ballDX = -ballDX;
    }


    if (ballY + ballRadius >= screenHeight)
    {
        ballY = screenHeight - ballRadius;
        ballDY = -ballDY;
    }


    if (ballDY < 0 &&
        ballY - ballRadius <= paddleY + paddleHeight &&
        ballX >= paddleX &&
        ballX <= paddleX + paddleWidth)
        ballDY = -ballDY;

    for (int i = 0; i < brickRows; i++)
        for (int j = 0; j < brickCols; j++)
        {
            if (!bricks[i][j] && !wallBricks[i][j]) continue;

            float bx = j * brickWidth + 35;
            float by = screenHeight - (i + 3) * brickHeight;

            if (ballX + ballRadius > bx &&
                ballX - ballRadius < bx + brickWidth &&
                ballY + ballRadius > by &&
                ballY - ballRadius < by + brickHeight)
            {
                float overlapX1 = (ballX + ballRadius) - bx;
                float overlapX2 = (bx + brickWidth) - (ballX - ballRadius);
                float overlapX = overlapX1 < overlapX2 ? overlapX1 : overlapX2;

                float overlapY1 = (ballY + ballRadius) - by;
                float overlapY2 = (by + brickHeight) - (ballY - ballRadius);
                float overlapY = overlapY1 < overlapY2 ? overlapY1 : overlapY2;

                if (overlapX < overlapY)
                {
                    if (ballX < bx)
                        ballX = bx - ballRadius;
                    else
                        ballX = bx + brickWidth + ballRadius;

                    ballDX = -ballDX;
                }
                else
                {
                    if (ballY < by)
                        ballY = by - ballRadius;
                    else
                        ballY = by + brickHeight + ballRadius;

                    ballDY = -ballDY;
                }

                    ballX += ballDX * 0.1f;
                    ballY += ballDY * 0.1f;


                        if (wallBricks[i][j])
                        {

                        }
                        else if (bricks[i][j])
                        {
                            bricks[i][j] = false;
                            score += 5;
                        }

                            goto done;

            }
        }

done:

    if (allBricksBrokenLevel2())
    {
        char msg[150];
        wsprintf(msg, "Congratulations! You cleared Level 2!\nYour Score: %d\n\nPlay this level again?\n(Yes = Play Again, No = Return to Menu)", score);
        int choice = MessageBox(0, msg, "Level Complete!", MB_YESNO | MB_ICONINFORMATION);

        score = 0;
        lives = 3;
        gameStarted = false;

        if (choice == IDYES)
        {
            initBricksLevel2();
            resetBall();
        }
        else
        {
            goToMenu();
            return;
        }
    }



    if (ballY - ballRadius < 0)
    {
        lives--;

        if (lives > 0)
        {
            char msg[100];
            wsprintf(msg, "You lost a life!\nLives remaining: %d", lives);
            MessageBox(0, msg, "DX Ball", MB_OK | MB_ICONWARNING);

            resetBall();
        }
        else
        {
            char msg[100];
            wsprintf(msg, "Game Over!\nYour Score: %d", score);
            MessageBox(0, msg, "DX Ball", MB_OK | MB_ICONWARNING);

            score = 0;
            lives = 3;
            gameStarted = false;
            goToMenu();
            return;
        }
    }




    glutPostRedisplay();
    glutTimerFunc(16, updateLevel2, timerGeneration);
}


void mouseMotionLevel2(int x, int)
{
    paddleX = x - paddleWidth / 2;
    if (paddleX < 0) paddleX = 0;
    if (paddleX + paddleWidth > screenWidth)
        paddleX = screenWidth - paddleWidth;
}

void mouseButtonLevel2(int b, int s, int x, int y)
{
    if (b == GLUT_LEFT_BUTTON && s == GLUT_DOWN)
    {
        if (isBackButtonClicked(x, y))
        {
            goToMenu();
            return;
        }

        if (!gameStarted)
        {
            gameStarted = true;
            if (ballDY < 0) ballDY = -ballDY;
        }
    }
}

void displayLevel2()
{
    glClear(GL_COLOR_BUFFER_BIT);


    drawSky();
    cloud(150,420);
    cloud(300,450);
    cloud(600,430);

    drawSea();

    boat(boat1X, 200);
    boat(boat2X, 200);
    boat(boat3X, 270);
    boat(boat4X, 270);

    drawSand();
    palmTree(120, 140);
    bush(105, 210);
    //palmTree(760, 150);

    bush(60, 80);
    bush(820, 90);

    bush(260, 80);
    bush(1020, 90);

    bush(460, 80);
    bush(1220, 90);

    bush(660, 80);
    bush(1420, 90);


    drawPaddleLevel2();
    drawBallLevel2();
    drawBricksLevel2();

    drawText(10, screenHeight - 30, "Score:");
    drawNumber(score, 80, screenHeight - 30);

    drawText(10, screenHeight - 60, "Lives:");
    drawNumber(lives, 80, screenHeight - 60);

    drawText(screenWidth - 140, screenHeight - 30, "Level:");
    drawNumber(level, screenWidth - 70, screenHeight - 30);

    drawBackButton();




    glFlush();
}


void initLevel2()
{
    glClearColor(0, 0, 0, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, screenWidth, 0, screenHeight);
    initBricksLevel2();
}


void openLevel2()
{
    // Reset Level 2 boats whenever Level 2 starts.
    boat1X = 50;
    boat2X = 400;
    boat3X = 700;
    boat4X = 250;

    // New level session: all previous level timers become invalid.
    timerGeneration++;

    glutDestroyWindow(menuWindow);

    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("DX Ball - Level 2");

    glClearColor(0, 0, 0, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, screenWidth, 0, screenHeight);

    level = 2;
    score = 0;
    lives = 3;
    gameStarted = false;

    initBricksLevel2();
    resetBall();
    glutDisplayFunc(displayLevel2);
    glutTimerFunc(16, updateLevel2, timerGeneration);

    glutPassiveMotionFunc(mouseMotionLevel2);
    glutMouseFunc(mouseButtonLevel2);
    glutKeyboardFunc(backKeyHandler);
}




//LEVEL 3




bool leftLightOn  = false;
bool rightLightOn = false;



float wallWidth = brickWidth * 2;
float wallHeight = brickHeight;

float wall1X = 0;
float wall2X = 600;

float wall1Y;
float wall2Y;

float wall1DX = 3;
float wall2DX = -3;


void drawLabelLevel3(float x, float y, const char* text)
{
    glColor3f(1,1,1);
    glRasterPos2f(x, y);
    for (int i = 0; text[i]; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
}

void drawNumberLevel3(float x, float y, int num)
{
    char d[10];
    int n = 0;
    if (num == 0) d[n++] = '0';
    while (num > 0)
    {
        d[n++] = (num % 10) + '0';
        num /= 10;
    }
    glRasterPos2f(x, y);
    for (int i = n - 1; i >= 0; i--)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, d[i]);
}


void initBricksLevel3()
{
    int butterfly[brickRows][brickCols] =
    {
        {0,0,1,0,0,0,0,1,0,0},
        {0,1,1,1,0,0,1,1,1,0},
        {1,1,1,1,1,1,1,1,1,1},
        {0,1,1,1,1,1,1,1,1,0},
        {0,0,1,1,1,1,1,1,0,0},
        {0,0,0,1,1,1,1,0,0,0},
        {0,0,0,0,1,1,0,0,0,0},
        {0,0,0,0,1,1,0,0,0,0}
    };

    for (int i = 0; i < brickRows; i++)
        for (int j = 0; j < brickCols; j++)
        {
            bricks[i][j] = butterfly[i][j];
            if (bricks[i][j])
                brickColor[i][j] = (i + j) % 5;
        }


    wall1Y = screenHeight - (brickRows + 4) * brickHeight;
    wall2Y = wall1Y - brickHeight;
}


void drawPaddleLevel3()
{
    // White paddle with black outline (stands out on the dark scene)
    drawPaddleShape(1, 1, 1, 0, 0, 0);
}

void drawBallLevel3()
{
    // Bright white ball with black outline so it stays visible
    // even when it passes over bright bricks
    drawBallShape(1, 1, 1, 0, 0, 0);
}

void drawBricks3()
{
    for (int i = 0; i < brickRows; i++)
        for (int j = 0; j < brickCols; j++)
        {
            if (!bricks[i][j]) continue;

            setBrickColorDark(brickColor[i][j]);

            float x = j * brickWidth + 35;
            float y = screenHeight - (i + 3) * brickHeight;

            fillRect(x, y, brickWidth, brickHeight);

            glColor3f(0, 0, 0);
            outlineRect(x, y, brickWidth, brickHeight);
        }
}

void drawWalls()
{
    // Light steel with a dark outline
    glColor3f(0.80f, 0.82f, 0.88f);
    fillRect(wall1X, wall1Y, wallWidth, wallHeight);
    fillRect(wall2X, wall2Y, wallWidth, wallHeight);

    glColor3f(0.0f, 0.0f, 0.0f);
    outlineRect(wall1X, wall1Y, wallWidth, wallHeight);
    outlineRect(wall2X, wall2Y, wallWidth, wallHeight);
}


void drawFloodLights()
{
    if (leftLightOn)
    {
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 1.0f, 0.6f, 0.6f);
        glVertex2f(0, screenHeight / 2);
        glColor4f(1.0f, 1.0f, 0.6f, 0.0f);
        glVertex2f(300, screenHeight);
        glVertex2f(300, 0);
        glEnd();
    }

    if (rightLightOn)
    {
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 1.0f, 0.6f, 0.6f);
        glVertex2f(screenWidth, screenHeight / 2);
        glColor4f(1.0f, 1.0f, 0.6f, 0.0f);
        glVertex2f(screenWidth - 300, 0);
        glVertex2f(screenWidth - 300, screenHeight);
        glEnd();
    }
}

void drawLightFrames()
{
    // Lighter gray frames with a yellow edge, visible on the dark background
    glColor3f(0.55f, 0.55f, 0.60f);
    fillRect(0, screenHeight / 2 - 20, 20, 40);
    fillRect(screenWidth - 20, screenHeight / 2 - 20, 20, 40);

    glColor3f(1.0f, 0.9f, 0.3f);
    outlineRect(0, screenHeight / 2 - 20, 20, 40);
    outlineRect(screenWidth - 20, screenHeight / 2 - 20, 20, 40);
}






void drawGradientBackground()
{
    glBegin(GL_QUADS);


    glColor3f(0.05f, 0.05f, 0.15f);
    glVertex2f(0, screenHeight);
    glVertex2f(screenWidth, screenHeight);


    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(screenWidth, 0);
    glVertex2f(0, 0);

    glEnd();
}

void drawStars()
{
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    glColor3f(0.9f, 0.9f, 1.0f);

    for (int i = 0; i < 80; i++)
    {
        float x = rand() % screenWidth;
        float y = rand() % screenHeight;
        glVertex2f(x, y);
    }

    glEnd();
}

// Dim, cool-toned checker pattern so the bright bricks
// and the white ball clearly stand out in front of it.
void drawCheckerBall(float cx, float cy, float r)
{
    int rings = 10;
    int slices = 20;

    for (int i = 0; i < rings; i++)
    {
        float r1 = r * i / rings;
        float r2 = r * (i + 1) / rings;

        for (int j = 0; j < slices; j++)
        {
            float a1 = 2 * 3.1416f * j / slices;
            float a2 = 2 * 3.1416f * (j + 1) / slices;

            if ((i + j) % 2 == 0)
                glColor3f(0.18f, 0.12f, 0.35f);
            else
                glColor3f(0.10f, 0.10f, 0.22f);

            glBegin(GL_QUADS);
            glVertex2f(cx + r1 * cos(a1), cy + r1 * sin(a1));
            glVertex2f(cx + r2 * cos(a1), cy + r2 * sin(a1));
            glVertex2f(cx + r2 * cos(a2), cy + r2 * sin(a2));
            glVertex2f(cx + r1 * cos(a2), cy + r1 * sin(a2));
            glEnd();
        }
    }
}


bool allBricksBrokenLevel3()
{
    for (int i = 0; i < brickRows; i++)
        for (int j = 0; j < brickCols; j++)
            if (bricks[i][j])
                return false;
    return true;
}


void wallCollision(float wx, float wy)
{
    if (ballX + ballRadius > wx &&
        ballX - ballRadius < wx + wallWidth &&
        ballY + ballRadius > wy &&
        ballY - ballRadius < wy + wallHeight)
    {
        float overlapLeft   = (ballX + ballRadius) - wx;
        float overlapRight  = (wx + wallWidth) - (ballX - ballRadius);
        float overlapTop    = (wy + wallHeight) - (ballY - ballRadius);
        float overlapBottom = (ballY + ballRadius) - wy;

        float minX = (overlapLeft < overlapRight) ? overlapLeft : overlapRight;
        float minY = (overlapTop < overlapBottom) ? overlapTop : overlapBottom;

        if (minX < minY)
        {

            ballDX = -ballDX;
            if (overlapLeft < overlapRight)
                ballX = wx - ballRadius;
            else
                ballX = wx + wallWidth + ballRadius;
        }
        else
        {

            ballDY = -ballDY;
            if (overlapBottom < overlapTop)
                ballY = wy - ballRadius;
            else


                ballY = wy + wallHeight + ballRadius;
        }


        ballX += ballDX * 0.1f;
        ballY += ballDY * 0.1f;
    }
}


void updateLevel3(int value)
{
    // If this timer belongs to an old level/window, stop it permanently.
    if (value != timerGeneration)
        return;

    if (gameStarted)
    {
        ballX += ballDX;
        ballY += ballDY;
    }
    else
    {
        ballX = paddleX + paddleWidth / 2;
        ballY = paddleY + paddleHeight + ballRadius;
    }

    wall1X += wall1DX;
    wall2X += wall2DX;

    if (wall1X <= 0 || wall1X + wallWidth >= screenWidth)
        wall1DX = -wall1DX;
    if (wall2X <= 0 || wall2X + wallWidth >= screenWidth)
        wall2DX = -wall2DX;

    if (ballX - ballRadius <= 0 || ballX + ballRadius >= screenWidth)
        ballDX = -ballDX;
    if (ballY + ballRadius >= screenHeight)
        ballDY = -ballDY;

    if (ballDY < 0 &&
        ballY - ballRadius <= paddleY + paddleHeight &&
        ballX >= paddleX &&
        ballX <= paddleX + paddleWidth)
        ballDY = -ballDY;

        wallCollision(wall1X, wall1Y);
        wallCollision(wall2X, wall2Y);


    for (int i = 0; i < brickRows; i++)
        for (int j = 0; j < brickCols; j++)
        {
            if (!bricks[i][j]) continue;

            float bx = j * brickWidth + 35;
            float by = screenHeight - (i + 3) * brickHeight;

            if (ballX + ballRadius > bx &&
                ballX - ballRadius < bx + brickWidth &&
                ballY + ballRadius > by &&
                ballY - ballRadius < by + brickHeight)
            {
                bricks[i][j] = false;
                score += 5;
                ballDY = -ballDY;
                goto done;
            }
        }

done:


    if (allBricksBrokenLevel3())
    {
        char msg[150];
        wsprintf(msg, "Congratulations! You cleared Level 3!\nYour Score: %d\n\nPlay this level again?\n(Yes = Play Again, No = Return to Menu)", score);
        int choice = MessageBox(0, msg, "Level Complete!", MB_YESNO | MB_ICONINFORMATION);

        score = 0;
        lives = 3;
        gameStarted = false;

        if (choice == IDYES)
        {
            initBricksLevel3();
            resetBall();
        }
        else
        {
            goToMenu();
            return;
        }
    }


    if (ballY - ballRadius < 0)
    {
        lives--;

        if (lives > 0)
        {
            char msg[100];
            wsprintf(msg, "You lost a life!\nLives remaining: %d", lives);
            MessageBox(0, msg, "DX Ball", MB_OK | MB_ICONWARNING);

            resetBall();
        }
        else
        {
            char msg[100];
            wsprintf(msg, "Game Over!\nYour Score: %d", score);
            MessageBox(0, msg, "DX Ball", MB_OK | MB_ICONWARNING);

            score = 0;
            lives = 3;
            gameStarted = false;
            goToMenu();
            return;
        }
    }


    glutPostRedisplay();
    glutTimerFunc(16, updateLevel3, timerGeneration);

}


void mouseMotion3(int x, int)
{
    paddleX = x - paddleWidth / 2;
    if (paddleX < 0) paddleX = 0;
    if (paddleX + paddleWidth > screenWidth)
        paddleX = screenWidth - paddleWidth;
}

void mouseButton3(int b, int s, int x, int y)
{
    if (b == GLUT_LEFT_BUTTON && s == GLUT_DOWN)
    {
        if (isBackButtonClicked(x, y))
        {
            goToMenu();
            return;
        }

        int my = screenHeight - y;


        if (x >= 20 && x <= 80 && my >= 20 && my <= 60)
            leftLightOn = !leftLightOn;


        if (x >= screenWidth - 80 && x <= screenWidth - 20 &&
            my >= 20 && my <= 60)
            rightLightOn = !rightLightOn;

        if (!gameStarted)
            gameStarted = true;
    }
}


void displayLevel3()
{
    glClear(GL_COLOR_BUFFER_BIT);



    drawGradientBackground();
    drawStars();
    drawCheckerBall(screenWidth / 2, screenHeight * 0.65f, 80);


    drawPaddleLevel3();
    drawBallLevel3();
    drawBricks3();
    drawWalls();
    drawFloodLights();

    drawLightFrames();


    drawLabelLevel3(10, screenHeight - 30, "Score: ");
    drawNumberLevel3(80, screenHeight - 30, score);

    drawLabelLevel3(10, screenHeight - 60, "Lives: ");
    drawNumberLevel3(80, screenHeight - 60, lives);

    drawLabelLevel3(screenWidth - 140, screenHeight - 30, "Level:  3");
    //drawNumberLevel3(screenWidth - 70, screenHeight - 30, level);

    drawBackButton();

    glFlush();
}



void initLevel3()
{
    glClearColor(0,0,0,1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, screenWidth, 0, screenHeight);
    initBricksLevel3();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == 'k' || key == 'K')
    {
        bool newState = !(leftLightOn || rightLightOn);
        leftLightOn  = newState;
        rightLightOn = newState;

        glutPostRedisplay();

    }

    // Esc also returns to the menu from Level 3
    backKeyHandler(key, x, y);
}



void openLevel3()
{
    // New level session: all previous level timers become invalid.
    timerGeneration++;

    glutDestroyWindow(menuWindow);

    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("DX Ball - Level 3");

    level = 3;
    score = 0;
    lives = 3;
    gameStarted = false;

    initLevel3();
    resetBall();

    glutDisplayFunc(displayLevel3);
    glutTimerFunc(16, updateLevel3, timerGeneration);
    glutPassiveMotionFunc(mouseMotion);
    glutMouseFunc(mouseButton3);
    glutKeyboardFunc(keyboard);
}




void menuDisplay()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Coloured buttons that match the click areas in menuMouse()
    // LEVEL 1 - blue
    glColor3f(0.05f, 0.30f, 0.75f);
    fillRect(330, 400, 140, 30);
    // LEVEL 2 - teal
    glColor3f(0.00f, 0.45f, 0.35f);
    fillRect(330, 340, 140, 30);
    // LEVEL 3 - purple
    glColor3f(0.40f, 0.15f, 0.65f);
    fillRect(330, 280, 140, 30);
    // EXIT - red
    glColor3f(0.70f, 0.10f, 0.10f);
    fillRect(330, 220, 140, 30);

    // White borders
    glColor3f(1, 1, 1);
    outlineRect(330, 400, 140, 30);
    outlineRect(330, 340, 140, 30);
    outlineRect(330, 280, 140, 30);
    outlineRect(330, 220, 140, 30);

    // White labels
    glColor3f(1, 1, 1);
    drawText(350, 410, GLUT_BITMAP_8_BY_13, "LEVEL 1");
    drawText(350, 350, GLUT_BITMAP_8_BY_13, "LEVEL 2");
    drawText(350, 290, GLUT_BITMAP_8_BY_13, "LEVEL 3");
    drawText(350, 230, GLUT_BITMAP_8_BY_13, "EXIT");

    glFlush();
}

void menuMouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        int my = screenHeight - y;

        if (x >= 330 && x <= 470)
        {
            if (my >= 400 && my <= 430)
                openLevel1();
            else if (my >= 340 && my <= 370)
                {
                    openLevel2();
                }

            else if (my >= 280 && my <= 310)
            {
                openLevel3();
            }

            else if (my >= 220 && my <= 250)
                exit(0);
        }
    }
}



int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(screenWidth, screenHeight);

    startWindow = glutCreateWindow("Magic Ball");

    glClearColor(0.02f, 0.03f, 0.12f, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, screenWidth, 0, screenHeight);

    glutDisplayFunc(startDisplay);
    glutMouseFunc(startMouse);

    glutMainLoop();
    return 0;
}
