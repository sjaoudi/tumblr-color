


#include <stdio.h>
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <string>

using namespace std;

//curl writefunction to be passed as a parameter
size_t write_data(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::ostringstream *stream = (std::ostringstream*)userdata;
    size_t count = size * nmemb;
    stream->write(ptr, count);
    return count;
}

//function to retrieve the image as Cv::Mat data type
cv::Mat curlImg(string image_url) {
    CURL *curl;
    CURLcode res;
    std::ostringstream stream;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "http://40.media.tumblr.com/35f116b3264cee8522b1e0f92807c3e5/tumblr_nxo1nk0u1R1qhttpto5_1280.jpg"); //the img url
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); // pass the writefunction
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream); // pass the stream ptr when the writefunction is called
    res = curl_easy_perform(curl); // start curl
    std::string output = stream.str(); // convert the stream into a string
    curl_easy_cleanup(curl); // cleanup
    std::vector<char> data = std::vector<char>( output.begin(), output.end() ); //convert string into a vector
    cv::Mat data_mat = cv::Mat(data); // create the cv::Mat datatype from the vector
    cv::Mat image = cv::imdecode(data_mat,1); //read an image from memory buffer
    return image;
}


int main(void) {
    string image_url = "http://40.media.tumblr.com/35f116b3264cee8522b1e0f92807c3e5/tumblr_nxo1nk0u1R1qhttpto5_1280.jpg";
    cv::Mat image = curlImg(image_url);
    cv::namedWindow( "Image output", CV_WINDOW_AUTOSIZE );
    cv::imshow("Image output",image); //display image
    cvWaitKey(0); // press any key to exit
    cv::destroyWindow("Image output");
}