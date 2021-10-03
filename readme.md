# Connection Broker #

## Description ##
Configurable multi thread connnection broker server capable of handling multiples clients.
The server intercept the message and logs it for each client, and timesout the inactive clients.
Configurable things:
1. Port number
2. Output directory
3. Maximum file size in bytes
4. File prefix
5. Timeout time in minutes

## Depends on ##
1. [boost 1.77](https://www.boost.org/)

## Requirements ##
1. [Clang](https://clang.llvm.org/) or [GCC](https://gcc.gnu.org/)
    1. Clang: `sudo apt install -y clang clang++`
    2. GCC: `sudo apt install -y gcc g++`
2. [CMake](https://cmake.org/): `sudo apt install -y cmake`

## Begin ##
1. Clone this repository
2. `cd connection-broker`

## Build ##
1. Run `./build.sh`

## Compile ##
1. Run `./compile.sh`

## Start the server ##
1. Run `./config.sh` to make the build directory and copy the config file (if the config file changes run this command again).
2. Run `./server.sh`

## Start the client ##
You can start as many clients as you want. Write the massage in the console to send it.
1. Run `./client.sh {port}`. The default port is **8080** (should be equal to the **config.json**).

## Notes ##
The **config.json** is in the **src/server** folder.

## Tempo gasto ##
Aproximadamente 3 dias.

## Dificuldades ##
1. Ambiente de desenvolvimento (Visual Studio para o VS Code)
2. Lentidão na indexação do VS Code devido ao tamanho do boost
3. Cmake
4. MSVC para GCC ou Clang
5. Não costume com a documentação do boost
