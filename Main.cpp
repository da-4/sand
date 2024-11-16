#include <SFML/Graphics.hpp>
#include <iostream>
#include <Windows.h>
#include <cmath>
#include <ctime>
#include <stdlib.h>
#include <vector>

#pragma warning(push)
#pragma warning(disable:6385)

#define winX 400
#define winY 400

#define particleSize 1
#define chunkDivisor 10

int chunkStart[chunkDivisor * chunkDivisor][2] = { 0 };
int chunkSize[2] = {winX / chunkDivisor, winY / chunkDivisor};
bool chunkSkip[chunkDivisor * chunkDivisor];

// this stores the state of all of the grid places
uint8_t particleGrid[winX][winY];

unsigned long clockOffset = 0;
int FPS;

// start the window
sf::RenderWindow window(sf::VideoMode(winX, winY), "Cube");

// make the particle
sf::RectangleShape particleShape(sf::Vector2f(particleSize, particleSize));

// make the chunk outline
sf::RectangleShape chunkOutline(sf::Vector2f(chunkSize[0], chunkSize[1]));

struct particleData {
};

// calculates the FPS and prints it to the terminal
void fps() {
	if (clock() - clockOffset >= 1000) {
		std::cout << FPS << "\n";
		clockOffset = clock();
		FPS = 0;
	} else {
		FPS++;
	}
}

void updateParticles() {
	// set the chunk number to 0, this is used for calculating which chunks are active
	int chunkNumber = 0;
	// reset the chunk skip flags to 0
	for (int i = 0; i < chunkDivisor * chunkDivisor; i++) {
		chunkSkip[i] = 0;
	}
	// declare the buffer
	uint8_t particleGridBuffer[winX][winY] = { 0 };
	for (int y = 0; y < winY; y++) {
		for (int x = 0; x < winX; x++) {
			// caclulate what chunk we are in
			/*if (x % chunkSize[0] == 0 || y % chunkSize[1] == 0) {
				chunkNumber = x / chunkSize[0];
				chunkNumber += y / chunkSize[1] * chunkDivisor;
			}*/
			// GENERAL COMMENT
			// after every change of the buffer we use the chunk number
			// to raise the flag for skipping that chunk or not

			// if the current grid cell is blank then skip it
			if (particleGrid[x][y] == 0) {
				continue;
			}
			// check if the grid cell below is clear and if not then first check if the diagonal cells
			// are clear, and if they aren't then copy where it is currently at to the buffer, and then skip
			if (particleGrid[x][y + 1] != 0) {
				// checks if the grid cells diagonaly down are clear, and if not then copy where it 
				// is currently to the buffer, and then skip. but if it is clear then pick a random
				// diagonal to move to
				if (x > 0 && x < winX - 1 && y > winY - 1) {
					// if both are open, chose one at random
					if (particleGrid[x + 1][y + 1] == 0 && particleGrid[x - 1][y + 1] == 0) {
						bool dir = rand() % 2;
						if (dir == true) {
							particleGridBuffer[x - 1][y + 1] = particleGrid[x][y];
							particleShape.setPosition(x - 1, y + 1);
							particleShape.setFillColor(sf::Color::Red);
							window.draw(particleShape);
							chunkSkip[chunkNumber] = 1;
							continue;
						} else {
							particleGridBuffer[x + 1][y + 1] = particleGrid[x][y];
							particleShape.setPosition(x + 1, y + 1);
							particleShape.setFillColor(sf::Color::Red);
							window.draw(particleShape);
							chunkSkip[chunkNumber] = 1;
							continue;
						}
					}
				}
				// if only one diagonal is open then move to that one
				if (x < winX - 1 && x > 0) {
					if (particleGrid[x + 1][y + 1] == 0) {
						particleGridBuffer[x + 1][y + 1] = particleGrid[x][y];
						particleShape.setPosition(x + 1, y + 1);
						particleShape.setFillColor(sf::Color::Red);
						window.draw(particleShape);
						chunkSkip[chunkNumber] = 1;
						continue;
					}
					if (particleGrid[x - 1][y + 1] == 0) {
						particleGridBuffer[x - 1][y + 1] = particleGrid[x][y];
						particleShape.setPosition(x - 1, y + 1);
						particleShape.setFillColor(sf::Color::Red);
						window.draw(particleShape);
						chunkSkip[chunkNumber] = 1;
						continue;
					}
				}
				particleGridBuffer[x][y] = particleGrid[x][y];
				particleShape.setPosition(x, y);
				particleShape.setFillColor(sf::Color::Cyan);
				window.draw(particleShape);
				continue;
			}
			// check if the grid cell is the bottom one and if it is then copy where it is
			// to the buffer, and then skip
			if (y >= winY - particleSize) {
				particleGridBuffer[x][y] = particleGrid[x][y];
				particleShape.setPosition(x, y);
				particleShape.setFillColor(sf::Color::Cyan);
				window.draw(particleShape);
				continue;
			}
			// if the cell below is clear and its not the bottom of the window then move down
			// we have to set the buffer instead of the actual grid because if a particle was moved
			// down to the next row, then when the next row is checked then it would move down again
			particleGridBuffer[x][y + 1] = particleGrid[x][y];
			particleShape.setPosition(x, y + particleSize);
			particleShape.setFillColor(sf::Color::Red);
			chunkSkip[chunkNumber] = 1;
			window.draw(particleShape);
		}
		/*for (int x = 0; x < winX; x++) {
			// caclulate what chunk we are in
			if (x % chunkSize[0] == 0 || y % chunkSize[1] == 0) {
				chunkNumber = x / chunkSize[0];
				chunkNumber += y / chunkSize[1] * chunkDivisor;
			}
			if (particleGrid[x][y] != particleGridBuffer[x][y]) {
				chunkSkip[chunkNumber] = 1;
			}
		}*/
	}
	// copy the buffer to the grid
	memcpy(particleGrid, particleGridBuffer, sizeof(particleGrid));
}

//**********//

void updateParticlesChunks() {
	int totalChunks = chunkDivisor * chunkDivisor;
	// declare the buffer
	uint8_t particleGridBuffer[winX][winY] = { 0 };
	for (int chunkNumber = 0; chunkNumber < totalChunks; chunkNumber++) {
		for (int y = chunkStart[chunkNumber][1]; y < chunkStart[chunkNumber][1] + chunkSize[1]; y++) {
			for (int x = chunkStart[chunkNumber][0]; x < chunkStart[chunkNumber][0] + chunkSize[0]; x++) {
				/*std::cout << "CHUNK NUMBER: " << chunkNumber << " Y START: " << chunkStart[chunkNumber][1] << " Y END: " << chunkStart[chunkNumber][1] + chunkSize[1];
				std::cout << " X START: " << chunkStart[chunkNumber][0] << " X END: " << chunkStart[chunkNumber][0] + chunkSize[0] << "\n";*/
				// GENERAL COMMENT
				// after every change of the buffer we use the chunk number
				// to raise the flag for skipping that chunk or not
				
				// if the current grid cell is blank then skip it
				if (particleGrid[x][y] == 0) {
					continue;
				}
				// check if the grid cell below is clear and if not then first check if the diagonal cells
				// are clear, and if they aren't then copy where it is currently at to the buffer, and then skip
				if (particleGrid[x][y + 1] != 0) {
					// checks if the grid cells diagonaly down are clear, and if not then copy where it 
					// is currently to the buffer, and then skip. but if it is clear then pick a random
					// diagonal to move to
					if (x > 0 && x < winX - 1 && y > winY - 1) {
						// if both are open, chose one at random
						if (particleGrid[x + 1][y + 1] == 0 && particleGrid[x - 1][y + 1] == 0) {
							bool dir = rand() % 2;
							if (dir == true) {
								particleGridBuffer[x - 1][y + 1] = particleGrid[x][y];
								particleShape.setPosition(x - 1, y + 1);
								particleShape.setFillColor(sf::Color::Red);
								window.draw(particleShape);
								chunkSkip[chunkNumber] = 1;
								continue;
							} else {
								particleGridBuffer[x + 1][y + 1] = particleGrid[x][y];
								particleShape.setPosition(x + 1, y + 1);
								particleShape.setFillColor(sf::Color::Red);
								window.draw(particleShape);
								chunkSkip[chunkNumber] = 1;
								continue;
							}
						}
					}
					// if only one diagonal is open then move to that one
					if (x < winX - 1 && x > 0) {
						if (particleGrid[x + 1][y + 1] == 0) {
							particleGridBuffer[x + 1][y + 1] = particleGrid[x][y];
							particleShape.setPosition(x + 1, y + 1);
							particleShape.setFillColor(sf::Color::Red);
							window.draw(particleShape);
							chunkSkip[chunkNumber] = 1;
							continue;
						}
						if (particleGrid[x - 1][y + 1] == 0) {
							particleGridBuffer[x - 1][y + 1] = particleGrid[x][y];
							particleShape.setPosition(x - 1, y + 1);
							particleShape.setFillColor(sf::Color::Red);
							window.draw(particleShape);
							chunkSkip[chunkNumber] = 1;
							continue;
						}
					}
					particleGridBuffer[x][y] = particleGrid[x][y];
					particleShape.setPosition(x, y);
					particleShape.setFillColor(sf::Color::Cyan);
					window.draw(particleShape);
					continue;
				}
				// check if the grid cell is the bottom one and if it is then copy where it is
				// to the buffer, and then skip
				if (y >= winY - particleSize) {
					particleGridBuffer[x][y] = particleGrid[x][y];
					particleShape.setPosition(x, y);
					particleShape.setFillColor(sf::Color::Cyan);
					window.draw(particleShape);
					continue;
				}
				// if the cell below is clear and its not the bottom of the window then move down
				// we have to set the buffer instead of the actual grid because if a particle was moved
				// down to the next row, then when the next row is checked then it would move down again
				particleGridBuffer[x][y + 1] = particleGrid[x][y];
				particleShape.setPosition(x, y + particleSize);
				particleShape.setFillColor(sf::Color::Red);
				chunkSkip[chunkNumber] = 1;
				window.draw(particleShape);
			}
		}
		// copy the buffer to the grid
		memcpy(particleGrid, particleGridBuffer, sizeof(particleGrid));
		//std::cout << "SLEEPING\n";
		//Sleep(1000);
	}
}




int main() {
	// set random seed
	srand(clock());
	chunkOutline.setFillColor(sf::Color(0, 0, 0, 0));
	chunkOutline.setOutlineColor(sf::Color::Red);
	chunkOutline.setOutlineThickness(1);
	// calculate the size of the chunks
	for (int i = 0; i < chunkDivisor * chunkDivisor; i++) {
		// calculate x value
		chunkStart[i][0] = chunkSize[0] * (i % chunkDivisor);
		// calculate the y value
		chunkStart[i][1] = chunkSize[0] * (i / chunkDivisor);
		//std::cout << "CHUNK START X:" << chunkStart[i][0] << " CHUNK START Y:" << chunkStart[i][1] << "\n";
		std::cout << "CHUNK NUMBER: " << i;
		std::cout <<  " X MIN: " << chunkStart[i][0] << " X MAX: " << chunkStart[i][0] + chunkSize[0] << " Y MIN:" << chunkStart[i][1] << " Y MAX:" << chunkStart[i][1] + chunkSize[1] << "\n";
	}
	// set the clock offset for FPS calculation
	clockOffset = clock();
	// while the window is open
	while (window.isOpen()) {
		// calculate the FPS and display it every second
		fps();
		// check if the window X was pressed
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}
		window.clear();
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			// get global mouse position
			sf::Vector2i position = sf::Mouse::getPosition(window);
			if (position.x < winX && position.x >= 0 && position.y < winY && position.y >= 0) {
				particleGrid[position.x][position.y] = 1;
			}
		}
		if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
			// get global mouse position
			sf::Vector2i position = sf::Mouse::getPosition(window);
			if (position.x < winX && position.x >= 0 && position.y < winY && position.y >= 0) {
				particleGrid[position.x][position.y] = 0;
			}
		}
		updateParticles();
		for (int i = 0; i < chunkDivisor * chunkDivisor; i++) {
			if (chunkSkip[i] == 1) {
				chunkOutline.setPosition(sf::Vector2f(chunkStart[i][0], chunkStart[i][1]));
				window.draw(chunkOutline);
			}
		}
		window.display();
	}
	return 0;
}