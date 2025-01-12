#ifndef _WIN32

#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include "../include/process_lib.h"

ProcessHandle* launch_background_process(const char* command) {
    auto* handle = new ProcessHandle();
    handle->exit_code = -1;

    pid_t pid = fork();
    if (pid < 0) {
        delete handle;
        return nullptr;
    }

    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", command, nullptr);
        _exit(1);
    }

    handle->process_handle = reinterpret_cast<void*>(static_cast<intptr_t>(pid));
    return handle;
}

int wait_for_process(ProcessHandle* handle) {
    if (!handle) return -1;

    pid_t pid = static_cast<pid_t>(reinterpret_cast<intptr_t>(handle->process_handle));
    int status;
    
    if (waitpid(pid, &status, 0) == -1) {
        return -1;
    }

    if (WIFEXITED(status)) {
        handle->exit_code = WEXITSTATUS(status);
        return handle->exit_code;
    }

    return -1;
}

void cleanup_process(ProcessHandle* handle) {
    delete handle;
}

#endif // !_WIN32 