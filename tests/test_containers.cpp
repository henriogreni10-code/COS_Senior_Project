#include "test_utils.h"
#include "../src/data_structures/Queue.h"
#include "../src/data_structures/Stack.h"
#include "../src/data_structures/MinHeap.h"
#include <stdexcept>
#include <string>

void run_container_tests() {
    {
        Queue<int> q;
        ASSERT_TRUE(q.empty());
        q.push(10);
        q.push(20);
        q.push(30);
        ASSERT_FALSE(q.empty());
        ASSERT_EQ(q.size(), 3u);
        ASSERT_EQ(q.front(), 10);
        q.pop();
        ASSERT_EQ(q.front(), 20);
        ASSERT_EQ(q.size(), 2u);
        q.pop();
        q.pop();
        ASSERT_TRUE(q.empty());
    }

    {
        Queue<int> q;
        bool threw = false;
        try {
            q.pop();
        } catch (const std::runtime_error&) {
            threw = true;
        }
        ASSERT_TRUE(threw);
    }

    {
        Stack<std::string> st;
        ASSERT_TRUE(st.empty());
        st.push("a");
        st.push("b");
        ASSERT_EQ(st.size(), 2u);
        ASSERT_EQ(st.top(), "b");
        st.pop();
        ASSERT_EQ(st.top(), "a");
        st.pop();
        ASSERT_TRUE(st.empty());
    }

    {
        MinHeap<int, char> heap;
        ASSERT_TRUE(heap.empty());
        heap.push(5, 'e');
        heap.push(1, 'a');
        heap.push(3, 'c');
        heap.push(2, 'b');

        auto p1 = heap.pop();
        auto p2 = heap.pop();
        auto p3 = heap.pop();
        auto p4 = heap.pop();

        ASSERT_EQ(p1.first, 1);
        ASSERT_EQ(p1.second, 'a');
        ASSERT_EQ(p2.first, 2);
        ASSERT_EQ(p3.first, 3);
        ASSERT_EQ(p4.first, 5);
        ASSERT_TRUE(heap.empty());
    }

    {
        MinHeap<int, int> heap;
        bool threw = false;
        try {
            heap.pop();
        } catch (const std::runtime_error&) {
            threw = true;
        }
        ASSERT_TRUE(threw);
    }
}