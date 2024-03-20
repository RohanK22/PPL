#include <iostream>
#include <filesystem>
#include "../../libs/EasyBMP.hpp"

using namespace std;
using namespace EasyBMP;
namespace fs = std::filesystem;

// Function to convert image to grayscale
void convertToGrayscale(EasyBMP::Image* image) {
    for (int x = 0; x < image->TellWidth(); x++) {
        for (int y = 0; y < image->TellHeight(); y++) {
            EasyBMP::RGBColor pixel = image->GetPixel(x, y);
            int average = (pixel.r + pixel.g + pixel.b) / 3;
            image->SetPixel(x, y, EasyBMP::RGBColor(average, average, average));
        }
    }
}

// Function to apply Gaussian blur to the image
void applyGaussianBlur(EasyBMP::Image* image, int num_passes) {
    int64_t width = image->TellWidth();
    int64_t height = image->TellHeight();
    EasyBMP::Image* blurred_image = new EasyBMP::Image(image->TellWidth(), image->TellHeight());

    for (int pass = 0; pass < num_passes; pass++) {
        for (int x = 2; x < image->TellWidth() - 2; x++) {
            for (int y = 2; y < image->TellHeight() - 2; y++) {
                // Apply 5x5 Gaussian blur
                int sum_red = 0, sum_green = 0, sum_blue = 0;
                for (int i = -2; i <= 2; i++) {
                    for (int j = -2; j <= 2; j++) {
                        EasyBMP::RGBColor pixel = image->GetPixel(x + i, y + j);
                        sum_red += pixel.r;
                        sum_green += pixel.g;
                        sum_blue += pixel.b;
                    }
                }
                int average_red = sum_red / 25;
                int average_green = sum_green / 25;
                int average_blue = sum_blue / 25;

                blurred_image->SetPixel(x, y, EasyBMP::RGBColor(average_red, average_green, average_blue));
            }
        }

        // Swap images for the next pass
        swap(image, blurred_image);
    }

    delete blurred_image;
}

// Function to write the image to file
void writeImageToFile(EasyBMP::Image* image, const string& output_path) {
    cout << "Writing image to " << output_path << endl;
    image->Write("abc.bmp");
}

int main(int argc, char* argv[]) {
    string input_folder_path = "../Node_Examples/Pipeline/TestImages";
    string output_folder_path = "../Node_Examples/Pipeline/BlurOutput/";
    int num_passes = 3;

    // Check if the input folder exists
    if (!fs::exists(input_folder_path) || !fs::is_directory(input_folder_path)) {
        cout << "Input folder does not exist." << endl;
        return 1;
    }

    // Create the output folder if it doesn't exist
    if (!fs::exists(output_folder_path)) {
        fs::create_directory(output_folder_path);
    }

    // Iterate over each image file in the input folder
    for (const auto& entry : fs::directory_iterator(input_folder_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".bmp") {
            string input_image_path = entry.path().string();
            string output_image_path = output_folder_path + entry.path().filename().stem().string() + "_blurred.bmp";

            //cout << "Processing image: " << input_image_path << endl;

            // Read the input image
            EasyBMP::Image* input_image = new EasyBMP::Image(input_image_path.c_str());

            // Convert the image to grayscale
            convertToGrayscale(input_image);

            // Blur
            applyGaussianBlur(input_image, num_passes);

            // Write the resulting image to file
            writeImageToFile(input_image, output_image_path);
        }
    }

    return 0;
}
