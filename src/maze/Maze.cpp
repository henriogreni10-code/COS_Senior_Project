#include "Maze.h"

// Constructor initializes maze dimensions and grid.
Maze::Maze(int w, int h)
{
    // Ensure dimensions are odd.
    // Recursive backtracking requires odd-sized grids so that:
    // - cells exist at odd indices
    // - walls exist at even indices between them
    if (w % 2 == 0) w -= 1;
    if (h % 2 == 0) h -= 1;

    m_width  = w;
    m_height = h;

    // Initialize grid with all walls (1 = wall, 0 = passage).
    m_grid.assign(h, std::vector<int>(w, 1));

    // Default start and goal positions.
    // Positioned inside the maze, not on borders.
    m_start = {1, 1};
    m_goal  = {w - 2, h - 2};
}

// Returns maze width.
int Maze::width() const { return m_width; }

// Returns maze height.
int Maze::height() const { return m_height; }

// Checks whether given coordinates are inside the maze boundaries.
bool Maze::inBounds(int x, int y) const {
    return (x >= 0 && x < m_width && y >= 0 && y < m_height);
}

// Returns true if the given cell is a wall.
bool Maze::isWall(int x, int y) const {
    return m_grid[y][x] == 1;
}

// Sets a cell as wall or passage.
void Maze::setWall(int x, int y, bool wall) {
    m_grid[y][x] = wall ? 1 : 0;
}

// Resets the entire maze to all walls.
void Maze::clear() {
    for (auto& row : m_grid)
        for (int& c : row)
            c = 1;
}

// Updates start position.
void Maze::setStart(const Coord& s) {
    m_start = s;
}

// Updates goal position.
void Maze::setGoal(const Coord& g) {
    m_goal = g;
}

// Returns start position.
Coord Maze::getStart() const { return m_start; }

// Returns goal position.
Coord Maze::getGoal() const { return m_goal; }

// Returns all valid neighboring cells (up, down, left, right)
// that are inside bounds and not walls.
std::vector<Coord> Maze::neighbors(const Coord& c) const
{
    // Direction vectors for 4-directional movement.
    static const int dx[4] = {1, -1, 0, 0};
    static const int dy[4] = {0, 0, 1, -1};

    std::vector<Coord> n;

    for (int i = 0; i < 4; i++) {
        int nx = c.first + dx[i];
        int ny = c.second + dy[i];

        // Only include neighbors that are inside the maze
        // and are not walls (i.e., traversable).
        if (inBounds(nx, ny) && !isWall(nx, ny))
            n.push_back({nx, ny});
    }

    return n;
}
