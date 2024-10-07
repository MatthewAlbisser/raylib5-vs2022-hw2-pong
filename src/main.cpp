#include "raylib.h"
#include "Math.h"
#include <thread>   // Included after looking for a way to hold .

constexpr float SCREEN_WIDTH = 1200.0f;
constexpr float SCREEN_HEIGHT = 800.0f;
constexpr Vector2 CENTER{ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f };

// Ball can move half the screen width per-second
constexpr float BALL_SPEED = SCREEN_WIDTH * 0.5f;
constexpr float BALL_SIZE = 40.0f;

// Paddles can move half the screen height per-second
constexpr float PADDLE_SPEED = SCREEN_HEIGHT * 0.5f;
constexpr float PADDLE_WIDTH = 40.0f;
constexpr float PADDLE_HEIGHT = 80.0f;

struct Box
{
    float xMin;
    float xMax;
    float yMin;
    float yMax;
};

bool BoxOverlap(Box box1, Box box2)
{
    bool x = box1.xMax >= box2.xMin && box1.xMin <= box2.xMax;
    bool y = box1.yMax >= box2.yMin && box1.yMin <= box2.yMax;
    return x && y;
}

Rectangle BoxToRec(Box box)
{
    Rectangle rec;
    rec.x = box.xMin;
    rec.y = box.yMin;
    rec.width = box.xMax - box.xMin;
    rec.height = box.yMax - box.yMin;
    return rec;
}

Box BallBox(Vector2 position)
{
    Box box;
    box.xMin = position.x - BALL_SIZE * 0.5f;
    box.xMax = position.x + BALL_SIZE * 0.5f;
    box.yMin = position.y - BALL_SIZE * 0.5f;
    box.yMax = position.y + BALL_SIZE * 0.5f;
    return box;
}

Box PaddleBox(Vector2 position)
{
    Box box;
    box.xMin = position.x - PADDLE_WIDTH * 0.5f;
    box.xMax = position.x + PADDLE_WIDTH * 0.5f;
    box.yMin = position.y - PADDLE_HEIGHT * 0.5f;
    box.yMax = position.y + PADDLE_HEIGHT * 0.5f;
    return box;
}

void ResetBall(Vector2& position, Vector2& direction)
{
    position = CENTER;
    direction.x = rand() % 2 == 0 ? -1.0f : 1.0f;
    direction.y = 0.0f;
    direction = Rotate(direction, Random(0.0f, 60.0f) * DEG2RAD);   // Changed float 360 to 60; dont need another direction flip.
}                                                                   // fixes stuck-in-center bug; ball has no X-axis movement.

void DrawBall(Vector2 position, Color color)
{
    Box ballBox = BallBox(position);
    DrawRectangleRec(BoxToRec(ballBox), color);
}

void DrawPaddle(Vector2 position, Color color)
{
    Box paddleBox = PaddleBox(position);
    DrawRectangleRec(BoxToRec(paddleBox), color);
}

int main()
{
    Vector2 ballPosition;
    Vector2 ballDirection;
    ResetBall(ballPosition, ballDirection);

    Vector2 paddle1Position, paddle2Position;
    paddle1Position.x = SCREEN_WIDTH * 0.05f;
    paddle2Position.x = SCREEN_WIDTH * 0.95f;
    paddle1Position.y = paddle2Position.y = CENTER.y;

    static int player1Points = 0;  // Local variable for player 1 points.
    static int player2Points = 0;  // Local variable for player 2 points.

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong");
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        float ballDelta = BALL_SPEED * dt;
        float paddleDelta = PADDLE_SPEED * dt;

        // Move paddle with key input
        if (IsKeyDown(KEY_W))
            paddle1Position.y -= paddleDelta;
        if (IsKeyDown(KEY_S))
            paddle1Position.y += paddleDelta;
        if (IsKeyDown(KEY_E))                   // Second player controls, because its much more fun with TWO players. 
            paddle2Position.y -= paddleDelta;
        if (IsKeyDown(KEY_D))
            paddle2Position.y += paddleDelta;

        // Mirror paddle 1 for now
        // paddle2Position.y = paddle1Position.y;   // Replaced for second player controls

        float phh = PADDLE_HEIGHT * 0.5f;
        paddle1Position.y = Clamp(paddle1Position.y, phh, SCREEN_HEIGHT - phh);
        paddle2Position.y = Clamp(paddle2Position.y, phh, SCREEN_HEIGHT - phh);

        // Change the ball's direction on-collision
        Vector2 ballPositionNext = ballPosition + ballDirection * ballDelta;
        Box ballBox = BallBox(ballPositionNext);
        Box paddle1Box = PaddleBox(paddle1Position);
        Box paddle2Box = PaddleBox(paddle2Position);

        if (ballBox.xMin < 0.0f || ballBox.xMax > SCREEN_WIDTH)
            // ballDirection.x *= -1.0f;            // Commented out so ball reset function can work without issue. 
            ResetBall(ballPosition, ballDirection); // Calls the ball reset function using local Vectors.

        if (ResetBall && ballBox.xMax > SCREEN_WIDTH)   // If ball is on the right when ball resets...
            player1Points += 1,                         // 1 point to player 1.
            paddle1Position.y = CENTER.y,               // Paddle 1 resets position.
            paddle2Position.y = CENTER.y;               // Paddle 2 resets position.
        if (ResetBall && ballBox.xMin < 0.0f)           // If ball is on the left when ball resets...
            player2Points += 1,                         // 1 point to player 2.
            paddle1Position.y = CENTER.y,               // Paddle 1 resets position.
            paddle2Position.y = CENTER.y;               // Paddle 2 resets position.

        if (player1Points == 5)
            DrawText("Player One Wins!", 250, 250, 100, LIME),
            player1Points = 0,
            player2Points = 0,
            EndDrawing(),
            std::this_thread::sleep_for(std::chrono::seconds(3)),
            exit(0);
        if (player2Points == 5)
            DrawText("Player Two Wins!", 250, 250, 100, LIME),
            player1Points = 0,
            player2Points = 0,
            EndDrawing(),
            std::this_thread::sleep_for(std::chrono::seconds(3)),
            exit(0);

        if (ballBox.yMin < 0.0f || ballBox.yMax > SCREEN_HEIGHT)
            ballDirection.y *= -1.0f;
        if (BoxOverlap(ballBox, paddle1Box) || BoxOverlap(ballBox, paddle2Box))
            ballDirection.x *= -1.0f;

        // Update ball position after collision resolution, then render
        ballPosition = ballPosition + ballDirection * ballDelta;

        BeginDrawing();
        ClearBackground(BLACK);
        DrawText(TextFormat("Player One: %i", player1Points), 20, 10, 20, GRAY);
        DrawText(TextFormat("Player Two: %i", player2Points), 1050, 10, 20, GRAY);
        DrawBall(ballPosition, WHITE);
        DrawPaddle(paddle1Position, WHITE);
        DrawPaddle(paddle2Position, WHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
