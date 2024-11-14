### 230825 Binder select x from y
duckdb b binder.cpp:44
这里是bind的入口
create table y (x int, z int, a int, b int, c int);
select x from y;

_结构
    _Node
        _QueryNode:
            type_ QueryNodeType

            SelectNode:
                // select的列在这里
                select_list_ list[ParsedExpression] 

        _BoundQueryNode:
            names_ list[string]
            types_ list[LogicalType]

            BoundSelectNode:
                select_list_ list[Expression]



    _BaseExpression:
        // 原始的编译过的基本对象
        // bind前的表达式
        ParsedExpression:
            expression_class_ String
            // Constant
            
            ConstantExpression: ParsedExpression
                value_ Value
            
            BoundExpression: ParsedExpression

        // 放的都是bind后的表达式
        Expression:
            return_type_ LogicalType

            // bind后转换的基本对象
            BoundConstantExpression: Expression

            // select中的列, 比如 select x from y中的x
            BoundColumnRefExpression:
    
    // 解析前的table结构
    TableRef:

        BaseTableRef:

    // 解析过table后的结构
    BoundTableRef:

    BindResult:
        expression_ Expression
    
    _Catalog
        Catalog:

        CatalogEntry:
            StandardEntry:
                TableCatalogEntry:

        Binding:

    _SQLStatement:
        type_ StatementType
        SelectStatement:
            node_ SelectNode:
                select_list_ [ParsedExpression]

                from_table_ TableRef:// select from [from_table]
                    type_
                    table_name_ String // from_table

        BoundStatement:
            names_ list[string]
            types_ list[LogicalType]

            node_ BoundQueryNode:
                from_table_ BoundTableRef

            toString()

    ExpressionBinder:
        binder_ Binder:
            bind_context_ BindContext:

_状态变换
8

    SQLStatement:
        type_ StatementType
        SelectStatement:
            node_ SelectNode:
                select_list_ [ParsedExpression]

                from_table_ TableRef:// select from [from_table]
                    type_
                    table_name_ String // from_table


    SQLStatement:
        type_ StatementType
        SelectStatement:
            node_ SelectNode:
                2 
                select_list_ [Expression]
                3
                select_list_ [BoundExpression]
                5
                select_list_ [ColumnRefExpression]
                    BindResult(BoundColumnRefExpression())
                    BindResult:
                        expression_=BoundColumnRefExpression()
                    6
                    expr= BoundExpression()
                        expr_ BoundColumnRefExpression()::Expression

                    7
                    expr BoundExpression
                    result BoundColumnRefExpression()::Expression

                1 
                from_table_ BoundTableRef:// select from [from_table]
                    type_
                    table_name_ String // from_table


    ExpressionBinder:
        binder_ Binder:
            bind_context_ BindContext:
                8 添加binding
                bindings map<string, Binding>
                ~Binding:
                    names_map_ <string, int>


8 先添加binding
"y":{x:1, y:2, z:3}// create table y (x int, y int, z int)
9 根据列x获取binding
{x:1, y:2, z:3}
5
TableBinding:
    table_index= Binding.table_index // y对应的index ?? 这个如何生成
    column_index= 0// x对应的index

test_BindSelectFrom():
    tryBind(statement):
        // 区分不同类型的语句 select
        Binder.bindStatement(statement SQLStatement): BoundStatement
            bind(statement SelectStatement): BoundStatement
                // bind node_
                bind(statement.node_ =node QueryNode): result BoundStatement
                    bindNode(node QueryNode): result BoundQueryNode
                        -node.type_ == SELECT_NODE
                            // bind_select_node.cpp:330
                            bindNode(node SelectNode): result BoundQueryNode
                                // bind from table:341
                                1
                                $$result.from_table= bind(node.from_table ref TableRef): result BoundTableRef
                                    -ref.type_ == "BASE_TABLE"
                                        $$result = bind(ref BaseTableRef): BoundTableRef
                                            table_or_view= Catalog.getEntry(ref.table_name_)
                                            8 bind_basetableref.cpp:140
                                            $$bind_context_.addBaseTable():
                                                // alias y
                                                // names: {x,y,z} 列

                                                $$binding= TableBinding():

                                                // alias 表名 y
                                                // binding {x:1, y:2, z:3}
                                                addBinding(alias, binding)
                                            ret BoundTableRef(table_or_view)

                                2
                                // bind ParsedExpression:443
                                ExpressionBinder.bind(node.select_list_[i] ParsedExpression): expr Expression
                                result.select_list_.add(expr)

// bind ParsedExpression
$$ExpressionBinder.bind(expr ParsedExpression): result Expression
    3
    $$bind(expr ParsedExpression, depth int): expr BoundExpression
        $$bindExpression(expr ParsedExpression, depth int): expr BoundExpression
            -expr.expression_class_ == "COLUMN_REF":
                4
                // bind_columnref_expression.cpp:286
                $$result BindResult= bindExpression(expr ColumnRefExpression, depth): result BindResult
                    colref ColumnRefExpression = expr
                    $$result BindResult= binder_.bind_context_~BindContext.bindColumn(colref ColumnRefExpression, depth): BindResult
                        9
                        // 根据from_table找到表, 所以这里还是比较重要的
                        $$binding= colref.getBinding(colref.getTableName()):
                            ~getTableName():
                                ret column_names_[0]
                            ~getBinding(table_name):
                                match= binding_.find(table_name)
                                ret match.second


                        // 根据from_table找要查找的列
                        $$ret binding~TableBinding.Bind(colref, depth):
                            column_name String= colref.getColumnName() // "x"
                            // 这里返回"x"列对应的idx
                            column_index= tryGetBindingIndex(column_name):
                                {x:0, z:1, a:2, b:3, c:4}
                            binding ColumnBinding= getColumnBinding(column_index)
                            5
                            ret BindResult(BoundColumnRefExpression(colref.getName(), binding))
                6
                expr= BoundExpression(result.expression)
    7
    result Expression= expr.expr_

                        

### 230823 Binder select 1

_Node
    _QueryNode:
        type_ QueryNodeType

        SelectNode:
            // select的列在这里
            select_list_ list[ParsedExpression] 

    _BoundQueryNode:
        names_ list[string]
        types_ list[LogicalType]

        BoundSelectNode:
            select_list_ list[Expression]

_SQLStatement:
    type_ StatementType
    SelectStatement:
        node_ SelectNode
    
    BoundStatement:
        names_ list[string]
        types_ list[LogicalType]

        node_ BoundQueryNode

        toString()

_Expression
    // select中的列, 比如 select x from y中的x
    BoundColumnRefExpression:

    原始的编译过的基本对象
    _ParsedExpression:
        expression_class_ String
        // Constant
        
        ConstantExpression: ParsedExpression
            value_ Value
        
        BoundExpression: ParsedExpression

    _Expression:
        return_type_ LogicalType

        // bind后转换的基本对象
        BoundConstantExpression: Expression

BindResult:
    expression_ Expression

test_BindSelectValue():
    tryBind(statement):
        // 区分不同类型的语句 select
        Binder.bindStatement(statement SQLStatement):
            bind(statement SelectStatement):
                // bind node_
                bind(SelectStatement.node_):
                    bind(node QueryNode):
                        bindNode(node QueryNode):
                            -node.type_ == SELECT_NODE
                                bindNode(node SelectNode):
                                    // bind ParsedExpression
                                    ExpressionBinder.bind(node.select_list_[i] ParsedExpression)

// bind ParsedExpression
ExpressionBinder.bind(expr ParsedExpression):
    bind(expr ParsedExpression, depth int):
        bindExpression(expr ParsedExpression, depth int):
            -expr.expression_class_ == "Constant":
                bindExpression(expr ConstantExpression, depth):
                    ret BindResult(expr.value_)
