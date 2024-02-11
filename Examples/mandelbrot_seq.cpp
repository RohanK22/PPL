#include <iostream>
#include "../Node.hpp"
#include "../FarmManager.hpp"
#include "../EasyBMP.hpp"

using namespace std;
using namespace EasyBMP;

class Mandelbrot : public Node {
public:
    Mandelbrot(string image_path, int width, int height, int maxIterations, double minReal, double maxReal, double minImag, double maxImag) {
        this->image_path = image_path;
        this->width = width;
        this->height = height;
        this->maxIterations = maxIterations;
        this->minReal = minReal;
        this->maxReal = maxReal;
        this->minImag = minImag;
        this->maxImag = maxImag;
    }

    void* run(void *task) {
        EasyBMP::Image image(width, height, image_path, EasyBMP::RGBColor(255, 255, 255));

        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
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

                int f = 2.05;
                int red = (n) % 256;
                int green = int(n * f) % 256;
                int blue = int(n * f * f) % 256;
                image.SetPixel(x, y, EasyBMP::RGBColor(red, green, blue));
            }
        }

        image.Write();
    }

private:
    string image_path;
    int width;
    int height;
    int maxIterations;
    double minReal;
    double maxReal;
    double minImag;
    double maxImag;
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Usage: ./mandelbrot_seq <width> <height> <maxIterations>" << endl;
        return 1;
    }

    // Read width, height, maxIterations
    int w = atoi(argv[1]);
    int h = atoi(argv[2]);
    int maxIterations = atoi(argv[3]);

//    int w = 1200 * 16;
//    int h = 800 * 16;
//    int maxIterations = 1000;
    string fname = "mandelbrot_" + to_string(w) + "x" + to_string(h) + ".bmp";
    Mandelbrot mandelbrot(fname, w, h, maxIterations, -2, 1, -1, 1);
    mandelbrot.run(NULL);
    return 0;
}