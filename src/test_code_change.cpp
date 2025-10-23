// Test code change for CI workflow verification
// This file tests that code changes trigger the build job

#include <iostream>

int main() {
    std::cout << "CI workflow test - build job should run for this code change" << std::endl;
    return 0;
}
