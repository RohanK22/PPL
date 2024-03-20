#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

unordered_map<string, int> countWords(const vector<string>& words) {
    unordered_map<string, int> word_count;
    for (const string& word : words) {
        word_count[word]++;
    }
    return word_count;
}

// Function to print the top three most frequent words
void printTopThree(const unordered_map<string, int>& word_count) {
    vector<pair<string, int>> word_count_vector(word_count.begin(), word_count.end());
    sort(word_count_vector.begin(), word_count_vector.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
        return a.second > b.second;
    });

    /*
    cout << "Top Three Words:" << endl;
    for (int i = 0; i < 3 && i < word_count_vector.size(); ++i) {
        cout << word_count_vector[i].first << ": " << word_count_vector[i].second << endl;
    }*/
}

// Function to remove stop words from the vector of words
vector<string> removeStopWords(const vector<string>& words) {
    unordered_set<string> stop_words = {"i", "my", "that", "this", "a", "an", "the", "in", "on", "at", "to", "of", "by", "I", "you", "he", "she", "it", "we", "they", "is", "are", "am", "was", "were", "be", "been", "being", "have", "has", "had", "do", "does", "did", "will", "shall", "would", "should", "can", "could", "may", "might", "must", "here", "there", "where", "when", "why", "how", "what", "which", "who", "whom", "whose", "and", "or", "but", "not", "no", "yes", "so", "if", "then", "else", "when", "while", "until", "before", "after", "because", "since", "although", "though", "even", "as", "if", "whether", "either", "neither", "both", "each", "every", "any", "all", "some", "most", "many", "few", "several", "such", "own", "other", "another", "more", "less", "least", "only", "very", "much", "more", "most", "little", "least", "fewest", "many", "few", "much"};
    vector<string> filtered_words;
    for (const string& word : words) {
        // Lowercase the word
        string lowercase_word = word;
        transform(lowercase_word.begin(), lowercase_word.end(), lowercase_word.begin(), ::tolower);
        if (stop_words.find(lowercase_word) == stop_words.end()) {
            filtered_words.push_back(word);
        }
    }
    return filtered_words;
}

int main() {
    string directory_path = "../Node_Examples/Pipeline/text_data/";

    // Check if the directory exists
    if (!fs::exists(directory_path) || !fs::is_directory(directory_path)) {
        cout << "Invalid directory path." << endl;
        return 1;
    }

    // Iterate over each file in the directory
    for (const auto& entry : fs::directory_iterator(directory_path)) {
        if (fs::is_regular_file(entry)) {
            string file_name = entry.path();
            //cout << "Processing file: " << file_name << endl;

            // Get vector of words
            vector<string> words;
            ifstream file(file_name);
            string word;
            while (file >> word) {
                words.push_back(word);
            }

            // Remove stop words
            vector<string> filtered_words = removeStopWords(words);

            // Count words
            unordered_map<string, int> word_count = countWords(filtered_words);

            // Print top three words
            printTopThree(word_count);
        }
    }

    return 0;
}
