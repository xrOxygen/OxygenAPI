#pragma once

#include <memory>

using std::swap;

template <typename T>
class xalloc {
public:
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using pointer = T * ;
    using const_pointer = const T*;
    using reference = T & ;
    using const_reference = const T&;
    using value_type = T;

    template <class Other>
    struct rebind {
        using other = xalloc<Other>;
    };

    pointer address(reference ref) const { return &ref; }
    const_pointer address(const_reference ref) const { return &ref; }

    xalloc() = default;
    xalloc(const xalloc<T>&) = default;

    template <class Other>
    xalloc(const xalloc<Other>&) {}

    template <class Other>
    xalloc& operator=(const xalloc<Other>&) {
        return *this;
    }
#pragma warning(push)
#pragma warning(disable: 4267)
    pointer allocate(const size_type n, const void* p = nullptr) const 
	{
		return (pointer) Memory.mem_alloc_for_arrays(n * sizeof(T), typeid(T).name());
		//return xr_alloc<T>(n); 
	}
#pragma warning(pop)

    void deallocate(pointer p, const size_type) const 
	{ 
		Memory.mem_free_for_arrays(p, typeid(T).name());
		//xr_free(p); 
	}

    void deallocate(void* p, const size_type) const 
	{ 
		Memory.mem_free_for_arrays(p, typeid(T).name());
		//xr_free(p); 
	}

    void construct(pointer p, const T& _Val) { new (p) T(_Val); }

    void destroy(pointer p) { p->~T(); }

    size_type max_size() const {
        constexpr size_type count = std::numeric_limits<size_type>::max() / sizeof(T);
		
		if constexpr(0 < count)
			return count;
		else 
			return 1;
    }
};

#if 0
// for list container ONLY!!
// do NOT use in std::vector or any container that allowing sequence access to elements
template <typename T>
class xrLazyAllocator
{
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using pointer = T * ;
	using const_pointer = const T*;
	using reference = T & ;
	using const_reference = const T&;
	using value_type = T;

	template<typename Elem = T>
	struct AllocatorNode
	{
		bool isDeleted;
		Elem theElement;
	};

	pointer address(reference ref) const { return &ref; }
	const_pointer address(const_reference ref) const { return &ref; }

	template <class Other>
	xrLazyAllocator(const xrLazyAllocator<Other>&) {}

	template <class Other>
	xrLazyAllocator& operator=(const xrLazyAllocator<Other>&) {
		return *this;
	}
#pragma warning(push)
#pragma warning(disable: 4267)
	pointer allocate(const size_type n, const void* p = nullptr) const 
	{ 
		// we should allocate very rare. Most of the time - use existent allocated space

		if (pBuffer == nullptr)
		{
			AllocatedNum = n * 5;
			Size = n;
			pBuffer = xr_alloc<AllocatorNode<T>>(n * 5);
		}
		else
		{
			// check if we don't have enough space
			if (Size + n > AllocatedNum)
			{
				AllocatedNum = AllocatedNum + n + 5;
				pBuffer = xr_realloc(pBuffer, AllocatedNum * sizeof(AllocatorNode<T>));
			}
			Size += n;
		}

		return (pointer)pBuffer[Size - n];
	}
#pragma warning(pop)

	void internalDeallocate(void* p)
	{
		size_t InternalIndex = (p - pBuffer) / sizeof(T);
		
	}

	void deallocate(pointer p, const size_type) const 
	{ 
		xr_free(p); 
	}

	void deallocate(void* p, const size_type) const 
	{ 
		xr_free(p); 
	}

	void construct(pointer p, const T& _Val) { new (p) T(_Val); }

	void destroy(pointer p) { p->~T(); }

	size_type max_size() const {
		const unsigned long long count = std::numeric_limits<size_type>::max() / sizeof(T);
		return 0 < count ? count : 1;
	}

	size_t AllocatedNum;
	size_t Size;
	void* pBuffer;
};

#endif

struct xr_allocator {
    template <typename T>
    struct helper {
        using result = xalloc<T>;
    };

    static void* alloc(const size_t n) { return xr_malloc(n); }

    template <typename T>
    static void dealloc(T*& p) {
        xr_free(p);
    }
};

template <class T, class Other>
bool operator==(const xalloc<T>&, const xalloc<Other>&) {
    return true;
}

template <class T, class Other>
bool operator!=(const xalloc<T>&, const xalloc<Other>&) {
    return false;
}

namespace std {

    template <class Tp1, class Tp2>
    xalloc<Tp2>& __stl_alloc_rebind(xalloc<Tp1>& a, const Tp2*) {
        return (xalloc<Tp2>&)(a);
    }

    template <class Tp1, class Tp2>
    xalloc<Tp2> __stl_alloc_create(xalloc<Tp1>&, const Tp2*) {
        return xalloc<Tp2>();
    }

} // namespace std

  // string(char)
  //using xstring = std::basic_string<char, std::char_traits<char>, xalloc<char>>;

  // vector
template <typename T, typename allocator = xalloc<T>>
using xvector = std::vector<T, allocator>;

template <typename T, typename Alloc>
void clearAndReserve(xvector<T, Alloc>& vec) {
    if (vec.capacity() <= (vec.size() / 2))
        vec.clear();
    else {
        vec.shrink_to_fit();
        vec.clear();
    }
}


// deque
template <typename T, typename allocator = xalloc<T>>
using xr_deque = std::deque<T, allocator>;

// queue
template <typename T, typename container = xr_deque<T>>
using xr_queue = std::queue<T, container>;

// stack
template <typename T, class C = xr_deque<T>>
using xr_stack = std::stack<T, C>;

template <typename T, typename allocator = xalloc<T>>
using xr_list = std::list<T, allocator>;

template <typename K, class P = std::less<K>, typename allocator = xalloc<K>>
using xr_set = std::set<K, P, allocator>;

template <typename K, class P = std::less<K>, typename allocator = xalloc<K>>
using xr_multiset = std::multiset<K, P, allocator>;

template <typename K, class V, class P = std::less<K>, typename allocator = xalloc<std::pair<const K, V>>>
using xr_map = std::map<K, V, P, allocator>;

template <typename K, class V, class P = std::less<K>, typename allocator = xalloc<std::pair<const K, V>>>
using xr_multimap = std::multimap<K, V, P, allocator>;

template <typename K, class V, class Traits = std::equal_to<K>,
    typename allocator = xalloc<std::pair<const K, V>>>
    using xr_hash_map = std::unordered_map<K, V, std::hash<K>, Traits, allocator>;

struct pred_str {
    bool operator()(const char* x, const char* y) const { return std::strcmp(x, y) < 0; }
};
struct pred_stri {
    bool operator()(const char* x, const char* y) const { return _stricmp(x, y) < 0; }
};

#include "FixedVector.h"
#include "buffer_vector.h"

// auxilary definition
using LPSTRVec = xvector<LPSTR>;
using U8Vec = xvector<u8>;
using U16Vec = xvector<u16>;
using U32Vec = xvector<u32>;
using FloatVec = xvector<float>;
using IntVec = xvector<int>;
using boolVec = xvector<bool>;

template <typename K, class V, class Hasher = std::hash<K>, class Traits = std::equal_to<K>,
    typename allocator = xalloc<std::pair<const K, V>>>
    using xr_unordered_map = std::unordered_map<K, V, Hasher, Traits, allocator>;



