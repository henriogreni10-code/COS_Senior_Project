#include "RendererSFML.h"

#include <algorithm>
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
    if (!m_font.openFromFile("../assets/arial.ttf")) {
        std::cerr << "Could not load font\n";
    }

    if (!m_algorithms.empty()) {
        m_selectedAlgorithm = m_algorithms.front().get();
    }

    // Temporary values used only so setupButtons() can wrap labels.
    m_winW = maze.width() * cellSize + m_statsWidth;
    m_winH = maze.height() * cellSize + static_cast<int>(m_menuHeight);

    setupButtons();

    const auto desktop = sf::VideoMode::getDesktopMode();
    const int maxWindowW = static_cast<int>(desktop.size.x) - 40;
    const int maxWindowH = static_cast<int>(desktop.size.y) - 120;

    const int maxMazeW = maxWindowW - m_statsWidth;
    const int maxMazeH = maxWindowH - static_cast<int>(m_menuHeight);

    const int fitByWidth = std::max(1, maxMazeW / m_maze.width());
    const int fitByHeight = std::max(1, maxMazeH / m_maze.height());

    m_cellSize = std::max(1, std::min(m_cellSize, std::min(fitByWidth, fitByHeight)));

    m_winW = m_maze.width() * m_cellSize + m_statsWidth;
    m_winH = m_maze.height() * m_cellSize + static_cast<int>(m_menuHeight);

    m_window = sf::RenderWindow(
        sf::VideoMode({static_cast<unsigned>(m_winW), static_cast<unsigned>(m_winH)}),
        "Maze Solver"
    );

    m_window.setFramerateLimit(60);
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
                if (e->position.y > static_cast<int>(m_menuHeight) &&
                    e->position.x < (m_winW - m_statsWidth))
                {
                    toggleCellAt(
                        e->position.x,
                        e->position.y - static_cast<int>(m_menuHeight)
                    );
                }
            }
        }
    }
}

void RendererSFML::setupButtons() {
    m_buttons.clear();

    const float margin = m_buttonMargin;
    const float rowHeight = m_buttonRowHeight;
    const float buttonHeight = 34.f;

    const float usableWidth = static_cast<float>(m_winW - m_statsWidth) - margin;
    float x = margin;
    float y = 10.f;

    auto placeButton = [&](const std::string& label, bool isAlgo, Algorithm* algoPtr) {
        float buttonWidth = 70.f + static_cast<float>(label.size()) * 8.f;

        if (buttonWidth < 90.f) {
            buttonWidth = 90.f;
        }

        if (x + buttonWidth > usableWidth) {
            x = margin;
            y += rowHeight;
        }

        m_buttons.emplace_back(
            label,
            m_font,
            sf::Vector2f(x, y),
            sf::Vector2f(buttonWidth, buttonHeight),
            isAlgo,
            algoPtr
        );

        x += buttonWidth + margin;
    };

    for (auto& algorithm : m_algorithms) {
        placeButton(algorithm->getName(), true, algorithm.get());
    }

    const std::vector<std::string> controls = {
        "Solve", "Clear", "Randomize", "Slow", "Normal", "Fast", "ClearStats", "Quit"
    };

    for (const auto& label : controls) {
        placeButton(label, false, nullptr);
    }

    m_menuHeight = y + rowHeight;
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
        "Solve","Clear","Randomize",
        "Slow","Normal","Fast",
        "ClearStats","Quit"
    };

    const float margin = 6.f;
    const float btnH = 32.f;
    const float y = 50.f;

    const float availableWidth = static_cast<float>(m_winW);
    const std::size_t count = labels.size();

    float btnW = (availableWidth - margin * (count + 1)) / count;

    if (btnW < 65.f) btnW = 65.f;

    float x = margin;

    for (const auto& l : labels) {
        m_buttons.emplace_back(
            l,
            m_font,
            sf::Vector2f(x, y),
            sf::Vector2f(btnW, btnH)
        );
        x += btnW + margin;
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
        } else if (algoName == "Bellman-Ford") {
            color = sf::Color(220, 60, 60);
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
                static_cast<float>(y * m_cellSize) + m_menuHeight
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
                static_cast<float>(v.second * m_cellSize) + m_menuHeight
            ));

            rect.setFillColor(sf::Color(150, 180, 255, 120));
            m_window.draw(rect);
        }
    }

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
                    static_cast<float>(p.second * m_cellSize) + m_menuHeight
                ));

                rect.setFillColor(m_pathColor);
                m_window.draw(rect);
            }
        }
    }

    {
        std::scoped_lock lock(m_mutex);

        const float panelX = static_cast<float>(m_winW - m_statsWidth);

        sf::RectangleShape panel(
            sf::Vector2f(
                static_cast<float>(m_statsWidth),
                static_cast<float>(m_winH) - m_menuHeight
            )
        );
        panel.setPosition(sf::Vector2f(panelX, m_menuHeight));
        panel.setFillColor(sf::Color(245, 245, 245));
        panel.setOutlineColor(sf::Color(180, 180, 180));
        panel.setOutlineThickness(2.f);
        m_window.draw(panel);

        float baseX = panelX + 12.f;
        float y = m_menuHeight + 16.f;

        sf::Text title(m_font);
        title.setString("Algorithm Data");
        title.setCharacterSize(20);
        title.setFillColor(sf::Color::Black);
        title.setPosition({baseX, y});
        m_window.draw(title);

        sf::Text state(m_font);
        state.setCharacterSize(14);
        state.setFillColor(sf::Color(90, 90, 90));
        state.setString(m_solverRunning ? "Running..." : "Idle");
        state.setPosition({baseX, y + 28.f});
        m_window.draw(state);

        float colAlgo  = baseX;
        float colTime  = baseX + 150.f;
        float colNodes = baseX + 250.f;
        float colPath  = baseX + 330.f;

        sf::Text header(m_font);
        header.setCharacterSize(13);
        header.setFillColor(sf::Color::Black);

        header.setString("Algo");
        header.setPosition({colAlgo, y + 60.f});
        m_window.draw(header);

        header.setString("Time");
        header.setPosition({colTime, y + 60.f});
        m_window.draw(header);

        header.setString("Nodes");
        header.setPosition({colNodes, y + 60.f});
        m_window.draw(header);

        header.setString("Path");
        header.setPosition({colPath, y + 60.f});
        m_window.draw(header);

        sf::Text row(m_font);
        row.setCharacterSize(13);

        float rowY = y + 84.f;
        char buffer[32];

        for (size_t i = 0; i < m_stats.size(); ++i)
        {
            const auto& s = m_stats[i];
            float ry = rowY + i * 22.f;

            row.setFillColor(s.color);

            row.setString(s.name);
            row.setPosition({colAlgo, ry});
            m_window.draw(row);

            std::snprintf(buffer, sizeof(buffer), "%.2f", s.timeMs);
            row.setString(buffer);
            row.setPosition({colTime, ry});
            m_window.draw(row);

            row.setString(std::to_string(s.nodes));
            row.setPosition({colNodes, ry});
            m_window.draw(row);

            row.setString(std::to_string(s.pathLength));
            row.setPosition({colPath, ry});
            m_window.draw(row);
        }
    }

    m_window.display();
}