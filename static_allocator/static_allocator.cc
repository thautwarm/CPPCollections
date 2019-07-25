#include <vector>
#include <type_traits>
#include <iostream>
#include <functional>
#include <tuple>
#include <cstdint>

class index_tools{
    public:
    template <bool m, typename A, typename... Args>
    constexpr static typename std::enable_if<m, int>::type
    index_impl(){
        return 0;
    }

    template <bool m, typename A, typename... Args>
    constexpr static typename std::enable_if<!m, int>::type
    index_impl(){
        return 1 + index_tools::index<A, Args...>();
    }

    template <typename A, typename T, typename... Args>
    constexpr static int index(){

        return index_tools::index_impl<std::is_same<A, T>::value, A, Args...>();
    }

    template<typename R, typename A, char* s, bool help=false>
    constexpr static
    typename std::enable_if<help, R*>::type
    error_report = nullptr;

    template<typename A>
    constexpr static int index(){
        return *error_report<int, A, "index not found">;
    }


    template<typename A, typename... Args>
    static constexpr int length = 1 + length<Args...>;

    template<typename A>
    static constexpr int length<A> = 1;
};


template<typename T>
using Ptr = T*;

template<typename... Args>
class Allocator{
    std::tuple<std::vector<Ptr<Args>>...> contents;

public:
    Allocator(){
        // contents = {};
        contents = std::make_tuple(std::vector<Ptr<Args>>()...);
    }

    template<typename T, typename... Tl>
    T* allocate(Tl... args){
        auto ptr = new T(args...);
        aware(ptr);
        return ptr;
    }

    template<typename T>
    void aware(T* a){
        auto constexpr n = index_tools::index<T, Args...>();
        std::get<n>(contents).push_back(a);
    }

    void release(){
        _help_no_ret(_release_no_ret<Args>()...);
    }

    auto release_with_cnts(){
        return _help(_release_ret<Args>()...);
    }
private:

    template<typename A>
    constexpr inline int _release_no_ret(){
        auto vec = std::get<index_tools::index<A, Args...>()>(contents);
        for(auto&& e: vec){
            delete e;
        }
        return 0;
    }

    template<typename A>
    constexpr inline int _release_ret(){
        auto vec = std::get<index_tools::index<A, Args...>()>(contents);
        size_t cnt = 0;
        for(auto&& e: vec){
            delete e;
            cnt++;
        }
        return cnt;
    }

    template<typename... Tl>
    constexpr inline void _help_no_ret(Tl... a){
    };

    template<typename T>
    using ConstInt = size_t;

    template<typename... Tl>
    constexpr inline auto _help(Tl... a){
        return std::make_tuple(a...);
    };
};