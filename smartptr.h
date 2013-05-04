#include "macros.h"
namespace mnb{
template<class T, class Dp>
class unique_ptr{
  public:
    unique_ptr(): impl_(nullptr){}
    explicit unique_ptr(T* p, Dp* d): impl_(p, d){}

    T& operator*(){
      return *(impl_.get() );
    }
    T* operator->(){
      return impl_.get();
    }
    bool operator==(T* p){
      return impl_.get() == p;
    }
    bool operator!=(T* p){
      return impl_.get() != p;
    }
    T* release(){
      return impl_.release();
    }
    void reset(T* p = nullptr){
      impl_.reset(p);
    }
    void swap(unique_ptr& u){
      impl_.swap(u);
    }
    Dp& get_deleter(){
      return impl_.get_deleter();
    }
    const Dp& get_deleter() const{
      return impl_.get_deleter();
    }
  private:
    unique_ptr_impl<T, Dp> impl_;
    DISALLOW_COPY_AND_ASSIGN(unique_ptr);
};
//template<class T, class D>
//void swap(unique_ptr<T,D>& p1, unique_ptr<T,D>& p2){
//  p1.swap(p2);
//}
template<class T, class Dp>
class unique_ptr<T[], Dp>{
  public:
    unique_ptr(): impl_(nullptr){}
    explicit unique_ptr(T* array): impl_(array){}
  
    void reset(T* array){
      impl_.reset(array);
    }
    T* release(){
      return impl_.release();
    }
    T& operator[](std::size_t i) const{
      return impl_.get()[i];
    }
    bool operator==(T* array){
      return impl_.get() == array;
    }
    bool operator!=(T* array){
      return impl_.get() != array;
    }
    T* get() const { 
      return impl_.get();
    }
    Dp& get_deleter(){
      return impl_.get_deleter();
    }
    const Dp& get_deleter() const{
      return impl_.get_deleter();
    }
    void swap(unique_ptr& rhs){
      impl_.swap(rhs);
    }
  private:
    unique_ptr_impl<T,Dp> impl_;
    DISALLOW_COPY_AND_ASSIGN(unique_ptr);
}
template<class T, class Dp>
class unique_ptr_impl{
  public:
    explicit unique_ptr_impl(T* p): data_(p){}
    unique_ptr_impl(T* p, Dp& d): data_(p, d){}
    template<class U, class V>
    unique_ptr_impl(unique_ptr_impl<U, V>* rhs)
      :data_(rhs->release(), rhs->get_deleter() ){}
    ~unique_ptr_impl(){
      if (data_.ptr_) {
        static_cast<Dp&>(data_)(data_.ptr_);
      }
    }
    void reset(T* p){
      if (p != data_.ptr_) {
        T* tmp = data_.ptr_;
        data_.ptr_ = p;
        if(tmp)
          static_cast<Dp&>(data_)(tmp);
      }
    }
    T* get(){
      return data_.ptr_;
    }
    T* release(){
      T* tmp = data_.ptr_;
      data_.ptr_ = nullptr;
      return tmp;
    }
    Dp& get_deleter(){ return data_;}
    const Dp& get_deleter() const { return data_;}
    void swap(unique__ptr_impl& p2) {
      // Standard swap idiom: 'using std::swap' ensures that std::swap is
      // present in the overload set, but we call swap unqualified so that
      // any more-specific overloads can be used, if available.
      using std::swap;
      swap(static_cast<Dp&>(data_), static_cast<Dp&>(p2.data_));
      swap(data_.ptr, p2.data_.ptr);
    }
  private:
    struct D : public Dp{
      explicit D(T* ptr) : ptr_(ptr){}
      D(T* ptr, const Dp& other): Dp(other), ptr_(ptr){}
      T* ptr_;
    }
    D data_;
    DISALLOW_COPY_AND_ASSIGN(unique_ptr_impl);
}
