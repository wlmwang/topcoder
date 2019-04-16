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














// vector<vector<int>> Connect(vector<vector<int>> Lv) {
//     vector<vector<int>> Ck;
//     vector<int> fir;
//     int num=0;
//     int length;
//     for (int i=0; i<Lv.size(); i++) {
//         for (int j=i+1; j<Lv.size(); j++) {
//             length = Lv[i].size();
//             if (length == 1)//即此时为频繁一项集
//             {
//                 fir.push_back(Lv[i][0]);//将元素两两合并
//                 fir.push_back(Lv[j][0]);
//                 Ck.push_back(fir);
//                 fir.clear();
//             } else {
//                 vector<int> res1;
//                 set_intersection(Lv[i].begin(),Lv[i].end()-1,Lv[j].begin(),Lv[j].end()-1,back_inserter(res1));//求交集
//                 if(res1.size() == length-1) {
//                     fir.assign(Lv[i].begin(), Lv[i].end());
//                     fir.push_back(Lv[j][length-1]);
//                     Ck.push_back(fir);//则项集合并
//                     fir.clear();
//                 }
//                 res1.clear();
//             }
//         }
//     }
//     return Ck;
// }



// int main() {
    
//     HashTree Hash[90000];   // 90000行

//     HashTree HashC1;        // 比对树（频繁集？）
//     vector<int> VC1;


//     ifstream file;
//     file.open("retail.dat");
//     if(!file)
//         cout<<"error"<<endl;

//     string line;
//     int TreeNum = 0, value = 0;
//     while(getline(file, line))   // 按行读取,遇到换行符结束
//     {
//         int temp, flag;

//         // 整行，空格分割
//         stringstream ss(line);
//         while(ss >> temp)    // 每次读取改行的一个数字
//         {
//             Hash[TreeNum].InsertNode(temp, 1);//每条交易记录对应一个HashTree
            
//             if (TreeNum == 0) {
//                 HashC1.InsertNode(temp, 1);
//                 VC1.push_back(temp);
//             }

//             if (TreeNum > 0) {
//                 if (HashC1.FindNode(temp, value)) {
//                     HashC1.DeleteNode(temp);
//                     HashC1.InsertNode(temp, value + 1);
//                 } else {
//                     HashC1.InsertNode(temp, 1);
//                     VC1.push_back(temp);
//                 }
//             }
//         }

//         TreeNum++;
//     }



//     //printf("请输入最小支持度：");
//     //cin>>minsup;

//     float flag;
//     flag = minsup * TreeNum;

//     vector<int> VL1;    // 超过阈值
//     vector<int> s1;
//     for (int i = 0; i < VC1.size(); i++) {
//         HashC1.FindNode(VC1[i], value);
        
//         if (value >= flag) {
//             VL1.push_back(VC1[i]);  // 用向量保存L1
//             value = 0;
//         }
//     }


//     sort(VL1.begin(), VL1.end()); //从小到大进行排序
    
//     int um = 0;
//     for(int n : VL1) {
//         HashC1.FindNode(n, um);
//         s1.push_back(um);
//     }

//     vector<vector<vector<int>>> result; //三维向量，保存结果的所有频繁项集
//     vector<vector<int>> second;     //二维临时向量
//     vector<int> first;  //一维临时向量
//     int temp=0;

//     for(int n:VL1)
//     {
//         first.push_back(n);
//         second.push_back(first);
//         first.clear();
//     }
//     result.push_back(second);
//     vector<vector<int>> Ck;
//     vector<vector<int>> Lk;
//     int k=0;
//     int Count;
//     int n,length;
//     vector<vector<int>> Sup;
//     Sup.push_back(s1);
//     vector<int> s;

//     while(result[k].size()>1)
//     {
//         Ck=Connect(result[k]);//二维向量
//         for(vector<int> temp:Ck)
//         {
//             Count=0;
//             length=temp.size();
//             for(int num=0; num<TreeNum; num++)
//             {
//                 for(n=0; n<length; n++)
//                 {
//                     if(!Hash[num].FindNode(temp[n],value))
//                         break;
//                 }
//                 if(n>=length)
//                     Count++;
//             }
//             if(Count>=flag)
//             {
//                 Lk.push_back(temp);//保存满足条件的Lk频繁项集
//                 s.push_back(Count);
//             }
//         }
//         Sup.push_back(s);
//         result.push_back(Lk);
//         k++;
//         s.clear();
//         Lk.clear();
//         Ck.clear();
//     }
//     //打印挖掘到的所有频繁项集
//     int num=0;
//     for(int i=0; i<result.size(); i++)
//     {
//         if(result[i].size()>=1)
//         {
//             printf("频繁%d项集：\n",i+1);
//             for(int j=0; j<result[i].size(); j++)
//             {
//                 int z;
//                 num++;
//                 printf("[{");
//                 for(z=0; z<result[i][j].size(); z++)
//                 {
//                     if(z==result[i][j].size()-1)
//                         printf("%d",result[i][j][z]);
//                     else
//                         printf("%d,",result[i][j][z]);
//                 }
//                 printf("}");
//                 printf(":%d] ",Sup[i][j]);
//             }
//             printf("\n");
//         }
//     }
//     printf("一共有%d个频繁项",num);
// }
//


#endif

