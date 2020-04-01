#include <list>
#include <stdexcept>
#include <vector>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> > class HashMap
{
private:
    //9
    //iterators

    template<typename ContainerType, typename ElementType, typename IterXType, typename IterYType>
    class Iterator2D
    {
    public:
        Iterator2D(const ContainerType *cont_, IterXType itx_, IterYType ity_, int skip = 0)
            : cont(cont_), itx(itx_), ity(ity_)
        {
            if (skip == 1)
                skip_forward();
        }
        
        //x {
        
        Iterator2D() : cont(0) { }
        
        Iterator2D(const Iterator2D<ContainerType, ElementType, IterXType, IterYType> &) = default;
            
        Iterator2D &operator=(const Iterator2D<ContainerType, ElementType, IterXType,
            IterYType> &) = default;
        
        //x }
        
        Iterator2D operator++()
        {
            ++ity;
            skip_forward();
            return *this;
        }
        
        Iterator2D operator++(int)
        {
            Iterator2D it(*this);
            ++(*this);
            return it;
        }
        
        bool operator==(const Iterator2D &other) const
        {
            return cont == other.cont && itx == other.itx && ity == other.ity;
        }
        
        bool operator!=(const Iterator2D &other) const
        {
            return !(*this == other);
        }
        
        ElementType &operator*()
        {
            return *ity;
        }
        
        ElementType *operator->()
        {
            return ity.operator->();
        }
        
    private:
        bool skip_forward()
        {
            while (ity == itx->end()) {
                ++itx;
                if (itx == cont->end())
                    return true;
                ity = itx->begin();
            }
            return false;
        }
        
        const ContainerType *cont;
        IterXType itx;
        IterYType ity;
    };
    
    typedef std::pair<KeyType, ValueType> PairType;
    typedef std::pair<const KeyType, ValueType> PairConstKeyType;
    typedef PairConstKeyType PairTypeContainer;
    typedef std::vector<std::list<PairTypeContainer> > ContainerType;
    
public:
    //1
    
    HashMap(const Hash &hash = Hash())
        : tsize{DEF_TABLE_SIZE, DEF_TABLE_SIZE}, t{ContainerType(tsize[0]),
            ContainerType(tsize[1])}, ti(0), asize{0, 0}, hasher(hash), idx(cnt++)
    {
    }
    
    //2
    
    template<class InputIt>
    HashMap(InputIt first, InputIt last, const Hash &hash = Hash())
        : tsize{DEF_TABLE_SIZE, DEF_TABLE_SIZE}, t{ContainerType(tsize[0]),
            ContainerType(tsize[1])}, ti(0), asize{0, 0}, hasher(hash), idx(cnt++)
    {
        for ( ; first != last; ++first)
            insert(*first);
    }
    
    //3
    
    HashMap(std::initializer_list<PairType> init, const Hash &hash = Hash())
        : tsize{DEF_TABLE_SIZE, DEF_TABLE_SIZE}, t{ContainerType(tsize[0]),
            ContainerType(tsize[1])}, ti(0), asize{0, 0}, hasher(hash), idx(cnt++)
    {
        for (auto it = init.begin(); it != init.end(); ++it)
            insert(*it);
    }
    
    //x
    
    HashMap(const HashMap &other)
        : tsize{other.tsize[0], other.tsize[1]}, t{other.t[0], other.t[1]}, ti(other.ti),
            asize{other.asize[0], other.asize[1]}, hasher(other.hasher), idx(cnt++)
    {
    }
    
    HashMap &operator=(const HashMap &other)
    {
        if (idx == other.idx)
            return *this;
        hasher = other.hasher;
        /*if (t[ti] == other.t[ti])
            return *this;*/
        clear();
        tsize[ti] = other.tsize[other.ti];
        t[ti].resize(tsize[ti]);
        for (auto it = other.begin(); it != other.end(); ++it)
            insert(*it);
        return *this;
    }
    
    //4
    
    // hash in arguments
    
    //5
    
    std::size_t size() const
    {
        return asize[ti];
    }
    
    std::size_t empty() const
    {
        return asize[ti] == 0;
    }
    
    //6
    
    const Hash &hash_function() const
    {
        return hasher;
    }
    
    //7
    
    void insert(const PairType &p)
    {
        std::size_t hashval = hasher_mod(p.first);
        for (typename ContainerType::value_type::const_iterator it = t[ti][hashval].begin();
            it != t[ti][hashval].end(); ++it)
            if (it->first == p.first)
                return;
        t[ti][hashval].push_back(p);
        update_changing(1);
    }
    
    //8
    
    void erase(const KeyType &k)
    {
        std::size_t hashval = hasher_mod(k);
        bool found = false;
        for (typename ContainerType::value_type::const_iterator it = t[ti][hashval].begin();
            it != t[ti][hashval].end(); ++it)
            if (it->first == k) {
                t[ti][hashval].erase(it);
                found = true;
                break;
            }
        if (found)
            update_changing(-1);
    }
    
    //9
    //iterators
    
    using iterator = Iterator2D<ContainerType, PairTypeContainer, typename ContainerType::iterator,
        typename ContainerType::value_type::iterator>;
    using const_iterator = Iterator2D<ContainerType, const PairTypeContainer,
        typename ContainerType::const_iterator, typename ContainerType::value_type::const_iterator>;
    
    iterator begin()
    {
        return iterator(&t[ti], t[ti].begin(), t[ti].begin()->begin(), 1);
    }
    
    const_iterator begin() const
    {
        return const_iterator(&t[ti], t[ti].begin(), t[ti].begin()->begin(), 1);
    }
    
    iterator end()
    {
        return iterator(&t[ti], t[ti].end(), (--t[ti].end())->end());
    }
    
    const_iterator end() const
    {
        return const_iterator(&t[ti], t[ti].end(), (--t[ti].end())->end());
    }
    
    //10
    
    const_iterator find(const KeyType &k) const
    {
        std::size_t hashval = hasher_mod(k);
        for (typename ContainerType::value_type::const_iterator it = t[ti][hashval].begin();
            it != t[ti][hashval].end(); ++it)
            if (it->first == k)
                return const_iterator(&t[ti], t[ti].begin() + hashval, it);
        return end();
    }
    
    iterator find(const KeyType &k)
    {
        std::size_t hashval = hasher_mod(k);
        for (typename ContainerType::value_type::iterator it = t[ti][hashval].begin();
            it != t[ti][hashval].end(); ++it)
            if (it->first == k)
                return iterator(&t[ti], t[ti].begin() + hashval, it);
        return end();
    }
    
    //11
    
    ValueType &operator[](const KeyType &k)
    {
        std::size_t hashval = hasher_mod(k);
        for (typename ContainerType::value_type::iterator it = t[ti][hashval].begin();
            it != t[ti][hashval].end(); ++it)
            if (it->first == k)
                return it->second;
        t[ti][hashval].push_back(PairTypeContainer{k, ValueType()});
        update_changing(1);
        {
            std::size_t hashval = hasher_mod(k);
            for (typename ContainerType::value_type::iterator it = t[ti][hashval].begin();
                it != t[ti][hashval].end(); ++it)
                if (it->first == k)
                    return it->second;
        }
        return t[ti].begin()->begin()->second; // to avoid warning
    }
    
    //12
    
    const ValueType &at(const KeyType &k) const
    {
        std::size_t hashval = hasher_mod(k);
        for (typename ContainerType::value_type::const_iterator it = t[ti][hashval].begin();
            it != t[ti][hashval].end(); ++it)
            if (it->first == k)
                return it->second;
        throw std::out_of_range("");
    }
    
    //13
    
    void clear()
    {
        t[ti].clear();
        tsize[ti] = DEF_TABLE_SIZE;
        t[ti].resize(tsize[ti]);
        asize[ti] = 0;
    }
    
private:
    
    std::size_t hasher_mod(const KeyType &k) const
    {
        return hasher(k) % tsize[ti];
    }
    
    void rehash(int dir)
    {
        tsize[ti ^ 1] = tsize[ti];
        if (dir == 1)
            tsize[ti ^ 1] *= TABLE_SIZE_FACTOR;
        else if (dir == -1)
            tsize[ti ^ 1] /= TABLE_SIZE_FACTOR;
        t[ti ^ 1].resize(tsize[ti ^ 1]);
        for (auto it = begin(); it != end(); ++it) {
            ti ^= 1;
            insert(*it);
            ti ^= 1;
        }
        clear();
        ti ^= 1;
    }
    
    void update_changing(int dir)
    {
        asize[ti] += dir;
        if (dir == 1) {
            if (asize[ti] >= tsize[ti] * FILLING_FACTOR)
                rehash(dir);
        } else if (dir == -1) {
            if (asize[ti] > 0 && asize[ti] <= tsize[ti] * FILLING_FACTOR_BACK)
                rehash(dir);
        }
    }
    
    static const std::size_t DEF_TABLE_SIZE = 1;
    static constexpr const long double FILLING_FACTOR = (long double)4 / 3;
    static const std::size_t TABLE_SIZE_FACTOR = 2;
    static constexpr const long double FILLING_FACTOR_BACK = (long double)1 / 3;
    
    std::size_t tsize[2];
    ContainerType t[2];
    std::size_t ti;
    
    std::size_t asize[2];
    
    Hash hasher;
    
    const std::size_t idx;
    
    static std::size_t cnt;
};

template<class KeyType, class ValueType, class Hash>
std::size_t HashMap<KeyType, ValueType, Hash>::cnt = 0;
