#ifndef REDIS_CLUSTER_H
#define REDIS_CLUSTER_H

#include "cluster_library.h"
#include <php.h>
#include <stddef.h>

/* Redis cluster hash slots and N-1 which we'll use to find it */
#define REDIS_CLUSTER_SLOTS 16384
#define REDIS_CLUSTER_MOD   (REDIS_CLUSTER_SLOTS-1)

/* Get attached object context */
#define GET_CONTEXT() \
    (redisCluster*)zend_object_store_get_object(getThis() TSRMLS_CC)

/* Command building/processing is identical for every command */
#define CLUSTER_BUILD_CMD(name, c, cmd, cmd_len, slot) \
    redis_##name##_cmd(INTERNAL_FUNCTION_PARAM_PASSTHRU, c->flags, &cmd, \
                       &cmd_len, &slot)

/* Simple 1-1 command -> response macro */
#define CLUSTER_PROCESS_CMD(cmdname, resp_func) \
    redisCluster *c = GET_CONTEXT(); \
    char *cmd; \
    int cmd_len; \
    short slot; \
    if(redis_##cmdname##_cmd(INTERNAL_FUNCTION_PARAM_PASSTHRU,c->flags, &cmd, \
                             &cmd_len, &slot)==FAILURE) { \
        RETURN_FALSE; \
    } \
    if(cluster_send_command(c,slot,cmd,cmd_len TSRMLS_CC)<0 || c->err!=NULL) {\
        efree(cmd); \
        RETURN_FALSE; \
    } \
    efree(cmd); \
    resp_func(INTERNAL_FUNCTION_PARAM_PASSTHRU, c); 
        
/* More generic processing, where only the keyword differs */
#define CLUSTER_PROCESS_KW_CMD(cmdfunc, kw, resp_func) \
    redisCluster *c = GET_CONTEXT(); \
    char *cmd; int cmd_len; short slot; \
    if(cmdfunc(INTERNAL_FUNCTION_PARAM_PASSTHRU, c->flags, kw, &cmd, &cmd_len,\
               &slot)==FAILURE) { \
        RETURN_FALSE; \
    } \
    if(cluster_send_command(c,slot,cmd,cmd_len TSRMLS_CC)<0 || c->err!=NULL) { \
        efree(cmd); \
        RETURN_FALSE; \
    } \
    efree(cmd); \
    resp_func(INTERNAL_FUNCTION_PARAM_PASSTHRU, c); 

/* For the creation of RedisCluster specific exceptions */
PHPAPI zend_class_entry *rediscluster_get_exception_base(int root TSRMLS_DC);

/* Create cluster context */
zend_object_value create_cluster_context(zend_class_entry *class_type 
                                         TSRMLS_DC);

/* Free cluster context struct */
void free_cluster_context(void *object TSRMLS_DC);

/* Inittialize our class with PHP */
void init_rediscluster(TSRMLS_D);

/* RedisCluster method implementation */
PHP_METHOD(RedisCluster, __construct);
PHP_METHOD(RedisCluster, get);
PHP_METHOD(RedisCluster, set);
PHP_METHOD(RedisCluster, setex);
PHP_METHOD(RedisCluster, psetex);
PHP_METHOD(RedisCluster, setnx);
PHP_METHOD(RedisCluster, getset);
PHP_METHOD(RedisCluster, exists);

#endif
