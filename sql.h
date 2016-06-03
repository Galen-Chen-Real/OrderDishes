/*
 *        'sql.h'
 *     (C) 2014.4 GordonChen
 *
 */
#ifndef _SQL_H
#define _SQL_H

#define SQLBUFMAX 128

//数据库数据组成的结构体类型
typedef struct
{
    int id;
    char name[24];
    int price;
    int num;
}DISHES;

//连接数据库;成功返回0,失败返回-1.
int sql_connect(void);
//关闭数据库
void sql_close(void);
//查看表tb中的数据,使用NULL依次遍历,成功返回1,结束返回0.注意:在遍历的过程中需要上锁.
int sql_select(char *tb, DISHES *dish);
//插入id指定的tbMenu中的数据到表tb中,并且会把tb.num置为1,把相对应的tbMenu.num减一;
//成功返回0,失败返回-1.注意:id的形式必须为"1,2,3...".
int sql_insert_tb(char *tb, char *id);
//删除表tb中的所有数据;成功返回0,失败返回-1.
int sql_delete_tb(char *tb);
//求price和;成功返回sum,失败返回-1.
int sql_sum_tb(char *tb);

#endif

