/**
 * @file mpqapi.h
 *
 * Interface of functions for creating and editing MPQ files.
 */
#ifndef __MPQAPI_H__
#define __MPQAPI_H__

void mpqapi_remove_hash_entry(const char *pszName);
void mpqapi_alloc_block(int block_offset, int block_size);
_BLOCKENTRY *mpqapi_new_block(int *block_index);
int FetchHandle(const char *pszName);
int mpqapi_get_hash_index(short index, int hash_a, int hash_b, int locale);
void mpqapi_remove_hash_entries(BOOL(*fnGetName)(DWORD, char *));
BOOL mpqapi_write_file(const char *pszName, const BYTE *pbData, DWORD dwLen);
_BLOCKENTRY *mpqapi_add_file(const char *pszName, _BLOCKENTRY *pBlk, int block_index);
BOOL mpqapi_write_file_contents(const char *pszName, const BYTE *pbData, DWORD dwLen, _BLOCKENTRY *pBlk);
int mpqapi_find_free_block(int size, int *block_size);
void mpqapi_rename(char *pszOld, char *pszNew);
BOOL mpqapi_has_file(const char *pszName);
BOOL OpenMPQ(const char *pszArchive, BOOL hidden, DWORD dwChar);
BOOL mpqapi_flush_and_close(const char *pszArchive, BOOL bFree, DWORD dwChar);
/* rdata */

/* data */

#endif /* __MPQAPI_H__ */
