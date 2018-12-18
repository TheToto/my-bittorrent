#include <jansson.h>
#include <stdio.h>
#include <string.h>

#include "bencode.h"
#include "misc.h"
#include "parser.h"

static json_t *parse_node(struct be_node *be, struct be_node **info);

static json_t *parse_list(struct be_node **list, struct be_node **info)
{
    json_t *json = json_array();
    for (size_t i = 0; list[i]; i++)
    {
        json_t *j_value = parse_node(list[i], info);
        json_array_append_new(json, j_value);
    }
    return json;
}

static json_t *parse_str(struct be_string *str)
{
    char *buf = fix_string(str->content, str->length);
    json_t *ret = json_string(buf);
    free(buf);
    return ret;
}

static json_t *parse_int(long long int num)
{
    return json_integer(num);
}

static json_t *parse_dict(struct be_dict **dict, struct be_node **info)
{
    json_t *json = json_object();
    for (size_t i = 0; dict[i]; i++)
    {
        json_t *j_key = parse_str(dict[i]->key);
        json_t *j_value = parse_node(dict[i]->val, info);
        if (info && !strcmp(json_string_value(j_key), "info"))
            *info = dict[i]->val;
        json_object_set_new(json, json_string_value(j_key), j_value);
        json_decref(j_key);
    }
    return json;
}

static json_t *parse_node(struct be_node *be, struct be_node **info)
{
    switch (be->type)
    {
    case BE_STR:
        return parse_str(be->element.str);
    case BE_INT:
        return parse_int(be->element.num);
    case BE_LIST:
        return parse_list(be->element.list, info);
    case BE_DICT:
        return parse_dict(be->element.dict, info);
    }
    return NULL;
}

void free_json(json_t *json)
{
    json_decref(json);
}

json_t *to_json(struct be_node *be, struct be_node **info)
{
    json_t *json = parse_node(be, info);
    return json;
}
