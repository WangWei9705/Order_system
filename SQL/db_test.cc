#include "db.hpp"
#include <iostream>

void testdish_table() {
using namespace Json;
using namespace order_system;
 bool ret = false;
 // 更友好的格式化显示 Json
 Json::StyledWriter writer;
 MYSQL* mysql = mysqlinit();
 Json::Value dish;
 dish["name"] = "红烧肉";
 dish["price"] = 2300;
 std::cout << "==============测试插入=====================" << std::endl;
 dish_table dish_table(mysql);
 ret = dish_table.Insert(dish);
 std::cout << "Insert: " << ret << std::endl;
 std::cout << "==============测试查找=====================" << std::endl;
 Json::Value dishes;
 ret = dish_table.SelectAll(&dishes);
 std::cout << "SelectAll: " << ret << std::endl
 << writer.write(dishes) << std::endl;
 std::cout << "==============测试更新=====================" << std::endl;
 dish["dish_id"] = 5;
 dish["name"] = "毛家红烧肉";
 dish["price"] = 2700;
 Json::Value dish_out;
 ret = dish_table.Update(dish);
 std::cout << "Update: " << ret << std::endl;
 ret = dish_table.SelectOne(5, &dish_out);
 std::cout << "SelectOne: " << ret << std::endl
 << writer.write(dish_out) << std::endl;
 std::cout << "==============测试删除=====================" << std::endl;
 int dish_id = 6;
 ret = dish_table.Delete(dish_id);
 std::cout << "Delete: " << ret << std::endl;
 mysqlrealse(mysql); 
}

int main()
{
	testdish_table();
 
   return 0;
}

