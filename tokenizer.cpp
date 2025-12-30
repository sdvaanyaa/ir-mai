#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;
using namespace std;

void to_lower_rus(string& s) {
    for (size_t i = 0; i < s.length(); i++) {
        unsigned char& c1 = (unsigned char&)s[i];
        
        if (c1 >= 'A' && c1 <= 'Z') {
            c1 += 32;
        } 
        else if (c1 == 0xD0 && i + 1 < s.length()) {
            unsigned char& c2 = (unsigned char&)s[i + 1];
            if (c2 >= 0x90 && c2 <= 0x9F) {
                c2 += 0x20;
            } 
            else if (c2 >= 0xA0 && c2 <= 0xAF) {
                c1 = 0xD1;
                c2 -= 0x20;
            }
            else if (c2 == 0x81) {
                c1 = 0xD1;
                c2 = 0x91;
            }
            i++; 
        }
    }
}

void clean_junk(string& str, const string& target) {
    size_t pos = 0;
    while ((pos = str.find(target, pos)) != string::npos) {
        str.replace(pos, target.length(), " ");
        pos += 1;
    }
}

bool is_split_char(unsigned char c) {
    string d = " \n\t\r.,!?:;()[]{}\"'`<>/|\\-=+_~@#$%^&*0123456789";
    return d.find(c) != string::npos;
}

int main() {
    string input_dir = "./data/articles";
    string output_dir = "./data/tokens";

    if (!fs::exists(output_dir)) fs::create_directories(output_dir);

    cout << "[SYSTEM] Начинаю чистую токенизацию..." << endl;
    int count = 0;

    for (const auto& entry : fs::directory_iterator(input_dir)) {
        if (entry.path().extension() == ".txt") {
            ifstream file(entry.path());
            string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            file.close();

            clean_junk(content, "—");
            clean_junk(content, "–");
            clean_junk(content, "«");
            clean_junk(content, "»");
            clean_junk(content, "…");

            string out_path = output_dir + "/" + entry.path().filename().string();
            ofstream outfile(out_path);
            
            string word = "";
            for (unsigned char c : content) {
                if (is_split_char(c)) {
                    if (word.length() > 2) { 
                        to_lower_rus(word);
                        outfile << word << " ";
                    }
                    word = "";
                } else {
                    word += c;
                }
            }
            if (word.length() > 2) {
                to_lower_rus(word);
                outfile << word;
            }
            
            count++;
            if (count % 100 == 0) cout << "Обработано файлов: " << count << endl;
        }
    }

    cout << "[DONE] Все готово. Файлов: " << count << endl;
    return 0;
}