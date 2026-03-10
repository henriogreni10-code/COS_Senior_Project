#pragma once
#include <vector>
#include <utility>
#include <functional>

template<typename Key, typename Value>
class MinHeap {
public:
    MinHeap() = default;
    void push(const Key& k, const Value& v) {
        data.emplace_back(k, v);
        siftUp(data.size()-1);
    }
    std::pair<Key,Value> pop() {
        if (data.empty()) throw std::runtime_error("pop empty heap");
        auto top = data.front();
        data[0] = data.back();
        data.pop_back();
        if (!data.empty()) siftDown(0);
        return top;
    }
    bool empty() const { return data.empty(); }
    void clear() { data.clear(); }
private:
    std::vector<std::pair<Key,Value>> data;
    void siftUp(size_t idx) {
        while (idx > 0) {
            size_t parent = (idx - 1) >> 1;
            if (data[idx].first < data[parent].first) {
                std::swap(data[idx], data[parent]);
                idx = parent;
            } else break;
        }
    }
    void siftDown(size_t idx) {
        size_t n = data.size();
        while (true) {
            size_t l = idx*2 + 1;
            size_t r = idx*2 + 2;
            size_t smallest = idx;
            if (l < n && data[l].first < data[smallest].first) smallest = l;
            if (r < n && data[r].first < data[smallest].first) smallest = r;
            if (smallest == idx) break;
            std::swap(data[idx], data[smallest]);
            idx = smallest;
        }
    }
};
