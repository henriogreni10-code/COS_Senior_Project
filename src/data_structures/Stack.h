#pragma once
#include <vector>

template<typename T>
class Stack {
public:
    void push(const T& v) { data.push_back(v); }
    void pop() { if (!data.empty()) data.pop_back(); }
    T& top() { return data.back(); }
    bool empty() const { return data.empty(); }
    size_t size() const { return data.size(); }
private:
    std::vector<T> data;
};
