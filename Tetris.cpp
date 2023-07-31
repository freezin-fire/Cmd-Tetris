#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

#include <Windows.h>

// Screen Dimensions
int screenW = 120;
int screenH = 40;

// Holds the block assets
std::wstring tetromino[7];
//std::wstring graphics[7];


// Playing Field
int fieldW = 12;
int fieldH = 18;
unsigned char *field = nullptr;

// Implementing the rotation function, returns index of piece after rotation 
// applied instead of using 2D arrays, we can use 1D array and label the 
// blocks accordingly, e.g. array[2][3] will become array[15]
int Rotate(int px, int py, int r) {
	switch (r % 4)
	{
		case 0: return py * 4 + px;			// Rotate the block 0 degrees
		case 1: return 12 + py - (px * 4);	// Rotate the block 90 degrees
		case 2: return 15 - (py * 4) - px;	// Rotate the block 180 degrees
		case 3: return 3 - py + (px * 4);	// Rotate the block 270 degrees
	}
	return 0;
}

// Block Collision detection
bool DoesPieceFit(int ntetromino, int rotation, int posX, int posY) {
	for (int px = 0; px < 4; px++) {
		for (int py = 0; py < 4; py++) {
			// getting index into piece
			int pi = Rotate(px, py, rotation);

			// getting index into field
			int fi = (posY + py) * fieldW + (posX + px);

			if (posX + px >= 0 && posX + px < fieldW)
			{
				if (posY + py >= 0 && posY + py < fieldH)
				{
					if (tetromino[ntetromino][pi] == L'X' && field[fi] != 0)
						return false;
				}
			}
		}
	}
	return true;
}

int main(){

	// Create Screen Buffer
	wchar_t* screen = new wchar_t[screenW * screenH];
	for (int i = 0; i < screenW * screenH; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	
	// Tetronimos 4x4
	tetromino[0].append(L"..X...X...X...X.");
	tetromino[1].append(L"..X..XX..X......");
	tetromino[2].append(L".X...XX...X.....");
	tetromino[3].append(L".....XX..XX.....");
	tetromino[4].append(L".XX...X...X.....");
	tetromino[5].append(L".XX..X...X......");
	tetromino[6].append(L"..X..XX...X.....");

	// Graphics for Tetrominos
	/*graphics[0].append(L"A");
	graphics[1].append(L"B");
	graphics[2].append(L"C");
	graphics[3].append(L"D");
	graphics[4].append(L"E");
	graphics[5].append(L"F");
	graphics[6].append(L"G");*/

	// Field initialize
	field = new unsigned char[fieldW*fieldH];
	// Assigning borders to field
	for (int x = 0; x < fieldW; x++)
		for (int y = 0; y < fieldH; y++)
			field[y * fieldW + x] = (x == 0 || x == fieldW - 1 || y == fieldH - 1) ? 9 : 0;
	
	// Game Logic ============================================
	bool gameOver = false;
	
	int currentPiece = 0;
	int currentRotation = 0;
	int currentX = fieldW / 2;
	int currentY = 0;

	bool key[4];
	bool rotKeyHold = false;

	int speed = 20;
	int speedCounter = 0;
	bool forceDown = false;
	int pieceCount = 0;

	int score = 0;

	std::vector<int> lines;

	while (!gameOver)
	{
		// Game Timing ===========================================
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		speedCounter++;
		forceDown = (speedCounter == speed);

		// Input =================================================

		for (int k = 0; k < 4; k++)
		{
			key[k] = (0x8000 & GetAsyncKeyState((unsigned char)("ADSW"[k]))) != 0;
		}

		// Left key (A) pressed
		if (key[0]) {
			if (DoesPieceFit(currentPiece, currentRotation, currentX - 1, currentY)) {
				currentX--;
			}
		}
		// Right key (D) pressed
		if (key[1]) {
			if (DoesPieceFit(currentPiece, currentRotation, currentX + 1, currentY)) {
				currentX++;
			}
		}
		// Down Key (S) pressed
		if (key[2]) {
			if (DoesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) {
				currentY++;
			}
		}
		// Rotation key (W) pressed
		if (key[3]) {
			if (!rotKeyHold && DoesPieceFit(currentPiece, currentRotation + 1, currentX, currentY)) {
				currentRotation++;
				rotKeyHold = true;
			}
			else
			{
				rotKeyHold = false;
			}
		}

		if (forceDown) {
			if (DoesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) {
				currentY++;
			}
			else
			{
				// Lock piece in the field
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tetromino[currentPiece][Rotate(px, py, currentRotation)] == L'X')
							field[(currentY + py) * fieldW + (currentX + px)] = currentPiece + 1;

				// Increase difficulty
				pieceCount++;
				if (pieceCount % 10 == 0)
					if (speed >= 10) speed--;
				
				// Check for completed lines
				// We don't actually need to check the whole board for lines
				// we only need to check the 4 lines that the current piece is occupying
				for (int py = 0; py < 4; py++) {
					// check for making sure the line we are checking is in playing field
					if (currentY + py < fieldH - 1) {
						bool line = true;
						for (int px = 1; px < fieldW - 1; px++)
						{
							// Iterating for empty spaces in the line and set false if detected void
							line &= (field[(currentY + py) * fieldW + px] != 0);
						}

						if (line) {
							// if line is true, set line to '=' sign
							for (int px = 1; px < fieldW - 1; px++)
							{
								field[(currentY + py) * fieldW + px] = 8;
							}
							lines.push_back(currentY+py);
						}
					}
				}
				score += 25;
				if (!lines.empty()) score += (1 << lines.size()) * 100;

				// Get new piece
				currentX = fieldW / 2;
				currentY = 0;
				currentRotation = 0;
				currentPiece = std::rand() % 7;

				// Piece can't fit in the board
				gameOver = !DoesPieceFit(currentPiece, currentRotation, currentX, currentY);
			}
			speedCounter = 0;
		}

		// Render Graphics =======================================

		// Draw the Field
		for (int x = 0; x < fieldW; x++)
			for (int y = 0; y < fieldH; y++)
				// 0-9 values will be index of the string array assigned
				// so, 0 = ' ', 3 = 'C' and so on 
				screen[(y + 2) * screenW + (x + 2)] = L" ABCDEFG=#"[field[y * fieldW + x]]; 

		// Draw Current piece
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tetromino[currentPiece][Rotate(px, py, currentRotation)] == L'X')
					screen[(currentY + py + 2) * screenW + (currentX + px + 2)] = currentPiece + 65;

		// Display score
		swprintf_s(&screen[(fieldH + 3) * screenW], 16, L"SCORE: %8d", score);

		// Showing that we are clearing the line
		if (!lines.empty()) {
			// show the lines for a moment
			WriteConsoleOutputCharacter(hConsole, screen, screenW* screenH, { 0,0 }, & dwBytesWritten);
			std::this_thread::sleep_for(std::chrono::milliseconds(400));

			for (auto& v : lines) {
				for (int px = 1; px < fieldW - 1; px++)
				{
					for (int py = v; py > 0; py--)
					{
						field[py * fieldW + px] = field[(py-1) * fieldW + px];
					}
						field[px] = 0;
				}
			}

			lines.clear();
		}

		// Print stuff on the screen/Display Buffer
		WriteConsoleOutputCharacter(hConsole, screen, screenW * screenH, { 0,0 }, &dwBytesWritten);
	}
	// Game Over Screen
	CloseHandle(hConsole);
	std::cout << "Game Over, Well Played!" << std::endl;
	std::cout << "Score : " << score << std::endl;
	system("pause");

	gameOver = false;

	return 0;
}