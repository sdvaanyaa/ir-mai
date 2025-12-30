#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;
using namespace std;

struct IndexPair {
    string term;
    int doc_id;
    bool operator<(const IndexPair& other) const {
        if (term != other.term) return term < other.term;
        return doc_id < other.doc_id;
    }
};

int main() {
    string stems_dir = "./data/stems";
    vector<IndexPair> pairs;
    vector<string> filenames;

    int doc_id = 0;
    for (const auto& entry : fs::directory_iterator(stems_dir)) {
        if (entry.path().extension() == ".txt") {
            filenames.push_back(entry.path().filename().string());
            ifstream file(entry.path());
            string word;
            while (file >> word) pairs.push_back({word, doc_id});
            doc_id++;
        }
    }

    sort(pairs.begin(), pairs.end());

    // Запись index.bin
    ofstream idx_file("index.bin", ios::binary);
    int unique_terms = 0;
    for (size_t i = 0; i < pairs.size(); ++i) 
        if (i == 0 || pairs[i].term != pairs[i-1].term) unique_terms++;
    
    idx_file.write((char*)&unique_terms, sizeof(int));

    for (size_t i = 0; i < pairs.size(); ) {
        string current_term = pairs[i].term;
        vector<int> ids;
        while (i < pairs.size() && pairs[i].term == current_term) {
            if (ids.empty() || ids.back() != pairs[i].doc_id) ids.push_back(pairs[i].doc_id);
            i++;
        }
        int len = current_term.length();
        int count = ids.size();
        idx_file.write((char*)&len, sizeof(int));
        idx_file.write(current_term.c_str(), len);
        idx_file.write((char*)&count, sizeof(int));
        idx_file.write((char*)ids.data(), sizeof(int) * count);
    }

    // Запись forward.bin
    ofstream fwd("forward.bin", ios::binary);
    int total_docs = filenames.size();
    fwd.write((char*)&total_docs, sizeof(int));
    for (int i = 0; i < total_docs; ++i) {
        int len = filenames[i].length();
        fwd.write((char*)&i, sizeof(int));
        fwd.write((char*)&len, sizeof(int));
        fwd.write(filenames[i].c_str(), len);
    }
    return 0;
}