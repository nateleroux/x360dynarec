#ifndef _codepages_h
#define _codepages_h

#ifdef __cplusplus
extern "C" {
#endif

// Import information
#define CODEPAGE_IMPORT_SECTIONBASE ((LPCSTR)1)
#define CODEPAGE_IMPORT_SECTIONSIZE ((LPCSTR)2)

// Allocation info
#define CODEPAGE_ALLOC_MAX_SIZE (128 * 1024 * 1024) // 128MB
#define CODEPAGE_ALLOC_PAGE_SIZE 0x100 // 256B
#define CODEPAGE_ALLOC_MAX_PAGE_COUNT (CODEPAGE_ALLOC_MAX_SIZE / CODEPAGE_ALLOC_PAGE_SIZE)

// Supported code section sizes
#define CODEPAGE_8MB 8
#define CODEPAGE_16MB 16
#define CODEPAGE_32MB 32
#define CODEPAGE_64MB 64
#define CODEPAGE_128MB 128

// Initialize the code page subsystem
VOID CodePage_Init();

// Reserve a code section
BOOL CodePage_ReserveSpace(DWORD SizeInMB);
VOID CodePage_FreeReservedSpace();

// Allocate/free executable pages
PVOID CodePage_AllocatePage();
VOID CodePage_FreePage(PVOID Page);

// Block any allocate/free operations on other threads
VOID CodePage_LockThread();
VOID CodePage_UnlockThread();

// Copy data into the code page(s)
VOID CodePage_CopyData(PVOID Destination, PVOID Source, UINT Count);

#ifdef __cplusplus
}
#endif

#endif // _codepages_h
