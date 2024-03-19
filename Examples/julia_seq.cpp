#include <iostream>
#include "../src/Node.hpp"
#include "../src/FarmManager.hpp"
#include "../src/EasyBMP.hpp"

using namespace std;
using namespace EasyBMP;

class JuliaSet : public Node {
public:
    JuliaSet(string image_path, int width, int height, int maxIterations, double minReal, double maxReal, double minImag, double maxImag, double cReal, double cImag)
            : image_path(image_path), width(width), height(height), maxIterations(maxIterations), minReal(minReal), maxReal(maxReal), minImag(minImag), maxImag(maxImag), cReal(cReal), cImag(cImag) {}

    void* run(void* task) {
        EasyBMP::Image image(width, height, image_path, EasyBMP::RGBColor(255, 255, 255));

        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
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
                image.SetPixel(x, y, EasyBMP::RGBColor(red, green, blue));


//                if (n == maxIterations) {
//                    image.SetPixel(x, y, EasyBMP::RGBColor(0, 0, 0));
//                } else {
//                    image.SetPixel(x, y, EasyBMP::RGBColor(255, 255, 255));
//                }
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
    double cReal;  // Julia constant (real part)
    double cImag;  // Julia constant (imaginary part)
};

int main() {
    int w = 1200 * 16;
    int h = 800 * 16;
    string fname = "julia_seq_" + to_string(w) + "x" + to_string(h) + ".bmp";

    // Specify the Julia constant (you can experiment with different values)
    double cReal = -0.7;
    double cImag = 0.27015;

    JuliaSet juliaSet(fname, w, h, 1000, -2, 2, -1.5, 1.5, cReal, cImag);
    juliaSet.run(NULL);

    return 0;
}
