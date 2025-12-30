#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;
using namespace std;

int main() {
    string tokens_path = "./data/tokens";
    map<string, int> freq_map;

    cout << "[*] Начинаю подсчет слов для закона Ципфа..." << endl;

    for (const auto& entry : fs::directory_iterator(tokens_path)) {
        ifstream file(entry.path());
        string word;
        while (file >> word) {
            freq_map[word]++;
        }
    }

    vector<pair<int, string>> words_v;
    for (auto const& [word, freq] : freq_map) {
        words_v.push_back({freq, word});
    }

    sort(words_v.rbegin(), words_v.rend());

    ofstream out("zipf_results.csv");
    out << "Rank,Word,Frequency,Constant_C\n";
    
    for (int i = 0; i < words_v.size(); ++i) {
        long long rank = i + 1;
        long long freq = words_v[i].first;
        out << rank << "," << words_v[i].second << "," << freq << "," << (rank * freq) << "\n";
        
        if (i < 15) cout << rank << ". " << words_v[i].second << " (" << freq << ")" << endl;
    }

    cout << "[DONE] Данные сохранены в zipf_results.csv" << endl;
    return 0;
}