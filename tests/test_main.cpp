#include <iostream>
#include <stdexcept>
#include <string>

void run_maze_tests();
void run_container_tests();
void run_algorithm_tests();
void run_generator_tests();

int main() {
    try {
        run_maze_tests();
        run_container_tests();
        run_algorithm_tests();
        run_generator_tests();
        std::cout << "[PASS] All tests passed.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[FAIL] " << e.what() << '\n';
        return 1;
    }
}