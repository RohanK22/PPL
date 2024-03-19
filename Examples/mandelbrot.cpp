#include <iostream>
#include <vector>
#include "../Node.hpp"
#include "../FarmManager.hpp"
#include "../EasyBMP.hpp"

using namespace std;
using namespace EasyBMP;

class MandelbrotChunk {
public:
    int startRow;
    int endRow;
    int startCol;
    int endCol;
    int width;
    int height;
    double minReal;
    double maxReal;
    double minImag;
    double maxImag;
    int maxIterations;

    vector<vector<RGBColor> > colors;  // 2D array to store RGB colors

    MandelbrotChunk(int startRow, int endRow, int startCol, int endCol, int width, int height,
                    double minReal, double maxReal, double minImag, double maxImag, int maxIterations)
            : startRow(startRow), endRow(endRow), startCol(startCol), endCol(endCol),
              width(width), height(height), minReal(minReal), maxReal(maxReal),
              minImag(minImag), maxImag(maxImag), maxIterations(maxIterations) {
        // Initialize the 2D array with dimensions (width x height)
        int chunk_width = endCol - startCol;
        int chunk_height = endRow - startRow;
        colors.resize(chunk_width, vector<RGBColor>(chunk_height));
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
                    double zReal2 = zReal * zReal - zImag * zImag;
                    double zImag2 = 2 * zReal * zImag;

                    zReal = zReal2 + real;
                    zImag = zImag2 + imag;

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

class MandelbrotEmitter : public Node <void*> {
public:
    MandelbrotEmitter(int width, int height, int numRowChunks, int numColChunks, int maxIterations)
            : width(width), height(height), numRowChunks(numRowChunks), numColChunks(numColChunks),
              maxIterations(maxIterations) {}

    void* run(void* task) override {
        int rowsPerChunk = height / numRowChunks;
        int colsPerChunk = width / numColChunks;

        for (int i = 0; i < numRowChunks; i++) {
            for (int j = 0; j < numColChunks; j++) {
                int startRow = i * rowsPerChunk;
                int endRow = (i == numRowChunks - 1) ? height : startRow + rowsPerChunk;
                int startCol = j * colsPerChunk;
                int endCol = (j == numColChunks - 1) ? width : startCol + colsPerChunk;

                MandelbrotChunk* chunk = new MandelbrotChunk(startRow, endRow, startCol, endCol, width, height, -2, 1, -1, 1, maxIterations);
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
};

class MandelbrotWorker : public Node <void*> {
public:
    void* run(void* task) override {
//        cout << "Worker received chunk" << endl;
        MandelbrotChunk* chunk = (MandelbrotChunk*)task;

        // Populate the colors in the chunk
        chunk->populateColors();

        // Return the populated chunk
        return (void*)chunk;
    }
};

class MandelbrotCollector : public Node <void*> {
public:
    MandelbrotCollector(int totalWidth, int totalHeight, int numRowChunks, int numColChunks, const string& image_path)
            : totalWidth(totalWidth), totalHeight(totalHeight), numRowChunks(numRowChunks), numColChunks(numColChunks),
              image_path(image_path),
              finalImage(totalWidth, totalHeight, EasyBMP::RGBColor(255, 255, 255)) {}

    void* run(void* task) override {
        MandelbrotChunk* chunk = (MandelbrotChunk*)task;

//        cout << "Collector received chunk" << endl;

        // Copy colors from the chunk to the final image
        for (int x = chunk->startCol; x < chunk->endCol; x++) {
            for (int y = chunk->startRow; y < chunk->endRow; y++) {
                finalImage.SetPixel(x, y, chunk->colors[x - chunk->startCol][y - chunk->startRow]);
            }
        }

        received++;
//        cout << "Progress % " << (received * 100) / numChunks << endl;

        if (received == numChunks) {
//            cout << "Saving image to " << image_path << endl;
            finalImage.Write(image_path);

            return nullptr;
        }

        return (void*)chunk;
    }

private:
    int totalWidth;
    int totalHeight;
    int numRowChunks;
    int numColChunks;
    string image_path;
    EasyBMP::Image finalImage;
    int received = 0;
    int numChunks = numRowChunks * numColChunks;  // Assuming this variable is accessible here
};

int main(int argc, char* argv[]) {
    // Read width, height, maxIterations, numRowChunks, numColChunks, numWorkers from command line
    if (argc != 7) {
        cout << "Usage: ./mandelbrot <width> <height> <maxIterations> <numRowChunks> <numColChunks> <numWorkers>" << endl;
        return 1;
    }

    int width = stoi(argv[1]);
    int height = stoi(argv[2]);
    int maxIterations = stoi(argv[3]);
    int numRowChunks = stoi(argv[4]);
    int numColChunks = stoi(argv[5]);
    int numWorkers = stoi(argv[6]);

    string fname = "mandelbrot_" + to_string(width) + "x" + to_string(height) + ".bmp";

    FarmManager<void*> *farm = new FarmManager<void*>();
    MandelbrotEmitter *emitter = new MandelbrotEmitter(width, height, numRowChunks, numColChunks, maxIterations);
    MandelbrotCollector *collector = new MandelbrotCollector(width, height, numRowChunks, numColChunks, fname);

    farm->add_emitter(emitter);
    farm->add_collector(collector);

    for (int i = 0; i < numWorkers; i++) {
        MandelbrotWorker *worker = new MandelbrotWorker();
        farm->add_worker(worker);
    }

    farm->run_until_finish();

    return 0;
}
