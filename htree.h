#ifndef __HTREE_H__
#define __HTREE_H__

#include <cstdlib>
#include <cstring>

#include <vector>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>


using namespace std;

// 确保能放下整个 int 数据（内存由外部控制）
// 13 = 32589, 17 => 543099
const int HashMask[12]  = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
const int NodeNum       = 37;

// node节点
template <class KEY = int, class VAL = int>
struct HashNode {
    KEY m_key;
    VAL m_value;
    
    bool m_occupied;         // 结点是否被占据，如果是：表示结点的关键字有效
    HashNode *child[NodeNum];// 结点的子结点数组

public:
    HashNode() {
        m_occupied = false;
        memset(child, 0, NodeNum * sizeof(HashNode*));
    }
    HashNode(KEY key, VAL value) {
        m_key = key;
        m_value = value;
        m_occupied = false;
        memset(child, 0, NodeNum * sizeof(HashNode*));
    }
};

// hash树
template <class KEY = int, class VAL = int>
class HashTree {
public:
    HashTree() {
        root = new HashNode<KEY,VAL>();
    }
    // 插入节点
    bool InsertNode(KEY key, VAL value) {
        return Insert(root, key, value, 0);
    }
    // 删除节点
    bool DeleteNode(KEY key) {
        return Delete(root, key, 0);
    }
    // 查找节点
    bool FindNode(KEY key, VAL &value) {
        return Find(root, key, value, 0);
    }

private:
    bool Insert(HashNode<KEY,VAL> *node, KEY key, VAL value, int level);
    bool Find(HashNode<KEY,VAL> *node, KEY key, VAL &value, int level);
    bool Delete(HashNode<KEY,VAL> *node, KEY key, int level);

    HashNode<KEY,VAL> *root;
};

template <class KEY, class VAL>
bool HashTree<KEY,VAL>::Insert(HashNode<KEY,VAL> *node, KEY key, VAL value, int level) {
    // 有空位
    if (node->m_occupied == false) {
        node->m_key = key;
        node->m_value = value;
        node->m_occupied = true;
        // cout << "Insert success:" << key << "|" << value << endl;
        return true;
    }

    if (level >= sizeof(HashMask)/sizeof(int)) {
        // 冲突...
        // cout << "Insert failed:" << key << endl;
        return false;
    }

    // 索引
    int index = key % HashMask[level];
    if (node->child[index] == NULL) {
        node->child[index] = new HashNode<KEY,VAL>();
    }
    
    Insert(node->child[index], key, value, ++level);
}

template <class KEY, class VAL>
bool HashTree<KEY,VAL>::Delete(HashNode<KEY,VAL> *node, KEY key, int level) {
    // 有节点
    if (node->m_occupied == true) {
        if (node->m_key == key) {
            node->m_occupied = false;
            // cout << "Delete success:" << key << endl;
            return true;
        }
    }

    if (level >= sizeof(HashMask)/sizeof(int)) {
        // 冲突...
        // cout << "Delete failed:" << key << endl;
        return false;
    }

    // 索引
    int index = key % HashMask[level];
    if (node->child[index] == NULL) {
        return false;
    }

    return Delete(node->child[index], key, level+1);
}

template <class KEY, class VAL>
bool HashTree<KEY,VAL>::Find(HashNode<KEY,VAL> *node, KEY key, VAL &value, int level) {
    // 有节点
    if (node->m_occupied == true) {
        if (node->m_key == key) {
            value = node->m_value;
            // cout << "Find success:" << key << "|" << value << endl;
            return true;
        }
    }
    
    if (level >= sizeof(HashMask)/sizeof(int)) {
        // 冲突...
        // cout << "Find failed:" << key << endl;
        return false;
    }

    // 索引
    int index = key % HashMask[level];
    if (node->child[index] == NULL) {
        return false;
    }

    return Find(node->child[index], key, value, level+1);
}

#endif

