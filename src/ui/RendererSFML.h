#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "../algorithm.h"
#include "../maze/Maze.h"

#include <SFML/Graphics.hpp>

class RendererSFML {
public:
    RendererSFML(Maze& maze, int cellSize = 8);
    ~RendererSFML();

    void run();

private:
    void handleEvent(const sf::Event& ev);
    bool handleButtonClick(int mx, int my);

    void setupButtons();
    void setupAlgorithmButtons();
    void setupControlButtons();

    void toggleCellAt(int px, int py);
    void startSolver();
    void stopSolverIfRunning();
    void draw();

    struct Button {
        sf::RectangleShape shape;
        sf::Text text;
        std::string label;
        bool isAlgo = false;
        Algorithm* algo = nullptr;

        Button(const std::string& l,
               const sf::Font& font,
               sf::Vector2f pos,
               sf::Vector2f size,
               bool isA = false,
               Algorithm* a = nullptr)
            : text(font), label(l), isAlgo(isA), algo(a)
        {
            shape.setSize(size);
            shape.setPosition(pos);
            shape.setFillColor(sf::Color(230, 230, 230));
            shape.setOutlineColor(sf::Color::Black);
            shape.setOutlineThickness(2.f);

            text.setString(l);
            text.setCharacterSize(12);
            text.setFillColor(sf::Color::Black);
            text.setPosition({pos.x + 10.f, pos.y + 8.f});
        }
    };

    struct AlgoStats {
        std::string name;
        double timeMs = 0.0;
        std::size_t nodes = 0;
        std::size_t pathLength = 0;
        bool found = false;
        sf::Color color = sf::Color::Black;
    };

    Maze& m_maze;

    int m_cellSize;
    int m_winW;
    int m_winH;
    int m_statsWidth = 380;

    float m_menuHeight = 60.f;
    float m_buttonRowHeight = 40.f;
    float m_buttonMargin = 6.f;

    sf::RenderWindow m_window;
    sf::Font m_font;

    std::vector<Button> m_buttons;

    std::vector<std::unique_ptr<Algorithm>> m_algorithms;
    Algorithm* m_selectedAlgorithm = nullptr;

    std::thread m_solverThread;
    std::mutex m_mutex;

    bool m_solverRunning = false;
    bool m_hasResult = false;

    Result m_result{};
    sf::Color m_pathColor = sf::Color::Green;

    std::vector<Coord> m_visited;
    int m_stepDelayMs = 15;

    std::vector<AlgoStats> m_stats;
};