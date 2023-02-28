# @处理特征
# 分词
def remove_punctuation(line):
    line = str(line)
    if line.strip()=='':
        return ''
    rule = re.compile(u"[^a-zA-Z0-9\u4E00-\u9FA5]")
    line = rule.sub('',line)
    return line
 
def stopwordslist(filepath):  
    stopwords = [line.strip() for line in open(filepath, 'r', encoding='utf-8').readlines()]  
    return stopwords  
 
# 加载停用词
stopwords = stopwordslist("./chineseStopWords.txt")
# 清除停顿词
import re
#data_frame['clean_title']= data_frame['title'].apply(remove_punctuation)

# 分词
import jieba as jb
def cut_words(line):
    seg_list= list(jb.cut(line))
    return " ".join([word for word in seg_list if word not in stopwords])

import sys
if __name__ == "__main__":
    #print(sys.argv)
    query_str= sys.argv[1]
    query_str= remove_punctuation(query_str)
    result_str=cut_words(query_str)
    print(result_str)
