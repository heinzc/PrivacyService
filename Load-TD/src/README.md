**Libraries used : cpr and nlohmann **

**CPR libray - C++ Requests**

To install cpr:

git clone https://github.com/whoshuu/cpr-example.git

Go to root dir 

git submodule add git@github.com:whoshuu/cpr.git

git submodule update --init --recursive

Mkdir build

Cd build 

Cmake ..

Make 

**To compile: **

Change in cmake.txt file : add_executable(load_TD load_TD.cpp)  --- load_TD  name of the cpp file

target_link_libraries(load_TD ${CPR_LIBRARIES}).     

Copy the cpr include file and lib file to usr/local/include & usr/local/lib

add compiler cpp and ld flags. 

Run cmake and make as before in build folder. 

load_TD binary is generated in build folder. 

./load_TD - run the compiled binary.


**Nlohmann library - JSON for c++**

To install nlohmann (MAC OS): refer --- https://github.com/nlohmann/homebrew-json

brew install nlohmann_json

**Load_TD.cpp : **

It reads the config file to get the thing-description from adapter endpoints and then creates a new thing-description file for encrypted device. 
Each time when it is executed it removes the newly created TD and loads the json objects from all adapter endpoints provided in the config file. 

**Build folder**
move the config file into build folder.
