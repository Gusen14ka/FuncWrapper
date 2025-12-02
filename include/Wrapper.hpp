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


template<typename Obj, typename ...Args>
class Wrapper{
public:   
    using func_ptr = std::function<int(Obj*, Args...)>;

    template<typename Callable,
        typename = std::enable_if_t<!std::is_same_v<std::decay_t<Callable>, Wrapper>>>
    Wrapper(Obj * obj, Callable callable, std::initializer_list<std::pair<std::string, int>> defaults = {});

    int invoke(std::initializer_list<std::pair<std::string, int>> args = {});

private:
    static constexpr size_t arity = sizeof...(Args);
    Obj * obj_;
    func_ptr method_;
    std::unordered_map<std::string, size_t> aliasToIdx_;
    std::vector<int> defArgs_;

    template<size_t ...Inds>
    int invoke_with_ind_seq(std::index_sequence<Inds...> indexes, std::vector<int> && args);
};


template<typename Obj, typename ...Args>
template<typename Callable, typename /*Определили внутри класса*/>
Wrapper<Obj,Args...>::Wrapper(Obj * obj, Callable callable, std::initializer_list<std::pair<std::string, int>> defaults)
    : obj_(obj)
{
    method_ = [callable](Obj * obj, Args... args){
        return std::invoke(callable, obj, args...);
    };

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