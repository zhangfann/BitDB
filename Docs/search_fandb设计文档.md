主要用于django的后端搜索服务

# 流程
## server
	get_list
		i:depth 第几层
		o:[Markdown:{id, title}...]
		select id, title from markdown where depth = @depth
		
	search
		cpp.搜索(query)
			Done query分词=>[word]
				Pending@client
			result_set=set()
			for word:[word]
				查询db
				=>result=[id1, id2...]
				set交集(result_set, result)
			=>set交集:[id1, id2...]

## client
###	parse_file
		i: md_file 文件路径, file_name 文件名
		result.title= file_name
		result.depth= 0
		result_list=[]
		; 获取content
		=>result.content
			for line in line_list[1...]
				-line不是markdown开头
					content.append(line)
				-line是markdown开头
					break
		; 获取nested_list 第2遍pass 分割子标题
		=> sub_line_list[#11_line_list, #12_line_list, ...]
			sub_index_list=[]
			for index, line in line_list[1...]
				-line不是markdown开头
					continue
				-line是markdown开头
					-是depth+1对应markdown
						sub_index_list.append(index)
					-不是depth对应markdown
						continue
		; 分别parse其中内容
			sub_result_list=[]
			for line_list in sub_line_list:
				sub_result=parse(line_list, depth+1)
				=>[#1:{title, 
						content, 
						nested_list:[#depth+1.1.title, #depth+1.2.title, ...#depth+1.n.title]}
					...]
				sub_result_list.append(sub_result)
			for sub_result in sub_result_list:
				nested_list.title= sub_result.title
			
			
###	 parse
		req
			line_list
				从markdown标题对应的第一行开始 到最后一行结束
				exp:
					0 #1
					1 aaa
					2 bbb
					3 #2
					line_list为[0,2]
			depth>=1 至少从第一层目录开始
		i: line_list 文本, depth markdown标题深度
		o: [#1:{title, 
				content, 
				nested_list:[#depth+1.1.title, #depth+1.2.title, ...#depth+1.n.title]}
			...]
		title , content , Done nested_list
		; 取第一行为title
			title= line_list[0]
			result.depth=depth
		; 第1遍pass 找content
		for line in line_list[1...]
			-line不是markdown开头
				content.append(line)
			-line是markdown开头
				break
		; 第2遍pass 分割子标题
		sub_index_list=[]
		for index, line in line_list[1...]
			-line不是markdown开头
				continue
			-line是markdown开头
				-是depth+1对应markdown
					sub_index_list.append(index)
				-不是depth对应markdown
					continue
		=> [#11index, #12index, ...]
		->sub_line_list:[[#11_index, #12index), [#12_index, #13index)...]
		sub_result_list=[]
		for sub_line in sub_line_list:
			parse(sub_line, depth+1)
			=>sub_result=[#11:{id, title, content, nested_list:[#depth+1.1.title, #depth+1.2.title, ...#depth+1.n.title]}...]
			nested_list.append(#11.title, #11.id)
			sub_result_list.append(sub_result)
		sub_result_list.append

	; parse2 
		req
			depth>=1 至少从第一层目录开始
		i: line_list 文本, depth markdown标题深度
		o: [#1:{title, content, nested_list:[#depth+1.1.title, #depth+1.2.title, ...#depth+1.n.title]}...]
		??title ,Done content , Done nested_list
		; 第一遍pass
		for line in line_list
			-line不是markdown开头
				content.append(line)
			-line是markdown开头
				-是depth对应markdown
					title
				-不是depth对应markdown
					parse(剩余内容, depth+1)
					=>[#11:{title, content, nested_list:[#depth+1.1.title, #depth+1.2.title, ...#depth+1.n.title]}...]
					nested_list=[#11.title, #12.title...]


	; parse函数1

		parse(text, depth)
			i: text 文本, depth markdown标题深度
			o: parsed_list:[#1:{title, content, nested_list}, #2, ...#n]
		parse #1 ;分析第1层
		=〉parsed_#1_list:[#1, #2, ...#n]
		for #1 in parsed_#1_list:
			title= #1.title
			content= #1.content
			nested_list=[]
			parse #11 ;分析第2层
			=〉parsed_#11_list:[#11, #12, ...#1n]
			nested_list= parsed_#11_list.title

	; parse算法
		-普通文字
			加入内容
		-下层markdown标题
			继续递归添加

		#t1
		#t2
		##t2.1
		##t2.2
		#t3
		##t3.1
		###t3.1.1
		=> 上层目录能包含下层目录的内容
		[
			{t1}
			{t2,[{t2.1}
				{t2.2}]}
			{t2.1}
			{t2.2}
			{t3, [{t3.1}
				{t3.1.1}]}
			{t3.1, [{t3.1.1}]}
			{t3.1.1}
		]

	; 整体流程
		爬取env目录下的文章
			cpp中文章安装小标题拆分
		->db

# 日志
## 220701 ??使用自研kv替代mysql
整个架构比较类似运维审计系统
这里面是一个client收集数据到mysql 一个server从mysql读取数据上报
?? 如果我自己写一个kv 能满足这种需求吗