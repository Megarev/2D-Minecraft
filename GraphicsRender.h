#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <list>

struct Tile {
	int x, y;
	char tileCharacter;

	Tile(int x, int y, char c) 
		: x(x), y(y), tileCharacter(c) {}
};

class Level {
private:
	std::vector<std::string> levelVector;
	uint32_t width, height;
public:
	Level() {
		width = height = 0;
	}

	Level(const std::vector<std::string>& level, uint32_t w, uint32_t h)
		: levelVector(level), width(w), height(h) {}

	void SetSize(uint32_t w, uint32_t h) {
		width = w;
		height = h;
	}

	void InitializeLevelString() {
		for (int i = 0; i < (int)height; i++) {
			std::string line;
			for (int j = 0; j < (int)width; j++) {
				line.push_back('.');
			}
			levelVector.push_back(line);
		}
	}

	void ClearLevel() {
		levelVector.clear();
	}

	void SetCharacter(uint32_t x, uint32_t y, char c) {
		if (x < 0 || y < 0 || x > (width - 1) || y > (height - 1)) return;
		levelVector[y][x] = c;
	}

	inline char GetCharacter(uint32_t x, uint32_t y) const {
		if (x < 0 || y < 0 || x > (width - 1) || y > (height - 1)) return '\0';
		return levelVector[y][x];
	}

	void InitializeLevelString(uint32_t w, uint32_t h) {

		width = w;
		height = h;

		for (int i = 0; i < (int)h; i++) {
			std::string line;
			for (int j = 0; j < (int)w; j++) {
				line.push_back('.');
			}
			levelVector.push_back(line);
		}
	}

	static Level LoadLevel(const std::string& filepath) {
		std::ifstream reader(filepath);
		
		Level level;

		if (reader.is_open()) {

			while (!reader.eof()) {
				std::string line;
				reader >> line;
				level.levelVector.push_back(line);
			}

			reader.close();
		}

		level.width = level.levelVector[0].size();
		level.height = level.levelVector.size();

		return level;
	}
	
	void SaveLevel(const std::string& filename) {
		std::ofstream writer("files/levels/" + filename);

		if (writer.is_open()) {
			for (uint32_t i = 0; i < height; i++) {
				writer << levelVector[i] << "\n";
			}
			writer.close();
		}
	}

	static Level LoadLevel(const std::list<Tile>& positions, uint32_t levelWidth, uint32_t levelHeight) {
		Level level;
		level.SetSize(levelWidth, levelHeight);
		level.InitializeLevelString();

		for (auto& pos : positions) {
			if (pos.x < 0 || pos.x > (int)(levelWidth - 1) || pos.y < 0 || pos.y > (int)(levelHeight - 1)) continue;
			level.levelVector[pos.y][pos.x] = pos.tileCharacter;
		}

		return level;
	}

	void PrintLevel() {
		system("cls");
		for (std::size_t i = 0; i < height; i++) {
			std::cout << levelVector[i] << std::endl;
		}
	}

	inline uint32_t GetWidth() const { return width; }
	inline uint32_t GetHeight() const { return height; }

	std::vector<std::string> GetLevel() const { return levelVector; }
};

void DrawLine(sf::RenderWindow& window, float x1, float y1, float x2, float y2, sf::Color color = sf::Color::White) {
	sf::VertexArray line(sf::LineStrip, 2);

	line[0].position = { x1, y1 };
	line[0].color = color;
	line[1].position = { x2, y2 };
	line[1].color = color;

	window.draw(line);
}

void DrawPoint(sf::RenderWindow& window, float x, float y, sf::Color color = sf::Color::White) {
	sf::RectangleShape pixel({ 2.0f, 2.0f });
	pixel.setPosition({ x, y });
	pixel.setFillColor(color);

	window.draw(pixel);
}

void DrawPolygon(sf::RenderWindow& window, const std::vector<sf::Vector2f>& points, sf::Color color = sf::Color::White) {
	for (std::size_t i = 1; i <= points.size(); i++) {
		auto [x1, y1] = i == points.size() ? points[0] : points[i - 1];
		auto [x2, y2] = i == points.size() ? points[points.size() - 1] : points[i];

		DrawLine(window, x1, y1, x2, y2, color);
	}
}

void DrawGrid(sf::RenderWindow& window, float size, sf::Color color = sf::Color::White) {
	auto [sizeX, sizeY] = window.getSize();

	for (uint32_t i = 0; i < sizeY / (uint32_t)size; i++) {
		DrawLine(window, 0.0f, i * size, (float)sizeX, i * size, color);
	}

	for (uint32_t i = 0; i < sizeX / (uint32_t)size; i++) {
		DrawLine(window, i * size, 0.0f, i * size, (float)sizeY, color);
	}
}

void DrawCircle(sf::RenderWindow& window, const sf::Vector2f& origin, float radius, sf::Color color = sf::Color::White) {
	auto [h, k] = origin;
	for (float i = 1; i < 361; i++) {

		float x = h + radius * cosf(i);
		float y = k + radius * sinf(i);

		DrawPoint(window, x, y, color);
	}
}

void DrawEllipse(sf::RenderWindow& window, const sf::Vector2f& origin, float width, float height, sf::Color color = sf::Color::White) {
	auto [h, k] = origin;
	for (float i = 1; i < 361; i++) {
		
		float x = h + width * cosf(i);
		float y = k + height * sinf(i);

		DrawPoint(window, x, y, color);
	}
}

void RenderText(sf::RenderWindow& window, const sf::Font& font, float x, float y, const std::string& str, sf::Color color = sf::Color::White, uint32_t characterSize = 32) {
	sf::Text text(str, font, characterSize);
	text.setPosition({ x, y });
	text.setFillColor(color);

	window.draw(text);
}

void DrawTextWithValue(sf::RenderWindow& window, const sf::Font& font, float x, float y, const std::string& str, float value, sf::Color color = sf::Color::White, uint32_t characterSize = 32) {
	sf::Text text(str, font, characterSize);
	text.setPosition({ x, y });
	text.setFillColor(color);

	std::stringstream ss;
	ss << str << " " << value;
	text.setString(ss.str());

	window.draw(text);
}