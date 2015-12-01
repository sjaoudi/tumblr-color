#include <stdio.h>
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <string>

using namespace std;

typedef tuple<float, float, float> pixel;

/* Function to write data to stream */
size_t write_data(char *ptr, size_t size, size_t nmemb, void *userdata) {
    ostringstream *stream = (ostringstream*)userdata;
    size_t count = size * nmemb;
    stream->write(ptr, count);
    return count;
}

/*
 *
 */
vector<string>* readUrlsFromFile(string filename) {
    ifstream inFile;
    inFile.open(filename.c_str());
    if(!inFile.is_open()) {
        throw runtime_error("couldn't open file");
    }
    
    vector<string> *image_urls = new vector<string>;
    
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
    ostringstream stream;
    curl = curl_easy_init();
    
    curl_easy_setopt(curl, CURLOPT_URL, image_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream);
    
    res = curl_easy_perform(curl);
    string output = stream.str();
    curl_easy_cleanup(curl);
    vector<char> data = vector<char>( output.begin(), output.end() );
    
    cout << image_url << endl;
    
    cv::Mat data_mat = cv::Mat(data);
    cv::Mat image = cv::imdecode(data_mat,1);
    
    return image;
}


pixel image_rgb_avg(const char *image_url) {
    
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
    
    cout << r_avg << "," << g_avg << "," << b_avg << endl;
    
    return tuple<float, float, float>(r_avg, g_avg, b_avg);
    
}

map<pixel, float>* k_means(cv::Mat image, int k) {
    
    cv::Mat samples(image.rows * image.cols, 3, CV_32F);
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            for (int z = 0; z < 3; z++) {
                samples.at<float>(y + x*image.rows, z) = image.at<cv::Vec3b>(y,x)[z];
            }
        }
    }
    
    cv::Mat labels;
    int attempts = 2;
    cv::Mat centers;
    kmeans(samples, k, labels, cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 10000, 0.0001),
           attempts, cv::KMEANS_PP_CENTERS, centers);
    
    //vector<tuple<float, float, float> > *colors = new vector<tuple<float, float, float> >;
    
    map<pixel, int> colors_pixels;
    
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
            
            pixel color = pixel(new_r, new_g, new_b);
            
            // Update if contains
            map<pixel, int>::iterator it = colors_pixels.find(color);
            if (it != colors_pixels.end()) {
                it->second++;
            }
            // Insert otherwise
            else {
                colors_pixels.insert( pair<pixel, int>(color, 1) );
            }
            
        }
    }
    
    int pixels = 0;
    map<pixel, int>::iterator pixels_it;
    for(pixels_it = colors_pixels.begin(); pixels_it != colors_pixels.end(); pixels_it++) {
        pixels += pixels_it->second;
    }

    map<pixel,float> *colors = new map<pixel,float>;
    
    map<pixel, int>::iterator it;
    for(it = colors_pixels.begin(); it != colors_pixels.end(); it++) {
        colors->insert(pair<pixel, float>(it->first, (float)it->second/pixels));
    }
    
//    imshow("clustered image", new_image);
//    cv::waitKey(0);
    
    return colors;
}

bool sort_by_color(pixel color1, pixel color2) {
    cv::Mat c1(1,1, CV_8UC3, cvScalar(0,0,255));
    c1.at<cv::Vec3b>(0,0)[2] = get<2>(color1);
    c1.at<cv::Vec3b>(0,0)[1] = get<1>(color1);
    c1.at<cv::Vec3b>(0,0)[0] = get<0>(color1);
    
    cv::Mat c2(1,1, CV_8UC3, cvScalar(0,0,255));
    c2.at<cv::Vec3b>(0,0)[2] = get<2>(color2);
    c2.at<cv::Vec3b>(0,0)[1] = get<1>(color2);
    c2.at<cv::Vec3b>(0,0)[0] = get<0>(color2);
    
    cv::Mat c1_hsv;
    cv::Mat c2_hsv;
    
    cv::cvtColor(c1, c1_hsv, CV_RGB2HSV);
    cv::cvtColor(c2, c2_hsv, CV_RGB2HSV);
    
    int v1 = c1_hsv.at<cv::Vec3b>(0,0)[0];
    int v2 = c2_hsv.at<cv::Vec3b>(0,0)[0];
    
    return v1 < v2;
}

vector<pixel >* sort_colors (map<pixel, float> *colors) {
    
    vector<pixel >* color_list = new vector<pixel >;
    map<pixel, float>::iterator it;
    for (it = colors->begin(); it != colors->end(); it++) {
        color_list->push_back(it->first);
    }
    
    sort(color_list->begin(), color_list->end(), sort_by_color);
    
    return color_list;
    
}

cv::Mat* draw_func(vector<pixel >* color_list, map<pixel, float> *colors, int width) {
    
    cv::Mat *image = new cv::Mat(cv::Mat::zeros(10, width, CV_8UC3) );
    
    int cur_width = 0;
    for (int i = 0; i < color_list->size(); i++) {
        pixel image_color = color_list->at(i);
        float percentage = colors->find(image_color)->second;
        int rectangle_width = width * percentage;
        
        rectangle(*image, cvPoint(cur_width, 0),
                          cvPoint(cur_width + rectangle_width, 20),
                          cvScalar(get<2>(image_color),
                                   get<1>(image_color),
                                   get<0>(image_color)),
                          -1, 1);
        
        cur_width += rectangle_width;
    }
    
//    cv::imshow("Image", *image);
//    cv::waitKey(0);
    
    return image;
    
}

void piece_together(vector<pixel >* color_list, map<pixel, float> *colors, vector<string> *images, int width) {
    unsigned long height = 20 * images->size();
    cv::Mat *image = new cv::Mat(cv::Mat::ones((int)height, width, CV_64F));
    for (int i = 0; i < images->size(); i++) {
        cv::Mat *row = draw_func(color_list, colors, width);
        image->push_back(row);
    }
}


int main(void) {
    
    string filename;
    cin >> filename;
    
    vector<string> *image_urls = readUrlsFromFile(filename);
    
    cv::Mat *the_image = new cv::Mat(cv::Mat::ones(0, 500, CV_64F));
    
    for (int image = 0; image < image_urls->size(); image++) {
        // get image url
        string image_url = image_urls->at(image);
        // download image
        cv::Mat src = curlImg(image_url.c_str());
        // get map of colors and percentages
        map<pixel, float> *colors = k_means(src, 12);
        // get list of colors in correct order
        vector<pixel > *color_list = sort_colors(colors);
        
        cv::Mat *row = draw_func(color_list, colors, 500);
        
        the_image->push_back(*row);
        
    }
    imshow("the image", *the_image);
    cv::waitKey(0);
}

