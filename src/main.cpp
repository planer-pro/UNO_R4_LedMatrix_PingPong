#include <Arduino.h>
#include <UNOR4WMatrixGFX.h>

// Constants
#define MAX_ROUNDS 10         // rounds until win
#define MAX_MISSES 10         // misses until game over
#define RESTART_DELAY 1000    // ms full-screen display on restart
#define PADDLE_ANIM_DELAY 200 // ms delay for paddle animation before round restart
#define INITIAL_PADDLE_X 4    // starting X position of paddle
#define PADDLE_WIDTH 4        // paddle width in pixels
#define OBSTACLE_ROWS 5       // number of top rows for obstacles

// Initialize the LED matrix display
UNOR4WMatrixGFX matrix;
// Canvas for rendering text
GFXcanvas8 canvas(96, 7);

// Ball state
int ballX, ballY;
int dx, dy;

// Obstacles
int roundNumber = 1;
int obstacleX[MAX_ROUNDS];
int obstacleY[MAX_ROUNDS];
bool obstacleActive[MAX_ROUNDS];
int obstaclesHit = 0; // count of obstacles hit

// Paddle
int paddleX = INITIAL_PADDLE_X;
int score = 0; // hits by paddle

// Missed balls
int missedBalls = 0;

// Game state
enum GameState
{
    RUNNING,
    GAME_OVER,
    WIN
};
GameState gameState = RUNNING;

// Timing control
const int delayTime = 100;   // Delay between game updates (ms)
const int initialBallDx = 1; // initial ball dx
const int initialBallDy = 1; // initial ball dy
const int paddleStep = 1;    // pixels to move paddle per press

// Function prototypes
void resetBall();
void displayMessage(const String &message);
void updateBall();
void drawGame();
void handlePaddle();
void writeOffsetRect(GFXcanvas8 &c, int x_offset, int y_offset);
void initObstacles();
void startRound();
void animatePaddle();

void setup()
{
    Serial.begin(115200);
    randomSeed(analogRead(0));
    matrix.begin();
    pinMode(3, INPUT_PULLUP);
    pinMode(2, INPUT_PULLUP);

    // Initial game start message
    Serial.println("Game started!");
    displayMessage("Welcome to Pong!");
    startRound();
}

void loop()
{
    if (gameState == RUNNING)
    {
        handlePaddle();
        updateBall();
        drawGame();
        delay(delayTime);
    }
    else
    {
        // End state: show message until both buttons pressed
        if (gameState == GAME_OVER)
        {
            while (true)
            {
                displayMessage("Game Over!");
                if (!digitalRead(3) && !digitalRead(2))
                    break;
            }
        }
        else
        {
            while (true)
            {
                displayMessage("You Win!");
                if (!digitalRead(3) && !digitalRead(2))
                    break;
            }
        }
        // Flash full screen before restart
        matrix.clearDisplay();
        for (int y = 0; y < 8; y++)
        {
            for (int x = 0; x < 12; x++)
            {
                matrix.drawPixel(x, y, 1);
            }
        }
        matrix.display();
        delay(RESTART_DELAY);

        // Restart game state
        roundNumber = 1;
        missedBalls = 0;
        score = 0;
        obstaclesHit = 0;
        gameState = RUNNING;
        startRound();
    }
}

void handlePaddle()
{
    if (!digitalRead(3) && paddleX > 0)
        paddleX -= paddleStep;
    if (!digitalRead(2) && paddleX < 12 - PADDLE_WIDTH)
        paddleX += paddleStep;
}

void updateBall()
{
    // Pre-move obstacle check for bounce
    int nextX = ballX + dx;
    int nextY = ballY + dy;
    for (int i = 0; i < roundNumber; i++)
    {
        if (obstacleActive[i] && nextX == obstacleX[i] && nextY == obstacleY[i])
        {
            obstacleActive[i] = false;
            dy = -dy; // bounce vertically
            obstaclesHit++;
            Serial.print("Obstacles hit: ");
            Serial.println(obstaclesHit);
            ballX += dx;
            ballY += dy;
            return;
        }
    }

    // Normal movement
    ballX += dx;
    ballY += dy;

    // Check all cleared
    bool allCleared = true;
    for (int i = 0; i < roundNumber; i++)
    {
        if (obstacleActive[i])
        {
            allCleared = false;
            break;
        }
    }
    if (allCleared)
    {
        roundNumber++;
        missedBalls = 0;
        if (roundNumber <= MAX_ROUNDS)
        {
            startRound();
        }
        else
        {
            gameState = WIN;
        }
        return;
    }

    // Wall collisions
    if (ballX < 0)
    {
        ballX = 0;
        dx = -dx;
    }
    if (ballX > 11)
    {
        ballX = 11;
        dx = -dx;
    }
    if (ballY < 0)
    {
        ballY = 0;
        dy = -dy;
    }

    // Paddle collision
    if (ballY == 6 && ballX >= paddleX && ballX < paddleX + PADDLE_WIDTH)
    {
        dy = -dy;
        score++;
        Serial.print("Paddle hits: ");
        Serial.println(score);
    }

    // Missed paddle
    if (ballY > 7)
    {
        missedBalls++;
        if (missedBalls >= MAX_MISSES)
        {
            gameState = GAME_OVER;
        }
        else
        {
            displayMessage("Loose " + String(missedBalls) + "/" + String(MAX_MISSES));
            resetBall();
            // reset and animate paddle
            paddleX = INITIAL_PADDLE_X;
            animatePaddle();
            // obstacles remain as cleared
        }
    }
}

void drawGame()
{
    matrix.clearDisplay();
    for (int i = 0; i < roundNumber; i++)
    {
        if (obstacleActive[i])
            matrix.drawPixel(obstacleX[i], obstacleY[i], 1);
    }
    matrix.drawPixel(ballX, ballY, 1);
    for (int i = 0; i < PADDLE_WIDTH; i++)
        matrix.drawPixel(paddleX + i, 7, 1);
    matrix.display();
}

void resetBall()
{
    ballX = random(0, 12);
    ballY = random(0, 4);
    dx = initialBallDx * (random(0, 2) * 2 - 1);
    dy = initialBallDy;
}

void initObstacles()
{
    for (int i = 0; i < roundNumber; i++)
    {
        obstacleX[i] = random(0, 12);
        obstacleY[i] = random(0, OBSTACLE_ROWS);
        obstacleActive[i] = true;
    }
}

void startRound()
{
    displayMessage("Round " + String(roundNumber) + "/" + String(MAX_ROUNDS));
    // reset and animate paddle
    paddleX = INITIAL_PADDLE_X;
    animatePaddle();
    initObstacles();
    resetBall();
}

void animatePaddle()
{
    for (int i = 0; i < PADDLE_WIDTH; i++)
    {
        matrix.clearDisplay();
        for (int j = 0; j <= i; j++)
        {
            matrix.drawPixel(paddleX + j, 7, 1);
        }
        matrix.display();
        delay(PADDLE_ANIM_DELAY);
    }
}

void displayMessage(const String &message)
{
    canvas.fillScreen(0);
    canvas.setCursor(0, 0);
    canvas.setTextSize(1);
    canvas.setTextColor(MATRIX_WHITE);
    canvas.print(message);
    uint8_t maxX = canvas.getCursorX();
    for (int x_offset = -11; x_offset < maxX; x_offset++)
    {
        matrix.clearDisplay();
        writeOffsetRect(canvas, x_offset, 0);
        matrix.display();
        delay(25);
    }
}

void writeOffsetRect(GFXcanvas8 &c, int x_offset, int y_offset)
{
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 12; x++)
            if (c.getPixel(x + x_offset, y + y_offset))
                matrix.drawPixel(x, y, 1);
}
