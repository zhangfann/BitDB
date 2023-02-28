# i: md_dir 笔记文件目录
# o: mysql
#python note_data.py
# 将文本笔记导入mysql 方便后续分析
#from model.peewee_models import *
import json
import os
if os.getenv('JARVIS_ENV')=='WSL':
    home_dir="/mnt/c/Users/zhangfan/Documents/workspace/Jarvis_200802/"
elif os.getenv('JARVIS_ENV')=='RONGPAN':
    home_dir="/mnt/d/zhangfan的文档/Jarvis_200802/"
elif os.getenv('JARVIS_ENV')=='MACOS':
    home_dir="/Users/zhangfan/Documents/Jarvis_200802/"
elif os.getenv('JARVIS_ENV')=='CENTOS':
    home_dir="/root/Jarvis_200802/"
else:
    home_dir=""

#class Data:
#    def __init__(self):
#        self.id=""
#        self.title=""
#        self.content=""


def like_list_get_from_file(md_file):
    return []

#markdown文件=>data_list:[{id,title,content}...]
#data_list->json文件
def to_json(md_dir):
#    md_file= home_dir+"210105-Notes/210517-Obsidian/zhangfan's note/home_archive/home_archive_2201_04.md"
    #unlike_list=ToutiaoToutiao.select().order_by(ToutiaoToutiao.create_time.desc())[:10000]
    like_list=like_list_get_from_file(md_dir)

#    i: list_list=[{id,title,content}...]
#   o:data_list=[{id,title,content}...]
    data_list=[]
    for item in like_list:
        tmp_dict={}
        tmp_dict['id']= item.id
        tmp_dict['title']= item.title
        tmp_dict['content']= item.content
        data_list.append(tmp_dict)

    #for item in unlike_list:
    #    tmp_dict={}
    #    tmp_dict['title']= item.title+ item.description
    #    tmp_dict['tag']= "None"
    #    data_list.append(tmp_dict)

    # for item in data_list:
    #     print(item)

    # dumps 输出到字符串
    # dump 输出到文件
    # json_str= json.dumps(data_list)
    # print(json_str)

    with open("./memo/memo.json", 'w') as f:
        json.dump(data_list, f)

    # 加载json字符串
    # with open('./data_list.json', 'r') as f:
    #     load_dict= json.load(f)

import json
def load_json():
    with open('./multiclass/multiclass.json', 'r') as f:
        json_str= json.load(f)
    return json_str
# /////////////////////////////////

def get_sharp_str(depth):
    res_str=""
    for i in range(depth+1):
        res_str=res_str+"#"
    res_str=res_str+" "
    return res_str


# i: markdown文件
# o:
def parse_file(md_file, file_name):
    md_memo_list=[]
    cur_idx=-1
    line_list=[]
    
    result={}
    result_list=[]
    
    title= file_name
#     print(title)
    result['title']=title
    
    with open(md_file, "r") as f:
        for line in f:
            line_list.append(line)
        

    content=""
    # 获取content
    for line in line_list:
        if line == None or line.isspace():
            continue
        else:
#                 line=line.lstrip(" ") # 去除开头空格
            if line[:2] == '# ':
                break
            else:
                content=content+line
#     print(content)
    result['content']=content
    # 获取nested_list 第2遍pass 分割子标题
    sub_index_list=[]
    for idx, line in enumerate(line_list):
        if line[:2] == '# ':
            sub_index_list.append(idx)
        else:
            continue
#     print(sub_index_list)
    sub_list_tuple_list=[]
    for idx, line_number in enumerate(sub_index_list):
        idx_tuple=()
#         print(idx, len(sub_index_list))
        if idx < len(sub_index_list)-1:
            
            idx_tuple=(line_number, sub_index_list[idx+1])
        else:
            idx_tuple=(line_number, len(line_list))
        sub_list_tuple_list.append(idx_tuple)
    sub_line_list=[]
    for idx_tuple in sub_list_tuple_list:
#         print(line_list[idx_tuple[0]: idx_tuple[1]])
        sub_line_list.append(line_list[idx_tuple[0]: idx_tuple[1]])
    # 分别parse其中内容
    sub_result_list=[]
    for line_list in sub_line_list:
        if line_list != []:
            sub_result= parse(line_list, 1)
            sub_result_list.append(sub_result)
            result_list= result_list+sub_result
    nested_list=[]
    for sub_result in sub_result_list:
        nested_res={}
#         print(sub_result)
        nested_res['title']=sub_result[-1]['title']
        nested_list.append(nested_res['title'])
    result['nested_list']=nested_list
    result_list.append(result)
#     print(result_list)

    return result_list


def parse(line_list, depth):
#     print("parse:", line_list, depth)
    result={}
    result_list=[]
#     print(line_list)
    result['title']=line_list[0]
    if len(line_list)==1:
        result['content']=""
        result['nested_list']=[]
        return result
    
    content=""
    # 获取content
    for line in line_list[1:]:
        if line == None or line.isspace():
            continue
        else:
#                 line=line.lstrip(" ") # 去除开头空格
            sharp_str=get_sharp_str(depth)
            if line[:depth+2] == sharp_str:
                break
            else:
                content=content+line
#     print(content)
    result['content']=content
    
    # 获取nested_list 第2遍pass 分割子标题
    sub_index_list=[]
    for idx, line in enumerate(line_list):
        sharp_str=get_sharp_str(depth)
        if line[:depth+2] == sharp_str:
#         if line[:2] == '# ':
            sub_index_list.append(idx)
        else:
            continue
#     print(sub_index_list)
    sub_list_tuple_list=[]
    for idx, line_number in enumerate(sub_index_list):
        idx_tuple=()
#         print(idx, len(sub_index_list))
        if idx < len(sub_index_list)-1:
            
            idx_tuple=(line_number, sub_index_list[idx+1])
        else:
            idx_tuple=(line_number, len(line_list))
        sub_list_tuple_list.append(idx_tuple)
    sub_line_list=[]
    for idx_tuple in sub_list_tuple_list:
#         print(line_list[idx_tuple[0]: idx_tuple[1]])
        sub_line_list.append(line_list[idx_tuple[0]: idx_tuple[1]])
    # 分别parse其中内容
    sub_result_list=[]
    for line_list in sub_line_list:
        if line_list != []:
            sub_result= parse(line_list, depth+1)
            sub_result_list.append(sub_result)
            result_list=result_list+sub_result
    nested_list=[]
    for sub_result in sub_result_list:
        nested_res={}
        nested_res['title']=sub_result[-1]['title']
        nested_list.append(nested_res['title'])
    result['nested_list']=nested_list
    result_list.append(result)
    return result_list
    
#     for line in line_list[1:]:
#         print(line)
        
#     nested_list=[]
#     return {"title":"title","content":"content", "nested_list":[]}
    

if __name__=="__main__":
#    md_file= home_dir+"210105-Notes/210517-Obsidian/zhangfan's note/home_archive/home_archive_2201_04.md"
    md_dir=home_dir+"190518-Enviroment"
#     parse_file(md_dir)

    # md_file= "/mnt/d/zhangfan的文档/Jarvis_200802/210105-Notes/200915-Vuepress/docs/index/推荐文章_210113.md"
    # like_list=like_list_get_from_file(md_file)
    # print(like_list)

