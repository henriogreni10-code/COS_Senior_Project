#include "Maze.h"


// Constructor
Maze::Maze(int w, int h)
{
    // recursive-backtracker
    if (w % 2 == 0) w -= 1;
    if (h % 2 == 0) h -= 1;

    m_width  = w;
    m_height = h;
    m_grid.assign(h, std::vector<int>(w, 1));

    m_start = {1, 1};
    m_goal  = {w - 2, h - 2};
}

int Maze::width()  const { return m_width; }
int Maze::height() const { return m_height; }

bool Maze::inBounds(int x, int y) const {
    return (x >= 0 && x < m_width && y >= 0 && y < m_height);
}

bool Maze::isWall(int x, int y) const {
    return m_grid[y][x] == 1;
}

void Maze::setWall(int x, int y, bool wall) {
    m_grid[y][x] = wall ? 1 : 0;
}

void Maze::clear() {
    for (auto& row : m_grid)
        for (int& c : row)
            c = 1;
}

void Maze::setStart(const Coord& s) {
    m_start = s;
}

void Maze::setGoal(const Coord& g) {
    m_goal = g;
}

Coord Maze::getStart() const { return m_start; }
Coord Maze::getGoal() const { return m_goal; }

std::vector<Coord> Maze::neighbors(const Coord& c) const
{
    static const int dx[4] = {1, -1, 0, 0};
    static const int dy[4] = {0, 0, 1, -1};

    std::vector<Coord> n;

    for (int i = 0; i < 4; i++) {
        int nx = c.first + dx[i];
        int ny = c.second + dy[i];

        if (inBounds(nx, ny) && !isWall(nx, ny))
            n.push_back({nx, ny});
    }

    return n;
}
