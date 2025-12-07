// Файл для корректной компиляции - форвад декларация враппера
#pragma once

// forward-declare template Wrapper so other headers may refer to it
template<typename Obj, typename... Args>
class Wrapper;