#include<cstdio>
#include<cstdint>
#include<cstdlib>
#include<vector>
#include<stack>
struct Runtime;

struct MemCell{
    uint32_t marked: 1, mark_f:31;
    uint8_t buf[0];
};

const size_t bufoffset = offsetof(MemCell, buf);
using reference = uint8_t*;
using Memory = std::vector<MemCell*>;
using MemToVisit = std::stack<MemCell*>;

template<typename T=reference, typename G>
T ptrAdd(G ptr, int64_t off){
    return reinterpret_cast<T>(
        reinterpret_cast<uint8_t*>(ptr) + off
    );
}

typedef void (*TraverseFn)(reference, Memory, MemToVisit&);
const int byte = sizeof(uint8_t);

MemCell* allocate(size_t n){
    auto &&cell = reinterpret_cast<MemCell*>(malloc((n + bufoffset) * byte));
    // printf("allocate: %u\n", cell);
    cell->marked = 0;
    return cell;
}

struct Runtime{
    Memory heap;
    std::vector<TraverseFn> mark_fns;
    Runtime() : heap(), mark_fns() {}
};

void gc(Runtime* rt, MemToVisit& root){
    auto&& heap = rt->heap;
    while (!root.empty()){
      auto&& TOS = root.top();
      root.pop();
      if (!TOS->marked){
          TOS->marked = 1;
          if (TOS->mark_f != 0)
             rt->mark_fns[TOS->mark_f](ptrAdd(TOS, bufoffset), heap, root);
      }
    }
    Memory newheap;

    for(auto &&m: heap){
        if (m->marked){
            newheap.push_back(m);
        }
        else
        {
            // printf("dealloc: %u\n", m);
            free(m);
        }
    }
    rt->heap.swap(newheap);

}


template<typename T>
MemCell* get_cell(T* buf){
    return ptrAdd<MemCell*>(buf, -bufoffset);

}

template<typename T>
T* to_buf(MemCell* cell){
    return ptrAdd<T*>(cell, bufoffset);
}

template<typename T>
void mark_f(uint8_t* buf, Memory heap, MemToVisit& to_visit){}

template<typename T>
uint32_t mark_fi(){ return 0; }

// =================== test GC =====================

struct OArr{
  size_t size;
  reference elts[0];
};

struct BI{
  int32_t i;
  int32_t j;
};

template<>
uint32_t mark_fi<OArr>(){ return 1; }

template<>
void mark_f<OArr>(reference buf, Memory heap, MemToVisit& to_visit){

    auto&& oarr = reinterpret_cast<OArr*>(buf);
    for(size_t&& i = 0; i < oarr->size; ++i){
       auto&& o = get_cell(oarr->elts[i]);
       to_visit.push(o);
    }
}


template<>
uint32_t mark_fi<BI>(){ return 0; }

reference new_bi(Runtime* rt, int32_t i, int32_t j){
   auto&& cell = allocate(8);
   cell->mark_f = mark_fi<BI>();
   BI* bi = to_buf<BI>(cell);
   bi->i = i;
   bi->j = j;
   rt->heap.push_back(cell);
   return reinterpret_cast<reference>(bi);
}

reference new_array_of_objs(Runtime* rt, size_t n){
   auto&& cell = allocate(8 * n + 8);
   cell->mark_f = mark_fi<OArr>();
   auto&& arr = to_buf<OArr>(cell);
   arr->size = n;
   for(size_t i = 0; i < n; i++){
       arr->elts[i] = new_bi(rt, 1, i);
   }
   rt->heap.push_back(cell);
   return reinterpret_cast<reference>(arr);
}

void setup(Runtime* rt){
  rt->mark_fns.push_back(nullptr);
  rt->mark_fns.push_back(mark_f<OArr>);
  rt->mark_fns.push_back(nullptr);
}

int main(){
    Runtime rt;
    setup(&rt);
    auto&& oarr = new_array_of_objs(&rt, 4);
    auto&& spec = reinterpret_cast<OArr*>(oarr);

    std::stack<MemCell*> st;
    st.push(get_cell(spec->elts[0]));
    printf("Heap size: %d\n", rt.heap.size()); // 5
    gc(&rt, st);
    printf("Heap size: %d\n", rt.heap.size()); // 1
    return 0;
}