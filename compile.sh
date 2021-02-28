rm a2.out
cp a.out a2.out
rm a.out
g++ -std=c++11 *.cpp -o a.out
chmod +x a.out
./a.out $1