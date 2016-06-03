/*
 *        'sql.c'
 *     (C) 2014.4 GordonChen
 *
 */
#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>

#include "sql.h"

static MYSQL *conn = NULL;

int sql_connect(void)
{
    conn = mysql_init(conn);
    if(!conn)
    {
        printf("%s\n", mysql_error(conn));
        return -1;
    }
    
    if(!mysql_real_connect(conn, "localhost", "root", NULL, "dishes", 
        0, NULL, 0))
    {
        printf("%s\n", mysql_error(conn));
        return -1;
    }
    
    return 0;
}

void sql_close(void)
{
    mysql_close(conn);
}

int sql_select(char *tb, DISHES *dish)
{
    char buf[SQLBUFMAX];
    static MYSQL_STMT *st;
    static MYSQL_BIND bind[4];  //NOTE:有一个非常值的测试的地方,关于static!
    
    if(tb != NULL)
    {
        st = mysql_stmt_init(conn);
        if(!st)
        {
           printf("%s\n",mysql_error(conn));
           return 0;
        }
        
        sprintf(buf, "select * from %s", tb);
        mysql_stmt_prepare(st, buf ,strlen(buf));

        memset(bind, 0, sizeof(bind));
        
        bind[0].buffer_type=MYSQL_TYPE_LONG;
        bind[0].buffer=&dish->id;

        bind[1].buffer_type=MYSQL_TYPE_STRING;
        bind[1].buffer=dish->name;
        bind[1].buffer_length=sizeof(dish->name);

        bind[2].buffer_type=MYSQL_TYPE_LONG;
        bind[2].buffer=&dish->price;

        bind[3].buffer_type=MYSQL_TYPE_LONG;
        bind[3].buffer=&dish->num;

        mysql_stmt_bind_result(st,bind);
        mysql_stmt_execute(st);
        mysql_stmt_store_result(st);
        
        return 0;
    }
    
    if(!mysql_stmt_fetch(st))
    {
        return 1;
    }
    else
    {
        mysql_stmt_close(st);
        return 0;
    }
}

int sql_insert_tb(char *tb, char *id)
{
    char buf[SQLBUFMAX];
    
    mysql_query(conn, "begin");
    
    sprintf(buf, "insert into %s select * from tbMenu where id in (%s)",
        tb, id); 
    printf("%s\n", buf);
    if(mysql_query(conn, buf))
    {
        mysql_query(conn, "rollback");
        return -1;
    }
    
    sprintf(buf, "update %s set num=1", tb);
    if(mysql_query(conn, buf))
    {
        mysql_query(conn, "rollback");
        return -1;
    }
    
    sprintf(buf, "update tbMenu set num=num-1 where id in (%s)", id);
    if(mysql_query(conn, buf))
    {
        mysql_query(conn, "rollback");
        return -1;
    }
    
    mysql_query(conn, "commit");
    
    return 0;
}

int sql_delete_tb(char *tb)
{
    char buf[SQLBUFMAX];
    
    mysql_query(conn, "begin");
    
    sprintf(buf, "delete from %s", tb);
    if(mysql_query(conn, buf))
    {
        mysql_query(conn, "rollback");
        return -1;
    }
    
    mysql_query(conn, "commit");
    
    return 0;
}

int sql_sum_tb(char *tb)
{
    char buf[SQLBUFMAX];
    MYSQL_STMT *st;
    MYSQL_BIND bind[1];
    int sum;
    
    st = mysql_stmt_init(conn);
    if(!st)
    {
       printf("%s\n",mysql_error(conn));
       return 0;
    }
    
    sprintf(buf, "select sum(price) from %s", tb);
    mysql_stmt_prepare(st, buf ,strlen(buf));

    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type=MYSQL_TYPE_LONG;
    bind[0].buffer=&sum;

    mysql_stmt_bind_result(st,bind);
    mysql_stmt_execute(st);
    mysql_stmt_store_result(st);
    
    if(!mysql_stmt_fetch(st))
    {
        mysql_stmt_close(st);
        return sum;
    }
    return -1;
}

