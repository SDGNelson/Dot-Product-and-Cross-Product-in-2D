#include "raylib.h"
#include "raymath.h"

// https://tauday.com/tau-manifesto
#define TAU (PI * 2.0f)

// Remap to [0, PI). Is this the correct name?
float NormalizeAngle(float angle)
{
    angle = fmodf(angle, TAU);
    return angle < 0.0f ? TAU + angle : angle;
}

// Calculate smallest angle in radians from angle1 to angle2. This means the result is always (-PI, PI].
float AngleBetween(float angle1, float angle2)
{
	angle1 = NormalizeAngle(angle1);
	angle2 = NormalizeAngle(angle2);

    if (angle2 < angle1)
    {
        float positive = TAU - angle1 + angle2;
		float negative = angle2 - angle1;
		return positive < -negative ? positive : negative;
    }
    else if (angle2 > angle1)
    {
		float positive = angle2 - angle1;
		float negative = TAU - angle2 + angle1;
		return positive < negative ? positive : -negative;
    }
    else
    {
        return 0.0f;
    }
}

// Math-y folks might not like calling this the 2D cross product.
// We are treating the 2D vectors as if they were 3D with a Z value of 0.0 and then just calculating the Z component of the 3D cross product.
// (if you look at Vector3CrossProduct the Z value is the same)
float Vector2CrossProduct(Vector2 v1, Vector2 v2)
{
    return v1.x * v2.y - v1.y * v2.x;
}

// Entry point.
int main()
{
    Vector2 lineStarts[2] =
    {
        { 210.0f, 200.0f },
        { 230.0f, 260.0f }
    };
    Vector2 lineEnds[2] =
    {
        { 230.0f, 220.0f },
		{ 290.0f, 250.0f }
    };

    Color lineColors[2] =
    {
        RED,
        BLUE,
    };
    Color dotProductColor = GOLD; // Not green on red arrow to help red-green colorblind viewers.
    Color crossProductColor = DARKGREEN;
    
	bool shouldDrawDotProductProjection = false;
	bool shouldDrawCrossProductProjection = false;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(500);
    InitWindow(600, 600, "Dot Product and Cross Product in 2D");

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        if (IsKeyDown(KEY_Q))
        {
            lineStarts[0] = GetMousePosition();
        }
		if (IsKeyDown(KEY_W))
		{
			lineEnds[0] = GetMousePosition();
		}
		if (IsKeyDown(KEY_E))
		{
            lineStarts[1] = GetMousePosition();
		}
		if (IsKeyDown(KEY_R))
		{
			lineEnds[1] = GetMousePosition();
		}
        if (IsKeyPressed(KEY_T))
        {
            shouldDrawDotProductProjection = !shouldDrawDotProductProjection;
		}
		if (IsKeyPressed(KEY_Y))
		{
			shouldDrawCrossProductProjection = !shouldDrawCrossProductProjection;
		}

		Vector2 normals[2];
        Vector2 clockwiseTangents[2];
        float angles[2]; // Counter-clockwise radians [0, TAU).
        for (int index = 0; index < 2; ++index)
        {
            Vector2 endRelativeToStart = Vector2Subtract(lineEnds[index], lineStarts[index]);
            normals[index] = endRelativeToStart;
			float lineLength = Vector2Length(endRelativeToStart);
            if (lineLength > 0.0f)
            {
                normals[index] = Vector2Scale(normals[index], 1.0f / lineLength);
            }
            clockwiseTangents[index].x = -normals[index].y;
            clockwiseTangents[index].y = normals[index].x;

            // Drawing is Y-down (relative to upper-left corner of the window), so we flip the Y axis to make positive angles turn
            // counter-clockwise, and remap the [-PI, PI] angle to [0, TAU) for user friendliness.
            angles[index] = NormalizeAngle(atan2f(-endRelativeToStart.y, endRelativeToStart.x));

            // Small center circle.
			DrawCircleV(lineStarts[index], 4.0f, lineColors[index]);
			
            // Line and arrowhead.
            DrawLineEx(lineStarts[index], lineEnds[index], 2.0f, lineColors[index]);
            DrawLineEx(lineEnds[index], Vector2Add(lineEnds[index], Vector2Add(Vector2Scale(normals[index], -8.0f), Vector2Scale(clockwiseTangents[index], 8.0f))), 2.0f, lineColors[index]);
			DrawLineEx(lineEnds[index], Vector2Add(lineEnds[index], Vector2Add(Vector2Scale(normals[index], -8.0f), Vector2Scale(clockwiseTangents[index], -8.0f))), 2.0f, lineColors[index]);
            
            // Arc illustrating counter-clockwise angle [0, TAU). Not the most efficient approach but c'est la vie.
            DrawRingLines(lineStarts[index], 0.0f, lineLength * 0.5f, 90.0f, angles[index] * RAD2DEG + 90.0f, 0, lineColors[index]);
        }

		if (shouldDrawDotProductProjection)
		{
            // Dot product of (normal, vector) is 1D closest position of vector along normal.
			Vector2 blueStartRelativeToRedStart = Vector2Subtract(lineStarts[1], lineStarts[0]);
			float projectionAlongNormal = Vector2DotProduct(normals[0], blueStartRelativeToRedStart);
			Vector2 pointAlongNormal = Vector2Add(lineStarts[0], Vector2Scale(normals[0], projectionAlongNormal));
			DrawLineEx(lineStarts[0], pointAlongNormal, 2.0f, dotProductColor);
			DrawLineEx(Vector2Add(pointAlongNormal, Vector2Scale(clockwiseTangents[0], -4.0f)), Vector2Add(pointAlongNormal, Vector2Scale(clockwiseTangents[0], 4.0f)), 2.0f, dotProductColor);
		}

		if (shouldDrawCrossProductProjection)
		{
			// Cross product of (normal, vector) is 1D closest position of vector along tangent perpendicular to normal.
			Vector2 blueStartRelativeToRedStart = Vector2Subtract(lineStarts[1], lineStarts[0]);
			float projectionAlongClockwiseTangent = Vector2CrossProduct(normals[0], blueStartRelativeToRedStart);
			Vector2 pointAlongClockwiseTangent = Vector2Add(lineStarts[0], Vector2Scale(clockwiseTangents[0], projectionAlongClockwiseTangent));
			DrawLineEx(lineStarts[0], pointAlongClockwiseTangent, 2.0f, crossProductColor);
			DrawLineEx(Vector2Add(pointAlongClockwiseTangent, Vector2Scale(normals[0], -4.0f)), Vector2Add(pointAlongClockwiseTangent, Vector2Scale(normals[0], 4.0f)), 2.0f, crossProductColor);
		}

        // Draw HUD

        float angleDelta = AngleBetween(angles[0], angles[1]);

        const int fontSize = 30;
        int textPosY = 10;
        DrawText(TextFormat("Red angle: %.0f deg (%.2f rad) cos: %.2f sin: %.2f", angles[0] * RAD2DEG, angles[0], cosf(angles[0]), sinf(angles[0])), 10, textPosY, fontSize, lineColors[0]);
		DrawText(TextFormat("Blue angle: %.0f deg (%.2f rad) cos: %.2f sin: %.2f", angles[1] * RAD2DEG, angles[1], cosf(angles[1]), sinf(angles[1])), 10, textPosY += fontSize, fontSize, lineColors[1]);
		DrawText(TextFormat("Angle delta: %.0f deg (%.2f rad) cos: %.2f sin: %.2f", angleDelta * RAD2DEG, angleDelta, cosf(angleDelta), sinf(angleDelta)), 10, textPosY += fontSize, fontSize, RAYWHITE);

		DrawText(TextFormat("Dot product: %.2f", Vector2DotProduct(normals[0], normals[1])), 10, textPosY += fontSize, fontSize, dotProductColor);
        // We cheat a little bit here by flipping the Y axis to match the Y axis flip we did when calculating angles.
        float displayCrossProduct = Vector2CrossProduct((Vector2) { normals[0].x, -normals[0].y }, (Vector2) { normals[1].x, -normals[1].y });
        DrawText(TextFormat("Cross product: %.2f", displayCrossProduct), 10, textPosY += fontSize, fontSize, crossProductColor);

		int screenHeight = GetScreenHeight();
		textPosY = screenHeight - 10;
		DrawText("Toggle cross product projection: [Y]", 10, textPosY -= fontSize, fontSize, crossProductColor);
		DrawText("Toggle dot product projection: [T]", 10, textPosY -= fontSize, fontSize, dotProductColor);
        DrawText("Blue start: [E] Blue end: [R]", 10, textPosY -= fontSize, fontSize, lineColors[1]);
		DrawText("Red start: [Q] Red end: [W]", 10, textPosY -= fontSize, fontSize, lineColors[0]);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
