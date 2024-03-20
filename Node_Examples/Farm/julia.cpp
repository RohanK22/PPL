#include <iostream>
#include <vector>
#include "../../src/Node.hpp"
#include "../../src/FarmManager.hpp"
#include "../../libs/EasyBMP.hpp"

using namespace std;
using namespace EasyBMP;

class JuliaSetChunk {
public:
    int startRow;
    int endRow;
    int startCol;
    int endCol;
    int width;
    int height;
    int maxIterations;
    double minReal;
    double maxReal;
    double minImag;
    double maxImag;
    double cReal;
    double cImag;

    vector<vector<RGBColor>> colors;  // 2D array to store RGB colors

    JuliaSetChunk(int startRow, int endRow, int startCol, int endCol, int width, int height,
                  int maxIterations, double minReal, double maxReal, double minImag, double maxImag, double cReal, double cImag)
            : startRow(startRow), endRow(endRow), startCol(startCol), endCol(endCol),
              width(width), height(height), maxIterations(maxIterations), minReal(minReal), maxReal(maxReal),
              minImag(minImag), maxImag(maxImag), cReal(cReal), cImag(cImag) {
        // Initialize the 2D array with dimensions (width x height)
        colors.resize(width, vector<RGBColor>(height, RGBColor(255, 255, 255)));
    }

    void populateColors() {
        for (int x = startCol; x < endCol; x++) {
            for (int y = startRow; y < endRow; y++) {
                double real = minReal + x * (maxReal - minReal) / width;
                double imag = minImag + y * (maxImag - minImag) / height;

                double zReal = real;
                double zImag = imag;

                int n;
                for (n = 0; n < maxIterations; n++) {
                    double zReal2 = zReal * zReal - zImag * zImag + cReal;
                    double zImag2 = 2 * zReal * zImag + cImag;

                    zReal = zReal2;
                    zImag = zImag2;

                    if (zReal * zReal + zImag * zImag > 4) {
                        break;
                    }
                }

                float f = 2.05;
                int red = (n) % 256;
                int green = int(n * f) % 256;
                int blue = int(n * f * f) % 256;

                colors[x - startCol][y - startRow] = RGBColor(red, green, blue);
            }
        }
    }
};

class JuliaSetEmitter : public Node {
public:
    JuliaSetEmitter(int width, int height, int numRowChunks, int numColChunks, int maxIterations,
                    double minReal, double maxReal, double minImag, double maxImag, double cReal, double cImag)
            : width(width), height(height), numRowChunks(numRowChunks), numColChunks(numColChunks),
              maxIterations(maxIterations), minReal(minReal), maxReal(maxReal), minImag(minImag),
              maxImag(maxImag), cReal(cReal), cImag(cImag) {}

    void* run(void* task) override {
        int rowsPerChunk = height / numRowChunks;
        int colsPerChunk = width / numColChunks;

        for (int i = 0; i < numRowChunks; i++) {
            for (int j = 0; j < numColChunks; j++) {
                int startRow = i * rowsPerChunk;
                int endRow = (i == numRowChunks - 1) ? height : startRow + rowsPerChunk;
                int startCol = j * colsPerChunk;
                int endCol = (j == numColChunks - 1) ? width : startCol + colsPerChunk;

                JuliaSetChunk* chunk = new JuliaSetChunk(startRow, endRow, startCol, endCol, width, height,
                                                         maxIterations, minReal, maxReal, minImag, maxImag, cReal, cImag);
                this->get_output_queue()->push((void*)chunk);
            }
        }

        return nullptr;
    }

private:
    int width;
    int height;
    int numRowChunks;
    int numColChunks;
    int maxIterations;
    double minReal;
    double maxReal;
    double minImag;
    double maxImag;
    double cReal;
    double cImag;
};

class JuliaSetWorker : public Node {
public:
    void* run(void* task) override {
        cout << "Worker received chunk" << endl;
        JuliaSetChunk* chunk = (JuliaSetChunk*)task;

        // Populate the colors in the chunk
        chunk->populateColors();

        // Return the populated chunk
        return (void*)chunk;
    }
};

class JuliaSetCollector : public Node {
public:
    JuliaSetCollector(int totalWidth, int totalHeight, int numRowChunks, int numColChunks, const string& image_path)
            : totalWidth(totalWidth), totalHeight(totalHeight), numRowChunks(numRowChunks), numColChunks(numColChunks),
              image_path(image_path),
              finalImage(totalWidth, totalHeight, RGBColor(255, 255, 255)) {}

    void* run(void* task) override {
        JuliaSetChunk* chunk = (JuliaSetChunk*)task;

        cout << "Collector received chunk" << endl;

        // Copy colors from the chunk to the final image
        for (int x = chunk->startCol; x < chunk->endCol; x++) {
            for (int y = chunk->startRow; y < chunk->endRow; y++) {
                finalImage.SetPixel(x, y, chunk->colors[x - chunk->startCol][y - chunk->startRow]);
            }
        }

        received++;
        cout << "Progress % " << (received * 100) / numChunks << endl;

        if (received == numChunks) {
            cout << "Saving image to " << image_path << endl;
            finalImage.Write(image_path);
        }

        return nullptr;
    }

private:
    int totalWidth;
    int totalHeight;
    int numRowChunks;
    int numColChunks;
    string image_path;
    Image finalImage;
    int received = 0;
    int numChunks = numRowChunks * numColChunks;  // Assuming this variable is accessible here
};

int main(int argc, char* argv[]) {
    // Read parameters from the command line
    if (argc != 7) {
        cout << "Usage: ./julia_set <width> <height> <maxIterations> <numRowChunks> <numColChunks> <numWorkers>" << endl;
        return 1;
    }

    int width = stoi(argv[1]);
    int height = stoi(argv[2]);
    int maxIterations = stoi(argv[3]);
    int numRowChunks = stoi(argv[4]);
    int numColChunks = stoi(argv[5]);
    int numWorkers = stoi(argv[6]);
    double cReal = -0.7;
    double cImag = 0.27015;
    string image_path = "julia_" + to_string(width) + "x" + to_string(height) + ".bmp";

    FarmManager* farm = new FarmManager();
    JuliaSetEmitter* emitter = new JuliaSetEmitter(width, height, numRowChunks, numColChunks, maxIterations, -2, 2, -1.5, 1.5, cReal, cImag);
    JuliaSetCollector* collector = new JuliaSetCollector(width, height, numRowChunks, numColChunks, image_path);

    farm->add_emitter(emitter);
    farm->add_collector(collector);

    for (int i = 0; i < numWorkers; i++) {
        JuliaSetWorker* worker = new JuliaSetWorker();
        farm->add_worker(worker);
    }

    farm->run(nullptr);

    return 0;
}
