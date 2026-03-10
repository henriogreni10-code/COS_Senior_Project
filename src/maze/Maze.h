#pragma once
#include <vector>
#include <utility>

using Coord = std::pair<int,int>;

class Maze {
private:
    int m_width, m_height;
    std::vector<std::vector<int>> m_grid;
    Coord m_start;
    Coord m_goal;

public:
    Maze(int w, int h);

    int width() const;
    int height() const;
    bool inBounds(int x, int y) const;
    bool isWall(int x, int y) const;
    void setWall(int x, int y, bool wall);
    void clear();
    void setStart(const Coord& s);
    void setGoal(const Coord& g);
    Coord getStart() const;
    Coord getGoal() const;
    std::vector<Coord> neighbors(const Coord& c) const;
};
