#pragma once
#include <stdexcept>
#include <sstream>
#include <string>

inline void test_fail(const std::string& expr, const std::string& file, int line) {
    std::ostringstream oss;
    oss << file << ":" << line << " assertion failed: " << expr;
    throw std::runtime_error(oss.str());
}

#define ASSERT_TRUE(expr) do { if (!(expr)) test_fail(#expr, __FILE__, __LINE__); } while (0)
#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))
#define ASSERT_EQ(a, b) do { if (!((a) == (b))) test_fail(#a " == " #b, __FILE__, __LINE__); } while (0)
#define ASSERT_NE(a, b) do { if (!((a) != (b))) test_fail(#a " != " #b, __FILE__, __LINE__); } while (0)
#define ASSERT_GE(a, b) do { if (!((a) >= (b))) test_fail(#a " >= " #b, __FILE__, __LINE__); } while (0)
#define ASSERT_GT(a, b) do { if (!((a) > (b))) test_fail(#a " > " #b, __FILE__, __LINE__); } while (0)
#define ASSERT_LE(a, b) do { if (!((a) <= (b))) test_fail(#a " <= " #b, __FILE__, __LINE__); } while (0)
#define ASSERT_LT(a, b) do { if (!((a) < (b))) test_fail(#a " < " #b, __FILE__, __LINE__); } while (0)