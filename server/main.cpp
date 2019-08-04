#include "db.hpp"
#include "httplib.h"
#include <signal.h>
#include <iostream>
using namespace std;

// 函数声明
bool GetDishes(const Json::Value& , Json::Value* , order_system::dish_table& );
int GetConsume(const Json::Value& );
MYSQL* mysql = NULL;

int main()
{
    
    using namespace httplib;
    using namespace order_system;
    Server server;
    // 数据库初始化和释放
    mysql = mysqlinit();
    // 由于不确定该在何时释放，因此可以使用信号
    // 只要按下ctrl+c调用回调函数mysqlrelase函数进行数据库释放
    signal(SIGINT, [](int) {
           mysqlrelease(mysql);
           exit(0);});
    dish_table dish_table(mysql);
    order_table order_table(mysql);

    // 设置路由
    // 新增菜品
    server.Post("/dish", [&dish_table](const Request& req,
                                       Response& resp) {
                cout << "新增菜品:" << req.body << endl;
                Json::Reader reader;
                Json::FastWriter writer;
                Json::Value req_json;
                Json::Value resp_json;

                // 将请求信息解析为json格式
                bool ret = reader.parse(req.body, req_json);
                if(!ret) {
                // 解析失败，返回400响应
                resp_json["ok"] = false;
                resp_json["reason"] = "parse Request failed !\n";
                resp.status = 400;
                resp.set_content(writer.write(resp_json), "application/json");
                return;
                }

                // 进行参数校验
                if(req_json["name"].empty() || req_json["price"].empty()) {
                    resp_json["ok"] = false;
                    resp_json["reason"] = "name or price error !\n";
                    resp.status = 400;
                    resp.set_content(writer.write(resp_json), "application/json");
                    return;
                }

                // 调用数据库接口进行数据操作
                ret = dish_table.Insert(req_json);
                if(!ret) {
                    resp_json["ok"] = false;
                    resp_json["reason"] = "Insert failed !\n";
                    resp.status = 500;
                    resp.set_content(writer.write(resp_json), "application/json");
                    return;
                }

                // 封装正确的返回结果
                resp_json["ok"] = true;
                resp.set_content(writer.write(resp_json), "application/json");
                return;
                });
    // 查看所有菜品
    server.Get("/dish", [&dish_table](const Request& req,
                                      Response& resp) {
               cout << "查看所有菜品:" << req.body << endl;
               Json::Reader reader;
               Json::FastWriter writer;
               Json::Value resp_json;
               // 由于查看菜品API没有请求参数所以不需要解析参数
               // 调用数据库接口进行数据查询
               Json::Value dishes;
               bool ret = dish_table.SelectAll(&dishes);
               if(!ret) {
               resp_json["ok"] = false;
               resp_json["reason"] = "SelectAll failed !\n";
               resp.status = 500;
               resp.set_content(writer.write(resp_json), "applocation/json");
               return;
               }

               // 构造结果响应信息
               resp.set_content(writer.write(dishes), "application/json");
               return;
               });

    // 删除指定id菜品
    // 此处需要用正则表达式来匹配指定菜品
    // \d+表示匹配任意一个数字
    server.Delete(R"(/dish/(\d+))", [&dish_table](const Request& req,
                                      Response& resp) {
                  Json::Value resp_json;
                  Json::FastWriter writer;

                  // 解析获取dish_id
                  // 使用matches[1]获取dish_id
                 int dish_id = stoi(req.matches[1]);
                 cout << "删除指定菜品:" << dish_id << endl;

                 // 调用数据库结构进行数据删除操作，删除指定菜品
                 bool ret = dish_table.Delete(dish_id);
                 if(!ret) {
                 resp_json["ok"] = false;
                 resp_json["reason"] = "SelectAll failed !\n";
                 resp.status = 500;
                 resp.set_content(writer.write(resp_json), "application/json");
                 return;
                 }

                 // 构造响应结果信息
                 resp_json["ok"] = true;
                 resp.set_content(writer.write(resp_json), "application/json");
                 return;
                  });
    // 根据dish_id修改菜品
    server.Put(R"(/dish/(\d+))", [&dish_table](const Request& req,
                                      Response& resp) {
               Json::Reader reader;
               Json::FastWriter writer;
               Json::Value req_json;
               Json::Value resp_json;

               // 获取菜品id
               int dish_id = stoi(req.matches[1]);
               cout << "修改指定菜品" << dish_id << endl;
               
               // 解析菜品信息
               // 将从body中解析出来的信息放入resp_json中
               bool ret = reader.parse(req.body, req_json);
               if(!ret) {
               resp_json["ok"] = false;
               resp_json["reason"] = "parse Request failed !\n";
               return;
               }
               // 需要将dish_id加入到json字段中
               req_json["dish_id"] = dish_id;
               // 进行菜品信息校验
               if(req_json["name"].empty() || req_json["price"].empty()) {
                   resp_json["ok"] = false;
                   resp_json["reason"] = "name or price error !\n";
                   resp.status = 400;
                   resp.set_content(writer.write(resp_json), "application/json");
                   return;
               }

               // 调用数据库接口修改指定菜品信息
               ret = dish_table.Update(req_json);
               if(!ret) {
                   resp_json["ok"] = false;
                   resp_json["reason"] = "update failed !\n";
                   resp.status = 500;
                   resp.set_content(writer.write(resp_json), "application/json");
                   return;
               }

               // 构造响应结果
               resp_json["ok"] = true;
               resp.set_content(writer.write(resp_json), "application/json");
               return;
                  });

    // 新增订单
    server.Post("/order", [&order_table](const Request& req,
                                       Response& resp) {
                cout << "新增订单:" << req.body <<endl;
                Json::Reader reader;
                Json::FastWriter writer;
                Json::Value req_json;
                Json::Value resp_json;

                // 解析请求
                bool ret = reader.parse(req.body, req_json);
                if(!ret) {
                resp_json["ok"] = false;
                resp_json["reason"] = "parse Request failed !\n";
                resp.status = 400;
                resp.set_content(writer.write(resp_json), "application/json");
                return;
                }

                // 校验订单
                if(req_json["table_id"].empty() || req_json["time"].empty()
                   || req_json["dish_ids"].empty()) {
                    resp_json["ok"] = false;
                    resp_json["reason"] = "table_id or time or dish_ids error !\n";
                    resp.status = 400;
                    resp.set_content(writer.write(resp_json), "application/json");
                    return;
                }

                // 调用数据库接口插入订单
                ret = order_table.Insert(req_json);
                if(!ret) {
                    resp_json["ok"] = false;
                    resp_json["reason"] = "Insert failed !\n";
                    resp.status = 500;
                    resp.set_content(writer.write(resp_json), "application/json");
                    return;
                }

                // 构造正确的响应结果
                resp_json["ok"] = true;
                resp.set_content(writer.write(resp_json), "application/json");
                return;
                });

    // 修改订单状态
    server.Put(R"(/order/(\d+))", [&order_table](const Request& req,
                                       Response& resp) {
               Json::Reader reader;
               Json::FastWriter writer;
               Json::Value req_json;
               Json::Value resp_json;
                
               // 获取订单id
               int order_id = stoi(req.matches[1]);
               cout << "修改订单" << order_id << "状态:" << req.body << endl;
               // 解析请求信息
               bool ret = reader.parse(req.body, req_json);
               if(!ret) {
                    resp_json["ok"] = false;
                    resp_json["reason"] = "update state failed !\n";
                    resp.status = 500;
                    resp.set_content(writer.write(resp_json), "application/json");
                    return;
               }

               // 需要将order_id加入到json字段中
               req_json["order_id"] = order_id;
               // 进行菜品信息校验
               if(req_json["table_id"].empty() || req_json["time"].empty()
                  || req_json["dish_ids"].empty()) {
                   resp_json["ok"] = false;
                   resp_json["reason"] = "table_id or time or dish_ids error !\n";
                   resp.status = 400;
                   resp.set_content(writer.write(resp_json), "application/json");
                   return;
               }

               // 调用数据库接口修改指定菜品信息
               ret = order_table.UpdateState(req_json);
               if(!ret) {
                   resp_json["ok"] = false;
                   resp_json["reason"] = "update state failed !\n";
                   resp.status = 500;
                   resp.set_content(writer.write(resp_json), "application/json");
                   return;
               }

               // 构造响应结果
               resp_json["ok"] = true;
               resp.set_content(writer.write(resp_json), "application/json");
                });
    // 查看订单信息
    // orders包含多个order, order包含多个dish
    server.Get("/order", [&order_table, &dish_table](const Request& req,
                                       Response& resp) {
               cout << "查看所有订单信息:" << req.body << endl;
               Json::Reader reader;
               Json::FastWriter writer;
               Json::Value resp_json;
               // 由于查询订单信息没有请求参数，所以不需进行参数校验
               // 调用数据库接口查询订单信息
               Json::Value orders;
               bool ret = order_table.Select(&orders);
               if(!ret) {
               resp_json["ok"] = false;
               resp_json["reason"] = "Select failed !\n";
               resp.status = 500;
               resp.set_content(writer.write(resp_json), "application/json");
               return;
               }
               // 循环处理查询结果
               for(uint32_t i = 0; i < orders.size(); i++) {
               cout << "正在处理第" << i << "订单!" << endl;
               Json::Value& order = orders[i];
               // 格式转换
               Json::Value dish_ids;
               ret = reader.parse(order["dish_ids_str"].asString(), dish_ids);
               if(!ret) {
                   cout  << "order_id:" << order["order_id"].asInt() << 
                       "has error dish_ids_str !" << endl;
                   continue;
               }

               // 将order中的dish_id 构造成dishes
               // 方便再次进入数据库，查询到每个菜品的信息
               GetDishes(dish_ids, &order["dishes"], dish_table);
               
               // 构造consune字段获取订单总价
               order["consunme"] = GetConsume(order["dishes"]);
               
               }
               // 构造正确的响应结果
               resp_json["ok"] = true;
               resp.set_content(writer.write(resp_json), "application/json");
               return;
                });

    // 获取客户端，匹配桌子id
    server.Get(R"(/client/table/(\S+))", [](const Request& req, 
                                            Response& resp) {
    }); 
    // 设置静态文件目录
    server.set_base_dir("./wwwroot");

    server.listen("0.0.0.0", 9000);
    return 0;
}

// 辅助函数
// 将dish_ids 转换成dishes
bool GetDishes(const Json::Value& dish_ids, Json::Value* dishes, order_system::dish_table& dish_table) {
    for(uint32_t i = 0; i < dish_ids.size(); i++) {
    cout << "正在处理第" << i << "个菜品" << endl;
    int dish_id = dish_ids[i].asInt();
    Json::Value dish_info;
    bool ret = dish_table.SelectOne(dish_id, &dish_info);
    if(!ret) {
        cout << "dish_id = " << i << "not found !" << endl;
        continue;
    }

    // 将菜品信息加入到dishes中
    dishes->append(dish_info);

    }
    return true;
}


// 获取订单总价
int GetConsume(const Json::Value& dishes) {
    int consume = 0;
    for(uint32_t i = 0; i < dishes.size(); i++) {
        consume += dishes["price"].asInt();
    }
    return consume;

}

