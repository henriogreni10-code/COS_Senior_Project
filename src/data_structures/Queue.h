#pragma once
#include <vector>
#include <stdexcept>

template<typename T>
class Queue {
public:
    Queue(): head(0) {}
    void push(const T& v) { data.push_back(v); }
    void pop() {
        if (empty()) throw std::runtime_error("pop empty queue");
        ++head;
        if (head > 32 && head*2 >= data.size()) {
            // compact
            data.erase(data.begin(), data.begin()+head);
            head = 0;
        }
    }
    T& front() { return data[head]; }
    bool empty() const { return head >= data.size(); }
    size_t size() const { return data.size() - head; }
private:
    std::vector<T> data;
    size_t head;
};
