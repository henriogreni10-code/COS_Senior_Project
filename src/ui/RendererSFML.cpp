#include "RendererSFML.h"

#include <iostream>
#include <chrono>
#include <thread>

#include "../algorithms/header/dfs.h"
#include "../algorithms/header/bfs.h"
#include "../algorithms/header/dijkstra.h"
#include "../algorithms/header/AStar.h"
#include "../maze/MazeGenerator.h"

// Factories
extern Algorithm* createDFS();
extern Algorithm* createBFS();
extern Algorithm* createDijkstra();
extern Algorithm* createAStar();

RendererSFML::RendererSFML(Maze& maze, int cellSize)
    : m_maze(maze),
      m_cellSize(cellSize)
{
    m_winW = maze.width() * cellSize + m_statsWidth;
    m_winH = maze.height() * cellSize + 60;

    m_window = sf::RenderWindow(
        sf::VideoMode({(unsigned)m_winW, (unsigned)m_winH}),
        "Maze Solver GUI"
    );

    m_window.setFramerateLimit(60);

    if (!m_font.openFromFile("../assets/arial.ttf")) {
        std::cerr << "⚠️ Could not load font\n";
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

void RendererSFML::setupButtons()
{
    std::vector<std::string> labels = {
        "DFS","BFS","Dijkstra","A*",
        "Randomize","Clear","Solve",
        "Slow","Normal","Fast","ClearStats","Quit"
    };

    float margin = 8.f;
    float btnH = 40.f;
    float totalW = (float)m_winW;
    float btnW = (totalW - margin * (labels.size() + 1)) / labels.size();

    m_buttons.clear();

    for (size_t i = 0; i < labels.size(); ++i) {
        float x = margin + i * (btnW + margin);
        m_buttons.emplace_back(
            labels[i],
            m_font,
            (int)i+1,
            sf::Vector2f(x, 10.f),
            sf::Vector2f(btnW, btnH)
        );
    }
}

bool RendererSFML::handleButtonClick(int mx, int my)
{
    for (auto& btn : m_buttons)
    {
        if (btn.shape.getGlobalBounds().contains(
            sf::Vector2f((float)mx, (float)my)))
        {
            std::string L = btn.label;

            if (L == "DFS") m_selectedAlgo = 1;
            else if (L == "BFS") m_selectedAlgo = 2;
            else if (L == "Dijkstra") m_selectedAlgo = 3;
            else if (L == "A*") m_selectedAlgo = 4;

            else if (L == "Randomize") {
                stopSolverIfRunning();
                std::scoped_lock lock(m_mutex);
                m_stats.clear();
                m_visited.clear();
                m_hasResult = false;
                MazeGenerator::recursiveBacktracker(m_maze, 0);
                return true;
            }

            else if (L == "Clear") {
                stopSolverIfRunning();
                std::scoped_lock lock(m_mutex);
                m_visited.clear();
                m_stats.clear();
                m_hasResult = false;

                for (int y = 0; y < m_maze.height(); y++)
                    for (int x = 0; x < m_maze.width(); x++)
                        m_maze.setWall(x,y,false);
                return true;
            }

            else if (L == "Solve") {
                startSolver(m_selectedAlgo);
                return true;
            }

            else if (L == "Slow")   m_stepDelayMs = 120;
            else if (L == "Normal") m_stepDelayMs = 40;
            else if (L == "Fast")   m_stepDelayMs = 0;

            else if (L == "ClearStats") {
                std::scoped_lock lock(m_mutex);
                m_stats.clear();
                return true;
            }

            else if (L == "Quit") {
                m_window.close();
                return true;
            }
        }
    }
    return false;
}

void RendererSFML::toggleCellAt(int px, int py)
{
    std::scoped_lock lock(m_mutex);
    if (m_solverRunning) return;

    int gx = px / m_cellSize;
    int gy = py / m_cellSize;

    if (!m_maze.inBounds(gx,gy)) return;

    m_hasResult = false;
    m_stats.clear();
    m_visited.clear();

    m_maze.setWall(gx, gy, !m_maze.isWall(gx,gy));
}

void RendererSFML::startSolver(int algo) {
    if (algo == 0) return;

    {
        std::scoped_lock lock(m_mutex);
        if (m_solverRunning) return;
        m_hasResult = false;
        m_visited.clear();
    }

    stopSolverIfRunning();

    {
        std::scoped_lock lock(m_mutex);
        m_solverRunning = true;
    }

    Maze mazeCopy = m_maze;

    m_solverThread = std::thread([this, mazeCopy, algo]() mutable {

        Algorithm* alg = nullptr;
        sf::Color color;
        std::string name;

        switch (algo)
        {
            case 1: alg=createDFS();      color=sf::Color(70,140,255);  name="DFS"; break;
            case 2: alg=createBFS();      color=sf::Color(80,220,80);   name="BFS"; break;
            case 3: alg=createDijkstra(); color=sf::Color(240,150,40);  name="Dijkstra"; break;
            case 4: alg=createAStar();    color=sf::Color(180,80,220);  name="A*"; break;
            default: break;
        }

        if (!alg) {
            std::scoped_lock lock(m_mutex);
            m_solverRunning = false;
            return;
        }

        // ==== TIMING RUN: No visualization, accurate timing ====
        auto t0 = std::chrono::high_resolution_clock::now();
        Result timingResult = alg->solve(mazeCopy, nullptr);  // nullptr = no callback!
        auto t1 = std::chrono::high_resolution_clock::now();

        double accurateTimeMs = std::chrono::duration<double,std::milli>(t1-t0).count();

        // ==== VISUALIZATION RUN: With callback for display ====
        Algorithm::StepCallback callback =
            [this](const Coord& c)
            {
                {
                    std::scoped_lock lock(m_mutex);
                    m_visited.push_back(c);
                }
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(m_stepDelayMs));
            };

        Result res = alg->solve(mazeCopy, callback);

        // Use accurate timing from first run
        res.timeMs = accurateTimeMs;

        delete alg;

        {
            std::scoped_lock lock(m_mutex);
            m_result = res;
            m_pathColor = color;
            m_hasResult = true;
            m_solverRunning = false;

            m_stats.push_back({name, res.timeMs, res.nodesExpanded, res.path.size(), color});
        }
    });
}

void RendererSFML::stopSolverIfRunning()
{
    if (m_solverThread.joinable())
        m_solverThread.join();

    std::scoped_lock lock(m_mutex);
    m_solverRunning = false;
}

void RendererSFML::draw()
{
    m_window.clear(sf::Color::White);

    // ---------------- BUTTONS ----------------
    for (auto& btn : m_buttons)
    {
        bool sel = (
            (btn.label=="DFS" && m_selectedAlgo==1) ||
            (btn.label=="BFS" && m_selectedAlgo==2) ||
            (btn.label=="Dijkstra" && m_selectedAlgo==3) ||
            (btn.label=="A*" && m_selectedAlgo==4)
        );

        btn.shape.setFillColor(sel ? sf::Color(160,200,255)
                                   : sf::Color(230,230,230));

        m_window.draw(btn.shape);
        m_window.draw(btn.text);
    }

    // ---------------- MAZE ----------------
    int mazeW = m_maze.width();
    int mazeH = m_maze.height();

    for (int y=0; y<mazeH; ++y)
    {
        for (int x=0; x<mazeW; ++x)
        {
            sf::RectangleShape tile(
                sf::Vector2f((float)m_cellSize,(float)m_cellSize));

            tile.setPosition(sf::Vector2f(
                (float)(x*m_cellSize),
                (float)(y*m_cellSize + 60)));

            Coord c{x,y};

            if (c == m_maze.getStart()) tile.setFillColor(sf::Color(50,150,255));
            else if (c == m_maze.getGoal()) tile.setFillColor(sf::Color(255,80,80));
            else if (m_maze.isWall(x,y)) tile.setFillColor(sf::Color::Black);
            else tile.setFillColor(sf::Color::White);

            m_window.draw(tile);
        }
    }

    // ---------------- VISITED CELLS ----------------
    {
        std::scoped_lock lock(m_mutex);
        for (auto& v : m_visited)
        {
            if (v == m_maze.getStart() || v == m_maze.getGoal())
                continue;

            sf::RectangleShape r(sf::Vector2f(
                (float)m_cellSize,(float)m_cellSize));

            r.setPosition(sf::Vector2f(
                (float)(v.first*m_cellSize),
                (float)(v.second*m_cellSize + 60)));

            r.setFillColor(sf::Color(150,180,255,120));
            m_window.draw(r);
        }
    }

    // ---------------- FINAL PATH ----------------
    {
        std::scoped_lock lock(m_mutex);

        if (m_hasResult && m_result.found)
        {
            for (auto& p : m_result.path)
            {
                if (p == m_maze.getStart() || p == m_maze.getGoal())
                    continue;

                sf::RectangleShape rect(sf::Vector2f(
                    (float)m_cellSize,(float)m_cellSize));

                rect.setPosition(sf::Vector2f(
                    (float)(p.first*m_cellSize),
                    (float)(p.second*m_cellSize + 60)));

                rect.setFillColor(m_pathColor);
                m_window.draw(rect);
            }
        }
    }

    // ---------------- RIGHT SIDE INFO PANEL ----------------
    {
        std::scoped_lock lock(m_mutex);

        float panelX = (float)(m_winW - m_statsWidth);

        sf::RectangleShape panel(
            sf::Vector2f((float)m_statsWidth,(float)m_winH - 60));
        panel.setPosition(sf::Vector2f(panelX,60.f));
        panel.setFillColor(sf::Color(245,245,245));
        panel.setOutlineColor(sf::Color(180,180,180));
        panel.setOutlineThickness(2.f);
        m_window.draw(panel);

        float textX = panelX + 20.f;
        float textY = 80.f;

        sf::Text title(m_font);
        title.setString("Algorithm Data");
        title.setCharacterSize(20);
        title.setFillColor(sf::Color::Black);
        title.setPosition(sf::Vector2f(textX,textY));
        m_window.draw(title);

        sf::Text idle(m_font);
        idle.setCharacterSize(14);
        idle.setFillColor(sf::Color(100,100,100));
        idle.setString(m_solverRunning ? "Running..." : "Idle");
        idle.setPosition(sf::Vector2f(textX,textY+28.f));
        m_window.draw(idle);

        sf::Text head(m_font);
        head.setCharacterSize(14);
        head.setFillColor(sf::Color::Black);
        head.setString("Algo   Time   Nodes   Path");
        head.setPosition(sf::Vector2f(textX,textY+55.f));
        m_window.draw(head);

        float rowY = textY + 80.f;
        sf::Text row(m_font);
        row.setCharacterSize(14);

        for (size_t i=0; i<m_stats.size(); ++i)
        {
            const auto& s = m_stats[i];
            row.setFillColor(s.color);
            char timeBuffer[32];

            // Inside the loop (line 398-403):
            snprintf(timeBuffer, sizeof(timeBuffer), "%.2f", s.timeMs);
            row.setString(
                s.name + "   " +
                std::string(timeBuffer) + "   " +
                std::to_string(s.nodes) + "   " +
                std::to_string(s.pathLength)
            );
            row.setPosition(sf::Vector2f(textX,rowY + i*22.f));
            m_window.draw(row);
        }
    }

    m_window.display();
}
