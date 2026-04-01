#include "mana/core/threads/threads.h"

int thread_create(thread_t* thread, void* (*start_routine)(void*), void* arg) {
#ifdef _WIN64
  *thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(uintptr_t)start_routine, arg, 0, NULL);
  return (*thread != NULL) ? 0 : -1;
#else
  return pthread_create(thread, NULL, start_routine, arg);
#endif
}

int thread_join(thread_t thread, void** retval) {
#ifdef _WIN64
  return WaitForSingleObject(thread, INFINITE) == WAIT_OBJECT_0 ? 0 : -1;
#else
  return pthread_join(thread, retval);
#endif
}

int thread_detach(thread_t thread) {
#ifdef _WIN64
  return CloseHandle(thread) ? 0 : -1;
#else
  return pthread_detach(thread);
#endif
}

int mutex_init(mutex_t* mutex) {
#ifdef _WIN64
  InitializeCriticalSection(mutex);
  return 0;
#else
  return pthread_mutex_init(mutex, NULL);
#endif
}

int mutex_lock(mutex_t* mutex) {
#ifdef _WIN64
  EnterCriticalSection(mutex);
  return 0;
#else
  return pthread_mutex_lock(mutex);
#endif
}

int mutex_unlock(mutex_t* mutex) {
#ifdef _WIN64
  LeaveCriticalSection(mutex);
  return 0;
#else
  return pthread_mutex_unlock(mutex);
#endif
}

int mutex_destroy(mutex_t* mutex) {
#ifdef _WIN64
  DeleteCriticalSection(mutex);
  return 0;
#else
  return pthread_mutex_destroy(mutex);
#endif
}

int cond_init(cond_t* cond) {
#ifdef _WIN64
  InitializeConditionVariable(cond);
  return 0;
#else
  return pthread_cond_init(cond, NULL);
#endif
}

int cond_wait(cond_t* cond, mutex_t* mutex) {
#ifdef _WIN64
  return SleepConditionVariableCS(cond, mutex, INFINITE) ? 0 : -1;
#else
  return pthread_cond_wait(cond, mutex);
#endif
}

int cond_signal(cond_t* cond) {
#ifdef _WIN64
  WakeConditionVariable(cond);
  return 0;
#else
  return pthread_cond_signal(cond);
#endif
}

int cond_broadcast(cond_t* cond) {
#ifdef _WIN64
  WakeAllConditionVariable(cond);
  return 0;
#else
  return pthread_cond_broadcast(cond);
#endif
}

int cond_destroy(cond_t* cond) {
#ifdef _WIN64
  // There is no function to destroy a condition variable on Windows.
  // However, a condition variable does not allocate any resources,
  // so there is nothing to do here.
  return 0;
#else
  return pthread_cond_destroy(cond);
#endif
}
