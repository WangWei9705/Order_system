FLAGS=-std=c++11 -L/usr/lib64/mysql -libmysqlclient -ljsoncpp -lpthread -g
.PHONY:all
	all:db_test

db_test:db_test.cc 
	g++ $^ -o $@ $(FALGS)

.PHONY:clean
	 clean:
	 rm db_test
	 
