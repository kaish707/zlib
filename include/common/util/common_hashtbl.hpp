
// common_hashtbl.hpp
// 2017 Jul 21

#ifndef __common_hashtbl_hpp__
#define __common_hashtbl_hpp__

#include <stdint.h>

#define DEFAULT_TABLE_SIZE	64

namespace common {

namespace hashFncs{
typedef uint32_t (*TypeHashFnc)(const void* key, uint32_t keySize);
}

template <typename DataType>
class HashTbl
{
public:
	HashTbl(uint32_t tableSize= DEFAULT_TABLE_SIZE);
	virtual ~HashTbl();

	int  AddEntry(const void* key, uint32_t keyLen, const DataType& data);
	bool RemoveEntry(const void* key, uint32_t keyLen);
	bool FindEntry(const void* key, uint32_t keyLen, DataType* dataPtr);

protected:
	template <typename DataType>
	struct HashItem {
		HashItem(const void* key, uint32_t keyLen, const DataType& data);
		~HashItem();
		HashItem *prev, *next; void* key; DataType data; uint32_t dataSize;
	};

protected:
	hashFncs::TypeHashFnc		m_fpHashFnc;
	HashItem<DataType>**		m_pTable;
	uint32_t					m_unRoundedTableSizeMin1;
};

}

#include "common_hashtbl.impl.hpp"



#endif  // #ifndef __common_hashtbl_hpp__
