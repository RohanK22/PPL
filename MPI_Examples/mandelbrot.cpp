#include <iostream>
#include <vector>
#include "../MPINode.hpp"
#include "../MPIFarmManager.hpp"
#include "../EasyBMP.hpp"
#include <boost/mpi.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>
#include <string>

using namespace std;
using namespace EasyBMP;
namespace mpi = boost::mpi;

// Make this serializable
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

    // Default Constructor
    MandelbrotChunk() = default;

    MandelbrotChunk(int startRow, int endRow, int startCol, int endCol, int width, int height,
                    double minReal, double maxReal, double minImag, double maxImag, int maxIterations)
            : startRow(startRow), endRow(endRow), startCol(startCol), endCol(endCol),
              width(width), height(height), minReal(minReal), maxReal(maxReal),
              minImag(minImag), maxImag(maxImag), maxIterations(maxIterations) {
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

    // Use boost serialization to serialize the object
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & startRow;
        ar & endRow;
        ar & startCol;
        ar & endCol;
        ar & width;
        ar & height;
        ar & minReal;
        ar & maxReal;
        ar & minImag;
        ar & maxImag;
        ar & maxIterations;
        ar & colors;
    }
};

class MandelbrotEmitter : public MPINode {
public:
    MandelbrotEmitter(int width, int height, int numRowChunks, int numColChunks, int maxIterations)
            : width(width), height(height), numRowChunks(numRowChunks), numColChunks(numColChunks),
              maxIterations(maxIterations) {
        int rowsPerChunk = height / numRowChunks;
        int colsPerChunk = width / numColChunks;

        for (int i = 0; i < numRowChunks; i++) {
            for (int j = 0; j < numColChunks; j++) {
                int startRow = i * rowsPerChunk;
                int endRow = (i == numRowChunks - 1) ? height : startRow + rowsPerChunk;
                int startCol = j * colsPerChunk;
                int endCol = (j == numColChunks - 1) ? width : startCol + colsPerChunk;

                chunks.push_back(MandelbrotChunk(startRow, endRow, startCol, endCol, width, height, -2, 1, -1, 1, maxIterations));
            }
        }
    }

    string run(string _) override {
        if (chunks_sent < numChunks) {
            MandelbrotChunk chunk = chunks[chunks_sent];
            chunks_sent++;

            // Serialize the chunk as string and return
            stringstream ss;
            {
                boost::archive::text_oarchive oa(ss);
                oa << chunk;
            }

            return ss.str();
        }
        return string("EOS");
    }

private:
    int width;
    int height;
    int numRowChunks;
    int numColChunks;
    int maxIterations;

    int chunks_sent = 0;
    int numChunks = numRowChunks * numColChunks;

    // Store chunks
    vector<MandelbrotChunk> chunks;
};

class MandelbrotWorker : public MPINode {
public:
    string run(string task) override {
        MandelbrotChunk chunk;
        stringstream ss(task);
        {
            boost::archive::text_iarchive ia(ss);
            ia >> chunk;
        }

        // Populate the colors in the chunk
        chunk.populateColors();

        // Serialize the chunk as string and return
        stringstream ss2;
        {
            boost::archive::text_oarchive oa(ss2);
            oa << chunk;
        }
        return ss2.str();
    }
};

class MandelbrotCollector : public MPINode {
public:
    MandelbrotCollector(int totalWidth, int totalHeight, int numRowChunks, int numColChunks, const string& image_path)
            : totalWidth(totalWidth), totalHeight(totalHeight), numRowChunks(numRowChunks), numColChunks(numColChunks),
              image_path(image_path),
              finalImage(totalWidth, totalHeight, EasyBMP::RGBColor(255, 255, 255)) {}

    string run(string task) override {
        MandelbrotChunk chunk;
        stringstream ss(task);
        {
            boost::archive::text_iarchive ia(ss);
            ia >> chunk;
        }

//        cout << "Collector received chunk" << endl;

        // Copy colors from the chunk to the final image
        for (int x = chunk.startCol; x < chunk.endCol; x++) {
            for (int y = chunk.startRow; y < chunk.endRow; y++) {
                finalImage.SetPixel(x, y, chunk.colors[x - chunk.startCol][y - chunk.startRow]);
            }
        }

        received++;
        cout << "Progress % " << (received * 100) / numChunks << endl;

        if (received == numChunks) {
            cout << "Saving image to " << image_path << endl;
            finalImage.Write(image_path);

            return string("EOS"); // Send EOS
        }

        return "Collected "; // There is more to be collected
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
    if (argc != 6) {
        cout << "Usage: ./mandelbrot <width> <height> <maxIterations> <numRowChunks> <numColChunks>\n";
        return 1;
    }

    int width = stoi(argv[1]);
    int height = stoi(argv[2]);
    int maxIterations = stoi(argv[3]);
    int numRowChunks = stoi(argv[4]);
    int numColChunks = stoi(argv[5]);

//    int width = 1200;
//    int height = 800;
//    int maxIterations = 1000;
//    int numRowChunks = 4 * 3;
//    int numColChunks = 4 * 3;
//    int numWorkers = 8;

    string fname = "mandelbrot_" + to_string(width) + "x" + to_string(height) + ".bmp";

    // Initialize MPI environment
    mpi::environment env;
    mpi::communicator world;

    int numWorkers = world.size() - 3;

    auto *farm = new MPIFarmManager(&env, &world);
    MandelbrotEmitter *emitter = new MandelbrotEmitter(width, height, numRowChunks, numColChunks, maxIterations);
    MandelbrotCollector *collector = new MandelbrotCollector(width, height, numRowChunks, numColChunks, fname);

    farm->add_emitter(emitter);
    farm->add_collector(collector);

    for (int i = 0; i < numWorkers; i++) {
        MandelbrotWorker *worker = new MandelbrotWorker();
        farm->add_worker(worker);
    }

    farm->run("");

    return 0;
}
