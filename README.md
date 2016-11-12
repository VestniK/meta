# meta
Экспериментальный язак программирования пытающийся реализовать императивную систему
метопрограммирования на этапе компиляции.

# Сборка (простой путь)

    git clone git@github.com:VestniK/meta.git
    mkdir -p meta/build/debug
    cd meta/build/debug
    conan install ../..
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ../..
    ninja

Если вы ретроград, то вместо ninja можно использовать make

Не забудте убедиться что у вас есть
 * система управления зависимостями conan.io
 * cmake >= 3.0
 * gcc >= 6.0 (местами используются концепты, в clang'е их пока пилят, посему пока gcc-only)

# Сборка (путь джедая)

Если уровень вашего желания зазря поднимать энтропию во вселенно очень хорошо описывается
цифрой 0, а в вашем дистрибутеве есть собранные библиотеки и тулзы которые нежны для
сборки данного проекта, то сборка будет чуть чуть другой:

    git clone git@github.com:VestniK/meta.git
    mkdir -p meta/build/debug
    cd meta/build/debug
    cp ../../conanfile.txt ./
    vim conanfile.txt # Удаляем из секции requires то что у вас есть в системе
    conan install
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ../..
    ninja

