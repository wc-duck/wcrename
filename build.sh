rm -rf build
mkdir build
../Qt5.7.0/5.7/gcc_64/bin/moc src/wcrename.cpp -o build/wcrename.moc.cpp
export LD_LIBRARY_PATH=../Qt5.7.0/5.7/gcc_64/lib/:$LD_LIBRARY_PATH
g++ -fPIC -std=c++11 src/wcrename.cpp -I build -I ../Qt5.7.0/5.7/gcc_64/include/ -L ../Qt5.7.0/5.7/gcc_64/lib/ -lQt5Core -lQt5Widgets -lQt5Gui -lQt5Charts -o build/wcrename

