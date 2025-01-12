#ifndef PROCESS_LIB_H
#define PROCESS_LIB_H

#ifdef _WIN32
    #define PROCESS_LIB_EXPORT __declspec(dllexport)
#else
    #define PROCESS_LIB_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int exit_code;
    void* process_handle;
} ProcessHandle;

/**
 * Запускает программу в фоновом режиме
 * @param command команда для запуска
 * @return handle процесса или NULL в случае ошибки
 */
PROCESS_LIB_EXPORT ProcessHandle* launch_background_process(const char* command);

/**
 * Ожидает завершения процесса
 * @param handle handle процесса
 * @return код возврата процесса или -1 в случае ошибки
 */
PROCESS_LIB_EXPORT int wait_for_process(ProcessHandle* handle);

/**
 * Освобождает ресурсы, связанные с процессом
 * @param handle handle процесса
 */
PROCESS_LIB_EXPORT void cleanup_process(ProcessHandle* handle);

#ifdef __cplusplus
}
#endif

#endif // PROCESS_LIB_H 