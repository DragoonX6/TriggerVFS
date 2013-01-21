#ifndef _HASH_H_
#define _HASH_H_

#ifndef NULL
#define NULL (0L)
#endif

#define MAX_HASH_ENTRIES 16000
#define TYPE			 (0x9C)		/* If You Change This, You Change The Hash Output! */

//
// this is my hash function
// it is built upon several months of research
// and should withstand VERY heavy testing without
// duplicate hashes
//
long toHash (char* pString);

//
// my generic hash table implementation, simply pass it the
// class that you would like to be contained
//
template<class T>
class CHashTable
{
protected:
	template<class D>
	struct hEntry_s
	{
		T		     entry;
		long	     type;
		hEntry_s<D>* next;
	};

	long nCount;

	hEntry_s<T>* pTable[MAX_HASH_ENTRIES];

public:
	CHashTable ()
	{
		for (int i = 0; i < MAX_HASH_ENTRIES; i++)
		{
			pTable[i] = NULL;
		}

		nCount = 0;
	}

	~CHashTable ()
	{
		hEntry_s<T> *pEnt, *pTmp;

		for (int i = 0; i < MAX_HASH_ENTRIES; i++)
		{
			if (pTable[i])
			{
				pEnt = pTable[i];

				while (pEnt)
				{
					pTmp = pEnt;
					pEnt = pEnt->next;
					delete pTmp;
				}

				pTable[i] = NULL;
			}
		}
	}

	int  getCount () { return nCount; }

	//
	// this adds an entry to a position in the hash table
	// I would typically 'hash' the name of the object
	// being inserted
	//
	bool addHash (long nHash, T entry)
	{
		long nPos, nType;

		nPos = (nHash & 0xFFFF) % MAX_HASH_ENTRIES;
		nType = nHash >> 16;

		hEntry_s<T> *pEnt;

		pEnt = pTable[nPos];

		if (pEnt)
		{
			while (pEnt->next != NULL)
				pEnt = pEnt->next;

			pEnt->next = new hEntry_s<T>;

			pEnt = pEnt->next;
		}
		else
		{
			pTable[nPos] = pEnt = new hEntry_s<T>;
		}

		pEnt->entry = entry;
		pEnt->type  = nType;
		pEnt->next  = NULL;

		nCount++;

		return true;
	}

	//
	// this is if you retrieve the entry (via getHash ())
	// and make some changes to it, this will update it
	//
	bool updateHash (long nHash, T* entry)
	{
		long nPos, nType;

		nPos = (nHash & 0xFFFF) % MAX_HASH_ENTRIES;
		nType = nHash >> 16;

		hEntry_s<T> *pEnt;

		pEnt = pTable[nPos];

		if (!pEnt)
			return false;

		while (pEnt->type != nType)
		{
			if (pEnt->next != NULL)
				pEnt = pEnt->next;
			else
				break;
		}

		if (pEnt->type == nType)
		{
			pEnt->entry = *entry;
			return true;
		}

		return false;
	}

	//
	// this will return the entry with the
	// given hash
	//
	bool getHash (long nHash, T* entry)
	{
		long nPos, nType;

		nPos = (nHash & 0xFFFF) % MAX_HASH_ENTRIES;
		nType = nHash >> 16;

		hEntry_s<T> *pEnt;

		pEnt = pTable[nPos];

		if (!pEnt)
			return false;

		while (pEnt->type != nType)
		{
			if (pEnt->next != NULL)
				pEnt = pEnt->next;
			else
				break;
		}

		if (pEnt->type == nType)
		{
			*entry = pEnt->entry;
			return true;
		}

		return false;
	}

	//
	// I use this when I use the table for pointers to memory
	// this will return the first entry in the table (not the
	// first slot, but the first actual entry) and it's paired
	// hash value
	//
	bool getHash (long* nHash, T* entry)
	{
		for (int i = 0; i < MAX_HASH_ENTRIES; i++)
			if (pTable[i])
			{
				*entry = pTable[i]->entry;
				*nHash = i | (pTable[i]->type << 16);
				return true;
			}

		return false;
	}

	//
	// this will return the next hash in the list, given
	// an original hash value (again, I use this for freeing memory)
	//
	// Watch Out: It searches for the previous entry to give the next entry.
	// If you erase the previously returned entry (removeHash ()) you must
	// use getHash () to return another entry, because getNext () won't find
	// anymore and return false
	//
	bool getNext (long nHash, long* nNext, T* entry)
	{
		long nPos, nType;

		nPos = (nHash & 0xFFFF) % MAX_HASH_ENTRIES;
		nType = nHash >> 16;

		hEntry_s<T> *pEnt;

		pEnt = pTable[nPos];

		if (!pEnt)
			return false;

		while (pEnt->type != nType)
		{
			if (pEnt->next != NULL)
				pEnt = pEnt->next;
			else
				break;
		}

		if (pEnt->type == nType)
		{
			if (pEnt->next != NULL)
			{
				pEnt = pEnt->next;
				*entry = pEnt->entry;
				*nNext = nPos | (pEnt->type << 16);

				return true;
			}

			int i;

			for (int d = 0; d < MAX_HASH_ENTRIES; d++)
			{
				i = d + nPos + 1;

				if (i >= MAX_HASH_ENTRIES)
					i -= MAX_HASH_ENTRIES;

				if (pTable[i])
				{
					*entry = pTable[i]->entry;
					*nNext = i | (pTable[i]->type << 16);
					return true;
				}
			}
		}

		return false;
	}

	//
	// this will remove a hash from the table, freeing
	// up the slot
	//
	bool removeHash (long nHash)
	{
		long nPos, nType;

		nPos = (nHash & 0xFFFF) % MAX_HASH_ENTRIES;
		nType = nHash >> 16;

		hEntry_s<T> *pEnt, *pPrev;

		pPrev = pEnt = pTable[nPos];

		if (!pEnt)
			return false;

		if (pEnt->type == nType)
		{
			pPrev = pEnt->next;
			delete pEnt;
			pTable[nPos] = pPrev;

			nCount--;

			return true;
		}

		pEnt = pEnt->next;

		if (!pEnt)
			return false;

		while (pEnt->type != nType)
		{
			if (pEnt->next != NULL)
			{
				pPrev = pEnt;
				pEnt = pEnt->next;
			}
			else
				break;
		}

		if (pEnt->type == nType)
		{
			pPrev->next = pEnt->next;
			delete pEnt;

			nCount--;

			return true;
		}

		return false;
	}
};

#endif