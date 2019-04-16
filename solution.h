#ifndef __SOLUTION_H__
#define __SOLUTION_H__

#include <vector>
#include <string>

#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdint>

#include "hash.h"
#include "htree.h"
#include "idnode.h"

using namespace std;

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

    // 输出
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
