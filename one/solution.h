#ifndef __SOLUTION_H__
#define __SOLUTION_H__

#include <vector>
#include <string>
#include <set>
#include <unordered_set>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cstdarg>

using namespace std;


// -----------misc------------------

static inline void FallThroughIntended() { }

inline uint32_t DecodeFixed32(const char* ptr) {
    uint32_t result;
    memcpy(&result, ptr, sizeof(result));
    return result;
}

// 散列
uint32_t Hash(const char* data, uint32_t n, uint32_t seed) {
    const uint32_t m = 0xc6a4a793;
    const uint32_t r = 24;
    const char* limit = data + n;
    uint32_t h = seed ^ (n * m);

    // Pick up four bytes at a time
    while (data + 4 <= limit) {
        uint32_t w = DecodeFixed32(data);
        data += 4;
        h += w;
        h *= m;
        h ^= (h >> 16);
    }

    switch (limit - data) {
        case 3:
        h += static_cast<unsigned char>(data[2]) << 16;
        FallThroughIntended();
        
        case 2:
        h += static_cast<unsigned char>(data[1]) << 8;
        FallThroughIntended();

        case 1:
        h += static_cast<unsigned char>(data[0]);
        h *= m;
        h ^= (h >> r);
        break;
    }
    return h;
}

std::vector<std::string> SplitString(const std::string& src, const std::string& delim) {
    std::vector<std::string> dst;
    std::string::size_type pos1 = 0, pos2 = src.find(delim);
    while (std::string::npos != pos2) {
        dst.push_back(src.substr(pos1, pos2-pos1));

        pos1 = pos2 + delim.size();
        pos2 = src.find(delim, pos1);
    }
    if (pos1 != src.length()) {
        dst.push_back(src.substr(pos1));
    }
    return dst;
}
// -----------misc------------------


// ---------------htree.h-----------
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
    ~HashTree() {
        // todo release kv
        delete root;
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
    
    return Insert(node->child[index], key, value, ++level);
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
// ---------------htree.h-----------

// --------------idnode.h-------------
const int IdMaxLength = 16;
const int HashSeed = 100000;

enum Off_t { NUL_OFF, IDS_OFF, LIN_OFF};

// id结构(4+16)
class IdNode_t {
public:
    int32_t curoff;         // 当前节点在ids中偏移
    int32_t prevoff;        // 同行节点上一个key偏移（ids中）
    int32_t nextoff;        // 同行节点下一个key偏移（ids中）
    uint32_t hcode;         // hash值
    char id[IdMaxLength];   // id字符串

    IdNode_t(int32_t curr = 0) : curoff(curr), prevoff(-1), nextoff(-1), hcode(0) {
        memset(id, 0, sizeof(id));
    }
    // IdNode_t(const IdNode_t& rhs) {
    //  nextoff = rhs.nextoff;
    //  hcode = rhs.hcode;
    //  strncpy(id, rhs.id, sizeof(id));
    // }
    bool operator=(const string& k) {
        hcode = Hash(k.data(), static_cast<uint32_t>(k.size()), HashSeed); // 没有结尾\0
        strncpy(id, k.data(), sizeof(id));
        return true;
    }
    uint32_t operator%(uint32_t mask) {
        return hcode % mask;
    }

    bool operator==(const IdNode_t& rhs) {
        // todo 读文件
        return hcode == rhs.hcode && !strncmp(id, rhs.id, sizeof(id));
    }

    void SetCurOff(int32_t curr) {
        curoff = curr;
    }
    
    // bool AppendIdNode(int32_t o);
};

ostream& operator<<(ostream& os, const IdNode_t& v) {
    os << "#id:[ " << v.id << " ]#" << ":h:" << v.hcode << ":c:" << v.curoff << ":p:" << v.prevoff << ":n:" << v.nextoff;
    return os;
}

// 单行结构(4*5)
class LineNode_t {
public:
    int32_t nextline;       // 行下个偏移（lines中偏移）
    int32_t prevline;       // 行上个偏移（lines中偏移）
    
    int32_t lineno;
    int32_t curoff;         // 当前节点在lines中偏移

    // union {
    //  int32_t nextoff;    // 同行子节点（lines中偏移）
    //  int32_t idsoff;     // id集合首偏移（ids中偏移）
    // };
    int32_t nextoff;    // 同行子节点（lines中偏移）
    int32_t prevoff;    // 同行子节点（lines中偏移）
    
    int32_t idsoff;     // 指向在ids中首偏移
    
    Off_t off;

    LineNode_t(int32_t curr = 0) : lineno(-1), curoff(0),nextoff(-1), prevoff(-1), idsoff(-1), off(NUL_OFF) {
        curoff = curr;
    }
    
    void SetCurOff(int32_t curr, int32_t line = 0) {
        curoff = curr;
        lineno = line;
    }

    int32_t getLineNodeIdsVector(vector<string>& ids) {
        return 0;
    }

    // bool AppendIdNode(int32_t o);
    // bool AppendLineNode(int32_t o);
};

// 所有id集合(12+N)
class IdsNode_t {
public:
    int32_t curoff;
    int32_t begoff;
    int32_t endoff;

    IdNode_t *ids;      // ids集合

    IdsNode_t() : curoff(0), begoff(0), endoff(0) { }

    ~IdsNode_t() { delete [] ids;}

    int32_t getNewIdNodeOff();
};

int32_t IdsNode_t::getNewIdNodeOff() {
    // todo 写文件
    if (curoff < endoff) {
        ids[curoff].SetCurOff(curoff);
        return curoff++;
    }

    IdNode_t *newids = new IdNode_t[endoff+=4];
    if (!newids) {
        cout << "getNewIdNodeOff new" << endl;
    }

    if (ids) {
        memcpy(newids, ids, sizeof(IdNode_t)*(curoff-begoff));
        delete [] ids;
    }
    ids = newids;

    ids[curoff].SetCurOff(curoff);
    return curoff++;
}

// 所有行集合(12+N)
class LinesNode_t {
public:
    int32_t curoff;
    int32_t begoff;
    int32_t endoff;

    int32_t loopoff;

    LineNode_t *lines;  // lines集合

    LinesNode_t() : curoff(0), begoff(0), endoff(0), loopoff(0) { }

    ~LinesNode_t() {
        delete [] lines;
    }

    int32_t getNewLineNodeOff();
    int32_t getNextLineIds(vector<string>& ids);
};

int32_t LinesNode_t::getNewLineNodeOff() {
    // todo 写文件
    if (curoff < endoff) {
        lines[curoff].SetCurOff(curoff, curoff);
        return curoff++;
    }

    LineNode_t *newlines = new LineNode_t[endoff+=4];
    if (!newlines) {
        cout << "getNewLineNodeOff new" << endl;
    }

    if (lines) {
        memcpy(newlines, lines, sizeof(LineNode_t)*(curoff-begoff));
        delete [] lines;
    }
    lines = newlines;

    lines[curoff].SetCurOff(curoff, curoff);
    return curoff++;
}

// 数据容器
class LinesIdsContainer_t {
public:
    int32_t headline;
    int32_t currline;

    LinesNode_t lines;
    IdsNode_t ids;

    LinesIdsContainer_t();

    int32_t getNewLineNodeOff();
    int32_t getNewIdNodeOff();

    int32_t getNextLineIds(vector<string>& ids);

    void SetIdNodeKey(int32_t off, const string& k) {
        ids.ids[off] = k;
    }
    int32_t GetIdNodeHTreePos(int32_t off, int32_t mask) {
        return ids.ids[off] % mask;
    }
    bool CompareIdNodeKey(int32_t lft, int32_t rhs) {
        return ids.ids[lft] == ids.ids[rhs];
    }
};

LinesIdsContainer_t::LinesIdsContainer_t() : headline(0),currline(0) {
    // headline = lines.getNewLineNodeOff();
}

int32_t LinesIdsContainer_t::getNewLineNodeOff() {
    return lines.getNewLineNodeOff();
}

int32_t LinesIdsContainer_t::getNewIdNodeOff() {
    return ids.getNewIdNodeOff();
}

int32_t LinesIdsContainer_t::getNextLineIds(vector<string>& res) {
    return lines.getNextLineIds(res);
}

// 全局
LinesIdsContainer_t globalLinesIds;

ostream& operator<<(ostream& os, const LineNode_t& v) {
    os << "LineNode_t:" << v.lineno << "|";

    LineNode_t* lhead = globalLinesIds.lines.lines;
    LineNode_t *lptr = lhead + v.curoff;
    if (lptr->off == LIN_OFF) {
        while (lptr && lptr->nextoff != -1) {
            lptr = lhead + lptr->nextoff;
        }
    }

    if (lptr->off == LIN_OFF) {
        os << "ostream failed";
        return os;
    }

    // ids集合
    {
        IdNode_t *ihead = globalLinesIds.ids.ids;
        IdNode_t *iptr = ihead + lptr->idsoff;

        while (iptr) {
            os << *iptr << "|" << iptr->nextoff;
            
            if (iptr->nextoff == -1) {
                break;
            }
            iptr = ihead + iptr->nextoff;
        }
    }
    return os;
}

bool LineAppendKey(int32_t lineoff, int32_t idoff/*单个id，或者多个id的首节点偏移*/) {
    // line链表
    LineNode_t *lhead = globalLinesIds.lines.lines;
    LineNode_t *lptr = lhead + lineoff;
    if (lptr->off == LIN_OFF) {
        while (lptr && lptr->nextoff != -1) {
            lptr = lhead + lptr->nextoff;
        }
    }

    if (lptr->off == LIN_OFF) {
        cout << "LineMergeKey failed" << endl;
        return false;
    }

    // 插入首个id节点
    if (lptr->off == NUL_OFF) {
        lptr->off = IDS_OFF;
        lptr->idsoff = idoff;
        return true;
    }

    // id插入已有链表
    {
        IdNode_t *ihead = globalLinesIds.ids.ids;
        IdNode_t *iptr = ihead + lptr->idsoff;  // 行首个id
        IdNode_t *cptr = ihead + idoff;

        // 是否已在链表中
        if (iptr->curoff == idoff) {
            return true;
        }

        // 行尾
        while (iptr && iptr->nextoff != -1) {
            iptr = ihead + iptr->nextoff;
            // 是否已在链表中
            if (iptr->curoff == idoff) {
                return true;
            }
        }

        iptr->nextoff = cptr->curoff;
        cptr->prevoff = iptr->curoff;

        return true;
    }
}

// lhs 插入 rhs 后面
bool LineMergeLine(int32_t lhs, int32_t rhs) {
    std::set<int32_t> offv = {lhs}; // lhs 行链表所有偏移值（不能有环）

    LineNode_t *head = globalLinesIds.lines.lines;
    
    // 尾节点
    LineNode_t *left = head + lhs;

    if (left->off == LIN_OFF) {
        while (left && left->nextoff != -1) {
            left = head + left->nextoff;
            offv.insert(left->curoff);
        }
    }

    // 头节点
    LineNode_t *right = head + rhs;
    LineNode_t *right_next = right;

    if (offv.find(right->curoff) != offv.end()) {
        cout << "LineMergeLine exists(samp)" << endl;
        return true;
    }

    if (right->off == LIN_OFF) {
        {
            while (right && right->prevoff != -1) {
                right = head + right->prevoff;
                if (offv.find(right->curoff) != offv.end()) {
                    cout << "LineMergeLine exists(prevoff)!" << endl;
                    return true;
                }
            }
        }
        {
            while (right_next && right_next->nextoff != -1) {
                right_next = head + right_next->nextoff;
                if (offv.find(right_next->curoff) != offv.end()) {
                    cout << "LineMergeLine exists(nextoff)!" << endl;
                    return true;
                }
            }
        }
    }

    if (left->off == LIN_OFF || left->idsoff == -1) {
        cout << "LineMergeLine failed" << endl;
        return false;
    }

    // id 合并
    LineAppendKey(right->curoff, left->idsoff);

    // 行合并
    left->off = LIN_OFF;
    left->idsoff = -1;
    left->nextoff = right->curoff;
    right->prevoff = left->curoff;

    // todo 右边设置同行，提升性能
    // right->lineno = left->lineno;
    return true;
}

bool LineNodeIdsSet(const LineNode_t& v, unordered_set<string>& result) {
    LineNode_t* lhead = globalLinesIds.lines.lines;
    LineNode_t *lptr = lhead + v.curoff;

    if (lptr->off == LIN_OFF) {
        cout << "LineNodeIdsSet failed" << endl;
        return false;
    }

    // ids集合
    {
        IdNode_t *ihead = globalLinesIds.ids.ids;
        IdNode_t *iptr = ihead + lptr->idsoff;

        while (iptr) {
            // 添加集合
            result.insert(iptr->id);

            if (iptr->nextoff == -1) {
                break;
            }
            iptr = ihead + iptr->nextoff;
        }
    }

    return true;
}

int32_t LinesNode_t::getNextLineIds(vector<string>& ids) {
    unordered_set<string> idset;

    LineNode_t *head = globalLinesIds.lines.lines;
    LineNode_t *node = NULL;
    while (loopoff < curoff) {
        node = head + loopoff++;
        if (node->off == IDS_OFF) {
            if (LineNodeIdsSet(*node, idset)) {
                break;
            }
        }
    }

    std::copy(idset.begin(), idset.end(), std::back_inserter(ids));
    return static_cast<int32_t>(ids.size());
}

// ---------------HashTree---k:v------------
class HashKey {
public:
    int32_t idoff;  // 在ids中的偏移

    HashKey() : idoff(-1) { }
    HashKey(const HashKey& rhs) : idoff(rhs.idoff) { }

    // 创建一个新key（id节点）
    HashKey(const string& k) {
        idoff = globalLinesIds.getNewIdNodeOff();
        globalLinesIds.SetIdNodeKey(idoff, k);
    }

    int32_t operator%(int32_t mask/*质数*/) {
        return globalLinesIds.GetIdNodeHTreePos(idoff, mask);
    }

    bool operator==(const HashKey& rhs) {
        return globalLinesIds.CompareIdNodeKey(idoff, rhs.idoff);
    }
};

ostream& operator<<(ostream& os, const HashKey& k) {
    os << "HashKey:" << k.idoff << "|" << globalLinesIds.ids.ids[k.idoff];

    return os;
}

class HashVal {
public:
    int32_t lineoff;    // 在lines中的偏移

    HashVal() : lineoff(-1) { }
    HashVal(const HashVal& rhs) : lineoff(rhs.lineoff) { }

    // 将 key 添加到当前 HashVal 的 ids 链表中（可能会创建一个新的LineNode）
    HashVal& AppendKey(const HashKey& key) {
        if (key.idoff == -1) {
            cout << "AppendKey failed" << endl;
            return *this;
        }

        // 创建一个新的 LineNode_t
        if (lineoff == -1) {
            lineoff = globalLinesIds.getNewLineNodeOff();
        }

        // 单个 key 添加到行
        if (!LineAppendKey(lineoff, key.idoff)) {
            cout << "LineAppendKey failed" << endl;
        }
        return *this;
    }

    HashVal& AppendLine(const HashVal& val) {
        if (lineoff == -1 || val.lineoff == -1) {
            cout << "AppendLine failed" << endl;
            return *this;
        }

        if (!LineMergeLine(lineoff, val.lineoff)) {
            cout << "LineMergeLine failed" << endl;
        }
        return *this;
    }
};

ostream& operator<<(ostream& os, const HashVal& v) {
    os << "HashVal:" << v.lineoff << "|" << globalLinesIds.lines.lines[v.lineoff];
    return os;
}
// --------------idnode.h-------------


/// Output one id set. Call this method in process() to output your result.
/// @param result id vector, can be unordered.
void addSet(const std::vector<std::string> &result);

class Solution {
public:
    /// Process the input file.
    /// @note You must implement this function.
    /// @param inputPath path to input file
    void process(const std::string &inputPath);

    /// Initialize.
    /// @param tempDir path to temparory directory
    void init(const std::string &tempDir);

private:
    // you can add your function here
    void handleRow();
    void outputRow();
private:
    // Path to temporary directory, in which you can write and read temp files.
    std::string m_tempDir;

    // you can add your data-structure here
    // ...

    int m_lineNo;
    vector<string> m_oneRow;
};

inline void Solution::init(const std::string &tempDir)
{
    m_tempDir = tempDir;

    // You can initialize your data structures here
    // ...

    m_lineNo = 0;
    m_oneRow.reserve(2);
}

/// Process the input file.
/// @note You must implement this function.
/// @param inputPath path to input file
inline void Solution::process(const std::string &inputPath) {
    // 文件读取
    ifstream file(inputPath, ios::in);
    if (!file.is_open()) {
        cout << "Error opening file" << endl;
        exit (1);
    }

    string buffer;
    while (getline(file, buffer)) {
        m_oneRow = SplitString(buffer, ",");
        m_lineNo++;

        handleRow();
    }
    outputRow();
}

extern LinesIdsContainer_t globalLinesIds;
HashTree<HashKey, HashVal> globalIdsMap;

void Solution::handleRow() {
    
    int repeatCount = 0;
    vector<HashKey> repeatKey(0); vector<HashVal> repeatVal(1);

    int addCount = 0;
    vector<HashKey> addKey(0); vector<HashVal> addVal(1);

    for (vector<string>::iterator it = m_oneRow.begin(); it != m_oneRow.end(); ++it) {
        if (it->empty()) {
            continue;
        }
        addKey.push_back(HashKey(*it));
        addVal[addCount] = addVal[0].AppendKey(addKey[addCount]);   // 合并到行首

        if (globalIdsMap.FindNode(addKey[addCount], repeatVal[repeatCount])) {
            repeatKey.push_back(addKey[addCount]);
            repeatVal.resize((++repeatCount)+1);
        }
        globalIdsMap.InsertNode(addKey[addCount], addVal[addCount]);
        
        addVal.resize((++addCount)+1);
    }

    if (!repeatCount) {
        return;
    }

    HashVal collectVal = repeatVal[0];  // 主合并行
    
    // 合并重复行
    for (int r = 1; r < repeatCount; r++) {
        collectVal.AppendLine(repeatVal[r]);
    }

    // 合并本行
    collectVal.AppendLine(addVal[0]);
}

void Solution::outputRow() {
    vector<string> ids;

    globalLinesIds.getNextLineIds(ids);
    while (globalLinesIds.getNextLineIds(ids) > 0) {
        
        addSet(ids);
        
        ids.resize(0);
    }
}

#endif // __SOLUTION_H__


