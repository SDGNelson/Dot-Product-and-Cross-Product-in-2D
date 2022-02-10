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

void DrawArrowLabel(Vector2 origin, Vector2 normal, float length, float displayAngle, float normalizedAngle, Color color)
{
	const int fontSize = 20;
	const int labelCount = 4;
	const float offsetFromArrowhead = 20.0;

	// Left-align label near the right, gradually center-align near top and bottom, and right-align near the left
	float labelRightAlignment = (cosf(normalizedAngle) - 1.0f) * -0.5f;
	// Top-align label near the bottom, gradually center-align near left and right, and bottom-align near the top.
	float labelBottomAlignment = sinf(normalizedAngle) * 0.5f + 0.5f;
	int labelPosX = (int) (origin.x + normal.x * (length + offsetFromArrowhead));
	int labelPosY = (int) (origin.y + normal.y * (length + offsetFromArrowhead) - labelBottomAlignment * fontSize * labelCount);
	const char* degText = TextFormat("%.0f deg", displayAngle * RAD2DEG);
	DrawText(degText, labelPosX - (int)(MeasureText(degText, fontSize) * labelRightAlignment), labelPosY, fontSize, color);
	const char* radText = TextFormat("%.2f rad", displayAngle);
	DrawText(radText, labelPosX - (int)(MeasureText(radText, fontSize) * labelRightAlignment), labelPosY += fontSize, fontSize, color);
	const char* cosText = TextFormat("cos: %.2f", cosf(displayAngle));
	DrawText(cosText, labelPosX - (int)(MeasureText(cosText, fontSize) * labelRightAlignment), labelPosY += fontSize, fontSize, color);
	const char* sinText = TextFormat("sin: %.2f", sinf(displayAngle));
	DrawText(sinText, labelPosX - (int)(MeasureText(sinText, fontSize) * labelRightAlignment), labelPosY += fontSize, fontSize, color);
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
    
    // These four options are for the video.
	bool shouldDrawSecondaryArrow = true;
	bool shouldDrawAngleBetweenArrow = true;
    bool shouldDrawAngleBetweenDotAndCrossProduct = true;
	bool shouldDrawDotAndCrossProjectionPrompt = true;

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
        if (IsKeyPressed(KEY_F1))
        {
            shouldDrawSecondaryArrow = !shouldDrawSecondaryArrow;
		}
		if (IsKeyPressed(KEY_F2))
		{
			shouldDrawAngleBetweenArrow = !shouldDrawAngleBetweenArrow;
		}
		if (IsKeyPressed(KEY_F3))
		{
			shouldDrawAngleBetweenDotAndCrossProduct = !shouldDrawAngleBetweenDotAndCrossProduct;
		}
        if (IsKeyPressed(KEY_F4))
        {
            shouldDrawDotAndCrossProjectionPrompt = !shouldDrawDotAndCrossProjectionPrompt;
        }

        float lineLengths[2];
		Vector2 normals[2];
        Vector2 clockwiseTangents[2];
        float angles[2]; // Counter-clockwise radians [0, TAU).
        for (int index = (shouldDrawSecondaryArrow ? 1 : 0); index >= 0; --index)
        {
            Vector2 endRelativeToStart = Vector2Subtract(lineEnds[index], lineStarts[index]);
            normals[index] = endRelativeToStart;
            lineLengths[index] = Vector2Length(endRelativeToStart);
            if (lineLengths[index] > 0.0f)
            {
                normals[index] = Vector2Scale(normals[index], 1.0f / lineLengths[index]);
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
            DrawRingLines(lineStarts[index], 0.0f, lineLengths[index] * 0.5f, 90.0f, angles[index] * RAD2DEG + 90.0f, 0, lineColors[index]);

            // Text label next to arrowhead.
            DrawArrowLabel(lineStarts[index], normals[index], lineLengths[index], angles[index], angles[index], lineColors[index]);

			if (shouldDrawDotProductProjection & shouldDrawDotAndCrossProjectionPrompt)
			{
				// Dot product of (normal, vector) is 1D closest position of vector along normal.
				Vector2 mouseRelativeToStart = Vector2Subtract(GetMousePosition(), lineStarts[index]);
				float projectionAlongNormal = Vector2DotProduct(normals[index], mouseRelativeToStart);
				Vector2 pointAlongNormal = Vector2Add(lineStarts[index], Vector2Scale(normals[index], projectionAlongNormal));
				DrawLineEx(lineStarts[index], pointAlongNormal, 2.0f, dotProductColor);
				DrawLineEx(Vector2Add(pointAlongNormal, Vector2Scale(clockwiseTangents[index], -8.0f)), Vector2Add(pointAlongNormal, Vector2Scale(clockwiseTangents[index], 8.0f)), 2.0f, dotProductColor);
			}

			if (shouldDrawCrossProductProjection & shouldDrawDotAndCrossProjectionPrompt)
			{
				// Cross product of (normal, vector) is 1D closest position of vector along tangent perpendicular to normal.
				Vector2 mouseRelativeToStart = Vector2Subtract(GetMousePosition(), lineStarts[index]);
				float projectionAlongClockwiseTangent = Vector2CrossProduct(normals[index], mouseRelativeToStart);
				Vector2 pointAlongClockwiseTangent = Vector2Add(lineStarts[index], Vector2Scale(clockwiseTangents[index], projectionAlongClockwiseTangent));
				DrawLineEx(lineStarts[index], pointAlongClockwiseTangent, 2.0f, crossProductColor);
				DrawLineEx(Vector2Add(pointAlongClockwiseTangent, Vector2Scale(normals[index], -8.0f)), Vector2Add(pointAlongClockwiseTangent, Vector2Scale(normals[index], 8.0f)), 2.0f, crossProductColor);
			}
        }

		// We cheat a little bit here by negating the angle so that positive angles turn counter-clockwise.
		float angleDelta = -AngleBetween(angles[0], angles[1]);

        if (shouldDrawSecondaryArrow & shouldDrawAngleBetweenArrow)
        {
            float averageLineLength = lineLengths[0] * 0.5f + lineLengths[1] * 0.5f;

			// Line and arrowhead.
            Vector2 overlayLineEnd = Vector2Add(lineStarts[1], Vector2Scale(normals[0], averageLineLength));
			DrawLineEx(lineStarts[1], overlayLineEnd, 2.0f, RAYWHITE);
			DrawLineEx(overlayLineEnd, Vector2Add(overlayLineEnd, Vector2Add(Vector2Scale(normals[0], -8.0f), Vector2Scale(clockwiseTangents[0], 8.0f))), 2.0f, RAYWHITE);
			DrawLineEx(overlayLineEnd, Vector2Add(overlayLineEnd, Vector2Add(Vector2Scale(normals[0], -8.0f), Vector2Scale(clockwiseTangents[0], -8.0f))), 2.0f, RAYWHITE);

			// Arc illustrating counter-clockwise angle [0, TAU).
            float startAngle = angles[1] * RAD2DEG + 90.0f;
			DrawRingLines(lineStarts[1], 0.0f, averageLineLength * 0.5f, startAngle, startAngle + angleDelta * RAD2DEG, 0, RAYWHITE);

			// Text label next to arrowhead.
			DrawArrowLabel(lineStarts[1], normals[0], averageLineLength, angleDelta, angles[0], RAYWHITE);
        }

        // Draw HUD

        const int fontSize = 30;
        int textPosY = 10;
        DrawText(TextFormat("Red angle: %.0f deg (%.2f rad) cos: %.2f sin: %.2f", angles[0] * RAD2DEG, angles[0], cosf(angles[0]), sinf(angles[0])), 10, textPosY, fontSize, lineColors[0]);
		
        if (shouldDrawSecondaryArrow)
        {
            DrawText(TextFormat("Blue angle: %.0f deg (%.2f rad) cos: %.2f sin: %.2f", angles[1] * RAD2DEG, angles[1], cosf(angles[1]), sinf(angles[1])), 10, textPosY += fontSize, fontSize, lineColors[1]);
            if (shouldDrawAngleBetweenArrow)
            {
                DrawText(TextFormat("Angle delta: %.0f deg (%.2f rad) cos: %.2f sin: %.2f", angleDelta * RAD2DEG, angleDelta, cosf(angleDelta), sinf(angleDelta)), 10, textPosY += fontSize, fontSize, RAYWHITE);
                if (shouldDrawAngleBetweenDotAndCrossProduct)
                {
                    DrawText(TextFormat("Dot product: %.2f", Vector2DotProduct(normals[0], normals[1])), 10, textPosY += fontSize, fontSize, dotProductColor);
                    DrawText(TextFormat("Cross product: %.2f", Vector2CrossProduct(normals[0], normals[1])), 10, textPosY += fontSize, fontSize, crossProductColor);
                }
            }
        }

		int screenHeight = GetScreenHeight();
		textPosY = screenHeight - 10;
        if (shouldDrawDotAndCrossProjectionPrompt)
        {
            DrawText("Toggle cross product projection: [Y]", 10, textPosY -= fontSize, fontSize, crossProductColor);
            DrawText("Toggle dot product projection: [T]", 10, textPosY -= fontSize, fontSize, dotProductColor);
        }
        if (shouldDrawSecondaryArrow)
		{
            DrawText("Blue start: [E] Blue end: [R]", 10, textPosY -= fontSize, fontSize, lineColors[1]);
        }
        DrawText("Red start: [Q] Red end: [W]", 10, textPosY -= fontSize, fontSize, lineColors[0]);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
