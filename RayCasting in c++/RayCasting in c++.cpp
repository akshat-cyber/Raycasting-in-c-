#include <iostream>
#include <stdio.h>
#include<vector>
#include<algorithm>
#include <Windows.h>
#include<chrono>
int nScreenWidth = 120;			// Console Screen Size X (columns)
int nScreenHeight = 40;			// Console Screen Size Y (rows)
int nMapWidth = 16;				// World Dimensions
int nMapHeight = 16;
float fPlayerX = 8.0f;			// Player Start Position
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;			// Player Start Rotation
float fFOV = 3.14159f / 4.0f;	// Field of View
float fDepth = 16.0f;			// Maximum rendering distance
float fSpeed = 5.0f;			// Walking Speed
using namespace std;
using namespace chrono;
int main()
{
	// Create Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;




	// Create Map of world space # = wall block, . = space
	wstring map;
	map += L"#########.......";
	map += L"#...............";
	map += L"#.......########";
	map += L"#..............#";
	map += L"#......##......#";
	map += L"#......##......#";
	map += L"#..............#";
	map += L"###............#";
	map += L"##.............#";
	map += L"#......####..###";
	map += L"#......#.......#";
	map += L"#......#.......#";
	map += L"#..............#";
	map += L"#......#########";
	map += L"#..............#";
	map += L"################";

	auto time1 = system_clock::now(); // gets current time	
	auto time2 = system_clock::now();

	while (1)
	{
		time2 = system_clock::now();
		duration<float> elapsedTime = time2 - time1;
		time1 = time2;
		float fElapsedTime = elapsedTime.count();

		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) {
			/// left
			fPlayerX -= 0.9f * fElapsedTime;
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			// down
			fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

			// collision detection 
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX += sinf(fPlayerA) * 2.5f * fElapsedTime;
				fPlayerY += cosf(fPlayerA) * 2.5f * fElapsedTime;
			 }

		}
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) {
			// right
			fPlayerX += 0.9f * fElapsedTime;
		}
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			// up
			fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX -= sinf(fPlayerA) * 3.0f * fElapsedTime;
				fPlayerY -= cosf(fPlayerA) * 3.0f * fElapsedTime;
			}
		}

		for (int x = 0; x < nScreenWidth; x++)
		{
			// For each column, calculate the projected ray angle into world space
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			// Find distance to wall
			float fStepSize = 0.1f;		  // Increment size for ray casting, decrease to increase
			float fDistanceToWall = 0.0f; //                                      resolution

			bool bHitWall = false;		// Set when ray hits wall block
			bool bBoundary = false;		// Set when ray hits boundary between two wall blocks

			float fEyeX = sinf(fRayAngle); // Unit vector for ray in player space
			float fEyeY = cosf(fRayAngle);

			// Incrementally cast ray from player, along ray angle, testing for
			// intersection with a block
			while (!bHitWall && fDistanceToWall < fDepth)
			{
				fDistanceToWall += fStepSize;
				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				// Test if ray is out of bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
				{
					bHitWall = true;			// Just set distance to maximum depth
					fDistanceToWall = fDepth;
				}
				else
				{
					// Ray is inbounds so test to see if the ray cell is a wall block
					if (map[nTestX * nMapWidth + nTestY] == '#')
					{




						// Ray has hit wall
						bHitWall = true;
					}
				}
			}

			// Calculate distance to ceiling and floor
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;
			short shade = ' ';
			if (fDistanceToWall < fDepth / 4.0f) { shade = 0x2588; } // very close
			else if (fDistanceToWall < fDepth / 3.0f) { shade = 0x2593; }
			else if (fDistanceToWall < fDepth / 2.0f) { shade = 0x2592; }
			else if (fDistanceToWall < fDepth) { shade = 0x2591; }
			else {
				shade = ' ';
			}
			for (int y = 0; y < nScreenHeight; y++)
			{
				// Each Row
				if (y <= nCeiling)
					screen[y * nScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor)
					screen[y * nScreenWidth + x] = shade;
				else // Floor
				{
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)		shade = '#';
					else if (b < 0.5)	shade = 'x';
					else if (b < 0.75)	shade = '.';
					else if (b < 0.9)	shade = '-';
					else				shade = ' ';
					screen[y * nScreenWidth + x] = shade;
				}
			}
		}
		
		screen[(int)fPlayerY * nScreenWidth + (int)fPlayerX] = 'P';
        // Display Frame
		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}
	return 0;
}