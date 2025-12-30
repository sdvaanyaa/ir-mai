#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;
using namespace std;

bool ends_with(wstring& str, wstring suffix) {
    if (str.length() < suffix.length()) return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

void stem_word(wstring& word) {
    // 1. Окончания прилагательных
    vector<wstring> adjectival = {L"ими", L"ыми", L"его", L"ого", L"ему", L"ому", L"их", L"ых", L"ую", L"юю", L"ая", L"яя", L"ою", L"ею", L"ие", L"ые", L"ое", L"ей", L"ий", L"ый", L"ой", L"ем", L"им", L"ым", L"ом"};
    // 2. Окончания существительных
    vector<wstring> noun = {L"иями", L"ями", L"ами", L"ией", L"иям", L"ием", L"ях", L"иях", L"ии", L"ий", L"ам", L"ом", L"а", L"е", L"и", L"й", L"о", L"у", L"ы", L"ь", L"ю", L"я"};
    // 3. Глагольные окончания
    vector<wstring> verb = {L"ила", L"ыла", L"ена", L"ете", L"ите", L"или", L"ыли", L"но", L"ет", L"ют", L"ть", L"ешь", L"л", L"н"};

    for (const auto& s : adjectival) {
        if (ends_with(word, s)) {
            word.erase(word.length() - s.length());
            return;
        }
    }
    for (const auto& s : verb) {
        if (ends_with(word, s)) {
            word.erase(word.length() - s.length());
            return;
        }
    }
    for (const auto& s : noun) {
        if (ends_with(word, s)) {
            word.erase(word.length() - s.length());
            return;
        }
    }
}

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");

    string in_dir = "./data/tokens";
    string out_dir = "./data/stems";

    if (!fs::exists(out_dir)) fs::create_directories(out_dir);

    cout << "[*] Начинаю стемминг..." << endl;
    int file_count = 0;

    for (const auto& entry : fs::directory_iterator(in_dir)) {
        if (entry.path().extension() == ".txt") {
            ifstream in(entry.path());
            string word;
            ofstream out(out_dir + "/" + entry.path().filename().string());

            while (in >> word) {
                wstring wword = fs::path(word).wstring();
                
                if (wword.length() > 3) { 
                    stem_word(wword);
                }
                
                string stemmed = fs::path(wword).string();
                out << stemmed << " ";
            }
            
            file_count++;
            if (file_count % 500 == 0) cout << "Обработано файлов: " << file_count << endl;
        }
    }

    cout << "[DONE] Стемминг завершен. Результаты в " << out_dir << endl;
    return 0;
}