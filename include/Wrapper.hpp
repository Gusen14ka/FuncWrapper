#pragma once

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include "Engine.hpp"

template<typename Obj, typename ...Args>
class Wrapper{
public:   
    using func_ptr = std::function<int(Obj*, Args...)>;

    /*
    Поддерживаем создание только от: 
    1) явных указателей на методы класса (const или нет) 
    2) std::function<int(Obj*, Args...)>
    3) std::function<int(Args...)> - просто длдя полноты картины
    */
    Wrapper(Obj* obj, func_ptr f_ptr, std::initializer_list<std::pair<std::string, int>> defaults = {});

    Wrapper(Obj* obj, std::function<int(Args...)> f_ptr, std::initializer_list<std::pair<std::string, int>> defaults = {});

    // Используем SFINAE для проверки того, что MFunc нужный указатель
    template<typename MFunc, 
        typename = std::enable_if_t<std::is_member_function_pointer_v<std::decay_t<MFunc>>>>
    Wrapper(Obj* obj, MFunc f_ptr, std::initializer_list<std::pair<std::string, int>> defaults = {});

    friend class Engine;

private:
    static constexpr size_t arity = sizeof...(Args);
    Obj * obj_;
    func_ptr method_;
    std::unordered_map<std::string, size_t> aliasToIdx_;
    std::vector<int> defArgs_;

    template<size_t ...Inds>
    int invoke_with_ind_seq(std::index_sequence<Inds...> indexes, std::vector<int> && args);

    void init(std::initializer_list<std::pair<std::string, int>> defaults);

    int invoke(std::initializer_list<std::pair<std::string, int>> args = {});

    
};

template<typename Obj, typename ...Args>
Wrapper<Obj,Args...>::Wrapper(Obj * obj, std::function<int(Obj*, Args...)> f_ptr, 
    std::initializer_list<std::pair<std::string, int>> defaults) : obj_(obj), method_(f_ptr)
{
    init(defaults);
}

template<typename Obj, typename ...Args>
Wrapper<Obj,Args...>::Wrapper(Obj* obj, std::function<int(Args...)> f_ptr, 
    std::initializer_list<std::pair<std::string, int>> defaults): obj_(obj)
{
    method_ = [f_ptr](Obj*, Args ...args){
        return f_ptr(args...);
    };
    init(defaults);
}

template<typename Obj, typename ...Args>
template<typename MFunc, typename /*Определили внутри класса*/>
Wrapper<Obj,Args...>::Wrapper(Obj * obj, MFunc f_ptr, std::initializer_list<std::pair<std::string, int>> defaults)
    : obj_(obj)
{
    method_ = std::mem_fn(f_ptr);
    init(defaults);
}

template<typename Obj, typename ...Args>
void Wrapper<Obj,Args...>::init(std::initializer_list<std::pair<std::string, int>> defaults){
    int count = 0;
    aliasToIdx_.reserve(arity);
    defArgs_.reserve(arity);
    for(auto& el: defaults){
        ++count;
        if(aliasToIdx_.find(el.first) != aliasToIdx_.end()){
            throw std::invalid_argument("Reusing the argument name");
        }
        aliasToIdx_[el.first] = count - 1;
        defArgs_.push_back(el.second);
    }
    // Контракт между обёрткой и методом - обязательное задание всех аргументов
    if(count != arity){
        throw std::invalid_argument("The number of arguments must be equal to the number of arguments of the method/function");
    }
}

template<typename Obj, typename ...Args>
int Wrapper<Obj,Args...>::invoke(std::initializer_list<std::pair<std::string, int>> args){
    auto final_args = defArgs_;
    for(auto& el : args){
        auto it = aliasToIdx_.find(el.first);
        if(it == aliasToIdx_.end()){
            throw std::invalid_argument("Unknown argument name: " + el.first);
        }
        final_args[it->second] = el.second;
    }
    return invoke_with_ind_seq(std::make_index_sequence<arity>(), std::move(final_args));
}

template<typename Obj, typename ...Args>
template<size_t ...Inds>
int Wrapper<Obj,Args...>::invoke_with_ind_seq(std::index_sequence<Inds...> indexes, std::vector<int> && args){
    return std::invoke(method_, obj_, args[Inds]...);
}

// CTAD - Нужно, тк из Callable компилятор не выведет Args...
template<typename Obj, typename... Args>
Wrapper(Obj*, int (Obj::*)(Args...), std::initializer_list<std::pair<std::string,int>> = {}) -> Wrapper<Obj, Args...>;

template<typename Obj, typename... Args>
Wrapper(Obj*, int (Obj::*)(Args...) const, std::initializer_list<std::pair<std::string,int>> = {}) -> Wrapper<Obj, Args...>;

template<typename Obj, typename... Args>
Wrapper(Obj*, std::function<int(Obj*, Args...)>, std::initializer_list<std::pair<std::string,int>> = {}) -> Wrapper<Obj, Args...>;

template<typename Obj, typename... Args>
Wrapper(Obj*, std::function<int(Args...)>, std::initializer_list<std::pair<std::string,int>> = {}) -> Wrapper<Obj, Args...>;
