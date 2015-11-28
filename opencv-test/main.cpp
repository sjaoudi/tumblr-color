


#include <stdio.h>
#include <curl/curl.h>
#include <sstream>
#include <iostream>
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


cv::Mat curlImg(char *image_url) {
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
    
    cv::Mat data_mat = cv::Mat(data);
    cv::Mat image = cv::imdecode(data_mat,1);
    
    return image;
}


int main(void) {
    char * image_url = "http://40.media.tumblr.com/35f116b3264cee8522b1e0f92807c3e5/tumblr_nxo1nk0u1R1qhttpto5_1280.jpg";
    cv::Mat image = curlImg(image_url);
    cv::namedWindow( "Image output", CV_WINDOW_AUTOSIZE );
    cv::imshow("Image output",image); //display image
    cvWaitKey(0); // press any key to exit
    cv::destroyWindow("Image output");
}