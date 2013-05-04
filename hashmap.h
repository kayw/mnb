namespace mnb {
namespace hash{
  using std::size_t;
  struct murmur2hash{
    size_t operator()(const void* v, size_t len) const{
      const size_t m = 0x5bd1e995;
      const size_t r = 24;
      size_t h = len;
      const unsigned char* data = static_cast<const unsigned char*>(v);//&((unsigned char&)v);
      for (; len >= 4; data += 4, len-=4) {
        size_t k = *(const size_t*)data;
        k *= m;
        k ^= k >> r;
        k *= m;
        h *= m;
        h ^= k; 
      }
      switch(len){
        case 3:
          h ^= data[2] << 16;
        case 2:
          h ^= data[1] << 8;
        case 1:
          h ^= data[0];
          h *= m;
      }
      h ^= h >> 13;
      h *= m;
      h ^= h >> 15;
      return h;
    }
  };
  template<class T>
  struct hash{
    size_t operator()(T& v) const{
      return murmur2hash()(&v, sizeof(T) );
    }
  };
  template<class T>
  struct hash<T*>{
    size_t operator()(T* v) const{
      union{
        T* t;
        size_t a;
      }u;
      u.t = v;
      return murmur2hash()(&u, sizeof(u) );
    }
  };
  
  template<>
  struct hash<char>{
    size_t operator()(char v) const { return static_cast<size_t>(v); }
  };
  
  template <>
  struct hash<signed char> {
    size_t operator()(signed char v) const {return static_cast<size_t>(v);}
  };
  
  template <>
  struct hash<unsigned char>
  {
      size_t operator()(unsigned char v) const {return static_cast<size_t>(v);}
  };
  template <>
  struct hash<wchar_t> {
    size_t operator()(wchar_t v) const {return static_cast<size_t>(v);}
  };
  
  template <>
  struct hash<short>
  {
    size_t operator()(short v) const {return static_cast<size_t>(v);}
  };
  
  template <>
  struct  hash<unsigned short>
  {
    size_t operator()(unsigned short v) const  {return static_cast<size_t>(v);}
  };
  
  template <>
  struct  hash<int>
  {
    size_t operator()(int v) const  {return static_cast<size_t>(v);}
  };
  
  template <>
  struct  hash<unsigned int>
  {
    size_t operator()(unsigned int v) const  {return static_cast<size_t>(v);}
  };
  
  template <>
  struct  hash<long>
  {
    size_t operator()(long v) const  {return static_cast<size_t>(v);}
  };
  
  template <>
  struct  hash<unsigned long>
  {
    size_t operator()(unsigned long v) const  {return static_cast<size_t>(v);}
  };
}
template<class NodePtr>
class HashIterator{
  public:
    using namespace std;
    typedef forward_iterator_tag                         iterator_category;
    typedef typename pointer_traits<NodePtr>::element_type::value_type value_type;
    typedef typename pointer_traits<NodePtr>::difference_type difference_type;
    typedef typename pointer_traits<NodePtr>::template rebind<value_type> pointer;
    typedef value_type&                                  reference;
    HashIterator(){}
    HashIterator(HashIterator& rhs)
      : node_(rhs.node_){}
    reference operator*() const {return node_->value_;}
    pointer operator->() const { return &(node_->value_);}
    HashIterator& operator++(){
      node_ = node_->next_;
      return *this;
    }
    HashIterator operator(int){
      HashIterator t(*this);
      ++(*this);
      return t;
    }
    bool operator==(const HashIterator& x, const HashIterator& y)
    {return x.node_ == y.node_;}
    bool operator!=(const HashIterator& x, const HashIterator& y)
    {return x.node_ == y.node_;}
  private:
    HashIterator(NodePtr p)
      : node_(p){}
    NodePtr node_;
}
template<class ConstNodePtr, class NodePtr>
class HashConstIterator{
  public:
    typedef forward_iterator_tag iterator_category;
    typedef typename pointer_traits<ConstNodePtr>::element_type::value_type value_type;
    typedef typename pointer_traits<ConstNodePtr>::difference_type difference_type;
    typedef typename pointer_traits<ConstNodePtr>::template rebind<value_type> pointer;
    typedef value_type& reference;
    HashConstIterator(){}
    HashConstIterator(HashIterator<NodePtr>& rhs)
      : node_(rhs.node_){}
    reference operator*() const { return node_->value_;}
    pointer operator->() const { return &(node_->value_);}
    HashConstIterator& operator++(){
      node_ = node_->next_;
      return *this;
    }
    HashConstIterator operator++(int){
      HashConstIterator t(*this);
      ++(*this);
      return t;
    }
    bool operator==(const HashConstIterator& x, const HashConstIterator& y)
    {return x.node_ == y.node_;}
    bool operator!=(const HashIterator& x, const HashIterator& y)
    {return x.node_ != y.node_;}
  private:
    HashConstIterator(NodePtr p)
      : node_(p){}
    NodePtr node_;
}

template<class Alloc>
class BucketDeleter{
  public:
  typedef AllocTraits::size_type size_type;
  typedef std::allocator_traits<Alloc> AllocTraits;
  typedef typename AllocTraits::pointer pointer;
  BucketDeleter()
    :size_(0){}
  size_type& size(){ return size_;}
  size_type size(){ return size_;}
  void operator()(pointer p){
    deallocate(Alloc(), p, size() );
  }
  private:
  size_type size_;
};
template<class NodePtr>
struct BucketNode{
    NodePtr next_;
    explicit BucketNode():next_(nullptr){}
};
template<class T>
struct HashNode : BucketNode<HashNode*>{
  std::size_t hash_;
  T value_;
  _MNB_INLINE HashNode():
    hash_(0){}
};
template<class Key, class T,
    class Hasher=hash::hash<Key>,
    class EqualKey=std::equal_to<Key>,
    class Alloc=std::allocator<std::pair<const Key, T> > >
class HashMap{
    public:
        typedef std::pair<const Key,T> value_type; 
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef std::size_t size_type;

    private:
        typedef std::pair<Key,T> ValueType; 
        typedef std::list<value_type, Alloc> ListImpl;
        typedef std::vector<ListImpl::iterator, Alloc> VecImpl;
        typedef std::allocator_traits<Alloc> AllocTraits;
        typedef typename AllocTraits::rebind_alloc<BucketNode>
            BucketAlloc;
        typedef std::allocator_traits<BucketAlloc> BucketAllocTraits;
        typedef BucketAllocTraits::pointer BucketPointer;
        typedef BucketAllocTraits::const_pointer BucketConstPointer;
        typedef HashNode<value_type> Node;
        typedef typename AllocTraits::rebind_alloc<Node>
            NodeAlloc;
        typedef std::allocator_traits<NodeAlloc> NodeAllocTraits;
        typedef NodeAllocTraits::pointer NodePointer;
        typedef NodeAllocTraits::const_pointer NodeConstPointer;
        typedef typename AllocTraits::rebind_alloc<NodePointer>
            NodePointerAlloc;
        typedef std::allocator_traits<NodePointerAlloc> NodePtrAllocTraits;
        typedef BucketDeleter<NodePointerAlloc> BucketListDeleter;
    public:
        typedef HashIterator<NodePointer> iterator;
        typedef HashConstIterator<NodeConstPointer, NodePointer> const_iterator;

    private:
        mnb::unique_ptr<NodePointer[], BucketListDeleter> bucket_list_;
        //NodePointer* bucket_list_;
        BucketNode bucket_start_node_;
        float mlf_;
        std::size_t bucket_count_;
        std::size_t size_;
        std::size_t max_load_;
        std::pair<Hasher, EqualKey> hash_function_pred_;

    public:
        explicit HashMap(size_type n=default_bucket_num,
                const Hasher& hf=Hasher(),
                const EqualKey& eq=EqualKey());
        _MNB_INLINE HashMap(){}

        //copy ctor
        explicit HashMap(const HashMap&);
        //dtor
        ~HashMap();
        //assign
        HashMap& operator=(HashMap const& right){
        }
        HashMap& operator=(HashMap& right){
        }

        //size and capacity
        bool isEmpty() const;
        size_type size() const;
        size_type capcity() const;

        // emplace/insert
        std::pair<iterator,bool> insert(const Key& key, const T& mapper);
        std::pair<iterator,bool> insert(value_type const& value);
        void insert(iterator first, iterator last);
        //iterator insertMulti(const Key& k, const T& mapper);
        //iterator insertMulti(const value_type& value);

        // accessors
        T& at(const Key& k);
        const T& at(const Key& k) const;
        T& operator[](const Key& k);
        size_type count(const Key& key) const;
        iterator find(const Key& key) const;
        const_iterator find(const Key& key) const;
        std::pair<iterator, iterator> equalRange(const Key& key);
        std::pair<const_iterator, const_iterator> equalRange(const Key& k) const;

        // earse
        iterator erase(const_iterator pos);
        size_type erase(const Key& key);
        iterator erase(const_iterator first, const_iterator last);
        void clear();

        // STL compatible
        // STL iterators
        iterator begin() const;
        iterator end() const;
        const_iterator begin() const;
        const_iterator end() const;
        
        // observers
        Hasher hash_function() const;
        EqualKey key_equal() const;

        // bucket
        size_type bucket_count() const;
        size_type max_bucket_count() const;
        size_type bucket_size(size_type n) const;
        size_type bucket(const Key& k) const;

        // hash policy
        float load_factor() const;
        float max_load_factor() const;
        void max_load_factor(float z);
        void rehash(size_type n);
    };
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
    void swap(HashMap<Key, T, Hasher, EqualKey, Alloc>& x, 
            HashMap<Key, T, Hasher, EqualKey, Alloc>& y);
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
    void operator==(const HashMap<Key, T, Hasher, EqualKey, Alloc>& x, 
            const HashMap<Key, T, Hasher, EqualKey, Alloc>& y);
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
    void operator!=(const HashMap<Key, T, Hasher, EqualKey, Alloc>& x, 
            const HashMap<Key, T, Hasher, EqualKey, Alloc>& y);
}
