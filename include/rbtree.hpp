// MIT License
// Copyright 2026 CleverElsie

#ifndef ELSIE_RBTREE
#define ELSIE_RBTREE
#include<any> // swap()
#include<new>
#include<cstdint>
#include<cstddef> // size_t
#include<utility> // forward(), move()
#include<compare>
#include<concepts>
#include<iterator>
#include<functional> // less<>
#include<type_traits>
#include<initializer_list>

namespace elsie{

template<class key_t, class val_t, bool is_multi, class Cmp, class Allocator>
class rbtree{
  constexpr static bool is_set=std::same_as<val_t, std::nullptr_t>;
  public:
  using key_type = key_t;
  using value_type = std::conditional_t<is_set, const key_t, std::pair<const key_t, val_t>>;
  using mapped_type = val_t;
  using key_compare = Cmp;
  using allocator_type = Allocator;
  using reference = value_type&;
  using const_reference = const value_type&;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type*;
  using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
  protected:
  using mutable_value_type = std::conditional_t<is_set, key_t, std::pair<key_t, val_t>>;
  struct node{
    constexpr static uint64_t filter_red=1;
    constexpr static uint64_t filter_black=~filter_red;
    using ip=node*;
    ip p,ch[2]; // ch0=L, ch1=R
    uint64_t time,size; // time: odd red, even black
    value_type val;
    node()=default;
    node(ip nil):p{nil},ch{nil,nil},time{0},size{1},val(){}

    template<class KEY_T> requires (std::same_as<std::decay_t<KEY_T>,key_t>) && (is_set)
    node(KEY_T&&k,uint64_t tm,ip P,ip L,ip R):p{P},ch{L,R},time{tm},size{1},val(std::forward<KEY_T>(k)){}

    template<class KEY_T,class VAL_T>
    requires (std::same_as<std::decay_t<KEY_T>,key_t>) && (std::same_as<std::decay_t<VAL_T>,val_t>)
    node(KEY_T&&k,VAL_T&&v,uint64_t tm,ip P,ip L,ip R)
    :p{P},ch{L,R},time{tm},size{1},val(std::forward<KEY_T>(k),std::forward<VAL_T>(v)){}

    ~node()=default;
    
    const key_t& key()const{
      if constexpr(is_set) return val;
      else return val.first;
    }
    val_t& value() requires(!is_set) { return val.second; }
    uint64_t update(){ return size=1+ch[0]->size+ch[1]->size; }
    bool is_black()const noexcept{ return (time&1)==0; }
    bool is_red()const noexcept{ return time&1; }
    void set_black()noexcept{ time&=filter_black; }
    void set_red()noexcept{ time|=filter_red; }
    uint64_t get_time()const noexcept{ return time&filter_black; }
  };
  using np=node*;
  template<bool is_const, bool is_reverse>
  struct iter{
    private:
    public:
    using difference_type = std::ptrdiff_t;
    using value_type = rbtree::value_type;
    using pointer = rbtree::pointer;
    using reference = rbtree::reference;
    using iterator_category = std::bidirectional_iterator_tag;
    iter()=default;
    iter(iter&&)=default;
    iter(const iter&)=default;
    iter(rbtree const* tree, node* ptr)requires (!is_const):tree(tree), ptr(ptr) {}
    iter(rbtree const* tree, node const* ptr)requires (is_const):tree(tree), ptr(ptr) {}
    iter& operator=(iter&&)=default;
    iter& operator=(const iter&)=default;
    
    iter& operator++(){
      ptr=tree->template prev_next<!is_reverse>(ptr);
      return *this;
    }
    iter& operator--(){
      ptr=tree->template prev_next<is_reverse>(ptr);
      return *this;
    }
    iter operator++(int32_t){
      iter cur=*this;
      ++*this;
      return cur;
    }
    iter operator--(int32_t){
      iter cur=*this;
      --*this;
      return cur;
    }
    auto& operator*(){ return ptr->val; }
    auto operator->(){ return &(ptr->val); }
    auto& operator*()const{ return ptr->val; }
    auto operator->()const{ return &(ptr->val); }
    template<bool is_const1, bool is_reverse1>
    bool operator==(const iter<is_const1,is_reverse1>&itr)const{ return ptr==itr.ptr; }
    template<bool is_const1>
    auto operator<=>(const iter<is_const1, is_reverse>&itr)const{
      if(tree!=itr.tree) throw std::domain_error("iter is not belonging to the same tree");
      return tree->order_of(*this)<=>itr.tree->order_of(itr);
    }
    iter& operator+=(difference_type n){
      size_t idx=tree->order_of(*this)+n;
      if constexpr(is_reverse) idx=tree->cur_size-idx-1;
      auto x=tree->find_by_order(idx);
      ptr=x.ptr;
      return*this;
    }
    iter& operator-=(difference_type n){
      size_t idx=tree->order_of(*this)-n;
      if constexpr(is_reverse) idx=tree->cur_size-idx-1;
      auto x=tree->find_by_order(idx);
      ptr=x.ptr;
      return*this;
    }
    iter operator+(difference_type n)const{
      return iter(tree,ptr)+=n;
    }
    iter operator-(difference_type n)const{
      return iter(tree,ptr)-=n;
    }
    template<bool is_const1>
    difference_type operator-(const iter<is_const1,is_reverse>&itr)const{
      if(tree!=itr.tree) throw std::domain_error("iter is not belonging to the same tree");
      return tree->order_of(*this)-tree->order_of(itr);
    }
    auto& operator[](difference_type n){ return *(*this+n); }
    auto& operator[](difference_type n)const{ return *(*this+n); }
    template<bool is_const1,bool is_reverse1>
    explicit operator iter<is_const1, is_reverse1>()const{
      static_assert(is_const && !is_const1, "elsie::rbtree::iterator invalid conversion, const -> non-const.");
      return iter<is_const1, is_reverse1>(tree,ptr);
    }
    private:
    rbtree const* tree;
    std::conditional_t<is_const,node const*, node*> ptr;
    friend class rbtree;
    void is_belonging_to(const rbtree*tree)const{
      #ifdef DEBUG
      if(tree!=this->tree)
        throw std::invalid_argument("iter is not belonging to the same tree");
      #endif
    }
  };
  template<bool is_const, bool is_reverse>
  friend struct iter;
  public:
  using iterator=iter<false,false>;
  using reverse_iterator=iter<false,true>;
  using const_iterator=iter<true,false>;
  using const_reverse_iterator=iter<true,true>;

  protected:
  using node_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<node>;
  using node_traits = typename std::allocator_traits<node_allocator>;
  constexpr static uint64_t unit_time=2;
  size_t cur_size,time;
  np root,nil;
  [[no_unique_address]] Cmp cmp;
  [[no_unique_address]] node_allocator alloc;
  public:
  rbtree():nil{nullptr}{ clear(); }
  rbtree(rbtree&&v):nil(nullptr){ *this=std::move(v); }
  rbtree(const rbtree&v):nil(nullptr){ *this=v; }
  template<class T>
  rbtree(std::initializer_list<T>init)
    requires (std::same_as<T,key_t>)||(std::same_as<T,std::pair<key_t,val_t>>)
    :rbtree(){
    for(const auto&x:init)
      this->insert(x);
  }
  explicit rbtree(const Cmp& Cmp_):nil(nullptr),cmp(Cmp_){ clear(); }
  explicit rbtree(const Allocator& Alloc):nil(nullptr),alloc(Alloc){ clear(); }
  explicit rbtree(const Cmp& Cmp_, const Allocator& Alloc):nil(nullptr),cmp(Cmp_),alloc(Alloc){ clear(); }

  ~rbtree(){
    clear();
    node_traits::destroy(alloc,nil);
    node_traits::deallocate(alloc,nil,1);
    nil=nullptr;
  }
  rbtree&operator=(rbtree&&rhs){
    if(nil) this->~rbtree();
    cur_size=rhs.cur_size, time=rhs.time;
    root=rhs.root, nil=rhs.nil;
    cmp=std::move(rhs.cmp);
    if constexpr(node_traits::propagate_on_container_move_assignment)
      alloc=std::move(rhs.alloc);
    rhs.root=rhs.nil=nullptr;
    rhs.cur_size=rhs.time=0;
    rhs.clear();
    return*this;
  }
  rbtree&operator=(const rbtree&rhs){
    if(nil) this->~rbtree();
    if constexpr(node_traits::propagate_on_container_copy_assignment)
      alloc=rhs.alloc;
    for(auto itr=rhs.begin();itr!=rhs.end();++itr)
      this->insert(*itr);
    return*this;
  }
  protected:
  //! @param to_max: 0 min, 1 max
  template<bool to_max, class NP>
    requires (std::same_as<NP,np>||std::same_as<NP,node const*>)
  NP min_max(NP p)const{
    if(!p||p==nil)return nil;
    for(;p&&p->ch[to_max]!=nil;p=p->ch[to_max]);
    return p;
  }
  //!@param to_next: 0 prev, 1 next
  template<bool to_next, class NP>
  requires (std::same_as<NP,np>||std::same_as<NP,node const*>)
  NP prev_next(NP x)const{
    if(x==nil)return min_max<!to_next>(root);
    if(x->ch[to_next]!=nil)return min_max<!to_next>(x->ch[to_next]);
    while(1){
      if(x==root)return nil;
      node const* y=x;
      x=x->p;
      if(x->ch[!to_next]==y)break;
    }
    return x;
  }
  np find_impl(const key_t&key)const{
    np x=root;
    while(x!=nil){
      if(bool L=cmp(key,x->key());L==cmp(x->key(),key)){
        if constexpr(is_multi){
          uint64_t xt=x->get_time(); // p->time == 0
          if(0==xt)break;
          else x=x->ch[1];
        }else break;
      }else x=x->ch[!L];
    }
    return x;
  }
  public:
  template<class... Args>
  requires(std::constructible_from<key_t, Args...>)
  iterator find(Args&&... args){
    return iterator(this,find_impl(std::forward<Args>(args)...));
  }
  template<class... Args>
  requires(std::constructible_from<key_t, Args...>)
  const_iterator find(Args&&... args)const{
    return const_iterator(this,find_impl(std::forward<Args>(args)...));
  }

  template<class... Args>
  requires (!is_set) && (std::constructible_from<key_t, Args...>)
  val_t& operator[](Args&&... args){
    key_t key(std::forward<Args>(args)...);
    auto it=find(key);
    if(it==end()){
      if constexpr(!std::is_default_constructible_v<val_t>)
        throw std::invalid_argument("elsie::rbtree out of map. val_t is not default constructible");
      it=insert(std::move(key),val_t());
    }
    return it->value();
  }
  
  const key_t& operator[](size_t idx)const requires (is_set){
    return find_by_order(idx).ptr->key();
  }
  
  private:
  np lower_bound_impl(const key_t&key)const{
    np cur=root,res=nil;
    while(cur!=nil){
      bool L=cmp(cur->key(),key);
      if(L==cmp(key,cur->key())){
        if constexpr(is_multi){
          uint64_t ct=cur->get_time();
          if(ct==0)return cur;
          else res=cur,cur=cur->ch[0];
        }else return cur;
      }else if(L)cur=cur->ch[1];
      else res=cur,cur=cur->ch[0];
    }
    return res;
  }
  public:
  template<class... Args>
  requires(std::constructible_from<key_t, Args...>)
  iterator lower_bound(Args&&... args){
    return iterator(this,lower_bound_impl(std::forward<Args>(args)...));
  }
  template<class... Args>
  requires(std::constructible_from<key_t, Args...>)
  const_iterator lower_bound(Args&&... args)const{
    return const_iterator(this,lower_bound_impl(std::forward<Args>(args)...));
  }
  private:
  np upper_bound_impl(const key_t&key)const{
    np cur=root,res=nil;
    while(cur!=nil){
      bool L=cmp(cur->key(),key);
      if(L||L==cmp(key,cur->key())){
        if constexpr(is_multi)
          if(uint64_t ct=cur->get_time();ct>0){
            res=cur,cur=cur->ch[0];
            continue;
          }
        cur=cur->ch[1];
      }else res=cur,cur=cur->ch[0];
    }
    return res;
  }
  public:
  template<class... Args>
  requires(std::constructible_from<key_t, Args...>)
  iterator upper_bound(Args&&... args){
    return iterator(this,upper_bound_impl(std::forward<Args>(args)...));
  }
  template<class... Args>
  requires(std::constructible_from<key_t, Args...>)
  const_iterator upper_bound(Args&&... args)const{
    return const_iterator(this,upper_bound_impl(std::forward<Args>(args)...));
  }
  private:
  np find_by_order_impl(int64_t idx)const{
    if(idx>=(int64_t)cur_size||-idx>(int64_t)cur_size) return nil;
    if(idx<0) idx=cur_size+idx;
    np cur=root;
    while(cur!=nil){
      size_t L=cur->ch[0]->size;
      if(L>idx)cur=cur->ch[0];
      else if(L<idx){
        cur=cur->ch[1];
        idx-=L+1;
      }else return cur;
    }
    return nil;
  }
  public:
  iterator find_by_order(int64_t idx){
    return iterator(this,find_by_order_impl(idx));
  }
  const_iterator find_by_order(int64_t idx)const{
    return const_iterator(this,find_by_order_impl(idx));
  }
  template<class... Args>
  requires(std::constructible_from<key_t, Args...>)
  size_t order_of(Args&&... args)const{
    return order_of(lower_bound(std::forward<Args>(args)...));
  }
  template<bool is_const, bool is_reverse>
  size_t order_of(const iter<is_const, is_reverse>&itr)const{
    node const* p = itr.ptr;
    if(p==nil) return cur_size;
    size_t R=p->ch[0]->size;
    while(p!=root){
      if(p==p->p->ch[1])
        R+=1+p->p->ch[0]->size;
      p=p->p;
    }
    if constexpr(is_reverse)
      return cur_size-R-1;
    else return R;
  }
  template<class... Args>
  requires(std::constructible_from<key_t, Args...>)
  size_t count(Args&&... args)const{
    if constexpr(is_multi){
      key_t tkey(std::forward<Args>(args)...);
      return order_of(upper_bound(tkey))-order_of(tkey);
    }else return contains(std::forward<Args>(args)...);
  }
  template<class... Args>
  requires(std::constructible_from<key_t, Args...>)
  bool contains(Args&&... args)const{ return end()!=find(std::forward<Args>(args)...); }
  protected:
  //! @param left_right: 0 left, 1 right
  void rotation(np x,bool left_right){
    np y=x->ch[!left_right];
    x->ch[!left_right]=y->ch[left_right];
    y->p=x->p;
    if(y->ch[left_right]!=nil)y->ch[left_right]->p=x;
    if(x->p==nil)root=y;
    else x->p->ch[x==x->p->ch[1]]=y;
    y->ch[left_right]=x;
    x->p=y;
    y->size=x->size;
    x->update();
  }
  void rb_insert_fixup(np z){
    while(z->p->is_red()){
      bool LR=z->p==z->p->p->ch[0];
      if(np y=z->p->p->ch[LR];y->is_red()){
        z->p->set_black(),y->set_black();
        z->p->p->set_red(),z=z->p->p;
      }else{
        if(z==z->p->ch[LR])rotation(z=z->p,!LR);
        z->p->set_black(),z->p->p->set_red();
        rotation(z->p->p,LR);
      }
    }
    root->set_black();
  }
  public:
  template<class... Args>
  requires (std::constructible_from<value_type, Args...>)
  iterator insert(Args&&... args){
    mutable_value_type t(std::forward<Args>(args)...);
    const key_t* key;
    if constexpr(is_set) key=&t;
    else key=&t.first;
    np x=root,y=nil;
    while(x!=nil){
      y=x;
      bool L=cmp(*key,x->key());
      if(L==cmp(x->key(),*key))
        if constexpr(!is_multi){
          if constexpr(!is_set)
            x->value()=std::move(t.second);
          return iterator(this,x);
        }else x=x->ch[1];
      else x=x->ch[!L];
    }
    cur_size+=1;
    time+=unit_time;
    np z=node_traits::allocate(alloc,1);
    if constexpr(is_set)
      node_traits::construct(alloc,z,std::move(t), time|node::filter_red,y,nil,nil);
    else node_traits::construct(alloc,z,std::move(t.first),std::move(t.second),time|node::filter_red,y,nil,nil);
    if(y==nil) root=z;
    else y->ch[!cmp(z->key(),y->key())]=z;
    for(np s=y;s!=nil;s=s->p)
      s->size+=1;
    rb_insert_fixup(z);
    return iterator(this,z);
  }
  protected:
  void rb_delete_fixup(np x){
    while(x!=root&&!(x->is_red())){
      bool LR=x==x->p->ch[0];
      np w=x->p->ch[LR];
      if(w->is_red()){
        w->set_black(),x->p->set_red();
        rotation(x->p,!LR);
        w=x->p->ch[LR];
      }
      if(!(w->ch[0]->is_red()||w->ch[1]->is_red()))
        w->set_red(),x=x->p;
      else{
        if(!(w->ch[LR]->is_red())){
          w->ch[!LR]->set_black(),w->set_red();
          rotation(w,LR);
          w=x->p->ch[LR];
        }
        if(x->p->is_red())w->set_red();
        else w->set_black();
        x->p->set_black(),w->ch[LR]->set_black();
        rotation(x->p,!LR);
        x=root;
      }
    }
    x->set_black();
  }
  void erase(np z){
    auto transplant=[&](np u,np v){
      if(u->p==nil)root=v;
      else u->p->ch[u==u->p->ch[1]]=v;
      v->p=u->p;
    };
    np x,y=z,w=z->p;
    bool y_was_red=y->is_red();
    if(z->ch[0]==nil) transplant(z,x=z->ch[1]);
    else if(z->ch[1]==nil) transplant(z,x=z->ch[0]);
    else{
      for(y=z->ch[1];1;y=y->ch[0]){
        y->size-=1;
        if(y->ch[0]==nil)break;
      }
      y_was_red=y->is_red();
      x=y->ch[1];
      if(y!=z->ch[1]){
        transplant(y,y->ch[1]);
        y->ch[1]=z->ch[1],y->ch[1]->p=y;
      }else x->p=y;
      transplant(z,y);
      y->ch[0]=z->ch[0];
      y->ch[0]->p=y;
      if(z->is_red())y->set_red();
      else y->set_black();
      y->update();
    }
    if(w!=nil)for(;w!=nil;w=w->p)
      w->size-=1;
    if(!y_was_red)rb_delete_fixup(x);
    node_traits::destroy(alloc,z);
    node_traits::deallocate(alloc,z,1);
  }
  public:
  // is_constはfalseじゃないと書き換えできないので，false
  template<bool is_reverse1>
  iter<false, is_reverse1> erase(iter<false, is_reverse1>&itr){
    if(itr.ptr!=nil){
      cur_size-=1;
      np nx=prev_next<!is_reverse1>(itr.ptr);
      erase(itr.ptr);
      itr.ptr=nx;
      return itr;
    }
    return iterator(this,nil);
  }
  template<bool is_reverse1>
  iter<false, is_reverse1> erase(iter<false, is_reverse1>&&itr){ return erase(itr); }
  template<class... Args>
  requires(std::constructible_from<key_t, Args...>)
  iterator erase(Args&&... args){
    return erase(find(std::forward<Args>(args)...));
  }
  // memory
  private:
  void clear(np x){
    if(x==nil) return;
    clear(x->ch[0]);
    clear(x->ch[1]);
    node_traits::destroy(alloc,x);
    node_traits::deallocate(alloc,x,1);
  }
  public:
  void clear(){
    if(nil==nullptr){
      nil=node_traits::allocate(alloc,1);
      node_traits::construct(alloc,nil,nullptr);
      nil->p=nil->ch[0]=nil->ch[1]=nil;
      nil->size=0;
    }else clear(root);
    time=cur_size=0;
    root=nil;
  }
  bool empty()const noexcept{return cur_size==0;}
  size_t size()const noexcept{return cur_size;}
  // iterator
  iterator begin(){ return iterator(this,cur_size?min_max<false>(root):nil); }
  iterator end(){ return iterator(this,nil); }
  const_iterator begin()const{ return cbegin();}
  const_iterator end()const{ return cend(); }
  const_iterator cbegin()const{ return const_iterator(this,cur_size?min_max<false>(root):nil); }
  const_iterator cend()const{ return const_iterator(this,nil); }
  reverse_iterator rbegin(){ return reverse_iterator(this,cur_size?min_max<true>(root):nil); }
  reverse_iterator rend(){ return reverse_iterator(this,nil); }
  const_reverse_iterator rbegin()const{ return crbegin(); }
  const_reverse_iterator rend()const{ return crend(); }
  const_reverse_iterator crbegin()const{ return const_reverse_iterator(this,cur_size?min_max<true>(root):nil); }
  const_reverse_iterator crend()const{ return const_reverse_iterator(this,nil); }
};

template<class key_t, class cmp=std::less<key_t>, class Allocator=std::allocator<key_t>>
using set=rbtree<key_t,std::nullptr_t,false,cmp,Allocator>;

template<class key_t, class val_t, class cmp=std::less<key_t>, class Allocator=std::allocator<std::pair<const key_t, val_t>>>
using map=rbtree<key_t,val_t,false,cmp,Allocator>;

template<class key_t, class cmp=std::less<key_t>, class Allocator=std::allocator<key_t>>
using multiset=rbtree<key_t,std::nullptr_t,true,cmp,Allocator>;

template<class key_t,class val_t,class cmp=std::less<key_t>,class Allocator=std::allocator<std::pair<const key_t, val_t>>>
using multimap=rbtree<key_t,val_t,true,cmp,Allocator>;

}

#endif // ELSIE_RBTREE