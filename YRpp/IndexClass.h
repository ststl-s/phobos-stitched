#pragma once

#include <YRPPCore.h>
#include <GenericList.h>

/*
* IndexClass, most impl from CCR
* --secsome
*/

// TKey is always int in WW's code
template<typename TKey, typename TValue>
class IndexClass
{
public:
	IndexClass(void);
	~IndexClass(void);

	constexpr bool AddIndex(TKey id, const TValue& data);
	bool AddIndex(TKey id, TValue&& data);
	bool RemoveIndex(TKey id);
	bool IsPresent(TKey id) const;
	int Count() const;
	const TValue& FetchIndex(TKey id) const;
	TValue& FetchIndex(TKey id);
	void Clear();
	bool Reverse(int nAmount);
	inline void Sort();

	struct NodeElement
	{
		TKey ID;
		TValue Data;
	};

	NodeElement* IndexTable;
	int IndexCount;
	int IndexSize;
	bool IsSorted;
	PROTECTED_PROPERTY(char, padding[3]);
	NodeElement* Archive;

	// ranged for support
	NodeElement* begin() const { return IndexTable; }
	NodeElement* end() const { return &IndexTable[IndexCount]; }

private:
	bool IncreaseTableSize(int nAmount);
	bool IsArchiveSame(TKey id) const;
	void InvalidateArchive();
	void SetArchive(NodeElement const* pNode);
	NodeElement const* SearchForNode(TKey id) const;

	static int __cdecl search_compfunc(void const * ptr, void const * ptr2);
};

template<typename TKey, typename TValue>
IndexClass<TKey, TValue>::IndexClass() :
	IndexTable(0),
	IndexCount(0),
	IndexSize(0),
	IsSorted(false),
	Archive(0)
{
	this->InvalidateArchive();
}

template<typename TKey, typename TValue>
IndexClass<TKey, TValue>::~IndexClass()
{
	this->Clear();
}

template<typename TKey, typename TValue>
void IndexClass<TKey, TValue>::Clear()
{
	GameDelete(this->IndexTable);
	this->IndexTable = 0;
	this->IndexCount = 0;
	this->IndexSize = 0;
	this->IsSorted = false;
	this->InvalidateArchive();
}

template<typename TKey, typename TValue>
bool IndexClass<TKey, TValue>::IncreaseTableSize(int amount)
{
	if (amount < 0) return false;

	NodeElement* table = GameCreateArray<NodeElement>(this->IndexSize + amount);
	if (table != nullptr)
	{
		for (int i = 0; i < this->IndexCount; ++i)
			table[i] = this->IndexTable[i];

		GameDelete(this->IndexTable);
		this->IndexTable = table;
		this->IndexSize += amount;
		this->InvalidateArchive();

		return true;
	}
	return false;
}

template<typename TKey, typename TValue>
bool IndexClass<TKey, TValue>::Reverse(int amount)
{
	Clear();
	return IncreaseTableSize(amount);
}

template<typename TKey, typename TValue>
inline void IndexClass<TKey, TValue>::Sort()
{
	if (!IsSorted)
	{
		qsort(&this->IndexTable[0], this->IndexCount, sizeof(this->IndexTable[0]), this->search_compfunc);
		InvalidateArchive();
		IsSorted = true;
	}
}

template<typename TKey, typename TValue>
int IndexClass<TKey, TValue>::Count() const
{
	return this->IndexCount;
}

template<typename TKey, typename TValue>
bool IndexClass<TKey, TValue>::IsPresent(TKey id) const
{
	if (!this->IndexCount)
		return false;

	if (this->IsArchiveSame(id))
		return true;

	NodeElement const* nodeptr = SearchForNode(id);

	if (nodeptr != nullptr)
	{
		const_cast<IndexClass<TKey, TValue>*>(this)->SetArchive(nodeptr);
		return true;
	}

	return false;
}

template<typename TKey, typename TValue>
const TValue& IndexClass<TKey, TValue>::FetchIndex(TKey id) const
{
	return this->IsPresent(id) ? this->Archive->Data : TValue();
}

template<typename TKey, typename TValue>
TValue& IndexClass<TKey, TValue>::FetchIndex(TKey id)
{
	if (!this->IsPresent(id))
	{
		this->AddIndex(id, TValue());
		this->IsPresent(id);
	}

	return this->Archive->Data;
}

template<typename TKey, typename TValue>
bool IndexClass<TKey, TValue>::IsArchiveSame(TKey id) const
{
	return this->Archive != 0 && this->Archive->ID == id;
}

template<typename TKey, typename TValue>
void IndexClass<TKey, TValue>::InvalidateArchive()
{
	this->Archive = nullptr;
}

template<typename TKey, typename TValue>
void IndexClass<TKey, TValue>::SetArchive(NodeElement const* node)
{
	this->Archive = const_cast<NodeElement*>(node);
}

template<typename TKey, typename TValue>
constexpr bool IndexClass<TKey, TValue>::AddIndex(TKey id, const TValue& data)
{
	if (this->IndexCount + 1 > this->IndexSize)
		if (!this->IncreaseTableSize(this->IndexSize == 0 ? 10 : this->IndexSize))
			return false;

	this->IndexTable[IndexCount].ID = std::move(id);
	this->IndexTable[IndexCount].Data = std::move(data);
	++this->IndexCount;
	this->IsSorted = false;

	return true;
}

template<typename TKey, typename TValue>
bool IndexClass<TKey, TValue>::AddIndex(TKey id, TValue&& data)
{
	if (this->IndexCount + 1 > this->IndexSize)
		if (!this->IncreaseTableSize(this->IndexSize == 0 ? 10 : this->IndexSize))
			return false;

	this->IndexTable[IndexCount].ID = std::move(id);
	this->IndexTable[IndexCount].Data = std::move(data);
	++this->IndexCount;
	this->IsSorted = false;

	return true;
}

template<typename TKey, typename TValue>
bool IndexClass<TKey, TValue>::RemoveIndex(TKey id)
{
	int found_index = -1;
	for (int i = 0; i < this->IndexCount; ++i)
		if (this->IndexTable[i].ID == id)
		{
			found_index = i;
			break;
		}

	if (found_index != -1)
	{
		for (int i = found_index + 1; i < this->IndexCount; ++i)
			IndexTable[i - 1] = IndexTable[i];
		--IndexCount;

		IndexTable[IndexCount] = std::move(NodeElement(TKey(), TValue()));		// zap last (now unused) element

		this->InvalidateArchive();
		return true;
	}

	return false;
}

template<typename TKey, typename TValue>
int __cdecl IndexClass<TKey, TValue>::search_compfunc(void const* ptr1, void const* ptr2)
{
	if (*(int const*)ptr1 == *(int const*)ptr2)
	{
		return 0;
	}
	if (*(int const*)ptr1 < *(int const*)ptr2)
	{
		return -1;
	}
	return 1;
}

template<typename TKey, typename TValue>
typename IndexClass<TKey, TValue>::NodeElement const* IndexClass<TKey, TValue>::SearchForNode(TKey id) const
{
	if (!this->IndexCount)
		return 0;

	const_cast<IndexClass<TKey, TValue>*>(this)->Sort();

	NodeElement node;
	node.ID = id;
	return(
			(NodeElement const*)bsearch(&node, &this->IndexTable[0], this->IndexCount,
			sizeof(this->IndexTable[0]), this->search_compfunc)
		);
}
