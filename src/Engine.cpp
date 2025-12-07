#include "Engine.hpp"

int Engine::execute(std::string const & name, init_list args){
    auto it = nameToInvokeFunc_.find(name);
    if(it == nameToInvokeFunc_.end()){
        //TODO - неизвестное имя враппера
        throw;
    }
    return it->second(args);
}