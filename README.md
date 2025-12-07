# FuncWrapper — лёгкая обёртка вызова методов по именованным аргументам

Проект написан на C++20 и демонстрирует простую обёртку `Wrapper`, которая позволяет
регистрировать методы объектов (включая `std::function`) под строковыми именами
и вызывать их через `Engine`, передавая аргументы по именам (в `initializer_list`).

**Ключевые возможности**
- Позволяет оборачивать: указатели на методы (const и non-const), `std::function<int(Obj*,...)>` и `std::function<int(...)>`.
- Поддержка именованных аргументов (alias) и значений по-умолчанию, заданных при конструировании `Wrapper`.
- Регистрация команд в `Engine` по имени и вызов через единую точку `execute`.

**Структура проекта**
- `main.cpp` — демонстрация работы (тесты-показы).
- `include/Wrapper.hpp` — реализация шаблонного класса `Wrapper`.
- `include/Engine.hpp`, `src/Engine.cpp` — простая регистрация и маршрутизация вызовов.
- `CMakeLists.txt` — сборка проекта с CMake (C++20).

**Пример сборки и запуска (Windows PowerShell)**

```powershell
# Создать папку сборки и собрать (используется генератор, который доступен в системе, например Ninja или MSVC)
mkdir build; cd build
cmake -S .. -B . -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Запустить
.\Wrapper.exe
```


**Как пользоваться (быстрая справка)**

1. Создаём объект-целевой класс (в примере `Subject` с методами `plus` и `mul`).
2. Создаём `Wrapper` для метода и указываем список пар `{"имя_аргумента", значение_по_умолчанию}` — количество пар должно совпадать с арностью метода.
3. Регистрируем `Wrapper` в `Engine` под некоторым именем и вызываем через `Engine::execute(name, init_list)`.

**API (кратко)**

- `Wrapper<Obj, Args...>` — шаблонный класс-обёртка.
  - Конструкторы:
    - `Wrapper(Obj* obj, std::function<int(Obj*, Args...)> f_ptr, defaults)`
    - `Wrapper(Obj* obj, std::function<int(Args...)> f_ptr, defaults)`
    - `Wrapper(Obj* obj, MFunc f_ptr, defaults)` — SFINAE-конструктор для указателей на методы класса.
  - `int invoke(std::initializer_list<std::pair<std::string,int>> args = {})` — вызвать метод, передав аргументы по именам (или использовать defaults).

- `Engine` — хранит имя->функцию и маршрутизирует вызовы.
  - `register_command(Wrapper<...>* wrap, std::string const& name)` — регистрирует обёртку.
  - `int execute(std::string const& name, init_list args)` — вызывает зарегистрированную команду или бросает `std::invalid_argument` если имя неизвестно.

**Замечания по реализации**

- `Wrapper` требует, чтобы при конструировании было передано ровно `arity` пар дефолтных значений (по контракту автора). Это обеспечивает, что на каждую позицию аргумента есть значение — либо из defaults, либо из `invoke`.
- Внутри используются `std::function`, `std::mem_fn`, `std::invoke` и `std::index_sequence` для распаковки аргументов.
- В `Wrapper.hpp` присутствуют CTAD-направления (CTAD guides), чтобы упростить создание объекта `Wrapper` без явного указания `<Obj, Args...>`.


