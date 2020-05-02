#pragma once
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Text.hpp>
#include <sstream>
#include <Windows.h>
using namespace sf;

class Slider {
private:
	RectangleShape sliderBar;
	CircleShape circle;

	int value;
public:
	Slider() {}
	Slider(const Vector2f& pos, const Vector2f& sliderBarSize, const float radius, const Color& sliderBarColor, const Color& circleColor) {
		sliderBar.setSize(sliderBarSize);
		sliderBar.setPosition(pos);
		sliderBar.setFillColor(sliderBarColor);

		circle.setRadius(radius);
		circle.setOrigin({ radius / 2.0f, radius / 2.0f });
		circle.setFillColor(circleColor);
		circle.setPosition(pos);
	}

	void Initialize(const Vector2f& pos, const Vector2f& sliderBarSize, const float radius, const Color& sliderBarColor, const Color& circleColor) {
		sliderBar.setSize(sliderBarSize);
		sliderBar.setPosition(pos);
		sliderBar.setFillColor(sliderBarColor);

		circle.setRadius(radius);
		circle.setOrigin({ radius / 2.0f, radius / 2.0f });
		circle.setFillColor(circleColor);
		circle.setPosition(pos);
	}

	void SetTexture(Texture& texture) {
		sliderBar.setFillColor(Color::White);
		sliderBar.setTexture(&texture);
	}

	bool IsPositionInBounds(const sf::Vector2f& pos) {
		return circle.getGlobalBounds().contains(pos);
	}

	inline int GetValue() const { return value; }

	void Logic(Vector2f mousePos) {
		float sliderLeft = sliderBar.getPosition().x;
		float sliderRight = sliderBar.getPosition().x + sliderBar.getSize().x;

		if (circle.getGlobalBounds().contains(mousePos)) {
			if (mousePos.x >= sliderLeft && mousePos.x <= sliderRight) {
				circle.setPosition({ mousePos.x, circle.getPosition().y });

				value = (int)(circle.getPosition().x - sliderLeft);
			}
		}
	}

	void Render(RenderWindow& window) {
		window.draw(sliderBar);
		window.draw(circle);
	}
};

class Button {
private:
	RectangleShape buttonBox;
	Color colors[3];
	bool isTexture, onPress, isButtonPressed;

	void OnMousePress(Vector2f& mousePos) {
		if (IsPositionInBounds(mousePos)) {
			buttonBox.setFillColor(colors[MousePressed]);
			onPress = true;
		}
	}

	void OnMouseHover(Vector2f& mousePos) {
		if (IsPositionInBounds(mousePos)) {
			buttonBox.setFillColor(colors[MouseHover]);
		}
	}

	void OnMouseRelease(Vector2f& mousePos) {
		if (IsPositionInBounds(mousePos)) {
			buttonBox.setFillColor(colors[MouseRelease]);
			onPress = false;
		}
	}

public:
	Button() {}
	
	Button(const Vector2f& pos, const Vector2f& buttonBoxSize) {
		buttonBox.setSize(buttonBoxSize);
		buttonBox.setPosition(pos);
	
		isTexture = false;
		onPress = false;
		isButtonPressed = false;
	}

	enum ButtonState {
		MousePressed = 0,
		MouseHover = 1,
		MouseRelease = 2
	};
	
	bool IsPositionInBounds(Vector2f& position) {
		return buttonBox.getGlobalBounds().contains(position);
	}

	void ResetColor() {
		buttonBox.setFillColor(colors[2]);
	}

	void Initialize(const Vector2f& pos, const Vector2f& buttonBoxSize) {
		buttonBox.setSize(buttonBoxSize);
		buttonBox.setPosition(pos);
	}

	void SetOutline(float thickness, const Color& color) {
		buttonBox.setOutlineThickness(thickness);
		buttonBox.setOutlineColor(color);
	}

	void SetColors(const Color& onPress, const Color& onHover, const Color& onRelease) {
		colors[MousePressed] = onPress;
		colors[MouseHover] = onHover;
		colors[MouseRelease] = onRelease;
	}

	void SetFillColor(const sf::Color& color) {
		buttonBox.setFillColor(color);
	}

	inline bool GetOnPress() const {
		return onPress;
	}

	bool GetIsPressed() { 
		if (!onPress) isButtonPressed = false;

		if (onPress && !isButtonPressed) {
			isButtonPressed = true;
			return true;
		}

		return false;
	}
	inline sf::Color const* GetColors() const { return colors; }

	inline sf::Vector2f GetPosition() const { return buttonBox.getPosition(); }

	void SetPosition(const sf::Vector2f& pos) {
		buttonBox.setPosition(pos);
	}

	bool SetTexture(Texture& texture) {
		if (texture.getSize().x > buttonBox.getSize().x && texture.getSize().y > buttonBox.getSize().y) {
			buttonBox.setTexture(&texture);
			isTexture = true;
			
			return true;
		}

		return false;
	}

	void Logic(Event e, Vector2f& mousePos) {
		if (!onPress) {
			ResetColor();	
		}

		switch (e.type) {
		case Event::MouseButtonPressed:
			switch (e.key.code) {
			case Mouse::Left:
				OnMousePress(mousePos);
				break;
			}
			break;
		case Event::MouseButtonReleased:
			switch (e.key.code) {
			case Mouse::Left:
				OnMouseRelease(mousePos);
				break;
			}
			break;
		case sf::Event::MouseMoved:
			if (!onPress) OnMouseHover(mousePos);
			break;
		}
	}

	void Render(RenderWindow& window) {
		window.draw(buttonBox);
	}
};

class TextBox {
private:
	RectangleShape box;
	std::ostringstream textStr;
	Text text;
	Color color;

	bool isSelected;

	void Input(uint32_t str) {
		if (isSelected) {
			if (str == 0x08 || str == 0x2E) {
				if (textStr.str().size() > 0) {
					const std::string& initStr = textStr.str();
					std::string newStr;

					for (std::size_t i = 0; i < initStr.size() - 1; i++) {
						newStr += initStr[i];
					}

					textStr.str("");
					textStr << newStr;
				}
			}
			else if (str < 128) {
				textStr << static_cast<char>(str);
			}
		}
	}
public:
	TextBox() {}
	TextBox(const sf::Vector2f& position, const sf::Vector2f& textBoxSize, sf::Color textBoxColor = sf::Color::Black) 
		: color(textBoxColor) {
		box.setSize(textBoxSize);
		box.setPosition(position);
		box.setFillColor(textBoxColor);
	
		isSelected = false;
	}

	void Initialize(const sf::Vector2f& position, const sf::Vector2f& textBoxSize, sf::Color textBoxColor = sf::Color::Black) {

		color = textBoxColor;

		box.setSize(textBoxSize);
		box.setPosition(position);
		box.setFillColor(textBoxColor);

		text.setPosition(position);

		isSelected = false;
	}

	void Logic(sf::Event e) {
		switch (e.type) {
		case sf::Event::TextEntered:
			Input(e.text.unicode);
			break;
		case sf::Event::MouseButtonPressed:
			switch (e.key.code) {
			case sf::Mouse::Left:
				
				isSelected = false;
				
				if (box.getGlobalBounds().contains(Vector2f((float)e.mouseButton.x, (float)e.mouseButton.y))) {
					isSelected = true;
				}
				break;
			}
			break;
		case sf::Event::KeyReleased:
			switch (e.key.code) {
			case sf::Keyboard::Return:
				if (isSelected) isSelected = false;
				textStr.str("");
				break;
			}
			break;
		}

		box.setFillColor(isSelected ? Color(color.r + 25, color.g + 25, color.b + 25) : color);
		text.setString(textStr.str() + (isSelected ? "_" : ""));
	}

	bool GetIsSelected() const { return isSelected; }

	std::string GetString() const {
		return textStr.str();
	}

	void SetPosition(const sf::Vector2f& pos) {
		box.setPosition(pos);
		text.setPosition(pos);
	}

	void SetFont(const Font& font) {
		text.setFont(font);
	}

	void Render(RenderWindow& window) {
		window.draw(box);
		window.draw(text);
	}
};