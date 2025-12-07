#include <iostream>
#include <string>
#include "Wrapper.hpp"
#include "Engine.hpp"

struct Subject {
    // const-метод
    int plus(int arg1, int arg2) const {
        std:: cout << "(arg1 = " << arg1 << "; arg2 = " << arg2 << ") ";
        return arg1 + arg2;
    }

    // non-const 
    int mul(int arg1, int arg2) {
        std:: cout << "(arg1 = " << arg1 << "; arg2 = " << arg2 << ") ";
        return arg1 * arg2;
    }
};

int main() {
    using init_list = std::initializer_list<std::pair<std::string,int>>;

    // Const метод
    {
        std::cout << "\n[TEST 1] const-method\n";
        Subject subj;
        Wrapper w1(&subj, &Subject::plus, { {"arg1", 0}, {"arg2", 0} });
        Engine engine;
        engine.register_command(&w1, "add");
        std::cout << "add(4,5) => " << engine.execute("add", { {"arg1",4}, {"arg2",5} }) << " (expected 9)\n";
    }
    
    // Const метод - поменяем поряд при вызове engine.execute
    {
        std::cout << "\n[TEST 2] const-method: args in different order\n";
        Subject subj;
        Wrapper w(&subj, &Subject::plus, { {"arg1",0}, {"arg2",0} });
        Engine engine;
        engine.register_command(&w, "add2");
        std::cout << "add2(arg2=7, arg1=3) => "
                  << engine.execute("add2", { {"arg2",7}, {"arg1",3} }) << " (expected 10)\n";
    }

    // Const-метод - проверка дефолт значений заданных при конструированиии wrapper
    {
        std::cout << "\n[TEST 3] defaults\n";
        Subject subj;
        Wrapper wdef(&subj, &Subject::plus, { {"arg1",10}, {"arg2",20} });
        Engine engine;
        engine.register_command(&wdef, "add_default");
        // передаём только arg1, arg2 должен подставиться из defaults
        std::cout << "add_default(arg1=5) => "
                  << engine.execute("add_default", { {"arg1",5} }) << " (expected 25)\n";
    }

    // Tест: неизвестное имя аргумента (ожидаем исключение)
    {
        std::cout << "\n[TEST 4] unknown arg name -> exception\n";
        Subject subj;
        Wrapper w(&subj, &Subject::plus, { {"arg1",0}, {"arg2",0} });
        Engine engine;
        engine.register_command(&w, "add_err");
        try {
            // "argX" не обучено - должен выбросить
            std::cout << engine.execute("add_err", { {"argX", 1}, {"arg2",2} }) << "\n";
            std::cout << "ERROR: expected exception for unknown arg name\n";
        } catch (const std::exception& ex) {
            std::cout << "caught exception as expected: " << ex.what() << "\n";
        } catch (...) {
            std::cout << "caught unknown exception (expected std::exception)\n";
        }
    }

    // Не const-метод
    {
        std::cout << "\n[TEST 5] non-const method\n";
        Subject subj;
        Wrapper wmul(&subj, &Subject::mul, { {"a",1}, {"b",1} });
        Engine engine;
        engine.register_command(&wmul, "mul");
        std::cout << "mul(6,7) => " << engine.execute("mul", { {"a",6}, {"b",7} }) << " (expected 42)\n";
    }

    // Tест: неизвестная команда (при вызове engine.execute)
    {
        std::cout << "\n[TEST 8] unknown command -> exception\n";
        Subject subj;
        Wrapper w(&subj, &Subject::plus, { {"arg1",0}, {"arg2",0} });
        Engine engine;
        engine.register_command(&w, "exists");
        try {
            std::cout << "calling 'does_not_exist'...\n";
            std::cout << engine.execute("does_not_exist", { {"arg1",1}, {"arg2",2} }) << "\n";
            std::cout << "ERROR: expected exception for unknown command name\n";
        } catch (const std::exception& ex) {
            std::cout << "caught exception for unknown command as expected: " << ex.what() << "\n";
        } catch (...) {
            std::cout << "caught unknown exception (expected std::exception)\n";
        }
    }

    // Возможность работы с std::function<int(Obj*, Args...)> и std::function<int(Args...)>
    {
        std::cout << "\n[TEST 9] work with std::function\n";
        Subject subj;

        // std::function<int(Obj*, Args...)> из mem_fn 
        std::function<int(Subject*,int,int)> f_mem = std::mem_fn(&Subject::plus);
        Wrapper w1(&subj, f_mem, { {"arg1",0}, {"arg2",0} });

        // std::function<int(Obj*, Args...)> из лямбды (берёт Subject* первым каргументом)
        std::function<int(Subject*,int,int)> f_lambda = [](Subject* s, int a, int b){
            return s->plus(a,b) + 10;
        };
        Wrapper w2(&subj, f_lambda, { {"arg1",0}, {"arg2",0} });

        // std::function<int(Args...)> из лямбды (без Subject*)
        std::function<int(int,int)> f_noobj = [](int a,int b){ 
            return a + b + 1000; 
        };
        Wrapper w3(&subj, f_noobj, { {"arg1",0}, {"arg2",0} });

        Engine engine;
        engine.register_command(&w1, "mem");
        engine.register_command(&w2, "lamf");
        engine.register_command(&w3, "noobj");

        std::cout << "std::function<int(Obj*, Args...)> from mem_fn\n" << "add(2,3) => " << engine.execute("mem", { {"arg1",2}, {"arg2",3} }) << " (expect 5)\n\n";
        std::cout << "std::function<int(Obj*, Args...)> from lambda\n" << "lamf(2,3) => " << engine.execute("lamf", { {"arg1",2}, {"arg2",3} }) << " (expect 15)\n\n";
        std::cout << "std::function<int(Args...)> from lambda\n" << "noobj(2,3) => " << engine.execute("noobj", { {"arg1",2}, {"arg2",3} }) << " (expect 1005)\n\n";
    }
    
    std::cout << "\n=== Test suite end ===\n";
    return 0;
}
