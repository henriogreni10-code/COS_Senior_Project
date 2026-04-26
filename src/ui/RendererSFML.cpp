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

// Constructor initializes the renderer with a maze reference and preferred cell size.
RendererSFML::RendererSFML(Maze& maze, int cellSize)
    : m_maze(maze),
      m_cellSize(cellSize),
      m_algorithms(createAlgorithms()) // Creates all available algorithm objects.
{
    // Load font used for buttons and statistics text.
    if (!m_font.openFromFile("../assets/arial.ttf")) {
        std::cerr << "Could not load font\n";
    }

    // Select the first algorithm by default.
    if (!m_algorithms.empty()) {
        m_selectedAlgorithm = m_algorithms.front().get();
    }

    // Temporary window size used before final scaling.
    // This allows setupButtons() to calculate button wrapping correctly.
    m_winW = maze.width() * cellSize + m_statsWidth;
    m_winH = maze.height() * cellSize + static_cast<int>(m_menuHeight);

    setupButtons();

    // Get desktop size so the app can fit inside the screen.
    const auto desktop = sf::VideoMode::getDesktopMode();
    const int maxWindowW = static_cast<int>(desktop.size.x) - 40;
    const int maxWindowH = static_cast<int>(desktop.size.y) - 120;

    // Reserve space for the statistics panel and top menu.
    const int maxMazeW = maxWindowW - m_statsWidth;
    const int maxMazeH = maxWindowH - static_cast<int>(m_menuHeight);

    // Calculate the largest cell size that fits horizontally and vertically.
    const int fitByWidth = std::max(1, maxMazeW / m_maze.width());
    const int fitByHeight = std::max(1, maxMazeH / m_maze.height());

    // Reduce cell size if needed so the whole maze fits on screen.
    m_cellSize = std::max(1, std::min(m_cellSize, std::min(fitByWidth, fitByHeight)));

    // Final window dimensions after scaling.
    m_winW = m_maze.width() * m_cellSize + m_statsWidth;
    m_winH = m_maze.height() * m_cellSize + static_cast<int>(m_menuHeight);

    // Create the SFML window.
    m_window = sf::RenderWindow(
        sf::VideoMode({static_cast<unsigned>(m_winW), static_cast<unsigned>(m_winH)}),
        "Maze Solver"
    );

    // Limit rendering to 60 frames per second.
    m_window.setFramerateLimit(60);
}

// Destructor ensures solver thread is finished before renderer is destroyed.
RendererSFML::~RendererSFML() {
    stopSolverIfRunning();
}

// Main application loop.
// Processes events first, then redraws the interface.
void RendererSFML::run() {
    while (m_window.isOpen()) {
        while (auto ev = m_window.pollEvent()) {
            handleEvent(*ev);
        }
        draw();
    }
}

// Handles window close events and mouse clicks.
void RendererSFML::handleEvent(const sf::Event& ev)
{
    // Close window when the user presses the window close button.
    if (ev.is<sf::Event::Closed>()) {
        m_window.close();
        return;
    }

    // Handle left mouse button presses.
    if (auto* e = ev.getIf<sf::Event::MouseButtonPressed>()) {
        if (e->button == sf::Mouse::Button::Left) {

            // First check if the click was on a button.
            if (!handleButtonClick(e->position.x, e->position.y)) {

                // If not a button, check if the click was inside the maze area.
                if (e->position.y > static_cast<int>(m_menuHeight) &&
                    e->position.x < (m_winW - m_statsWidth))
                {
                    // Toggle wall/passable cell at clicked maze position.
                    toggleCellAt(
                        e->position.x,
                        e->position.y - static_cast<int>(m_menuHeight)
                    );
                }
            }
        }
    }
}

// Creates both algorithm buttons and control buttons.
// Buttons wrap to the next row when there is not enough horizontal space.
void RendererSFML::setupButtons() {
    m_buttons.clear();

    const float margin = m_buttonMargin;
    const float rowHeight = m_buttonRowHeight;
    const float buttonHeight = 34.f;

    // Buttons only occupy the maze area, not the statistics panel.
    const float usableWidth = static_cast<float>(m_winW - m_statsWidth) - margin;
    float x = margin;
    float y = 10.f;

    // Helper function for placing a single button.
    auto placeButton = [&](const std::string& label, bool isAlgo, Algorithm* algoPtr) {
        // Estimate width from label size.
        float buttonWidth = 70.f + static_cast<float>(label.size()) * 8.f;

        // Minimum button width for readability.
        if (buttonWidth < 90.f) {
            buttonWidth = 90.f;
        }

        // Move to a new row if the button does not fit.
        if (x + buttonWidth > usableWidth) {
            x = margin;
            y += rowHeight;
        }

        // Add button to the button list.
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

    // Create one button for each algorithm.
    for (auto& algorithm : m_algorithms) {
        placeButton(algorithm->getName(), true, algorithm.get());
    }

    // Create control buttons.
    const std::vector<std::string> controls = {
        "Solve", "Clear", "Randomize", "Slow", "Normal", "Fast", "ClearStats", "Quit"
    };

    for (const auto& label : controls) {
        placeButton(label, false, nullptr);
    }

    // Final menu height depends on how many rows were needed.
    m_menuHeight = y + rowHeight;
}

// Older/separate helper for creating only algorithm buttons.
// Kept for structure or possible reuse.
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

// Older/separate helper for creating only control buttons.
// Kept for structure or possible reuse.
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

    // Spread buttons evenly across the available width.
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

// Checks whether a mouse click hits a button and performs the matching action.
bool RendererSFML::handleButtonClick(int mx, int my) {
    for (auto& b : m_buttons) {

        // Ignore buttons that do not contain the click point.
        if (!b.shape.getGlobalBounds().contains({static_cast<float>(mx), static_cast<float>(my)})) {
            continue;
        }

        // Algorithm button: select this algorithm.
        if (b.isAlgo) {
            m_selectedAlgorithm = b.algo;
            return true;
        }

        // Run selected algorithm.
        if (b.label == "Solve") {
            startSolver();
            return true;
        }

        // Clear maze, visited nodes, and current result.
        if (b.label == "Clear") {
            stopSolverIfRunning();

            std::scoped_lock lock(m_mutex);
            m_visited.clear();
            m_hasResult = false;

            // Remove all walls.
            for (int y = 0; y < m_maze.height(); ++y) {
                for (int x = 0; x < m_maze.width(); ++x) {
                    m_maze.setWall(x, y, false);
                }
            }
            return true;
        }

        // Generate a new random maze.
        if (b.label == "Randomize") {
            stopSolverIfRunning();

            std::scoped_lock lock(m_mutex);
            m_visited.clear();
            m_hasResult = false;

            // Recursive backtracker creates a maze layout.
            MazeGenerator::recursiveBacktracker(m_maze, 0);
            return true;
        }

        // Slow visualization speed.
        if (b.label == "Slow") {
            m_stepDelayMs = 120;
            return true;
        }

        // Normal visualization speed.
        if (b.label == "Normal") {
            m_stepDelayMs = 40;
            return true;
        }

        // Fast visualization speed.
        if (b.label == "Fast") {
            m_stepDelayMs = 0;
            return true;
        }

        // Clear stored statistics without changing the maze.
        if (b.label == "ClearStats") {
            std::scoped_lock lock(m_mutex);
            m_stats.clear();
            return true;
        }

        // Exit application.
        if (b.label == "Quit") {
            m_window.close();
            return true;
        }

        return true;
    }

    // Click was not on any button.
    return false;
}

// Toggles a maze cell between wall and open space.
void RendererSFML::toggleCellAt(int px, int py)
{
    std::scoped_lock lock(m_mutex);

    // Prevent editing while an algorithm is running.
    if (m_solverRunning) {
        return;
    }

    // Convert pixel coordinates to grid coordinates.
    const int gx = px / m_cellSize;
    const int gy = py / m_cellSize;

    // Ignore clicks outside maze bounds.
    if (!m_maze.inBounds(gx, gy)) {
        return;
    }

    // Clear old visualization because the maze changed.
    m_hasResult = false;
    m_visited.clear();

    // Flip wall state.
    m_maze.setWall(gx, gy, !m_maze.isWall(gx, gy));
}

// Starts the selected algorithm in a background thread.
void RendererSFML::startSolver() {
    if (!m_selectedAlgorithm) {
        return;
    }

    // Ensure no previous solver thread is active.
    stopSolverIfRunning();

    {
        std::scoped_lock lock(m_mutex);

        if (m_solverRunning) {
            return;
        }

        // Reset visualization state.
        m_solverRunning = true;
        m_hasResult = false;
        m_visited.clear();
    }

    // Work on a copy so the algorithm is not affected by external maze changes.
    Maze copy = m_maze;

    Algorithm* algo = m_selectedAlgorithm;
    const std::string algoName = algo->getName();

    // Run the solver on a separate thread so the UI does not freeze.
    m_solverThread = std::thread([this, copy, algo, algoName]() mutable {
        sf::Color color = sf::Color::Green;

        // Assign a unique color to each algorithm path.
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

        // First run: measure algorithm time without visualization delay.
        auto t0 = std::chrono::high_resolution_clock::now();
        Result timingResult = algo->solve(copy, nullptr);
        auto t1 = std::chrono::high_resolution_clock::now();

        double accurateTimeMs =
            std::chrono::duration<double, std::milli>(t1 - t0).count();

        // Second run: collect visited nodes for visual animation.
        Result visualResult = algo->solve(copy, [this](const Coord& c) {
            {
                std::scoped_lock lock(m_mutex);
                m_visited.push_back(c);
            }

            // Delay controls animation speed.
            if (m_stepDelayMs > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(m_stepDelayMs));
            }
        });

        // Store real computation time, not visualization time.
        visualResult.timeMs = accurateTimeMs;

        {
            std::scoped_lock lock(m_mutex);

            // Save result for drawing.
            m_result = visualResult;
            m_pathColor = color;
            m_hasResult = true;
            m_solverRunning = false;

            // Add row to statistics panel.
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

// Waits for the solver thread to finish if it exists.
void RendererSFML::stopSolverIfRunning() {
    if (m_solverThread.joinable()) {
        m_solverThread.join();
    }

    std::scoped_lock lock(m_mutex);
    m_solverRunning = false;
}

// Draws the entire application window.
void RendererSFML::draw()
{
    m_window.clear(sf::Color::White);

    // Draw all buttons.
    for (auto& btn : m_buttons)
    {
        bool selected = btn.isAlgo && (btn.algo == m_selectedAlgorithm);

        // Highlight selected algorithm button.
        btn.shape.setFillColor(
            selected ? sf::Color(160, 200, 255)
                     : sf::Color(230, 230, 230)
        );

        m_window.draw(btn.shape);
        m_window.draw(btn.text);
    }

    const int mazeW = m_maze.width();
    const int mazeH = m_maze.height();

    // Draw base maze grid.
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

            // Draw start, goal, wall, or empty space.
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

    // Draw visited cells from the current visualization.
    {
        std::scoped_lock lock(m_mutex);

        for (const auto& v : m_visited)
        {
            // Keep start and goal visually distinct.
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

            // Semi-transparent blue overlay for explored cells.
            rect.setFillColor(sf::Color(150, 180, 255, 120));
            m_window.draw(rect);
        }
    }

    // Draw final path if an algorithm found one.
    {
        std::scoped_lock lock(m_mutex);

        if (m_hasResult && m_result.found)
        {
            for (const auto& p : m_result.path)
            {
                // Keep start and goal colors unchanged.
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

    // Draw statistics panel.
    {
        std::scoped_lock lock(m_mutex);

        const float panelX = static_cast<float>(m_winW - m_statsWidth);

        // Background panel on the right side.
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

        // Panel title.
        sf::Text title(m_font);
        title.setString("Algorithm Data");
        title.setCharacterSize(20);
        title.setFillColor(sf::Color::Black);
        title.setPosition({baseX, y});
        m_window.draw(title);

        // Shows whether an algorithm is currently running.
        sf::Text state(m_font);
        state.setCharacterSize(14);
        state.setFillColor(sf::Color(90, 90, 90));
        state.setString(m_solverRunning ? "Running..." : "Idle");
        state.setPosition({baseX, y + 28.f});
        m_window.draw(state);

        // Column positions.
        float colAlgo  = baseX;
        float colTime  = baseX + 150.f;
        float colNodes = baseX + 250.f;
        float colPath  = baseX + 330.f;

        // Header row.
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

        // Draw one row for each completed algorithm run.
        sf::Text row(m_font);
        row.setCharacterSize(13);

        float rowY = y + 84.f;
        char buffer[32];

        for (size_t i = 0; i < m_stats.size(); ++i)
        {
            const auto& s = m_stats[i];
            float ry = rowY + i * 22.f;

            // Row text uses the algorithm color.
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

    // Present the rendered frame.
    m_window.display();
}