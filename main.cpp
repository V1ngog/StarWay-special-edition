#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <algorithm>

using namespace sf;


enum class GameState {Game, Menu, LeaderBoard};
GameState state = GameState::Menu;

std::vector<int> loadBestScores() {
    std::vector<int> scores;
    std::ifstream file("save.txt");
    int value;
    while (file >> value) {
        scores.push_back(value);
    }

    // Сортируем по убыванию (чтобы первые элементы были наибольшими)
    std::sort(scores.begin(), scores.end(), std::greater<int>());

    // Убедимся, что длина вектора ровно 3 (если меньше — дополним нулями)
    while (scores.size() < 3) scores.push_back(0);
    if (scores.size() > 3) scores.resize(3);

    return scores;
}

// Сохраняет первые 3 значения из scores в файл (каждое значение — с новой строки)
void saveBestScores(const std::vector<int>& scores) {
    std::ofstream file("save.txt", std::ios::trunc);
    for (size_t i = 0; i < scores.size() && i < 3; ++i) {
        file << scores[i] << "\n";
    }
}

void static showPetrol(int& petrol, Text& petrolText) {
    petrolText.setString("Petrol: " + std::to_string(petrol));
    petrolText.setPosition(10.f, 10.f);
}

void static showScore(int& score, Text& scoreText) {
    scoreText.setString("Score: " + std::to_string(score));
    FloatRect textBounds = scoreText.getLocalBounds();
    scoreText.setPosition(800.f - textBounds.width - 10.f, 10.f);
}

void static resetGame(Sprite& ship,
    std::vector<Sprite>& asteroids,
    Clock& spawnClock,
    Clock& timeSpawnAsteroidClock,
    Clock& timeScore,
    const Texture& ShipTexture,
    int& score,
    Text& scoreText,
    float& speed,
    Clock& timeSpawnPetrol,
    float& timeSpawnAsteroid,
    int& petrol,
    Text& petrolText,
    Clock& timeDecreasePetrol) {

    ship.setPosition(400.f, 300.f);
    asteroids.clear();
    spawnClock.restart();
    timeSpawnAsteroidClock.restart();
    timeScore.restart();
    score = 0;
    speed = 5.f;
    timeSpawnPetrol.restart();
    timeSpawnAsteroid = 1.f;
    petrol = 100;
    timeDecreasePetrol.restart();

    showScore(score, scoreText);
    showPetrol(petrol, petrolText);

}


int main() {
    setlocale(LC_ALL, "RU");

    const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
    int score = 0, petrol = 100;
    std::vector<int> bestScores = loadBestScores();
    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Star Way");
    window.setFramerateLimit(60);

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

    Font MyFont;
    if (!MyFont.loadFromFile("font/myfont.ttf")) {
        MessageBox(NULL, L"Не удалось загрузить шрифт!", L"Ошибка", MB_OK | MB_ICONERROR);
        return -1;
    }

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

    SoundAsteroid.setBuffer(SoundAsteroidBuff);
    SoundPetrol.setBuffer(SoundPetrolBuff);

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


    Sprite background(BackgroundTexture);
    background.setScale(
        WINDOW_WIDTH / (float)(BackgroundTexture.getSize().x),
        WINDOW_HEIGHT / (float)(BackgroundTexture.getSize().y)
    );

    Sprite ship(ShipTexture);
    ship.setScale(0.2f, 0.2f);
    ship.setOrigin(ShipTexture.getSize().x / 2.f, ShipTexture.getSize().y / 2.f);
    ship.setPosition(400.f, 300.f);

    float speedShip = 5.f, speed = 5.f, scalePetrol = 0.1f;
    std::vector<Sprite> asteroids;
    std::vector<Sprite> conisters;
    Clock spawnClock, timeSpawnAsteroidClock, timeScore, timeSpawnPetrol, timeDecreasePetrol;
    float timeSpawnAsteroid = 1.f;
    bool gameOver = false, showMessage = false;

    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) {
                window.close();
            }

            if (state == GameState::Menu && e.type == Event::MouseButtonPressed) {
                Vector2i mousePos = Mouse::getPosition(window);

                if (startText.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                    state = GameState::Game;
                    resetGame(ship, asteroids, spawnClock, timeSpawnAsteroidClock,
                        timeScore, ShipTexture, score, scoreText, speed,
                        timeSpawnPetrol, timeSpawnAsteroid, petrol, petrolText, timeDecreasePetrol);
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

            if (state == GameState::LeaderBoard && e.type == Event::MouseButtonPressed) {
                Vector2i mousePos = Mouse::getPosition(window);
                if (exitFromLeaderBoard.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                    state = GameState::Menu;
                }
            }
        }

        window.clear(Color::Black);

        if (state == GameState::Menu) {
            window.draw(startText);
            window.draw(exitText);
            window.draw(leaderText);
        }

        else if (state == GameState::LeaderBoard) {
            for (int i = 0; i < 3; ++i) {
                Text t(std::to_string(i + 1) + ". " + std::to_string(bestScores[i]), MyFont, 28);
                t.setFillColor(Color::Yellow);
                t.setPosition(300.f, 180.f + i * 40.f);
                window.draw(t);
            }
            window.draw(exitFromLeaderBoard);
        }

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

                for (auto& a : conisters) {
                    a.move(-speed, 0);
                }

                conisters.erase(
                    std::remove_if(conisters.begin(), conisters.end(),
                        [](const Sprite& a) { return a.getPosition().x < -200.f; }),
                    conisters.end()
                );

                for (auto it = conisters.begin(); it != conisters.end();) {
                    if (ship.getGlobalBounds().intersects(it->getGlobalBounds())) {
                        it = conisters.erase(it);
                        petrol += 25;
                        showPetrol(petrol, petrolText);
                        SoundPetrol.play();
                    }
                    else {
                        ++it;
                    }
                }


                // Получаем размер текста
                FloatRect textBounds = scoreText.getLocalBounds();

                // Ставим позицию в правый верхний угол
                scoreText.setPosition(
                    WINDOW_WIDTH - textBounds.width - 10.f,
                    10.f
                );

                if (timeScore.getElapsedTime().asSeconds() > 1.f) {
                    score += 100;
                    showScore(score, scoreText);
                    timeScore.restart();
                }

                petrolText.setPosition(10.f, 10.f);
                if (timeDecreasePetrol.getElapsedTime().asSeconds() > 1.f) {
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

        if (state == GameState::Menu) {
            window.draw(startText);
            window.draw(exitText);
            window.draw(leaderText);
        }

        else if (state == GameState::LeaderBoard) {
            std::vector<sf::Text> leaderboardTexts;
            for (int i = 0; i < 3; ++i) {
                sf::Text t(" " + std::to_string(i + 1) + ". " + std::to_string(bestScores[i]), MyFont, 28);
                t.setFillColor(sf::Color::Yellow);
                t.setPosition(300.f, 180.f + i * 40.f);
                leaderboardTexts.push_back(t);
            }
            window.draw(exitFromLeaderBoard);
        }

        else if (state == GameState::Game) {
            window.draw(background);
            window.draw(ship);
            window.draw(scoreText);
            window.draw(petrolText);
            for (auto& a : asteroids) window.draw(a);
            for (auto& c : conisters) window.draw(c);
            if (gameOver) {
                Text overText("GAME OVER", MyFont, 48);
                overText.setFillColor(Color::Red);
                FloatRect tb = overText.getLocalBounds();
                overText.setPosition(WINDOW_WIDTH / 2.f - tb.width / 2.f, WINDOW_HEIGHT / 2.f - tb.height / 2.f);
                window.draw(overText);
            }
        }

        window.display();

        if (state == GameState::Game && gameOver && !showMessage) {
            showMessage = true;

            bestScores.push_back(score);
            std::sort(bestScores.begin(), bestScores.end(), std::greater<int>());
            if (bestScores.size() > 3) bestScores.resize(3);

            saveBestScores(bestScores);

            std::wstring msg = L"Вы столкнулись!\nСчёт: " + std::to_wstring(score) + L"\nНачать заново?";
            int result = MessageBox(NULL, msg.c_str(), L"Game Over", MB_OKCANCEL | MB_ICONEXCLAMATION);
            if (result == IDOK) {
                resetGame(ship,
                    asteroids,
                    spawnClock,
                    timeSpawnAsteroidClock,
                    timeScore,
                    ShipTexture,
                    score,
                    scoreText,
                    speed,
                    timeSpawnPetrol,
                    timeSpawnAsteroid,
                    petrol,
                    petrolText,
                    timeDecreasePetrol);
                gameOver = false;
                showMessage = false; // можно снова показывать в будущем
            }
            else {
                state = GameState::Menu;
            }
        }
    }
}