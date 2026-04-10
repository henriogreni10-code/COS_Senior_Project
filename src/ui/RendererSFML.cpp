#include "RendererSFML.h"

#include <chrono>
#include <cstdio>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>

#include "../algorithms/header/AlgorithmFactory.h"
#include "../maze/MazeGenerator.h"

RendererSFML::RendererSFML(Maze& maze, int cellSize)
    : m_maze(maze),
      m_cellSize(cellSize),
      m_algorithms(createAlgorithms())
{
    m_winW = maze.width() * cellSize + m_statsWidth;
    m_winH = maze.height() * cellSize + 60;

    m_window = sf::RenderWindow(
        sf::VideoMode({static_cast<unsigned>(m_winW), static_cast<unsigned>(m_winH)}),
        "Maze Solver"
    );

    m_window.setFramerateLimit(60);

    if (!m_font.openFromFile("../assets/arial.ttf")) {
        std::cerr << "Could not load font\n";
    }

    if (!m_algorithms.empty()) {
        m_selectedAlgorithm = m_algorithms.front().get();
    }

    setupButtons();
}

RendererSFML::~RendererSFML() {
    stopSolverIfRunning();
}

void RendererSFML::run() {
    while (m_window.isOpen()) {
        while (auto ev = m_window.pollEvent()) {
            handleEvent(*ev);
        }
        draw();
    }
}

void RendererSFML::handleEvent(const sf::Event& ev)
{
    if (ev.is<sf::Event::Closed>()) {
        m_window.close();
        return;
    }

    if (auto* e = ev.getIf<sf::Event::MouseButtonPressed>()) {
        if (e->button == sf::Mouse::Button::Left) {
            if (!handleButtonClick(e->position.x, e->position.y)) {
                if (e->position.y > 60 &&
                    e->position.x < (m_winW - m_statsWidth))
                {
                    toggleCellAt(e->position.x, e->position.y - 60);
                }
            }
        }
    }
}

void RendererSFML::setupButtons() {
    m_buttons.clear();
    setupAlgorithmButtons();
    setupControlButtons();
}

void RendererSFML::setupAlgorithmButtons() {
    const float margin = 8.f;
    const float btnH = 40.f;
    const float btnW = 110.f;

    float x = margin;

    for (auto& a : m_algorithms) {
        m_buttons.emplace_back(
            a->getName(),
            m_font,
            sf::Vector2f(x, 10.f),
            sf::Vector2f(btnW, btnH),
            true,
            a.get()
        );
        x += btnW + margin;
    }
}

void RendererSFML::setupControlButtons() {
    const std::vector<std::string> labels = {
        "Solve", "Clear", "Randomize", "Slow", "Normal", "Fast", "ClearStats", "Quit"
    };

    const float margin = 8.f;
    float x = margin + static_cast<float>(m_algorithms.size()) * (110.f + margin);

    for (const auto& l : labels) {
        m_buttons.emplace_back(
            l,
            m_font,
            sf::Vector2f(x, 10.f),
            sf::Vector2f(110.f, 40.f)
        );
        x += 118.f;
    }
}

bool RendererSFML::handleButtonClick(int mx, int my) {
    for (auto& b : m_buttons) {
        if (!b.shape.getGlobalBounds().contains({static_cast<float>(mx), static_cast<float>(my)})) {
            continue;
        }

        if (b.isAlgo) {
            m_selectedAlgorithm = b.algo;
            return true;
        }

        if (b.label == "Solve") {
            startSolver();
            return true;
        }

        if (b.label == "Clear") {
            stopSolverIfRunning();

            std::scoped_lock lock(m_mutex);
            m_visited.clear();
            m_hasResult = false;

            for (int y = 0; y < m_maze.height(); ++y) {
                for (int x = 0; x < m_maze.width(); ++x) {
                    m_maze.setWall(x, y, false);
                }
            }
            return true;
        }

        if (b.label == "Randomize") {
            stopSolverIfRunning();

            std::scoped_lock lock(m_mutex);
            m_visited.clear();
            m_hasResult = false;
            MazeGenerator::recursiveBacktracker(m_maze, 0);
            return true;
        }

        if (b.label == "Slow") {
            m_stepDelayMs = 120;
            return true;
        }

        if (b.label == "Normal") {
            m_stepDelayMs = 40;
            return true;
        }

        if (b.label == "Fast") {
            m_stepDelayMs = 0;
            return true;
        }

        if (b.label == "ClearStats") {
            std::scoped_lock lock(m_mutex);
            m_stats.clear();
            return true;
        }

        if (b.label == "Quit") {
            m_window.close();
            return true;
        }

        return true;
    }

    return false;
}

void RendererSFML::toggleCellAt(int px, int py)
{
    std::scoped_lock lock(m_mutex);

    if (m_solverRunning) {
        return;
    }

    const int gx = px / m_cellSize;
    const int gy = py / m_cellSize;

    if (!m_maze.inBounds(gx, gy)) {
        return;
    }

    m_hasResult = false;
    m_visited.clear();

    m_maze.setWall(gx, gy, !m_maze.isWall(gx, gy));
}

void RendererSFML::startSolver() {
    if (!m_selectedAlgorithm) {
        return;
    }

    stopSolverIfRunning();

    {
        std::scoped_lock lock(m_mutex);
        if (m_solverRunning) {
            return;
        }
        m_solverRunning = true;
        m_hasResult = false;
        m_visited.clear();
    }

    Maze copy = m_maze;
    Algorithm* algo = m_selectedAlgorithm;
    const std::string algoName = algo->getName();

    m_solverThread = std::thread([this, copy, algo, algoName]() mutable {
        sf::Color color = sf::Color::Green;

        if (algoName == "DFS") {
            color = sf::Color(70, 140, 255);
        } else if (algoName == "BFS") {
            color = sf::Color(80, 220, 80);
        } else if (algoName == "Dijkstra") {
            color = sf::Color(240, 150, 40);
        } else if (algoName == "A*") {
            color = sf::Color(180, 80, 220);
        }

        auto t0 = std::chrono::high_resolution_clock::now();
        Result timingResult = algo->solve(copy, nullptr);
        auto t1 = std::chrono::high_resolution_clock::now();
        double accurateTimeMs =
            std::chrono::duration<double, std::milli>(t1 - t0).count();

        Result visualResult = algo->solve(copy, [this](const Coord& c) {
            {
                std::scoped_lock lock(m_mutex);
                m_visited.push_back(c);
            }

            if (m_stepDelayMs > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(m_stepDelayMs));
            }
        });

        visualResult.timeMs = accurateTimeMs;

        {
            std::scoped_lock lock(m_mutex);
            m_result = visualResult;
            m_pathColor = color;
            m_hasResult = true;
            m_solverRunning = false;

            m_stats.push_back({
                algoName,
                visualResult.timeMs,
                static_cast<std::size_t>(visualResult.nodesExpanded),
                visualResult.path.size(),
                visualResult.found,
                color
            });
        }
    });
}

void RendererSFML::stopSolverIfRunning() {
    if (m_solverThread.joinable()) {
        m_solverThread.join();
    }

    std::scoped_lock lock(m_mutex);
    m_solverRunning = false;
}

void RendererSFML::draw()
{
    m_window.clear(sf::Color::White);

    // ---------------- BUTTONS ----------------
    for (auto& btn : m_buttons)
    {
        bool selected = btn.isAlgo && (btn.algo == m_selectedAlgorithm);

        btn.shape.setFillColor(
            selected ? sf::Color(160, 200, 255)
                     : sf::Color(230, 230, 230)
        );

        m_window.draw(btn.shape);
        m_window.draw(btn.text);
    }

    // ---------------- MAZE ----------------
    const int mazeW = m_maze.width();
    const int mazeH = m_maze.height();

    for (int y = 0; y < mazeH; ++y)
    {
        for (int x = 0; x < mazeW; ++x)
        {
            sf::RectangleShape tile(
                sf::Vector2f(static_cast<float>(m_cellSize), static_cast<float>(m_cellSize))
            );

            tile.setPosition(sf::Vector2f(
                static_cast<float>(x * m_cellSize),
                static_cast<float>(y * m_cellSize + 60)
            ));

            Coord c{x, y};

            if (c == m_maze.getStart()) {
                tile.setFillColor(sf::Color(50, 150, 255));
            }
            else if (c == m_maze.getGoal()) {
                tile.setFillColor(sf::Color(255, 80, 80));
            }
            else if (m_maze.isWall(x, y)) {
                tile.setFillColor(sf::Color::Black);
            }
            else {
                tile.setFillColor(sf::Color::White);
            }

            m_window.draw(tile);
        }
    }

    // ---------------- VISITED CELLS ----------------
    {
        std::scoped_lock lock(m_mutex);

        for (const auto& v : m_visited)
        {
            if (v == m_maze.getStart() || v == m_maze.getGoal()) {
                continue;
            }

            sf::RectangleShape rect(
                sf::Vector2f(static_cast<float>(m_cellSize), static_cast<float>(m_cellSize))
            );

            rect.setPosition(sf::Vector2f(
                static_cast<float>(v.first * m_cellSize),
                static_cast<float>(v.second * m_cellSize + 60)
            ));

            rect.setFillColor(sf::Color(150, 180, 255, 120));
            m_window.draw(rect);
        }
    }

    // ---------------- FINAL PATH ----------------
    {
        std::scoped_lock lock(m_mutex);

        if (m_hasResult && m_result.found)
        {
            for (const auto& p : m_result.path)
            {
                if (p == m_maze.getStart() || p == m_maze.getGoal()) {
                    continue;
                }

                sf::RectangleShape rect(
                    sf::Vector2f(static_cast<float>(m_cellSize), static_cast<float>(m_cellSize))
                );

                rect.setPosition(sf::Vector2f(
                    static_cast<float>(p.first * m_cellSize),
                    static_cast<float>(p.second * m_cellSize + 60)
                ));

                rect.setFillColor(m_pathColor);
                m_window.draw(rect);
            }
        }
    }

    // ---------------- RIGHT PANEL ----------------
    {
        std::scoped_lock lock(m_mutex);

        const float panelX = static_cast<float>(m_winW - m_statsWidth);

        sf::RectangleShape panel(
            sf::Vector2f(static_cast<float>(m_statsWidth), static_cast<float>(m_winH - 60))
        );
        panel.setPosition(sf::Vector2f(panelX, 60.f));
        panel.setFillColor(sf::Color(245, 245, 245));
        panel.setOutlineColor(sf::Color(180, 180, 180));
        panel.setOutlineThickness(2.f);
        m_window.draw(panel);

        float textX = panelX + 16.f;
        float textY = 76.f;

        sf::Text title(m_font);
        title.setString("Algorithm Data");
        title.setCharacterSize(20);
        title.setFillColor(sf::Color::Black);
        title.setPosition(sf::Vector2f(textX, textY));
        m_window.draw(title);

        sf::Text state(m_font);
        state.setCharacterSize(14);
        state.setFillColor(sf::Color(90, 90, 90));
        state.setString(m_solverRunning ? "Running..." : "Idle");
        state.setPosition(sf::Vector2f(textX, textY + 28.f));
        m_window.draw(state);

        sf::Text header(m_font);
        header.setCharacterSize(13);
        header.setFillColor(sf::Color::Black);
        header.setString("Algo     Time(ms)   Nodes   Path   Found");
        header.setPosition(sf::Vector2f(textX, textY + 56.f));
        m_window.draw(header);

        sf::Text row(m_font);
        row.setCharacterSize(13);

        float rowY = textY + 82.f;
        char timeBuffer[32];

        for (std::size_t i = 0; i < m_stats.size(); ++i)
        {
            const auto& s = m_stats[i];
            std::snprintf(timeBuffer, sizeof(timeBuffer), "%.2f", s.timeMs);

            row.setFillColor(s.color);
            row.setString(
                s.name + "     " +
                std::string(timeBuffer) + "     " +
                std::to_string(s.nodes) + "     " +
                std::to_string(s.pathLength) + "     " +
                (s.found ? "Y" : "N")
            );
            row.setPosition(sf::Vector2f(textX, rowY + static_cast<float>(i) * 22.f));
            m_window.draw(row);
        }
    }

    m_window.display();
}