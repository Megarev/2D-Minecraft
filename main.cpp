#include <SFML/Graphics.hpp>
#include "AssetManager.h"
#include "GraphicsRender.h"
#include <ctime>
#include <thread>

class GameState {
private:
	void LoadAssets() {
		AssetHolder::Get().AddTexture("TileSet", "files/images/TileSet1.png");
		AssetHolder::Get().AddFont("sansationBold", "files/fonts/Sansation_Bold.ttf");
	}
public:
	sf::Vector2u windowSize = { 512, 512 };
	virtual void Logic(float) = 0;
	virtual void Render(sf::RenderWindow&) = 0;
	virtual void Input() {}
	virtual void ManageEvent(sf::Event e, sf::Vector2f) {}

	GameState() {
		LoadAssets();
	}

	template<class T>
	T GetRandomValue(const std::vector<T>& values) {
		return values[rand() % values.size()];
	}
	
	static bool KeyPress(sf::Keyboard::Key key) {
		return sf::Keyboard::isKeyPressed(key);
	};

	static bool MousePress(sf::Mouse::Button button) {
		return sf::Mouse::isButtonPressed(button);
	}
};

class Player {
private:
	sf::Vector2f position, velocity, offset;
	float tileSize, speed, gSpeed, gMax, jumpSpeed;
	bool isContact;

	bool TileMapCollision(const Level& level) {
		int tileLeft = (int)((position.x + 2) / tileSize);
		int tileRight = (int)ceilf((position.x - 2 + tileSize) / tileSize);
		int tileTop = (int)((position.y + 2) / tileSize);
		int tileBottom = (int)ceilf((position.y - 2 + tileSize) / tileSize);
	
		if (tileLeft < 0 || tileTop < 0 || tileRight > (int)(level.GetWidth() + 1) || tileBottom > (int)(level.GetHeight() + 1)) return false;

		for (int i = tileTop; i < tileBottom; i++) {
			for (int j = tileLeft; j < tileRight; j++) {
				switch (level.GetLevel()[i][j]) {
				case '0':
				case '1':
				case '2':
					return true;
					break;
				}
			}
		}

		return false;
	}
public:
	Player() {}
	Player(const sf::Vector2f& pos, float size)
		: position(pos), tileSize(size) {
		speed = 100.0f;
		gSpeed = 30.0f;
		gMax = 160.0f;
		jumpSpeed = 350.0f;
		isContact = false;
	}

	void Input() {
		if (GameState::KeyPress(sf::Keyboard::A)) velocity.x = -speed;
		if (GameState::KeyPress(sf::Keyboard::D)) velocity.x = speed;
		if (GameState::KeyPress(sf::Keyboard::W) && isContact) {
			velocity.y = -jumpSpeed;
			isContact = false;
		}
	}

	inline sf::Vector2f GetPosition() const { return position; }
	void SetPosition(float x, float y) {
		position = { x, y };
	}
	
	inline sf::Vector2f GetOffset() const { return offset; }
	void SetPlayerOffset(const sf::Vector2u& windowSize) {
		if (position.x > (windowSize.x / 2.0f)) offset.x = (position.x - (windowSize.x / 2.0f));
		if (position.y > (windowSize.y / 2.0f)) offset.y = (position.y - (windowSize.y / 2.0f));
	}

	void Logic(float dt, const Level& level) {
		sf::Vector2f initPosition;

		initPosition.x = position.x;
		position.x += velocity.x * dt;
		if (TileMapCollision(level)) {
			position.x = initPosition.x;

		}

		initPosition.y = ceilf(position.y);
		isContact = false;

		if (!isContact) velocity.y += gSpeed;
		if (velocity.y > gMax) velocity.y = gMax;

		position.y += velocity.y * dt;
		if (TileMapCollision(level)) {
			position.y = initPosition.y;
			if (velocity.y > 0.0f) {
				isContact = true;
				velocity.y = 0.0f;
			}
		}
	
		velocity.x = 0.0f;
	}
};

class InventoryUI {
private:
	std::vector<std::pair<int, char>> tiles;
public:
	InventoryUI() {}

	void AddTile(char tileCharacter) {

		if (tileCharacter == '.') return;

		bool isTileInVector = false;

		for (auto& tile : tiles) {
			if (tileCharacter == tile.second) {
				tile.first++;
				isTileInVector = true;
				break;
			}
		}

		if (!isTileInVector) {
			tiles.emplace_back(1, tileCharacter);
		}
	}

	void PlaceTile(uint32_t x, uint32_t y, char tileCharacter, Level& level) {

		if (level.GetCharacter(x, y) != '.') return;
		
		bool isTileInContact = false;
		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				if (level.GetCharacter(x + j, y + i) != '.') {
					isTileInContact = true;
				}
			}
		}

		if (!isTileInContact) return;

		bool isTileInVector = false;

		for (auto& tile : tiles) {
			if (tileCharacter == tile.second) {
				if (tile.first > 0) {
					isTileInVector = true;
					tile.first--;
					break;
				}
			}
		}

		if (isTileInVector) {
			level.SetCharacter(x, y, tileCharacter);
		}
	}

	void Render(sf::RenderWindow& window, char tileCharacter) {

		if (GameState::KeyPress(sf::Keyboard::Q)) {
			for (auto& tile : tiles) {
				std::cout << tile.second << " : " << tile.first << std::endl;
			}
		}

		int tileCount = 0;
		for (auto& tile : tiles) {
			if (tileCharacter == tile.second) {
				tileCount = tile.first;
				break;
			}
		}

		DrawTextWithValue(window, AssetHolder::Get().GetFont("sansationBold"), 32.0f, -2.0f, "", tileCount);
	}
};

class PlayState : public GameState {
private:
	Level gameLevel;
	float tileSize;
	sf::Sprite tile, inventoryTile;
	sf::Vector2u levelSize;

	InventoryUI inventoryUI;
	std::string tileString;
	int index;

	Player player;
	sf::RectangleShape box;

	Level GenerateTerrain(uint32_t w, uint32_t h) {
		Level level;
		level.InitializeLevelString(w, h);

		//Height Map
		float startingHeight = (windowSize.y / (2.0f * tileSize)) + GetRandomValue<int>({ -3, -2, -1, 0, 1, 2, 3 });
		float alternateHeight = startingHeight;

		float alternateLevel = 0.0f;

		for (int i = 0; i < (int)w; i++) {
			float hLevel1 = alternateHeight + GetRandomValue<int>({ 2, 3, 3, 4, 5, 4 });
			float hLevel2 = h;

			level.SetCharacter((uint32_t)i, (uint32_t)alternateHeight, '0');

			for (int j = alternateHeight; j < hLevel1; j++) {
				level.SetCharacter((uint32_t)i, (uint32_t)(j + 1), '1');
				alternateLevel = j;
			}

			for (int j = alternateLevel; j < hLevel2; j++) {
				level.SetCharacter((uint32_t)i, (uint32_t)(j + 2), '2');
			}

			alternateHeight += GetRandomValue<int>({ 0, 0, 1, 1, 1 }) * GetRandomValue<int>({ -1, 1 });
		}

		return level;
	}

	void ResetTerrain(uint32_t w, uint32_t h) {
		system("cls");
		
		gameLevel.ClearLevel();
		levelSize = { w, h };
		gameLevel.InitializeLevelString(w, h);
		gameLevel = GenerateTerrain(w, h);
		gameLevel.PrintLevel();
	}

	void SetTile(int x, int y) {
		tile.setTextureRect(sf::IntRect(x * tileSize, y * tileSize, (int)tileSize, (int)tileSize));
	}
public:
	PlayState() {
		tileSize = 32.0f;
		tileString = "012";
		index = 1;

		gameLevel = GenerateTerrain(128, 25);
		levelSize = { gameLevel.GetWidth(), gameLevel.GetHeight() };
		gameLevel.PrintLevel();

		player = Player({ 0.0f, 0.0f }, tileSize);
		box.setSize({ 30.0f, 30.0f });
		box.setFillColor(sf::Color::Yellow);

		tile.setTexture(AssetHolder::Get().GetTexture("TileSet"));
		inventoryTile.setTexture(AssetHolder::Get().GetTexture("TileSet"));
	}

	void Input() override {
		player.Input();
	}

	void Logic(float dt) override {
		player.Logic(dt, gameLevel);
		player.SetPlayerOffset(windowSize);
		box.setPosition(player.GetPosition() - player.GetOffset());

		inventoryTile.setTextureRect(sf::IntRect(index * tileSize, 0, (int)tileSize, (int)tileSize));
	}

	void ManageEvent(sf::Event e, sf::Vector2f mousePos) override {
		switch (e.type) {
		case sf::Event::KeyPressed:
			switch (e.key.code) {
			case sf::Keyboard::Space:
				player.SetPosition(0.0f, 0.0f);
				ResetTerrain(128, 25);
				break;
			}
			break;
		case sf::Event::MouseWheelScrolled:
			switch ((int)e.mouseWheelScroll.delta) {
			case -1:
				index--;
				if (index < 0) index = tileString.size() - 1;
				break;
			case 1:
				index++;
				if (index > (tileString.size() - 1)) index = 0;
				break;
			}
			break;
		case sf::Event::MouseButtonPressed:
			
			auto [x, y] = sf::Vector2u((uint32_t)((e.mouseButton.x + player.GetOffset().x) / tileSize),
									   (uint32_t)((e.mouseButton.y + player.GetOffset().y) / tileSize));
			
			switch (e.key.code) {
			case sf::Mouse::Left:
				inventoryUI.AddTile(gameLevel.GetCharacter(x, y));
				gameLevel.SetCharacter(x, y, '.');
				break;
			case sf::Mouse::Right:
				inventoryUI.PlaceTile(x, y, tileString[index], gameLevel);
				break;
			}
			break;
		}
	}

	virtual void Render(sf::RenderWindow& window) {

		uint32_t tileLeft = (player.GetOffset().x / tileSize);
		uint32_t tileRight = tileLeft + (windowSize.x / tileSize) + 1;
		uint32_t tileTop = (player.GetOffset().y / tileSize);
		uint32_t tileBottom = tileTop + (windowSize.y / tileSize) + 1;

		if (tileRight > gameLevel.GetWidth()) tileRight = gameLevel.GetWidth();
		if (tileBottom > gameLevel.GetHeight()) tileBottom = gameLevel.GetHeight();

		for (uint32_t i = tileTop; i < tileBottom; i++) {
			for (uint32_t j = tileLeft; j < tileRight; j++) {
				switch (gameLevel.GetLevel()[i][j]) {
				case '0':
					SetTile(0, 0);
					break;
				case '1':
					SetTile(1, 0);
					break;
				case '2':
					SetTile(2, 0);
					break;
				case '.':
					continue;
					break;
				}

				tile.setPosition({ (float)((int)(j * tileSize - player.GetOffset().x)), (float)((int)(i * tileSize - player.GetOffset().y)) });
				window.draw(tile);
			}
		}
	
		inventoryUI.Render(window, tileString[index]);
		window.draw(inventoryTile);
		window.draw(box);
	}
};

class Game {
private:
	sf::RenderWindow Window;
	sf::Vector2u WindowSize;

	sf::Clock clock;
	float initDt;

	bool renderFPS;

	std::vector<std::unique_ptr<GameState>> gameStates;

	void PushState(const std::string_view gameState) {
		if (gameState == "PlayState") {
			gameStates.push_back(std::make_unique<PlayState>());
		}
	}

	void PopState() {
		if (gameStates.size() > 0) {
			gameStates.pop_back();
		}
	}
public:
	Game(uint32_t x, uint32_t y, const sf::String& title)
		: WindowSize(x, y),
		  Window({ x, y }, title) {
		Window.setFramerateLimit(60);

		PushState("PlayState");

		initDt = 0.0f;
		renderFPS = false;
	}

	void Logic() {

		while (Window.isOpen()) {
			sf::Event e;

			std::unique_ptr<GameState>& currentGameState = gameStates.back();

			float currentDt = (float)clock.getElapsedTime().asSeconds();
			float frameDt = (currentDt - initDt);
			initDt = currentDt;
			if (frameDt > 0.25f) frameDt = 0.25f;

			sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(Window);

			while (Window.pollEvent(e)) {
				switch (e.type) {
				case sf::Event::Closed:
					Window.close();
					break;
				case sf::Event::KeyPressed:
					switch (e.key.code) {
					case sf::Keyboard::Z:
						renderFPS = !renderFPS ? true : false;
						break;
					}
					break;
				}

				currentGameState->ManageEvent(e, mousePos);
			}

			currentGameState->Input();
			currentGameState->Logic(frameDt);

			Window.clear(sf::Color(80, 120, 255));
			currentGameState->Render(Window);
			if (renderFPS) {
				DrawTextWithValue(Window, AssetHolder::Get().GetFont("sansationBold"), 0.0f, 36.0f, "FPS : ", 1 / frameDt);
			}
			Window.display();
		}
	}

	void Run() {
		Logic();
	}
};

int main() {

	srand((unsigned)time(0));

	Game game(512, 512, "Game");
	game.Run();

	return 0;
}