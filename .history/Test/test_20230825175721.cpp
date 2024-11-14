#include <iostream>
#include <string>
#include <vector>

using namespace std;

class BoundQueryNode {
public:
    vector<string> names_;
    vector<string> types_;

    virtual string toString() = 0;
};

class BoundSelectNode : public BoundQueryNode {
public:
    vector<string> select_list_;

    string toString() override {
        string result_str;
        for (const auto& item : select_list_) {
            result_str += item;
        }
        return result_str;
    }
};

class BoundIntNode : public BoundQueryNode {
public:
    vector<int> int_list_;

    string toString() override {
        string result_str;
        for (const auto& item : int_list_) {
            result_str += to_string(item);
        }
        return result_str;
    }
};

int main() {
	auto node = new BoundSelectNode();
    node->select_list_ = { "A", "B", "C" };

	auto int_node= new BoundIntNode();
	int_node->int_list_= {1,2,3};

    BoundQueryNode* node1= (BoundQueryNode*)node;
    cout << node->toString() << endl;

	node1=(BoundQueryNode*)int_node;
	cout<< node->toString()<<endl<<endl;
    delete node;
    return 0;
}
