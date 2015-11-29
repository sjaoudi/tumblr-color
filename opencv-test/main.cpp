


#include <stdio.h>
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <string>

using namespace std;


size_t write_data(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::ostringstream *stream = (std::ostringstream*)userdata;
    size_t count = size * nmemb;
    stream->write(ptr, count);
    return count;
}


std::vector<string>* readUrlsFromFile(string filename) {
    ifstream inFile;
    inFile.open(filename.c_str());
    if(!inFile.is_open()) {
        throw runtime_error("couldn't open file");
    }
    
    std::vector<string> *image_urls = new vector<string>;
    
    string url;
    
    inFile >> url;
    while(!(inFile.eof())) {
        image_urls->push_back(url);
        inFile >> url;
    }
          
    inFile.close();
          
    return image_urls;
}


cv::Mat curlImg(const char *image_url) {
    
    CURL *curl;
    CURLcode res;
    std::ostringstream stream;
    curl = curl_easy_init();
    
    curl_easy_setopt(curl, CURLOPT_URL, image_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream);
    
    res = curl_easy_perform(curl);
    std::string output = stream.str();
    curl_easy_cleanup(curl);
    std::vector<char> data = std::vector<char>( output.begin(), output.end() );
    
    std::cout << image_url << std::endl;
    
    cv::Mat data_mat = cv::Mat(data);
    cv::Mat image = cv::imdecode(data_mat,1);
    
    return image;
}


std::tuple<float, float, float> image_rgb_avg(const char *image_url) {
    
    cv::Mat image = curlImg(image_url);
    //cv::namedWindow( "Image output", CV_WINDOW_AUTOSIZE );
    //cv::imshow("Image output",image);
    //cvWaitKey(0); // press any key to exit
    //cv::destroyWindow("Image output");
    
    vector<cv::Mat> channels;
    split(image, channels);
    cv::Scalar r_channel = mean(channels[2]); // r
    cv::Scalar g_channel = mean(channels[1]); // g
    cv::Scalar b_channel = mean(channels[0]); // b
    
    float r_avg = r_channel[0];
    float g_avg = g_channel[0];
    float b_avg = b_channel[0];
    
    std::cout << r_avg << "," << g_avg << "," << b_avg << std::endl;
    
    return tuple<float, float, float>(r_avg, g_avg, b_avg);
    
}

std::map<std::tuple<float, float, float>, float>* k_means(cv::Mat image, int k) {
    
    cv::Mat samples(image.rows * image.cols, 3, CV_32F);
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            for (int z = 0; z < 3; z++) {
                samples.at<float>(y + x*image.rows, z) = image.at<cv::Vec3b>(y,x)[z];
            }
        }
    }
    
    cv::Mat labels;
    int attempts = 5;
    cv::Mat centers;
    kmeans(samples, k, labels, cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 10000, 0.0001),
           attempts, cv::KMEANS_PP_CENTERS, centers);
    
    //std::vector<std::tuple<float, float, float> > *colors = new std::vector<std::tuple<float, float, float> >;
    
    std::map<std::tuple<float, float, float>, int> colors_pixels;
    
    cv::Mat new_image(image.size(), image.type());
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            int cluster_idx = labels.at<int>(y + x*image.rows, 0);
            
            int new_r = centers.at<float>(cluster_idx, 2);
            int new_g = centers.at<float>(cluster_idx, 1);
            int new_b = centers.at<float>(cluster_idx, 0);
            
            new_image.at<cv::Vec3b>(y,x)[2] = new_r;
            new_image.at<cv::Vec3b>(y,x)[1] = new_g;
            new_image.at<cv::Vec3b>(y,x)[0] = new_b;
            
            tuple<float, float, float> color = tuple<float, float, float>(new_r, new_g, new_b);
            
            // Update if contains
            std::map<tuple<float, float, float>, int>::iterator it = colors_pixels.find(color);
            if (it != colors_pixels.end()) {
                it->second++;
            }
            // Insert otherwise
            else {
                colors_pixels.insert( std::pair<std::tuple<float, float, float>, int>(color, 1) );
            }
            
        }
    }
    
    int pixels = 0;
    std::map<tuple<float, float, float>, int>::iterator pixels_it;
    for(pixels_it = colors_pixels.begin(); pixels_it != colors_pixels.end(); pixels_it++) {
        pixels += pixels_it->second;
    }

    std::map<std::tuple<float, float, float>,float> *colors = new std::map<std::tuple<float, float, float>,float>;
    
    std::map<tuple<float, float, float>, float>::iterator it;
    for(it = colors->begin(); it != colors->end(); it++) {
        colors->insert(std::pair<std::tuple<float, float, float>, float>(it->first, (float)it->second/pixels));
    }
    
    //imshow("clustered image", new_image);
    //cv::waitKey(0);
    
    return colors;
}


int main(void) {
    //string image_url = "http://41.media.tumblr.com/72d7fa871b17b518fc5605ea66c4a375/tumblr_na3ixsXuO91sgt2tfo1_1280.jpg";
    
    string image_url = "http://40.media.tumblr.com/1bbad480554d6745898aa5d4819c2262/tumblr_nycfxd8qk51t7b5qro1_1280.jpg";
    
//    image_rgb_avg(image_url.c_str());
    /*
//    string filename;
    string filename = "colored-moments-photos.txt";
//    std::cin >> filename;
    
    vector<string> *image_urls = readUrlsFromFile(filename);
    
    for (int image = 0; image < image_urls->size(); image++) {
        image_rgb_avg(image_urls->at(image).c_str());
    }
    
    */
    cv::Mat src = curlImg(image_url.c_str());
    
    k_means(src, 6);
   
    
}

