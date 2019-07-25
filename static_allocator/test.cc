#include "static_allocator.cc"

class S{
    public:
};

class D : public S{
    public:
    int k;
    D(int k){this->k = k;}
};

class K: public S{
    public:
};

int g(){
    Allocator<int, S, float> alloc;
    S* a = new D(1);
    S* b = new K();
    alloc.aware(a);
    alloc.aware(b);

    alloc.allocate<int>();
    alloc.allocate<int>();
    alloc.allocate<float>();

    alloc.release();
    return 0;
}
int main(){
    return g();
}