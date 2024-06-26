#include <iostream>
#include "../../src/PipelineManager.hpp"
#include "../../src/FarmManager.hpp"
#include "../../libs/EasyBMP.hpp"
#include <filesystem>

using namespace std;
using namespace EasyBMP;
namespace fs = std::filesystem;

class ImageReader : public Node <void*> {
public:
    ImageReader(const string& folder_path) : folder_path(folder_path) {}

    void* run(void*) {
        // Iterate through the images in the specified folder
        for (const auto& entry : fs::directory_iterator(folder_path)) {
            string image_path = entry.path().string();
            EasyBMP::Image* image = new EasyBMP::Image(image_path.c_str());
            this->get_output_queue()->push(static_cast<void*>(image));
        }

        return EOS;
    }

private:
    string folder_path;
};

class GrayscaleConverter : public Node <void*> {
public:
    void* run(void* task) {
        EasyBMP::Image* image = static_cast<EasyBMP::Image*>(task);
        // Convert image to grayscale
        for (int x = 0; x < image->TellWidth(); x++) {
            for (int y = 0; y < image->TellHeight(); y++) {
                EasyBMP::RGBColor pixel = image->GetPixel(x, y);
                int average = (pixel.r + pixel.g + pixel.b) / 3;
                image->SetPixel(x, y, EasyBMP::RGBColor(average, average, average));
            }
        }

        return static_cast<void*>(image);
    }
};

class GaussianBlurWorker : public Node <void*> {
public:
    GaussianBlurWorker(int num_passes) : num_passes(num_passes) {}

    void* run(void* task) {
        EasyBMP::Image* image = static_cast<EasyBMP::Image*>(task);
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

        return static_cast<void*>(image);
    }

private:
    int num_passes;
};

class ImageWriter : public Node <void*> {
public:
    ImageWriter(const string& output_path) : output_path(output_path) {}

    void* run(void* task) {
        EasyBMP::Image* image = static_cast<EasyBMP::Image*>(task);
        cout << "Writing image to " << output_path << endl;
        image->Write(output_path.c_str() + to_string(receive_count) + ".bmp");
        delete image;  // Clean up memory
        receive_count++;
        return (void*) std::to_string(receive_count).c_str();
    }

private:
    string output_path;
    int receive_count = 0;
};

int main(int argc, char* argv[]) {
    string input_image_path = "../Node_Examples/Pipeline/TestImages";
    string output_image_path = "../Node_Examples/Pipeline/BlurOutput/";
    int num_workers = 6;
    int num_passes = 3;

    PipelineManager<void*> *pipeline = new PipelineManager<void*>();
    ImageReader* image_reader = new ImageReader(input_image_path);
    GrayscaleConverter* grayscale_converter = new GrayscaleConverter();

    // Make a farm of GaussianBlurWorkers
    FarmManager<void*> *gaussian_blur_farm = new FarmManager<void*>();

    for (int i = 0; i < num_workers; i++) {
        GaussianBlurWorker* gaussian_blur_worker = new GaussianBlurWorker(num_passes);
        gaussian_blur_farm->add_worker(gaussian_blur_worker);
    }

    ImageWriter* image_writer = new ImageWriter(output_image_path);

    pipeline->add_stage(image_reader);
    pipeline->add_stage(grayscale_converter);
    pipeline->add_stage(gaussian_blur_farm);
    pipeline->add_stage(image_writer);

    cout << "Number of stages: " << pipeline->get_num_pipeline_stages() << endl;

    pipeline->run_until_finish();

    return 0;
}
