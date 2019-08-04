FLAGS=-std=c++11 -L/usr/lib64/mysql -lmysqlclient -ljsoncpp -lpthread -g
main:main.cpp db.hpp
	 g++  main.cpp -o main $(FLAGS)
.PHONY:clean
	clean:
	 rm main
