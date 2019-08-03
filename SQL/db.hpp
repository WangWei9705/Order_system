#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <jsoncpp/json/json.h>

namespace order_system{

// 初始化数据库
static MYSQL*  mysqlinit() {
    MYSQL* mysql_fd = mysql_init(NULL);
    if(mysql_real_connect(mysql_fd, "127.0.0.1", "root", "", "order_system", 3306, NULL, 0) == NULL) {
        printf("数据库连接失败！%s\n", mysql_error(mysql_fd));
        return NULL;
    }

    // 设置编码
    mysql_set_character_set(mysql_fd, "utf8");
    return mysql_fd;
}

// 断开数据库连接
static void mysqlrealse(MYSQL* mysql) {
    mysql_close(mysql);
}

// 构建dish_table表
class dish_table {
public:
    // 构造函数
    dish_table(MYSQL* mysql)
        : _mysql(mysql) {}

    // 输出型参数 *   输入型参数&
    // 增加菜品(商家)
    bool Insert(const Json::Value& dish) {
        // 构造sql语句
        char SQL[1024*4] = {0};
        sprintf(SQL, "insert into dish values(null, '%s', %d)",
                dish["name"].asCString(), dish["price"].asInt());
        // 执行sql语句
        int ret = mysql_query(_mysql, SQL);
        if(!ret) {
            printf("insert falied ! sql = %s, error = %s\n",
                   SQL, mysql_error(_mysql));
            return false;
        }
        return true;
    }
    // 查看所有菜品信息(商家/用户)
    bool SelectAll(Json::Value* dishes) {
        // 构造sql语句
        char SQL[1024*4] = {0};
        sprintf(SQL, "select dish_id, name, price from dish_table");

        // 执行SQL语句
        int ret = mysql_query(_mysql, SQL);
        if(!ret) {
            printf("SelectAll falied !  sql = %s, error = %s\n",
                   SQL, mysql_error(_mysql));
            return false;
        }

        // 获取查询结果
        MYSQL_RES* result = mysql_store_result(_mysql);
        if(result == NULL) {
            printf("get retsult falied! error = %s\n", mysql_error(_mysql));
            return false;

        }

        // 遍历每一行
        int rows = mysql_num_rows(result);
        for(int i = 0; i < rows; i++) {
            MYSQL_ROW row = mysql_fetch_row(result);
            Json::Value value;
            value["dish_id"] = atoi(row[0]);
            value["name"] = row[1];
            value["price"] = atoi(row[2]);
            // 将遍历结果插入到dishes中
            dishes->append(value);
        }
        return true;
    }

    // 查看指定编号的菜品信息
    bool SelectOne(int32_t dish_id, Json::Value* dish) {
        // 构造SQL语句
        char SQL[1024*4] = {0};
        sprintf(SQL, "select dish_id, name, price where dish_id = %d", dish_id);

        // 执行SQL语句
        int ret = mysql_query(_mysql, SQL);
        if(!ret) {
            printf("SelectOne falied ! sql = %s, error = %s\n",
                   SQL, mysql_error(_mysql));
            return false;
        }

        // 获取查询结果
        MYSQL_RES* result = mysql_store_result(_mysql);
        if(result == NULL) {
            printf("get result falied ! error = %s\n", mysql_error(_mysql));
            return false;
        }
        // 得到查询结果的行数
        int rows = mysql_num_rows(result);
        if(rows != 1) {
            printf("result is morethan one ! row = %d\n", rows);
            return false;
        }
        // 遍历查询结果
        for(int i = 0; i < rows; i++) {
            MYSQL_ROW row = mysql_fetch_row(result);
            (*dish)["dish_id"] = atoi(row[0]);
            (*dish)["name"] = row[1];
            (*dish)["price"] = atoi(row[2]);
            break;
        }
        return true;
    }
    // 修改菜品(商家)
    bool Update(const Json::Value& dish) {
        // 构造SQL语句
        char SQL[1024*4] = {0};
        sprintf(SQL, "update dish_table SET name='%s', price=%d where dish_id=%d",
                dish["name"].asCString(), dish["price"].asInt(), dish["dish_id"].asInt());

        // 执行SQL语句
        int ret = mysql_query(_mysql, SQL);
        if(!ret) {
            printf("update dish_table falied ! sql = %s, error = %s\n", SQL, mysql_error(_mysql));
            return false;
        }
        return true;
        
    }
    // 删除菜品(商家)
   bool Delete(int32_t dish_id) {
        // 构造SQL语句
        char SQL[1024*4] = {0};
        sprintf(SQL, "delete from dish_table where dish_id=%d", dish_id);

        // 执行SQl语句
        int ret = mysql_query(_mysql, SQL);
        if(!ret) {
            printf("delete falied ! error = %s\n", mysql_error(_mysql));
            return false;
        }
        return true;
    }
    
private:
    MYSQL* _mysql;
};

class order_table{
public:
    order_table(MYSQL* mysql)
        : _mysql(mysql) {}
    
    // 查看订单(商家)
    bool Select(Json::Value* orders) {}
    // 提交订单(用户)
    bool Insert(const Json::Value& order) {}
    // 修改订单状态(商家)
    bool UpdateState(const Json::Value& order) {}
    
private:
    MYSQL* _mysql;
};
    
}   // end namespace order_system
