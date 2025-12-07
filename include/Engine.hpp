#pragma once

#include "Wrapper.hpp"
#include <functional>
#include <string>
#include <unordered_map>

class Engine{
    using init_list = std::initializer_list<std::pair<std::string, int>>;
    using wrap_invoke_ptr = std::function<int(init_list)>;
public:
    Engine() = default;

    template<typename ...WrapT>
    void register_command(Wrapper<WrapT...>* wrap, std::string const & name);
    int execute(std::string const & name, init_list args);
private:
    std::unordered_map<std::string, wrap_invoke_ptr> nameToInvokeFunc_;
};


template<typename ...WrapT>
void Engine::register_command(Wrapper<WrapT...>* wrap, std::string const & name){
    nameToInvokeFunc_[name] = [wrap](std::initializer_list<std::pair<std::string, int>> args){
        return wrap->invoke(args);
    };
}