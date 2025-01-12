#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include "../lib/process_lib/include/process_lib.h"

std::string get_platform_command(const std::string& windows_cmd, const std::string& unix_cmd) {
#ifdef _WIN32
    return "cmd.exe /c " + windows_cmd;
#else
    return unix_cmd;
#endif
}

void test_simple_command() {
    std::cout << "Test 1: Simple command execution" << std::endl;
    std::string cmd = get_platform_command("echo Hello, World!", "echo 'Hello, World!'");

    auto* handle = launch_background_process(cmd.c_str());
    if (!handle) {
        std::cerr << "Failed to launch process" << std::endl;
        return;
    }

    int exit_code = wait_for_process(handle);
    std::cout << "Process finished with exit code: " << exit_code << std::endl;
    cleanup_process(handle);
}

void test_long_running_process() {
    std::cout << "\nTest 2: Long running process" << std::endl;
    std::string cmd = get_platform_command("timeout 3", "sleep 3");

    auto* handle = launch_background_process(cmd.c_str());
    if (!handle) {
        std::cerr << "Failed to launch process" << std::endl;
        return;
    }

    std::cout << "Process started, waiting..." << std::endl;
    int exit_code = wait_for_process(handle);
    std::cout << "Process finished with exit code: " << exit_code << std::endl;
    cleanup_process(handle);
}

void test_invalid_command() {
    std::cout << "\nTest 3: Invalid command" << std::endl;
    const char* cmd = "this_command_does_not_exist";

    auto* handle = launch_background_process(cmd);
    if (!handle) {
        std::cerr << "Failed to launch process (expected for invalid command)" << std::endl;
        return;
    }

    int exit_code = wait_for_process(handle);
    std::cout << "Process finished with exit code: " << exit_code << std::endl;
    cleanup_process(handle);
}

void test_directory_listing() {
    std::cout << "\nTest 4: Directory listing" << std::endl;
    std::string cmd = get_platform_command("dir", "ls -la");

    auto* handle = launch_background_process(cmd.c_str());
    if (!handle) {
        std::cerr << "Failed to launch process" << std::endl;
        return;
    }

    int exit_code = wait_for_process(handle);
    std::cout << "Process finished with exit code: " << exit_code << std::endl;
    cleanup_process(handle);
}

void test_multiple_commands() {
    std::cout << "\nTest 5: Multiple commands" << std::endl;
    std::string cmd = get_platform_command(
        "cd . && echo Current dir && dir",
        "pwd && echo Current dir && ls"
    );

    auto* handle = launch_background_process(cmd.c_str());
    if (!handle) {
        std::cerr << "Failed to launch process" << std::endl;
        return;
    }

    int exit_code = wait_for_process(handle);
    std::cout << "Process finished with exit code: " << exit_code << std::endl;
    cleanup_process(handle);
}

int main() {
    std::cout << "Starting process library tests..." << std::endl;
    std::cout << "Running on: " << 
#ifdef _WIN32
        "Windows"
#else
        "Unix"
#endif
        << " platform" << std::endl;

    test_simple_command();
    test_long_running_process();
    test_invalid_command();
    test_directory_listing();
    test_multiple_commands();

    std::cout << "\nAll tests completed." << std::endl;
    return 0;
} 