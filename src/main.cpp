#include <Arduino.h>
#include <UNOR4WMatrixGFX.h>

// Constants
#define MAX_ROUNDS 10 // rounds until win
#define MAX_MISSES 10 // misses until game over

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
int paddleX = 4;
const int paddleWidth = 3;
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
const int delayTime = 200; // Delay between game updates (ms)
const int initialBallDx = 1;
const int initialBallDy = 1;
const int paddleStep = 1;

// Function prototypes
void resetBall();
void displayMessage(const String &message);
void updateBall();
void drawGame();
void handlePaddle();
void writeOffsetRect(GFXcanvas8 &c, int x_offset, int y_offset);
void initObstacles();
void startRound();

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
        // End state: show message
        if (gameState == GAME_OVER)
        {
            while (true)
            {
                displayMessage("Game Over!");
                if (!digitalRead(3) && !digitalRead(2))
                    break;
            }
        }
        else if (gameState == WIN)
        {
            while (true)
            {
                displayMessage("You Win!");
                if (!digitalRead(3) && !digitalRead(2))
                    break;
            }
        }
        // Restart
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
    if (!digitalRead(2) && paddleX < 12 - paddleWidth)
        paddleX += paddleStep;
}

void updateBall()
{
    ballX += dx;
    ballY += dy;

    // Obstacle collision
    for (int i = 0; i < roundNumber; i++)
    {
        if (obstacleActive[i] && ballX == obstacleX[i] && ballY == obstacleY[i])
        {
            obstacleActive[i] = false;
            dy = -dy; // bounce
            break;
        }
    }
    // Check if all cleared => next round
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
    if (ballY == 6 && ballX >= paddleX && ballX < paddleX + paddleWidth)
    {
        dy = -dy;
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
            // Display Loose message with count
            displayMessage("Loose " + String(missedBalls) + "/" + String(MAX_MISSES));
            resetBall();
            initObstacles();
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
    for (int i = 0; i < paddleWidth; i++)
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
        obstacleY[i] = random(0, 3);
        obstacleActive[i] = true;
    }
}

void startRound()
{
    displayMessage("Round " + String(roundNumber) + "/" + String(MAX_ROUNDS));
    initObstacles();
    resetBall();
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
