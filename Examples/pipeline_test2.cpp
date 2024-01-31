// We will try to use the pipeline to process a bunch of text files and count the number of words in each file.

#include <iostream>
#include "../PipelineManager.hpp"
#include <filesystem>
#include <fstream>
#include <vector>
#include <unordered_map>

using namespace std;

// Reads a file and returns a vector of words for each file
class FileReader: public Node  {
public:
    FileReader(string file_directory_name) {
        this->file_directory_name = file_directory_name;

        // Set as emitter
        this->set_is_emitter(true);
    }

    void* run(void*) override {
        // Print out current directory
        cout << "Current directory: " << filesystem::current_path() << endl;
        for (const auto & entry : filesystem::directory_iterator(file_directory_name)) {
            string file_name = entry.path();
            cout << "Reading file: " << file_name << endl;
            vector<string> *words = new vector<string>();
            ifstream file(file_name);
            string word;
            while (file >> word) {
                words->push_back(word);
            }
            // Push to output queue
            this->get_output_queue()->push((void*) words);
        }
        return nullptr; // EOS
    }

private:
    string file_directory_name;
};

// Counts the frequency of each word
class WordCounter: public Node  {
public:
    void* run(void* task) override {
        vector<string> *words = (vector<string> *) task;
        unordered_map<string, int> *word_count = new unordered_map<string, int>();

        for (string word : *words) {
            if (word_count->find(word) == word_count->end()) {
                (*word_count)[word] = 1;
            } else {
                (*word_count)[word] += 1;
            }
        }
        return (void*) word_count;
    }
};

// Prints Top Three most frequent words
class PrintTopThree: public Node  {
public:
    void* run(void* task) override {
        unordered_map<string, int> *word_count = (unordered_map<string, int> *) task;
        vector<pair<string, int>> word_count_vector;
        for (auto & it : *word_count) {
            word_count_vector.push_back(it);
        }
        sort(word_count_vector.begin(), word_count_vector.end(), [](pair<string, int> &a, pair<string, int> &b) {
            return a.second > b.second;
        });
        cout << "Top Three Words for task " << receive_count << endl;
        for (int i = 0; i < 3; i++) {
            cout << word_count_vector[i].first << ": " << word_count_vector[i].second << endl;
        }
        receive_count++;
        return nullptr;
    }

private:
    int receive_count = 0;
};

int main() {
    PipelineManager *pipeline = new PipelineManager();
    FileReader *file_reader = new FileReader("../Examples/pipeline_test2_data");
    WordCounter *word_counter = new WordCounter();
    PrintTopThree *print_top_three = new PrintTopThree();

    pipeline->add_stage(file_reader);
    pipeline->add_stage(word_counter);
    pipeline->add_stage(print_top_three);

    cout << "Number of stages: " << pipeline->get_num_pipeline_stages() << endl;

    pipeline->run(nullptr);
    return 0;
}