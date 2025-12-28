#include <Windows.h>
#include <objbase.h>
#include <stdio.h>

#include "lib.h"

void shmex_generate_shm_name(char *name, int attempt) {
  GUID g;
  if (CoCreateGuid(&g) != S_OK) return;
  snprintf(name, SHMEX_SHM_NAME_LEN, SHMEX_SHM_NAME_PREFIX "%08lX%04hX%04hX%02X%02X#%03d", 
          g.Data1, g.Data2, g.Data3, g.Data4[0], g.Data4[1], attempt);
}

ShmexLibResult shmex_allocate_unguarded(Shmex *payload) {
  ShmexLibResult res;
  HANDLE handle;

  if (payload->name != NULL) {
    handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, payload->capacity, (LPCSTR) payload->name);
  } else {
    payload->name = ALLOC(SHMEX_SHM_NAME_LEN);
    int attempt = 0;
    do {
      shmex_generate_shm_name(payload->name, attempt);
      handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, payload->capacity, (LPCSTR) payload->name);
      attempt++;
    } while (!handle && GetLastError() == ERROR_ALREADY_EXISTS && attempt < SHMEX_ALLOC_MAX_ATTEMPTS);
  }

  if (!handle || GetLastError() == ERROR_ALREADY_EXISTS) {
    if (handle) CloseHandle(handle);
    res = SHMEX_ERROR_SHM_OPEN;
    goto shm_create_exit; 
  }

  payload->handle = handle;
  res = SHMEX_RES_OK;

shm_create_exit:
  return res;
}

ShmexLibResult shmex_open_and_mmap(Shmex *payload) {
  if (!payload->handle) {
    return SHMEX_ERROR_SHM_OPEN;
  }

  payload->mapped_memory = MapViewOfFile((HANDLE) payload->handle, FILE_MAP_ALL_ACCESS, 0, 0, (DWORD) payload->capacity);
  if (!payload->mapped_memory) {
    payload->mapped_memory = MAP_FAILED;
    return SHMEX_ERROR_MMAP;
  }

  return SHMEX_RES_OK;
}

ShmexLibResult shmex_set_capacity(Shmex *payload, size_t capacity) {
  if (payload->capacity == capacity) {
    return SHMEX_RES_OK;
  }

  if (!payload->handle) {
    return SHMEX_ERROR_SHM_OPEN;
  }

  HANDLE new_handle;
  ShmexLibResult result;
  char *data = NULL;
  int data_size = min(payload->size, capacity);

  payload->mapped_memory = MapViewOfFile(payload->handle, FILE_MAP_ALL_ACCESS, 0, 0, payload->capacity);
  if (!payload->mapped_memory) {
    result = SHMEX_ERROR_MMAP;
    goto shmex_set_capacity_exit;
  }

  data = ALLOC(data_size);
  memcpy(data, payload->mapped_memory, data_size);

  UnmapViewOfFile(payload->mapped_memory);
  CloseHandle(payload->handle);

  payload->handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, capacity, (LPCSTR) payload->name);
  if (!payload->handle) {
    result = SHMEX_ERROR_SHM_OPEN;
    goto shmex_set_capacity_exit;
  }

  payload->mapped_memory = MapViewOfFile(payload->handle, FILE_MAP_ALL_ACCESS, 0, 0, data_size);
  if (!payload->mapped_memory) {
    result = SHMEX_ERROR_MMAP;
    goto shmex_set_capacity_exit;
  }

  memcpy(payload->mapped_memory, data, data_size);
  payload->capacity = capacity;
  payload->size = data_size;
  
  result = SHMEX_RES_OK;

shmex_set_capacity_exit:
  if (data) FREE(data);
  return result;
}

void shmex_unmap(Shmex *payload) {
  if (payload->mapped_memory != MAP_FAILED) {
    UnmapViewOfFile(payload->mapped_memory);
  }

  payload->mapped_memory = MAP_FAILED;
}

void shmex_shm_unlink(void *handle) {
  if (handle) {
    CloseHandle(handle);
  }
}