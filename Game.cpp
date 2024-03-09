#include "Game.h"

void Game::initVariables()
{
	this->endgame = false;
	this->window = nullptr;
	currentlevel = 1;
	this->points = 0;
	this->health = 50;
	this->enemySpawnTimerMax = 75.f;
	this->enemySpawnTimer = this->enemySpawnTimerMax;
	this->maxEnemies = 10;
	this->MouseHold = false;
	this->backgroundVelocity = 1.f;
	this->asteroidspeed = 1.0f;
}
void Game::initWindow()
{
	this->videomode.height = 600;
	this->videomode.width = 800;
	this->window = new sf::RenderWindow(sf::VideoMode(1400,800), "Game1", sf::Style::Titlebar | sf::Style::Close);

	this->window->setFramerateLimit(144);

	this->backgroundView.setSize(this->window->getSize().x, this->window->getSize().y);
	this->backgroundView.setCenter(this->window->getSize().x / 2, this->window->getSize().y / 2);;
	//this->backgroundView.zoom(1.f);
}
void Game::initFonts()
{
	if (this->font.loadFromFile("Fonts.ttf"))
	{
		std::cout << "ERROR: initfonts Failed to load" << std::endl;
	}
}
void Game::inittext()
{
	this->uiText.setFont(this->font);
	this->uiText.setCharacterSize(18);
	this->uiText.setFillColor(sf::Color::Red);
	this->uiText.setString("NONE");

	this->gameOverText.setFont(this->font);
	this->gameOverText.setCharacterSize(50);
	this->gameOverText.setFillColor(sf::Color::Red);
	this->gameOverText.setString("GAME OVER! \n you won");
	this->gameOverText.setPosition(800 / 2.f, 600 / 2.f);

}
void Game::initEnemies()
{
	this->enemies.clear();
	if (!this->asteroidTexture.loadFromFile("asteroid01.png"))
	{
		std::cout << "ERROR: Could not load asteroid texture" << std::endl;
	}
	else
	{
		this->enemy.setTexture(&this->asteroidTexture); // Set the texture only if it's loaded successfully
	}
	this->enemy.setPosition(10.0f, 10.0f);
	this->enemy.setSize(sf::Vector2f(50.f, 50.f));
	this->enemy.setScale(sf::Vector2f(0.5f, 0.5f));// reduces the size of enemy by half (initial value of scale is 1) so it becomes 0.5*1
	this->enemy.setTexture(&this->asteroidTexture); // Use the asteroid texture
	this->enemy.setOutlineThickness(1.f);
	this->enemy.setOutlineColor(sf::Color::Black);
}
void Game::initBackground()
{
	if (!backgroundTexture.loadFromFile("space2.jpg"))
	{
		std::cout << "ERROR loading background" << std::endl;
	}
	// Initialize the first background sprite
	backgroundSprite1.setTexture(backgroundTexture);
	backgroundSprite1.setScale(
		float(window->getSize().x) / backgroundTexture.getSize().x,
		float(window->getSize().y) / backgroundTexture.getSize().y);
	// Initialize the second background sprite, positioned above the first
	backgroundSprite2.setTexture(backgroundTexture);
	backgroundSprite2.setScale(backgroundSprite1.getScale());
	backgroundSprite2.setPosition(0, -static_cast<float>(window->getSize().y));
}
Game::Game()
{
	this->initVariables();
	this->initWindow();
	this->initFonts();
	this->inittext();
	this->initEnemies();
	this->initBackground();
	this->initAudio();
}
Game::~Game()
{
	delete this->window;
}
const bool Game::running() const
{
	return this->window->isOpen();
}
const bool Game::getEndgame() const
{
	return this->endgame;
}
void Game::spawnEnemy()
{
	this->enemy.setTexture(&this->asteroidTexture);
	this->enemy.setPosition(
		static_cast<float>(rand() % static_cast<int>(this->window->getSize().x - this->enemy.getSize().x)),
		0.f
	);
	int type = rand() % 5;
	float scale;
	switch (type)
	{
	case 0:
		scale = 0.2f;
		break;
	case 1:
		scale = 0.4f;
		break;
	case 2:
		scale = 0.6f;
		break;
	case 3:
		scale = 0.8f;
		break;
	case 4:
		scale = 1.0f;
		break;
	default:
		scale = 1.2f;
		break;
	}
	float baseSizeX = 272; 
	float baseSizeY = 184;
	this->enemy.setSize(sf::Vector2f(baseSizeX * scale, baseSizeY * scale));
	this->enemies.push_back(this->enemy);
}
void Game::PollEvents()
{
	while (this->window->pollEvent(this->ev))
	{
		switch (this->ev.type)
		{
		case sf::Event::Closed:
			this->window->close();
			break;
		}
	}
}
void Game::UpdateMousePos()
{
	this->mousePosWindow = sf::Mouse::getPosition(*this->window);
	this->mousePosView = this->window->mapPixelToCoords(this->mousePosWindow);
}
void Game::updateText()
{
	std::stringstream ss;
	ss << "Points: " << this->points << std::endl << "health: " << this->health << std::endl << "Level: " << this->currentlevel<< std::endl;
	this->uiText.setString(ss.str());
}
//moving and upadating the enemy
void Game::UpdateEnemies()
{
	// If the game has ended, no need to update enemies
	if (this->endgame)
		return;

	// Check if it's time to spawn a new enemy
	if (this->enemies.size() < this->maxEnemies)
	{
		if (this->enemySpawnTimer >= this->enemySpawnTimerMax)
		{
			// Spawn the enemy and reset the timer
			this->spawnEnemy();
			this->enemySpawnTimer = 0.f;
		}
		else
			this->enemySpawnTimer += 1.f;
	}

	// Move and update the enemies
	for (int i = 0; i < this->enemies.size(); i++)
	{
		bool deleted = false;

		this->enemies[i].move(0.f, this->enemies[i].getScale().y * 5.f);

		if (this->enemies[i].getPosition().y > this->window->getSize().y)
		{
			this->enemies.erase(this->enemies.begin() + i);
			this->health -= 1;
			deleted = true;
		}
		if (!deleted)
		{
			// Check if an enemy was clicked
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				if (this->enemies[i].getGlobalBounds().contains(this->mousePosView))
				{
					// If enemy was clicked, remove it and increase points
					deleted = true;

					// Get the scale of the asteroid
					float scale = this->enemies[i].getScale().x; // Assuming uniform scaling in x and y

					// Initialize pointsAwarded variable
					int pointsAwarded = 0;

					// Compare the scale and award points accordingly
					if (scale >= 0.7f)
					{
						pointsAwarded = 10; // Award more points for smaller asteroids
					}
					else if (scale < 0.7f && scale >= 0.4f)
					{
						pointsAwarded = 8; // Award fewer points for larger asteroids
					}
					else if (scale < 0.4f)
					{
						pointsAwarded = 5;
					}

					// Increase points based on the scale of the asteroid
					this->points += pointsAwarded;

					// Play the asteroid click sound
					this->asteroidClickSound.play();

					// Remove the enemy from the vector
					this->enemies.erase(this->enemies.begin() + i);
				}
			}
		}
	}
}
void Game::Update()
{
	this->PollEvents();
	this->UpdateMousePos();
	this->updateText();
		if (this->points > 500)
		{
			this->endgame = true; // Set the endgame condition to true
		}
		// Check points and update the level accordingly
		else if (this->points > 300)
		{
			this->currentlevel = 3;
			this->health = 50; // Reset health to 50
			this->asteroidspeed += 150.0f; // Increased asteroid speed for level 3
			this->backgroundVelocity = 5.f; // Increased background velocity for level 3
			// Update other game parameters for each level, if needed
		}
		else if (this->points > 100)
		{
			this->currentlevel = 2;
			this->health = 50; // Reset health to 50
			this->asteroidspeed += 125.0f; // Increased asteroid speed for level 2
			this->backgroundVelocity = 2.f; // Increased background velocity for level 2
			// Update other game parameters for each level, if needed
		}
	if (this->health <= 0)
	{
		this->endgame = true;
	}// Always update the mouse position and the text to reflect the current score and health
	if (!this->endgame)
	{ // Move both background sprites down
		backgroundSprite1.move(0, backgroundVelocity);
		backgroundSprite2.move(0, backgroundVelocity);
		// Check if the first background sprite has moved completely off the bottom of the screen
		if (backgroundSprite1.getPosition().y >= window->getSize().y)
		{
			backgroundSprite1.setPosition(0, -static_cast<float>(window->getSize().y));
		}
		// Check if the second background sprite has moved completely off the bottom of the screen
		if (backgroundSprite2.getPosition().y >= window->getSize().y)
		{
			backgroundSprite2.setPosition(0, -static_cast<float>(window->getSize().y));
		}
		this->UpdateEnemies();
	}
	else
	{// Game over conditions
		this->gameOverText.setPosition(sf::Vector2f(800 / 2.0f - this->gameOverText.getGlobalBounds().width / 2.0f, 600 / 2.0f - this->gameOverText.getGlobalBounds().height));
		this->tryagainText.setPosition(sf::Vector2f(800 / 2.0f - this->tryagainText.getGlobalBounds().width / 2.0f, 600 / 2.0f + 30.f));
	}
}
void Game::renderText(sf::RenderTarget& target)
{
	target.draw(this->uiText);
}
void Game::initAudio()
{

	if (!asteroidClickBuffer.loadFromFile("asaudio.mp3"))
	{
		std::cout << "ERROR: Loading sound" << std::endl;
	}
	asteroidClickSound.setBuffer(asteroidClickBuffer);
}
void Game::RenderEnemies(sf::RenderTarget& target)
{
	for (auto& e : this->enemies)
	{
		target.draw(e);
	}
}
void Game::Render()
{
	this->window->clear();
	this->window->draw(backgroundSprite1);
	this->window->draw(backgroundSprite2);
	if (!this->endgame)
	{
		this->renderText(*this->window);
		this->RenderEnemies(*this->window);
	}
	else
	{// Draw game over and try again texts
		this->window->draw(this->gameOverText);
		this->window->draw(this->tryagainText);
	}
	this->window->display();
}