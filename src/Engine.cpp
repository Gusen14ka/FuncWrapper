#include "Engine.hpp"
#include <stdexcept>

int Engine::execute(std::string const & name, init_list args){
    auto it = nameToInvokeFunc_.find(name);
    if(it == nameToInvokeFunc_.end()){
        throw std::invalid_argument("Unknown wrapper name");
    }
    return it->second(args);
}