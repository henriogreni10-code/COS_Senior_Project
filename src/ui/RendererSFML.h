#pragma once

#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "../algorithm.h"
#include "../maze/Maze.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

class RendererSFML {
public:
    RendererSFML(Maze& maze, int cellSize = 10);
    ~RendererSFML();

    void run();

private:
    void handleEvent(const sf::Event& ev);
    bool handleButtonClick(int mx, int my);
    void setupButtons();

    void toggleCellAt(int px, int py);
    void startSolver(int algo);
    void stopSolverIfRunning();
    void draw();

    // Button widget
    struct Button {
        sf::RectangleShape shape;
        sf::Text text;
        std::string label;
        int id = 0;

        Button() = delete;

        Button(const std::string& labelText,
               const sf::Font& font,
               int id_,
               sf::Vector2f pos,
               sf::Vector2f size)
            : text(font), label(labelText), id(id_)
        {
            shape.setSize(size);
            shape.setPosition(pos);
            shape.setFillColor(sf::Color(230, 230, 230));
            shape.setOutlineColor(sf::Color::Black);
            shape.setOutlineThickness(2.f);

            text.setString(labelText);
            text.setCharacterSize(14);
            text.setFillColor(sf::Color::Black);
            text.setPosition(sf::Vector2f(pos.x + 10.f, pos.y + 8.f));
        }
    };

    struct AlgoStats {
        std::string name;
        double timeMs;
        size_t nodes;
        size_t pathLength;
        sf::Color color;
    };

    // Members
    Maze& m_maze;
    int m_cellSize;
    int m_winW;
    int m_winH;

    int m_statsWidth = 260;   // RIGHT PANEL WIDTH

    sf::RenderWindow m_window;
    sf::Font m_font;
    std::vector<Button> m_buttons;

    std::thread m_solverThread;
    std::mutex m_mutex;

    bool m_solverRunning = false;
    bool m_hasResult = false;
    int  m_selectedAlgo = 1;

    Result m_result{};
    sf::Color m_pathColor = sf::Color::Green;

    std::vector<Coord> m_visited;

    int m_stepDelayMs = 15;

    std::vector<AlgoStats> m_stats;
};
