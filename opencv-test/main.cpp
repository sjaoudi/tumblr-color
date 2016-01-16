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

int row_height = 20;
int row_width = 500;
int number_images = 60;


/* readUrlsFromFile - creates a list of strings from every line in a given file
 */
vector<string>* read_urls_from_file(string filename) {
    
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


/* write_data - used by curl_img for buffering data to stream
 */
size_t write_data(char *ptr, size_t size, size_t nmemb, void *userdata) {
    
    ostringstream *stream = (ostringstream*)userdata;
    size_t count = size * nmemb;
    stream->write(ptr, count);
    
    return count;
}


/* curl_img - retrieves an image of type cv::Mat from a given url
 */
cv::Mat curl_img(const char *image_url) {
    
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
    vector<char> data = vector<char>(output.begin(), output.end());
    
    cout << image_url << endl;
    
    cv::Mat data_mat = cv::Mat(data);
    cv::Mat image = cv::imdecode(data_mat,1);
    
    return image;
}


/* curl_img - performs k_means clustering on a given image and k value
 */
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
    
    // Number of "attempts" for the algorithm
    int attempts = 1;
    cv::Mat centers;
    kmeans(samples, k, labels, cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 10000, 0.0001),
           attempts, cv::KMEANS_PP_CENTERS, centers);
    
    map<pixel, int> colors_pixels;
    
//    cv::Mat clustered_image(image.size(), image.type());
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            int cluster_idx = labels.at<int>(y + x*image.rows, 0);
            
            int new_r = centers.at<float>(cluster_idx, 2);
            int new_g = centers.at<float>(cluster_idx, 1);
            int new_b = centers.at<float>(cluster_idx, 0);
            
//            clustered_image.at<cv::Vec3b>(y,x)[2] = new_r;
//            clustered_image.at<cv::Vec3b>(y,x)[1] = new_g;
//            clustered_image.at<cv::Vec3b>(y,x)[0] = new_b;
            
            pixel color = pixel(new_r, new_g, new_b);
            
            // Update dict if it already contains the color
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
    
    // Count the total number of pixels for each k-color
    int pixels = 0;
    map<pixel, int>::iterator pixels_it;
    for(pixels_it = colors_pixels.begin(); pixels_it != colors_pixels.end(); pixels_it++) {
        pixels += pixels_it->second;
    }

    // Create and return new dict that contains the percentage of each k color cluster to
    // the total image
    map<pixel,float> *colors = new map<pixel,float>;
    map<pixel, int>::iterator it;
    for(it = colors_pixels.begin(); it != colors_pixels.end(); it++) {
        colors->insert(pair<pixel, float>(it->first, (float)it->second/pixels));
    }
    
//    imshow("clustered image", clustered_image);
//    cv::waitKey(0);
    
    return colors;
}


/* sort_by_hsv - custom function for used for std::sort to order pixels by a hsv paramater
 *                instead of RGB content
 * @template hsv_option - sort by hue (2), saturation (1), or value (0)
 */

template<int hsv_option>
bool sort_by_hsv(pixel color1, pixel color2) {
    
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
    
    // Convert RGB -> HSV
    cv::cvtColor(c1, c1_hsv, CV_RGB2HSV);
    cv::cvtColor(c2, c2_hsv, CV_RGB2HSV);
    
    int v1 = c1_hsv.at<cv::Vec3b>(0,0)[hsv_option];
    int v2 = c2_hsv.at<cv::Vec3b>(0,0)[hsv_option];
    
    // Return the lower of the two
    return v1 < v2;
}


/* sort_colors - creates a list of each color cluster sorted by hue for referencing when
 *               drawing each row of the visualization
 */
vector<pixel >* sort_colors_by_hsv(map<pixel, float> *colors) {
    
    vector<pixel >* color_list = new vector<pixel >;
    map<pixel, float>::iterator it;
    for (it = colors->begin(); it != colors->end(); it++) {
        color_list->push_back(it->first);
    }
    
    sort(color_list->begin(), color_list->end(), &sort_by_hsv<2>);
    
    return color_list;
    
}


/* draw_row - uses the sorted list of color clusters in an image, and a dict containing
 *            the prevalence of each color, and draws a cv::Mat of rectangles of the 
 *            appropriate size
 * @param separators - 1 pixel white line between each rectangle
 */
cv::Mat* draw_row(vector<pixel >* color_list, map<pixel, float> *colors, bool separators) {
    
    cv::Mat *row = new cv::Mat(cv::Mat::zeros(10, row_width, CV_8UC3) );
    
    int cur_width = 0;
    for (int i = 0; i < color_list->size(); i++) {
        pixel image_color = color_list->at(i);
        float percentage = colors->find(image_color)->second;
        
        int rectangle_width = row_width * percentage;
        
        if (separators) {
            rectangle_width++;
        }
        
        rectangle(*row, cvPoint(cur_width, 0),
                        cvPoint(cur_width + rectangle_width, row_height),
                        cvScalar(get<2>(image_color),
                                 get<1>(image_color),
                                 get<0>(image_color)), -1, 1);
        
        if (separators) {
            rectangle(*row, cvPoint(cur_width + rectangle_width, 0),
                            cvPoint(cur_width + rectangle_width + 1, row_height),
                            cvScalar(255, 255, 255), -1, 1);
            cur_width++;
        }
        
        cur_width += rectangle_width;
        
    }
    
//    cv::imshow("Image", *image);
//    cv::waitKey(0);
    
    return row;
    
}


void build_image(vector<string> *image_urls, vector<cv::Mat *> *rows_list) {
    
    cv::Mat *rows = new cv::Mat(cv::Mat::ones(0, row_width, CV_64F));
    
    for (int i = 0; i < number_images; i++) {
        cv::Mat *row = rows_list->at(i);
        rows->push_back(*row);
        
        delete row;
        
    }
    
    imshow("the image", *rows);
    imwrite("output.png", *rows);
    cv::waitKey(0);
    
}


template<int hsv_option>
bool sort_by_avg_hsv(cv::Mat *row1, cv::Mat *row2) {
    // get average h, s, and v for each row, and order
    
    cv::Mat row1_hsv;
    cv::Mat row2_hsv;
    
    cv::cvtColor(*row1, row1_hsv, CV_RGB2HSV);
    cv::cvtColor(*row2, row1_hsv, CV_RGB2HSV);
    
    vector<cv::Mat> row1_channels;
    split(*row1, row1_channels);
    
    vector<cv::Mat> row2_channels;
    split(*row1, row2_channels);
    
    float row1_mean = mean(row1_channels[hsv_option])[0];
    float row2_mean = mean(row2_channels[hsv_option])[0];
    
    return row1_mean < row2_mean;
    
}


void order_by_hsv_avg(vector<string> *image_urls, vector<cv::Mat *> *rows_list) {
    sort(rows_list->begin(), rows_list->end(), &sort_by_avg_hsv<2>);
    build_image(image_urls, rows_list);
}



int main(void) {
    
    string filename;
    cin >> filename;
    
    vector<string> *image_urls = read_urls_from_file(filename);
    
    vector<cv::Mat*>* rows_list = new vector<cv::Mat *>;
    
    for (int i = 0; i < number_images; i++) {
        
        // Get image url
        string image_url = image_urls->at(i);
        
        // Download image
        cv::Mat src = curl_img(image_url.c_str());
        
        // Get map of color clusters and their percentages
        map<pixel, float> *colors = k_means(src, 12);
        
        // Get list of colors sorted by hue
        vector<pixel > *color_list = sort_colors_by_hsv(colors);

        // Draw row of rectangles of each k-cluster's color and proportion.
        // Optional - 1 pixel separators between each rectangle and row
        cv::Mat *row = draw_row(color_list, colors, false);
        
        // Append the drawn row
        rows_list->push_back(row);
        
        
        delete colors;
        delete color_list;
        
    }
    
    build_image(image_urls, rows_list);
    
}
