#include "hashmap.h"
namespace mnb{
template<class T>
  T* addressof(T& p){
    return (T*)&(char&)p;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
HashMap<Key, T, Hasher, EqualKey, Alloc>::HashMap(size_type n=default_bucket_num,
                const Hasher& hf=Hasher(),
                const EqualKey& eq=EqualKey())
    :bucket_list_(nullptr)
    ,bucket_count_(0)
    ,size_(0)
    ,mlf_(1.0f)
    ,hash_function_pred_(hf, eq){
    rehash(n);
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
HashMap<Key, T, Hasher, EqualKey, Alloc>::HashMap(const HashMap& rhs)
  :bucket_list_(nullptr)
  ,bucket_count_(rhs.bucket_count() )
  ,size_(rhs.size() )
  ,mlf_(rhs.max_load_factor() )
  ,hash_function_pred_(rhs.hash_function(), rhs.key_equal() ){
    rehash(rhs.bucket_count() );
    insert(rhs.begin(), rhs.end() );
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
HashMap<Key, T, Hasher, EqualKey, Alloc>::~HashMap(){
    deleteNodes();
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
bool HashMap<Key, T, Hasher, EqualKey, Alloc>::
  isEmpty(){
    return size_ == 0;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
size_type HashMap<Key, T, Hasher, EqualKey, Alloc>::
  size(){
    return size_;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
size_type HashMap<Key, T, Hasher, EqualKey, Alloc>::
  capacity(){
    return max_load_count() - 1;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
T& HashMap<Key, T, Hasher, EqualKey, Alloc>::
  at(const Key& k) {
    iterator i = find(k);
    //if (i != end() ) {
    return i->second;
    //}
    //return end();
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
const T& HashMap<Key, T, Hasher, EqualKey, Alloc>::
  at(const Key& k) const{
    const_iterator ci = find(k);
    return ci->second;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
T& HashMap<Key, T, Hasher, EqualKey, Alloc>::operator[](const Key& k){
    iterator i = find(k);
    if(i != end() )
        return i->second;
    i = addNode(ValueType(k) );
    return i->second;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
size_type HashMap<Key, T, Hasher, EqualKey, Alloc>::
  count(const Key& key) const{
    return find(key) != end() ? 1 : 0;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
std::pair<iterator, iterator> HashMap<Key, T, Hasher, EqualKey, Alloc>::
  equalRange(const Key& key){
    iterator i = find(key);
    iterator j = i;
    if(j != end() ) ++j;
    return std::pair<iterator, iterator>(i, j);
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
std::pair<const_iterator, const_iterator> HashMap<Key, T, Hasher, EqualKey, Alloc>::
  equalRange(const Key& key){
    const_iterator ci = find(key);
    const_iterator cj = ci;
    if(cj != end() ) ++cj;
    return std::pair<const_iterator, const_iterator>(ci, cj);
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
iterator HashMap<Key, T, Hasher, EqualKey, Alloc>::
  find(const Key& key) const {
    return iterator(findNode(key) );
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
const_iterator HashMap<Key, T, Hasher, EqualKey, Alloc>::
  find(const Key& key) const {
    return const_iterator(findNode(key) );
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
NodePointer HashMap<Key, T, Hasher, EqualKey, Alloc>::
  findNode(const Key& k) const{
    if (bucket_count_ == 0) {
      return nullptr;
    }
    std::size_t h = hash_function()(k);
    size_type curr_index = bucketIndex(h, bucket_count_);
    NodePointer np = bucket_list_[curr_index];
    while(np != nullptr) {
      np = np->next_;
      if (bucketIndex(np->hash_, bucket_count_) == curr_index
          && key_equal()(np->value_.first, k) ) {
        break;
      }
    }
    return np;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
iterator HashMap<Key, T, Hasher, EqualKey, Alloc>::
  erase(const_iterator pos){
    iterator next = iterator(pos.node_);
    ++next;
    removeNode(pos.node_);
    return next;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
size_type HashMap<Key, T, Hasher, EqualKey, Alloc>::
  erase(const Key& key){
    const_iterator ci = find(key);
    if (ci != end() ) {
      return 0
    }
    erase(ci);
    return 1;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
iterator HashMap<Key, T, Hasher, EqualKey, Alloc>::
  erase(const_iterator first, const_iterator last){
    for (const_iterator i = first; i != last; i = first) {
      ++first;
      erase(i);
    }
    return iterator(last.node_);
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
void HashMap<Key, T, Hasher, EqualKey, Alloc>::
  clear(){
    deleteNodes();
    bucket_start_node_.next_ = nullptr;
    for (int i = 0; i < bucket_count_; ++i) {
      bucket_list_[i] = nullptr;
    }
    size_ = 0;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
void HashMap<Key, T, Hasher, EqualKey, Alloc>::
  deleteNodes(){
    //NodePointerAlloc& npa = NodePointerAlloc();
    //NodePointer np = bucket_start_node_->next_;
    NodePointer next;
    //while(np != nullptr){
    //  next = np->next_;
    //  NodePtrAllocTraits::destroy(npa, &(np->value_) );
    //  NodePtrAllocTraits::deallocate(nap, np, 1);
    //  np = next;
    //}
    NodePointer np = bucket_start_node_.next_;
    while(np != nullptr){
      next = np->next_;
      delete np;
      np = next;
    }
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
void HashMap<Key, T, Hasher, EqualKey, Alloc>::
  removeNode(NodePointer prem){
    size_type index = bucketIndex(prem->hash_, bucket_count_);
    NodePointer pn = bucket_list_[index];
    assert(pn);
    for (; pn->next_ != prem; pn = pn->next_)
      ;
    if (pn == static_cast<NodePointer>(&bucket_start_node_)
        || bucketIndex(pn->hash_, bucket_count_) != index){
      if (prem->next_ == nullptr
          || bucketIndex(prem->next_->hash_, bucket_count_) != index)
        bucket_list_[index] = nullptr;
    }
    if (prem->next_ != nullptr) {
      size_type after_index = bucketIndex(prem->next_->hash_, bucket_count_);
      if (after_index != index)
        bucket_list_[after_index] = pn;
    }
    pn->next_ = prem->next_;
    --size_;
    prem->next_ = nullptr;
    delete prem;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
std::pair<iterator, bool> HashMap<Key, T, Hasher, EqualKey, Alloc>::
  insert(const Key& key, const T& mapper){
    return insert(ValueType(key, mapper) );
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
std::pair<iterator, bool> HashMap<Key, T, Hasher, EqualKey, Alloc>::
  insert(const ValueType& obj){
    iterator i = find(k);
    if (i != end() )
      return std::pair<iterator, bool>(i, false);
    i = addNode(obj);
    return std::pair<iterator, bool>(i, true);
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
void HashMap<Key, T, Hasher, EqualKey, Alloc>::
  insert(iterator first, iterator last){
    for(; first != last; ++first)
      insert(*first);
  }    
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
iterator HashMap<Key, T, Hasher, EqualKey, Alloc>::
  addNode(const ValueType& v){
    NodePointer np = new Node();
    np->hash_ = hash_function()(v.first);
    np->value_ = v;
    if (size_ + 1 > max_load_) {
      rehash(size_+1);
    }
    size_type index = bucketIndex(np->hash_, bucket_count_);
    if (bucket_list_[index] == nullptr) {
      NodePointer pStart = static_cast<NodePointer>(&bucket_start_node_);
      np->next_ = pStart->next_;
      pStart->next_ = np;
      bucket_list_[index] = pStart;
      if (np->next_ != nullptr) {
        bucket_list_[bucketIndex(np->next_->hash_, bucket_count_)]
          = np->next_;
      }
    }
    else{
      np->next_ = bucket_list_[index]->next_;
      bucket_list_[index]->next_ = np;
    }
    ++size_;
    return iterator(np);
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
iterator HashMap<Key, T, Hasher, EqualKey, Alloc>::
  begin(){
    return iterator(bucket_start_node_.next_);
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
iterator HashMap<Key, T, Hasher, EqualKey, Alloc>::
  end(){
    return iterator(nullptr);
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
const_iterator HashMap<Key, T, Hasher, EqualKey, Alloc>::
  begin(){
    return const_iterator(bucket_start_node_.next_);
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
const_iterator HashMap<Key, T, Hasher, EqualKey, Alloc>::
  end(){
    return const_iterator(nullptr);
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
Hasher HashMap<Key, T, Hasher, EqualKey, Alloc>::
  hash_function() const {
    hash_function_pred_.first;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
EqualKey HashMap<Key, T, Hasher, EqualKey, Alloc>::
  key_equal() const {
    hash_function_pred_.second;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
size_type HashMap<Key, T, Hasher, EqualKey, Alloc>::
  bucket_count() const {
    return bucket_count_;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
size_type HashMap<Key, T, Hasher, EqualKey, Alloc>::
  max_bucket_count() const {
    return NodePtrAllocTraits::max_size();
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
size_type HashMap<Key, T, Hasher, EqualKey, Alloc>::
  bucket_size(size_type n) const{
    NodePointer np = bucket_list_[n];
    size_type rsize = 0;
    while(np != nullptr){
      np = np->next_;
      if (bucketIndex(np->hash_, bucket_count_) )
        ++rsize;
    }
    return rsize;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
size_type HashMap<Key, T, Hasher, EqualKey, Alloc>::
  bucket(const Key& k) const{
    std::size_t h = hash_function()(k);
    return bucketIndex(h, bucket_count_);
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
float HashMap<Key, T, Hasher, EqualKey, Alloc>::
  load_factor() const{
    return bucket_count_ ? (float)size_/bucket_count_ : 0.f;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
float HashMap<Key, T, Hasher, EqualKey, Alloc>::
  max_load_factor() const{
    return mlf_;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
void HashMap<Key, T, Hasher, EqualKey, Alloc>::
  max_load_factor(float z){
    mlf_ = std::max(z, load_factor() );
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
void HashMap<Key, T, Hasher, EqualKey, Alloc>::
  rehash(size_type n){
    if (n == 1) {
        n <<= 5;
    }
    else if(n&(n-1)){
        n = bucket_policy::types::nextPow2(n);
    } 
    if (bucket_count_ < n) {
        rehashImpl(n);
    }
    else if (bucket_count_ > n) {
        n = std::max<size_type>(n, 
               bucket_policy::types::nextPow2(size_t(ceil(float(size_) / mlf_))) );
        rehashImpl(n);
    }
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
void HashMap<Key, T, Hasher, EqualKey, Alloc>::
  rehashImpl(size_type n){
    assert(n != bucket_count_);
    if (createBucket(n) ){
        refillBucket();
    }
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
bool HashMap<Key, T, Hasher, EqualKey, Alloc>::
  createBucket(size_type n){
    NodePointerAlloc& npa = bucket_list_.Deleter().Alloc();
    bucket_list_.reset(n > 0 ?
            NodePtrAllocTraits::allocate(npa, n) :nullptr);
    if (bucket_list_ == nullptr) {
        return false;
    }
    for (size_type i = 0; i < n;++i) {
      bucket_list_[i] = nullptr;
    }
    bucket_count_ = n;
    recalculate_max_load();
    return true;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
void HashMap<Key, T, Hasher, EqualKey, Alloc>::
  recalculate_max_load(){
    // Only resize when size >= mlf_ * count
    max_load_ = buckets_ ? max_load_count() : 0;
}
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
std::size_t HashMap<Key, T, Hasher, EqualKey, Alloc>::
  max_load_count(){
    double db = std::ceil(static_cast<double>(mlf_)
        *static_cast<double>(bucket_count_) );
    double dbmax = static_cast<double>(std::numeric_limits<std::size_t>::max);
    return db >= max ? std::numeric_limits<std::size_t>::max
                : static_cast<std::size_t>db;
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
void HashMap<Key, T, Hasher, EqualKey, Alloc>::
  refillBucket(){
     NodePointer pprev = static_cast<NodePointer>(&bucket_start_node_); 
     NodePointer pcurr = pprev->next_;
     if (pcurr != nullptr) {
         size_type curr_index = bucketIndex(pcurr->hash_);
         bucket_list_[curr_index] = pcurr;
         //size_type prev_index = curr_index;
         for (pprev = pcurr, pcurr = pcurr->next_; pcurr != nullptr; pcurr = pprev->next_) {
             curr_index = bucketIndex(pcurr->hash_);
             //if (curr_index == prev_index) {
             //    pprev = pcurr;
             //    continue;
             //}
             if (bucket_list_[curr_index] == nullptr) {
                 bucket_list_[curr_index] = pprev;
                 pprev = pcurr;
                 //prev_index = curr_index;
             }
             else{
                 NodePointer pp = pcurr;
                 NodePointer np = pcurr->next_;
                 Key curkey = pcurr->value_.first;
                 Key nkey = np->value_.first;
                 for(; np != nullptr && key_equal()(curkey, nkey);
                   np = np->next_,nkey = np->value_.first,pp = pp->next_)
                   ;
                 pprev->next_ = np;
                 pp->next_ = bucket_list_[curr_index]->next_;
                 bucket_list_[curr_index]->next_ = pcurr;
            }
        }
     }
  }
template<class Key, class T, class Hasher, class EqualKey, class Alloc>
size_type HashMap<Key, T, Hasher, EqualKey, Alloc>::
  bucketIndex(size_type hash_value){
      return bucket_policy::types::bucketIndex(bucket_count_, hash_value);
  }
template<class SizeT>
sturct int64_policy{
  static inline SizeT bucketIndex(SizeT bucket_count, SizeT hash) {
         return hash & (bucket_count - 1);
     }

   static inline SizeT nextPow2(SizeT min) {
       if (min <= 4) return 4;
       --min;
       min |= min >> 1;
       min |= min >> 2;
       min |= min >> 4;
       min |= min >> 8;
       min |= min >> 16;
       min |= min >> 32;
       return min + 1;
   }

   static inline SizeT prev_bucket_count(SizeT max) {
       max |= max >> 1;
       max |= max >> 2;
       max |= max >> 4;
       max |= max >> 8;
       max |= max >> 16;
       max |= max >> 32;
       return (max >> 1) + 1;
   }
}
template<class SizeT>
sturct int32_policy{
  static inline SizeT bucketIndex(SizeT bucket_count, SizeT hash) {
         return hash & (bucket_count - 1);
     }

  static inline SizeT nextPow2(SizeT min) {
      if (min <= 4) return 4;
      --min;
      min |= min >> 1;
      min |= min >> 2;
      min |= min >> 4;
      min |= min >> 8;
      min |= min >> 16;
      return min + 1;
  }

  static inline SizeT prev_bucket_count(SizeT max) {
      max |= max >> 1;
      max |= max >> 2;
      max |= max >> 4;
      max |= max >> 8;
      max |= max >> 16;
      return (max >> 1) + 1;
  }
}
struct bucket_policy{
#ifdef _INT64
    typedef int64_policy<std::size_t> types;
#else
    typedef int32_policy<std::size_t> types;
#endif
    static bool isPow2(size_type x){
        return x > 2 && (x & (x-1) );
    }
}
// Precondition:  __x != 0                                                                                        
inline _LIBCPP_INLINE_VISIBILITY                                                                                  
unsigned                                                                                                          
__clz(unsigned __x)                                                                                               
{                                                                                                                 
    return static_cast<unsigned>(__builtin_clz(__x));                                                             
}                                                                                                                 
                                                                                                                  
inline _LIBCPP_INLINE_VISIBILITY                                                                                  
unsigned long                                                                                                     
__clz(unsigned long __x)                                                                                          
{                                                                                                                 
    return static_cast<unsigned long>(__builtin_clzl (__x));                                                      
}                                                                                                                 
                                                                                                                  
inline _LIBCPP_INLINE_VISIBILITY                                                                                  
unsigned long long                                                                                                
__clz(unsigned long long __x)                                                                                     
{                                                                                                                 
    return static_cast<unsigned long long>(__builtin_clzll(__x));                                                 
}               
#ifdef  MSVC
_LIBCPP_ALWAYS_INLINE int __builtin_clz( unsigned int x )
{
   DWORD r = 0;
   _BitScanForward(&r, x);
   return static_cast<int>(r);
}
// sizeof(long) == sizeof(int) on Windows
_LIBCPP_ALWAYS_INLINE int __builtin_clzl( unsigned long x )
{ return __builtin_clz( static_cast<int>(x) ); }
_LIBCPP_ALWAYS_INLINE int __builtin_clzll( unsigned long long x )
{
    DWORD r = 0;
    _BitScanForward64(&r, x);
    return static_cast<int>(r);
}

}
