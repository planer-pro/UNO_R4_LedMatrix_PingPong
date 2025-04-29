#include <Arduino.h>
#include <UNOR4WMatrixGFX.h>

// Initialize the LED matrix display
UNOR4WMatrixGFX matrix;
// Create a canvas for rendering text (96x7 pixels, 1-bit)
GFXcanvas8 canvas(96, 7); // Used for any messages

// Variables for the ball's position and movement
int ballX, ballY; // Ball coordinates (x, y)
int dx, dy;       // Ball direction/speed (x and y increments)

// Variables for the paddle
int paddleX = 4;           // Initial paddle position (centered)
const int paddleWidth = 3; // Paddle width in pixels

// Pin assignments for buttons (swapped for left/right control)
const int leftButtonPin = 3;  // Pin for left movement button
const int rightButtonPin = 2; // Pin for right movement button

// Game counters
int missedBalls = 0; // Tracks missed balls
int score = 0;       // Tracks successful paddle hits

// Game speed control
const int delayTime = 200; // Delay between game updates (ms)

// Initial ball speed
const int initialBallDx = 1; // Initial x-direction speed
const int initialBallDy = 1; // Initial y-direction speed

// Paddle movement step per cycle
const int paddleStep = 1; // Pixels to move paddle per button press

// Function prototypes
void resetBall();                                                     // Reset ball to starting position
void displayMessage(String message);                                  // Display any message
void updateBall();                                                    // Update ball position and collisions
void drawGame();                                                      // Draw ball and paddle on matrix
void handlePaddle();                                                  // Handle paddle movement via buttons
void writeOffsetRect(GFXcanvas8 &canvas, int x_offset, int y_offset); // Draw canvas portion on matrix

void setup()
{
    // Initialize Serial communication for debugging
    Serial.begin(115200);
    Serial.println("Game started!");

    // Seed random number generator using analog noise
    randomSeed(analogRead(0));

    // Set initial ball position and direction
    resetBall();

    // Initialize the LED matrix
    matrix.begin();

    // Configure button pins with internal pull-up resistors
    pinMode(leftButtonPin, INPUT_PULLUP);
    pinMode(rightButtonPin, INPUT_PULLUP);

    displayMessage("Welcome to Pong!"); // Show "Welcome to Pong!" message
    displayMessage("Round 1");          // Show "Round 1" message
}

void loop()
{
    // Handle paddle movement based on button input
    handlePaddle();

    // Update ball position and check for collisions
    updateBall();

    // Render the game state on the matrix
    drawGame();

    // Control game speed with a delay
    delay(delayTime);
}

// Handle paddle movement based on button presses
void handlePaddle()
{
    // Move paddle left if button pressed and not at left edge
    if (!digitalRead(leftButtonPin) && paddleX > 0)
    {
        paddleX -= paddleStep;
    }
    // Move paddle right if button pressed and not at right edge
    if (!digitalRead(rightButtonPin) && paddleX < 12 - paddleWidth)
    {
        paddleX += paddleStep;
    }
}

// Update ball position and handle collisions
void updateBall()
{
    // Update ball position
    ballX += dx;
    ballY += dy;

    // Collision with left wall
    if (ballX < 0)
    {
        ballX = 0; // Keep ball within bounds
        dx = -dx;  // Reverse x-direction
    }
    // Collision with right wall
    if (ballX > 11)
    {
        ballX = 11; // Keep ball within bounds
        dx = -dx;   // Reverse x-direction
    }

    // Collision with top wall
    if (ballY < 0)
    {
        ballY = 0; // Keep ball within bounds
        dy = -dy;  // Reverse y-direction
    }

    // Check for paddle collision at row 6 (just above paddle)
    if (ballY == 6 && ballX >= paddleX && ballX < paddleX + paddleWidth)
    {
        dy = -dy; // Reverse y-direction (bounce up)
        score++;  // Increment score for successful hit
        Serial.print("Score: ");
        Serial.println(score);
    }

    // Check if ball passes paddle (missed)
    if (ballY > 7)
    {
        missedBalls++; // Increment missed balls counter
        Serial.print("Missed balls: ");
        Serial.println(missedBalls);
        displayMessage("Lose"); // Show "You Lose!" message
        resetBall();            // Reset ball for new round
    }
}

// Draw the game state (ball and paddle) on the matrix
void drawGame()
{
    // Clear the matrix display
    matrix.clearDisplay();

    // Draw the ball at its current position
    matrix.drawPixel(ballX, ballY, 1);

    // Draw the paddle at row 7
    for (int i = 0; i < paddleWidth; i++)
    {
        matrix.drawPixel(paddleX + i, 7, 1);
    }

    // Update the matrix to show the new frame
    matrix.display();
}

// Reset the ball to a random starting position
void resetBall()
{
    ballX = random(0, 12);                       // Random x-position (0-11)
    ballY = random(0, 4);                        // Random y-position in top half (0-3)
    dx = initialBallDx * (random(0, 2) * 2 - 1); // Random x-direction (-1 or 1)
    dy = initialBallDy;                          // Always move downward initially
}

// Display "You Lose!" message with scrolling effect
void displayMessage(String message)
{
    // Clear the canvas (not the display)
    canvas.fillScreen(0);

    // Set text properties for "You Lose!" message
    canvas.setCursor(0, 0);            // Set cursor to top-left (baseline for fonts)
    canvas.setTextSize(1);             // 1:1 pixel scale
    canvas.setTextColor(MATRIX_WHITE); // White text
    canvas.print(message);             // Write message to canvas

    // Get the x-position after text (width of text)
    uint8_t canvas_max_x = canvas.getCursorX();

    int x_offset = -11; // Start at right edge of canvas
    int y_offset = 0;

    // Scroll text horizontally across the matrix
    for (x_offset = -11; x_offset < canvas_max_x; x_offset++)
    {
        matrix.clearDisplay(); // Clear matrix for new frame

        // Draw portion of canvas on matrix
        writeOffsetRect(canvas, x_offset, y_offset);

        // Update matrix display
        matrix.display();

        // Delay for smooth scrolling effect
        delay(25);
    }
}

// Draw a portion of the canvas on the matrix at an offset
void writeOffsetRect(GFXcanvas8 &canvas, int x_offset, int y_offset)
{
    matrix.clearDisplay(); // Clear matrix for new frame
    // Loop through matrix pixels (12x8)
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 12; x++)
        {
            // Get pixel color from canvas at offset position
            uint8_t color;
            if ((color = canvas.getPixel(x + x_offset, y + y_offset)))
            {
                // Draw pixel on matrix if color is non-zero
                matrix.drawPixel(x, y, color);
            }
        }
    }
}