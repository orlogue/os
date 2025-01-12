#ifdef _WIN32

#include <windows.h>
#include "../include/process_lib.h"

ProcessHandle* launch_background_process(const char* command) {
    auto* handle = new ProcessHandle();
    handle->exit_code = -1;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char* cmd = _strdup(command);
    if (!cmd) {
        delete handle;
        return nullptr;
    }

    if (!CreateProcessA(
        nullptr,
        cmd,
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi
    )) {
        free(cmd);
        delete handle;
        return nullptr;
    }

    free(cmd);
    CloseHandle(pi.hThread);

    handle->process_handle = pi.hProcess;
    return handle;
}

int wait_for_process(ProcessHandle* handle) {
    if (!handle) return -1;

    HANDLE process_handle = static_cast<HANDLE>(handle->process_handle);
    if (WaitForSingleObject(process_handle, INFINITE) == WAIT_FAILED) {
        return -1;
    }

    DWORD exit_code;
    if (!GetExitCodeProcess(process_handle, &exit_code)) {
        return -1;
    }

    handle->exit_code = static_cast<int>(exit_code);
    return handle->exit_code;
}

void cleanup_process(ProcessHandle* handle) {
    if (handle && handle->process_handle) {
        CloseHandle(static_cast<HANDLE>(handle->process_handle));
    }
    delete handle;
}

#endif // _WIN32 