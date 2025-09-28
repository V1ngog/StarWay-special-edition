#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <algorithm>

using namespace sf;

// Состояния игры
enum class GameState {Game, Menu, LeaderBoard, Paused};
GameState state = GameState::Menu;

// Загрузка лучших результатов
std::vector<int> loadBestScores() {
    std::vector<int> scores;
    std::ifstream file("save.txt");
    int value;
    while (file >> value) {
        scores.push_back(value);
    }

    std::sort(scores.begin(), scores.end(), std::greater<int>());

    while (scores.size() < 3) scores.push_back(0);
    if (scores.size() > 3) scores.resize(3);

    return scores;
}

// Сохранение лучших результатов
void saveBestScores(const std::vector<int>& scores) {
    std::ofstream file("save.txt", std::ios::trunc);
    for (size_t i = 0; i < scores.size() && i < 3; ++i) {
        file << scores[i] << "\n";
    }
}

// Обновление текста топлива
void static showPetrol(int& petrol, Text& petrolText) {
    petrolText.setString("Petrol: " + std::to_string(petrol));
    petrolText.setPosition(10.f, 10.f);
}

// Обновление текста счёта
void static showScore(int& score, Text& scoreText) {
    scoreText.setString("Score: " + std::to_string(score));
    FloatRect textBounds = scoreText.getLocalBounds();
    scoreText.setPosition(800.f - textBounds.width - 10.f, 10.f);
}

void updateText(Text& text, const std::string& label, int value, float x, float y) {
    text.setString(label + ": " + std::to_string(value));
    text.setPosition(x, y);
}
// Создание спрайта с центровкой и масштабированием(Жаль что я сначала пишу потом думаю)
//Sprite createSprite(const Texture& texture, float scale, float posX, float posY) {
//    Sprite s(texture);                     
//    s.setScale(scale, scale);              
//    FloatRect bounds = s.getLocalBounds();  
//    s.setOrigin(bounds.width / 2.f, bounds.height / 2.f); 
//    s.setPosition(posX, posY);             
//    return s;                               
//}

//Создание текста ( жаль я только щас допёр что так можно(((( )
Text createText(const std::string& str, Font& font, unsigned int size, Color color, float x, float y) {
    Text t(str, font, size);
    t.setFillColor(color);
    t.setPosition(x, y);
    return t;
}

// Сброс игры
void resetGame(
    sf::Sprite& ship,
    std::vector<sf::Sprite>& asteroids,
    std::vector<sf::Sprite>& conisters,
    std::vector<sf::Sprite>& bullets,
    sf::Clock& spawnClock,
    sf::Clock& timeSpawnAsteroidClock,
    sf::Clock& timeScore,
    int& score,
    sf::Text& scoreText,
    float& speed,
    sf::Clock& timeSpawnPetrol,
    float& timeSpawnAsteroid,
    int& petrol,
    sf::Text& petrolText,
    sf::Clock& timeDecreasePetrol,
    float& intTimeSpawnBullet,
    float& intTimeDecreasePetrol,
    float& intTimeScore,
    int& numberBullet,
    sf::Text& bulletText
) {
    // позиция корабля
    ship.setPosition(400.f, 300.f);

    // очистка контейнеров
    asteroids.clear();
    conisters.clear();
    bullets.clear();

    // перезапуск часов
    spawnClock.restart();
    timeSpawnAsteroidClock.restart();
    timeScore.restart();
    timeSpawnPetrol.restart();
    timeDecreasePetrol.restart();

    // восстановление значений
    score = 0;
    speed = 5.f;
    timeSpawnAsteroid = 1.f;
    petrol = 100;
    intTimeSpawnBullet = 5.f;
    intTimeDecreasePetrol = 1.f;
    intTimeScore = 1.f;
    numberBullet = 5;

    // обновляем тексты
    showScore(score, scoreText);
    showPetrol(petrol, petrolText);

    // обновляем текст пуль и выставляем его под petrolText
    FloatRect boundsPetrol = petrolText.getGlobalBounds();
    updateText(bulletText, "Bullet", numberBullet, 10.f, boundsPetrol.top + boundsPetrol.height + 10.f);
}



int main() {
    setlocale(LC_ALL, "RU");

	// Создание окна
    const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
    int score = 0, petrol = 100, numberBullet = 5;
    std::vector<int> bestScores = loadBestScores();
    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Star Way");
    window.setFramerateLimit(60);

	// Инициализация рандома
    srand((unsigned)(time(0)));

    // Загрузка текстур
    Texture ShipTexture;
    if (!ShipTexture.loadFromFile("image/ship.png")) {
        MessageBox(NULL, L"Не удалось загрузить картинку корабля!", L"Ошибка", MB_OK | MB_ICONERROR);
        return -1;
    }

    Texture BackgroundTexture;
    if (!BackgroundTexture.loadFromFile("image/background.png")) {
        MessageBox(NULL, L"Не удалось загрузить картинку фона!", L"Ошибка", MB_OK | MB_ICONERROR);
        return -1;
    }

    Texture asteroidTexture;
    if (!asteroidTexture.loadFromFile("image/asteroid.png")) {
        MessageBox(NULL, L"Не удалось загрузить картинку астероида!", L"Ошибка", MB_OK | MB_ICONERROR);
        return -1;
    }

    Texture PetrolTexture;
    if (!PetrolTexture.loadFromFile("image/petrol.png")) {
        MessageBox(NULL, L"Не удалось загрузить картинку конистры!", L"Ошибка", MB_OK | MB_ICONERROR);
        return -1;
    }

    Texture BulletTexture;
    if (!BulletTexture.loadFromFile("image/bullet.png")) {
        MessageBox(NULL, L"Не удалось загрузить картинку пули!", L"Ошибка", MB_OK | MB_ICONERROR);
        return -1;
    }

	// Загрузка шрифта
    Font MyFont;
    if (!MyFont.loadFromFile("font/myfont.ttf")) {
        MessageBox(NULL, L"Не удалось загрузить шрифт!", L"Ошибка", MB_OK | MB_ICONERROR);
        return -1;
    }

	// Загрузка звуков
    SoundBuffer SoundPetrolBuff;
    Sound SoundPetrol;
    if (!SoundPetrolBuff.loadFromFile("sound/petrol.wav")) {
        MessageBox(NULL, L"Не удалось загрузить звук конистры!", L"Ошибка", MB_OK | MB_ICONERROR);
        return -1;
    }

    SoundBuffer SoundAsteroidBuff;
    Sound SoundAsteroid;
    if (!SoundAsteroidBuff.loadFromFile("sound/laugh.wav")) {
        MessageBox(NULL, L"Не удалось загрузить звук астероида!", L"Ошибка", MB_OK | MB_ICONERROR);
        return -1;
    }

    SoundBuffer SoundConflictBuff;
    Sound SoundConflict;
    if (!SoundConflictBuff.loadFromFile("sound/conflict.wav")) {
        MessageBox(NULL, L"Не удалось загрузить звук астероида!", L"Ошибка", MB_OK | MB_ICONERROR);
        return -1;
    }

    SoundConflict.setBuffer(SoundConflictBuff);
    SoundAsteroid.setBuffer(SoundAsteroidBuff);
    SoundPetrol.setBuffer(SoundPetrolBuff);

	// Создание текста
    Text scoreText;
    scoreText.setFont(MyFont);
    scoreText.setString("Score: " + std::to_string(score));
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(Color::White);

    Text petrolText;
    petrolText.setFont(MyFont);
    petrolText.setString("Petrol: " + std::to_string(petrol));
    petrolText.setCharacterSize(24);
    petrolText.setFillColor(Color::White);

    Text startText("START", MyFont, 40);
    startText.setFillColor(Color::White);
    startText.setPosition(350, 200);


    Text exitText("EXIT", MyFont, 40);
    exitText.setFillColor(Color::White);
    exitText.setPosition(360, 400);

    Text leaderText("LEADERBOARD", MyFont, 40);
    leaderText.setFillColor(Color::White);
    leaderText.setPosition(260, 300);

    Text exitFromLeaderBoard("exit", MyFont, 20);
    exitFromLeaderBoard.setFillColor(Color::White);
    exitFromLeaderBoard.setPosition(750, 570);

    Text pauseTextPaused("PAUSED", MyFont, 48);
    pauseTextPaused.setFillColor(Color::Red);
    FloatRect tb = pauseTextPaused.getLocalBounds();
    pauseTextPaused.setPosition(WINDOW_WIDTH / 2.f - tb.width / 2.f, WINDOW_HEIGHT / 2.f - tb.height / 2.f);

    Text pauseTextExit("Exit", MyFont, 48);
    pauseTextExit.setFillColor(Color::Red);
    FloatRect ht = pauseTextExit.getGlobalBounds();
    pauseTextExit.setPosition(
        WINDOW_WIDTH / 2.f - ht.width / 2.f,
        (WINDOW_HEIGHT / 2.f - tb.height / 2.f) + tb.height + 20.f
    );

    FloatRect boundsPetrol = petrolText.getGlobalBounds();
    Text bulletText = createText("Bullet: 5", MyFont, 20, Color::Yellow, 10.f, boundsPetrol.top + boundsPetrol.height + 5.f);

	// Создание спрайтов
    Sprite background(BackgroundTexture);
    background.setScale(
        WINDOW_WIDTH / (float)(BackgroundTexture.getSize().x),
        WINDOW_HEIGHT / (float)(BackgroundTexture.getSize().y)
    );

    Sprite ship(ShipTexture);
    ship.setScale(0.2f, 0.2f);
    ship.setOrigin(ShipTexture.getSize().x / 2.f, ShipTexture.getSize().y / 2.f);
    ship.setPosition(400.f, 300.f);

   

    float speedShip = 5.f, speed = 5.f, scalePetrol = 0.1f, scaleBullet = 0.5f, speedBullet = 10.f;
    std::vector<Sprite> asteroids;
    std::vector<Sprite> conisters;
    std::vector<Sprite> bullets;
    Clock spawnClock, timeSpawnAsteroidClock, timeScore, timeSpawnPetrol, timeDecreasePetrol, timeSpawnBullet;
    float timeSpawnAsteroid = 1.f, intTimeSpawnBullet = 5.f, intTimeDecreasePetrol = 1.f, intTimeScore = 1.f;
    bool gameOver = false, showMessage = false;
    std::string inputBuffer;

	// Главный цикл
    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) {
                window.close();
            }

            if (e.type == Event::TextEntered) {
                if (e.text.unicode == '\b') {

                    if (!inputBuffer.empty())
                        inputBuffer.pop_back();
                }
                else if (e.text.unicode < 128) {
                    inputBuffer += static_cast<char>(e.text.unicode);
                }

                if (inputBuffer.find("bullinf") != std::string::npos) {
                    intTimeSpawnBullet = 0.f; 

                    inputBuffer.clear();
                }

                if (inputBuffer.find("petrolinf") != std::string::npos) {
                    intTimeDecreasePetrol = 0.f;

                    inputBuffer.clear();
                }

                if (inputBuffer.find("scoreinf") != std::string::npos) {
                    intTimeScore = 0.f;

                    inputBuffer.clear();
                }
            }
			// Menu mouse events
            if (state == GameState::Menu && e.type == Event::MouseButtonPressed) {
                Vector2i mousePos = Mouse::getPosition(window);

                if (startText.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                    state = GameState::Game;
                    resetGame(
                        ship,
                        asteroids,
                        conisters,
                        bullets,
                        spawnClock,
                        timeSpawnAsteroidClock,
                        timeScore,
                        score,
                        scoreText,
                        speed,
                        timeSpawnPetrol,
                        timeSpawnAsteroid,
                        petrol,
                        petrolText,
                        timeDecreasePetrol,
                        intTimeSpawnBullet,
                        intTimeDecreasePetrol,
                        intTimeScore,
                        numberBullet,
                        bulletText
                    );

                    gameOver = false;
                    showMessage = false;
                }

                if (leaderText.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                    state = GameState::LeaderBoard;
                }

                if (exitText.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                    window.close();
                }
            }

			// Leaderboard menu mouse events
            if (state == GameState::LeaderBoard && e.type == Event::MouseButtonPressed) {
                Vector2i mousePos = Mouse::getPosition(window);
                if (exitFromLeaderBoard.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                    state = GameState::Menu;
                }
            }

			// Paused menu key events
            if (state == GameState::Game && e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                state = GameState::Paused;
            }

            else if (state == GameState::Paused && e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                state = GameState::Game;
            }

			// Paused menu mouse events
            if (state == GameState::Paused && e.type == Event::MouseButtonPressed) {
                Vector2i mousePos = Mouse::getPosition(window);

                if (pauseTextExit.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                    state = GameState::Menu;
                }
            }
            
            if (state == GameState::Game && e.type == Event::KeyPressed && e.key.code == Keyboard::Space) {

                if (numberBullet > 0) {
                    Sprite bull(BulletTexture);

                    bull.setScale(scaleBullet, scaleBullet);
                    FloatRect localBounds = bull.getLocalBounds();
                    bull.setOrigin(localBounds.width, localBounds.height / 2.f);

                    Vector2f shipPos = ship.getPosition();
                    FloatRect shipBounds = ship.getGlobalBounds();
                    bull.setPosition(shipPos.x + shipBounds.width / 2.f, shipPos.y);

                    bullets.push_back(bull);
                    numberBullet--;
                }
            }
        }

        window.clear(Color::Black);

		// Меню логика
        if (state == GameState::LeaderBoard) {
            std::vector<sf::Text> leaderboardTexts;
            for (int i = 0; i < 3; ++i) {
                sf::Text t(" " + std::to_string(i + 1) + ". " + std::to_string(bestScores[i]), MyFont, 28);
                t.setFillColor(sf::Color::Yellow);
                t.setPosition(10.f, 10.f + i * 40.f);
                leaderboardTexts.push_back(t);
            }
            window.draw(exitFromLeaderBoard);
        }

		// Игровой процесс логика
        else if (state == GameState::Game) {
            if (!gameOver) {
                // Управление кораблём
                if (Keyboard::isKeyPressed(Keyboard::Up)) ship.move(0, -speedShip);
                if (Keyboard::isKeyPressed(Keyboard::Down)) ship.move(0, speedShip);
                if (Keyboard::isKeyPressed(Keyboard::Left)) ship.move(-speedShip, 0);
                if (Keyboard::isKeyPressed(Keyboard::Right)) ship.move(speedShip, 0);

                // Границы окна
                FloatRect bounds = ship.getGlobalBounds();
                if (bounds.left < 0) ship.setPosition(bounds.width / 2.f, ship.getPosition().y);
                if (bounds.left + bounds.width > WINDOW_WIDTH) ship.setPosition(WINDOW_WIDTH - bounds.width / 2.f, ship.getPosition().y);
                if (bounds.top < 0) ship.setPosition(ship.getPosition().x, bounds.height / 2.f);
                if (bounds.top + bounds.height > WINDOW_HEIGHT) ship.setPosition(ship.getPosition().x, WINDOW_HEIGHT - bounds.height / 2.f);


                // Спавн астероидов
                if (spawnClock.getElapsedTime().asSeconds() > timeSpawnAsteroid) {
                    Sprite asteroid(asteroidTexture);

                    float scale = 0.15f + static_cast<float>(rand()) / RAND_MAX * 0.1f;
                    asteroid.setScale(scale, scale);

                    FloatRect localBounds = asteroid.getLocalBounds();
                    asteroid.setOrigin(localBounds.width / 2.f, localBounds.height / 2.f);

                    float posY = static_cast<float>(rand() % WINDOW_HEIGHT);
                    float halfHeight = (localBounds.height * scale) / 2.f;
                    if (posY < halfHeight) posY = halfHeight;
                    if (posY > WINDOW_HEIGHT - halfHeight) posY = WINDOW_HEIGHT - halfHeight;

                    asteroid.setPosition(WINDOW_WIDTH + (localBounds.width * scale) / 2.f, posY);
                    asteroids.push_back(asteroid);
                    spawnClock.restart();
                }

				// Увеличение сложности
                if (timeSpawnAsteroidClock.getElapsedTime().asSeconds() > 10.f) {
                    if (timeSpawnAsteroid > 0.5) {
                        timeSpawnAsteroid -= 0.1f;
                    }
                    if (speed < 15) {
                        speed += 1.f;
                    }
                    timeSpawnAsteroidClock.restart();
                }

                // Движение астероидов
                for (auto& a : asteroids) a.move(-speed, 0.f);

                asteroids.erase(
                    std::remove_if(asteroids.begin(), asteroids.end(),
                        [](const Sprite& a) { return a.getPosition().x < -200.f; }),
                    asteroids.end()
                );

                // Проверка столкновений
                for (auto& a : asteroids) {
                    if (ship.getGlobalBounds().intersects(a.getGlobalBounds())) {
                        SoundAsteroid.play();
                        gameOver = true;
                    }
                }

				// Спавн канистр
                if (timeSpawnPetrol.getElapsedTime().asSeconds() > 5.f) {
                    Sprite conister(PetrolTexture);

                    conister.setScale(scalePetrol, scalePetrol);

                    FloatRect localBounds = conister.getLocalBounds();
                    conister.setOrigin(localBounds.width / 2.f, localBounds.height / 2.f);

                    float posY = static_cast<float>(rand() % WINDOW_HEIGHT);
                    float halfHeight = (localBounds.height * scalePetrol) / 2.f;
                    if (posY < halfHeight) posY = halfHeight;
                    if (posY > WINDOW_HEIGHT - halfHeight) posY = WINDOW_HEIGHT - halfHeight;

                    conister.setPosition(WINDOW_WIDTH + (localBounds.width * scalePetrol) / 2.f, posY);
                    conisters.push_back(conister);
                    timeSpawnPetrol.restart();
                }

				// Движение канистр
                for (auto& a : conisters) {
                    a.move(-speed, 0);
                }

				// Удаление канистр за пределами экрана
                conisters.erase(
                    std::remove_if(conisters.begin(), conisters.end(),
                        [](const Sprite& a) { return a.getPosition().x < -200.f; }),
                    conisters.end()
                );

				// Проверка столкновений с канистрами
                for (auto it = conisters.begin(); it != conisters.end();) {
                    if (ship.getGlobalBounds().intersects(it->getGlobalBounds())) {
                        it = conisters.erase(it);
                        petrol += 22;
                        showPetrol(petrol, petrolText);
                        SoundPetrol.play();
                    }
                    else {
                        ++it;
                    }
                }



                // Движение пуль
                for (auto& a : bullets) {
                    a.move(speedBullet, 0);
                }

                // Удаление пуль
                bullets.erase(
                    std::remove_if(bullets.begin(), bullets.end(),
                        [](const Sprite& a) { return a.getPosition().x > WINDOW_WIDTH; }),
                    bullets.end()
                );

                // Столкновение пуль и астероидов
                for (auto it = bullets.begin(); it != bullets.end();) {
                    bool BulledErasted = false;

                    for (auto ait = asteroids.begin(); ait != asteroids.end();) {
                        if (it->getGlobalBounds().intersects(ait->getGlobalBounds())) {

                            ait = asteroids.erase(ait);
                            it = bullets.erase(it);
                            SoundConflict.play();
                            updateText(bulletText, "Bullet", numberBullet, 10.f, boundsPetrol.top + boundsPetrol.height + 10.f);
                            BulledErasted = true;
                        }
                        else {
                            ++ait;
                        }
                    }
                    if (!BulledErasted) {
                        ++it;
                    }
                }

                // Cпавн пуль
                if (timeSpawnBullet.getElapsedTime().asSeconds() > intTimeSpawnBullet) {
                    numberBullet += 1;
                    updateText(bulletText, "Bullet", numberBullet, 10.f, boundsPetrol.top + boundsPetrol.height + 10.f);
                    timeSpawnBullet.restart();
                }

				// Обновление позиции счёта
                FloatRect textBounds = scoreText.getLocalBounds();
                scoreText.setPosition(
                    WINDOW_WIDTH - textBounds.width - 10.f,
                    10.f
                );

				// Обновление счёта
                if (timeScore.getElapsedTime().asSeconds() > intTimeScore) {
                    score += 100;
                    showScore(score, scoreText);
                    timeScore.restart();
                }

				// Отображение топлива и уменьшение его
                petrolText.setPosition(10.f, 10.f);
                if (timeDecreasePetrol.getElapsedTime().asSeconds() > intTimeDecreasePetrol) {
                    if (petrol > 0) {
                        petrol -= 4;
                        showPetrol(petrol, petrolText);
                        timeDecreasePetrol.restart();
                    }
                    else {
                        gameOver = true;
                    }
                }

                
            }
        }

		// Меню отрисовка
        if (state == GameState::Menu) {
            window.draw(startText);
            window.draw(exitText);
            window.draw(leaderText);
        }

		// Таблица рекордов отрисовка
        else if (state == GameState::LeaderBoard) {
            for (int i = 0; i < 3; ++i) {
                Text t(std::to_string(i + 1) + ". " + std::to_string(bestScores[i]), MyFont, 28);
                t.setFillColor(Color::Yellow);
                t.setPosition(10.f, 10.f + i * 40.f);
                window.draw(t);
            }
            window.draw(exitFromLeaderBoard);
        }

		// Игровой процесс отрисовка
        else if (state == GameState::Game) {
            window.draw(background);
            window.draw(ship);
            window.draw(scoreText);
            window.draw(petrolText);
            window.draw(bulletText);
            for (auto& a : asteroids) window.draw(a);
            for (auto& c : conisters) window.draw(c);
            for (auto& j : bullets) window.draw(j);
            if (gameOver) {
                Text overText("GAME OVER", MyFont, 48);
                overText.setFillColor(Color::Red);
                FloatRect tb = overText.getLocalBounds();
                overText.setPosition(WINDOW_WIDTH / 2.f - tb.width / 2.f, WINDOW_HEIGHT / 2.f - tb.height / 2.f);
                window.draw(overText);
            }
        }

		// Paused
        else if (state == GameState::Paused) {
            window.draw(background);
            window.draw(ship);
            window.draw(scoreText);
            window.draw(petrolText);
            for (auto& a : asteroids) window.draw(a);
            for (auto& c : conisters) window.draw(c);

            window.draw(pauseTextPaused);
            window.draw(pauseTextExit);
        }

        window.display();

        // Обработка окончания игры
		// Чтобы MessageBox не появлялся несколько раз
        if (state == GameState::Game && gameOver && !showMessage) {
            showMessage = true;

            bestScores.push_back(score);
            std::sort(bestScores.begin(), bestScores.end(), std::greater<int>());
            if (bestScores.size() > 3) bestScores.resize(3);

            saveBestScores(bestScores);

            std::wstring msg = L"Вы проиграли!\nСчёт: " + std::to_wstring(score) + L"\nНачать заново?";
            int result = MessageBox(NULL, msg.c_str(), L"Game Over", MB_OKCANCEL | MB_ICONEXCLAMATION);
            if (result == IDOK) {
                resetGame(
                    ship,
                    asteroids,
                    conisters,
                    bullets,
                    spawnClock,
                    timeSpawnAsteroidClock,
                    timeScore,
                    score,
                    scoreText,
                    speed,
                    timeSpawnPetrol,
                    timeSpawnAsteroid,
                    petrol,
                    petrolText,
                    timeDecreasePetrol,
                    intTimeSpawnBullet,
                    intTimeDecreasePetrol,
                    intTimeScore,
                    numberBullet,
                    bulletText
                );


                gameOver = false;
                showMessage = false;
            }
            else {
                state = GameState::Menu;
            }
        }
    }
}