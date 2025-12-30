#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <stack>
#include <filesystem>

using namespace std;

struct PostingList {
    string term;
    vector<int> ids;
};

vector<PostingList> inv_idx;
vector<string> doc_names;

bool ends_with(wstring& str, wstring suffix) {
    return str.length() >= suffix.length() && str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

void stem_word(wstring& word) {
    vector<wstring> adjectival = {L"ими", L"ыми", L"его", L"ого", L"ему", L"ому", L"их", L"ых", L"ую", L"юю", L"ая", L"яя", L"ою", L"ею", L"ие", L"ые", L"ое", L"ей", L"ий", L"ый", L"ой", L"ем", L"им", L"ым", L"ом"};
    vector<wstring> noun = {L"иями", L"ями", L"ами", L"ией", L"иям", L"ием", L"ях", L"иях", L"ии", L"ий", L"ам", L"ом", L"а", L"е", L"и", L"й", L"о", L"у", L"ы", L"ь", L"ю", L"я"};
    for (auto& s : adjectival) if (ends_with(word, s)) { word.erase(word.length()-s.length()); return; }
    for (auto& s : noun) if (ends_with(word, s)) { word.erase(word.length()-s.length()); return; }
}

string get_stem(string word) {
    wstring wword = std::filesystem::path(word).wstring();
    if (wword.length() > 3) stem_word(wword);
    return std::filesystem::path(wword).string();
}

vector<int> op_and(const vector<int>& a, const vector<int>& b) {
    vector<int> res;
    set_intersection(a.begin(), a.end(), b.begin(), b.end(), back_inserter(res));
    return res;
}

vector<int> op_or(const vector<int>& a, const vector<int>& b) {
    vector<int> res;
    set_union(a.begin(), a.end(), b.begin(), b.end(), back_inserter(res));
    return res;
}

vector<int> op_not(const vector<int>& a) {
    vector<int> res;
    for (int i = 0; i < (int)doc_names.size(); ++i) {
        if (!binary_search(a.begin(), a.end(), i)) res.push_back(i);
    }
    return res;
}

void load_data() {
    ifstream fwd("forward.bin", ios::binary);
    if (!fwd) return;
    int total; fwd.read((char*)&total, sizeof(int));
    doc_names.resize(total);
    for (int i = 0; i < total; ++i) {
        int id, len; fwd.read((char*)&id, sizeof(int)); fwd.read((char*)&len, sizeof(int));
        char* b = new char[len+1]; fwd.read(b, len); b[len]='\0'; doc_names[id]=b; delete[] b;
    }
    ifstream idx("index.bin", ios::binary);
    if (!idx) return;
    int terms; idx.read((char*)&terms, sizeof(int));
    for (int i = 0; i < terms; ++i) {
        int len, count; idx.read((char*)&len, sizeof(int));
        char* b = new char[len+1]; idx.read(b, len); b[len]='\0';
        idx.read((char*)&count, sizeof(int));
        vector<int> ids(count); idx.read((char*)ids.data(), sizeof(int)*count);
        inv_idx.push_back({b, ids}); delete[] b;
    }
}

int get_priority(string op) { return op == "!" ? 3 : (op == "&&" ? 2 : (op == "||" ? 1 : 0)); }

void apply_op(stack<vector<int>>& vals, string op) {
    if (vals.empty()) return;
    if (op == "!") {
        auto v = vals.top(); vals.pop();
        vals.push(op_not(v));
    } else if (vals.size() >= 2) {
        auto r = vals.top(); vals.pop();
        auto l = vals.top(); vals.pop();
        if (op == "&&") vals.push(op_and(l, r)); 
        else if (op == "||") vals.push(op_or(l, r));
    }
}

vector<int> process_search(string q) {
    stack<vector<int>> vals; stack<string> ops;
    stringstream ss(q); string t;
    while (ss >> t) {
        if (t == "(") ops.push("(");
        else if (t == ")") {
            while (!ops.empty() && ops.top() != "(") { apply_op(vals, ops.top()); ops.pop(); }
            if (!ops.empty()) ops.pop();
        } else if (t == "&&" || t == "||" || t == "!") {
            while (!ops.empty() && get_priority(ops.top()) >= get_priority(t)) { apply_op(vals, ops.top()); ops.pop(); }
            ops.push(t);
        } else {
            string s = get_stem(t);
            auto it = lower_bound(inv_idx.begin(), inv_idx.end(), s, [](const PostingList& a, const string& b){ return a.term < b; });
            vals.push((it != inv_idx.end() && it->term == s) ? it->ids : vector<int>{});
        }
    }
    while (!ops.empty()) { apply_op(vals, ops.top()); ops.pop(); }
    return vals.empty() ? vector<int>{} : vals.top();
}

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    load_data();
    cout << "Index loaded. Terms: " << inv_idx.size() << endl;
    string q;
    while (cout << "Search > " && getline(cin, q) && q != "exit") {
        if (q.empty()) continue;
        auto res = process_search(q);
        cout << "Found " << res.size() << " docs:" << endl;
        for (int id : res) cout << "[" << id << "] " << doc_names[id] << endl;
    }
    return 0;
}