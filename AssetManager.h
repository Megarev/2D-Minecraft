#pragma once
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <unordered_map>
#include <iostream>

template<typename Asset>
class AssetManager {
private:
	std::unordered_map<std::string, Asset*> assets;
public:
	AssetManager() {}

	bool LoadAsset(const std::string& assetName, const std::string& filepath) {
		Asset* asset = new Asset();
		if (!asset->loadFromFile(filepath)) {
			std::cout << "Couldn't load the asset " << assetName << std::endl;
			delete asset;
			return false;
		}

		assets.insert(std::make_pair(assetName, asset));
		return true;
	}

	const Asset& GetAsset(const std::string& assetName) {
		return *assets[assetName];
	}

	~AssetManager() {
		for (auto it = assets.begin(); it != assets.end();) {
			it = assets.erase(it);
			
			if (!assets.empty()) {
				it++;
			}
		}
	}
};

class AssetHolder {
private:
	AssetManager<sf::Texture> textureManager;
	AssetManager<sf::SoundBuffer> soundManager;
	AssetManager<sf::Font> fontManager;
	
	AssetHolder() {}
public:

	static AssetHolder& Get() {
		static AssetHolder assetMain;
		return assetMain;
	}

	bool AddTexture(const std::string& textureName, const std::string& filepath) {
		return textureManager.LoadAsset(textureName, filepath);
	}
	
	bool AddSoundBuffer(const std::string& soundBufferName, const std::string& filepath) {
		return soundManager.LoadAsset(soundBufferName, filepath);
	}
	
	bool AddFont(const std::string& fontName, const std::string& filepath) {
		return fontManager.LoadAsset(fontName, filepath);
	}

	const sf::Texture& GetTexture(const std::string& textureName) { return textureManager.GetAsset(textureName); }
	const sf::SoundBuffer& GetSoundBuffer(const std::string& soundBufferName) { return soundManager.GetAsset(soundBufferName); }
	const sf::Font& GetFont(const std::string& fontName) { return fontManager.GetAsset(fontName); }
};