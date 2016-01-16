# Tumblr API + OpenCV

This was inspired by [WSJ - 50 Years of 'Avengers' Comic Book Covers Through Color](http://graphics.wsj.com/avengers/). 

I thought it'd be fun to make some of these myself. A good number of users on [Tumblr](http://tumblr.com) choose to devote their blogs to a distinct color palette, and it'd be neat to visualize a certain blog's color distribution in a condensed manner.

Here are some cool examples of output visualizations of the 60 most recent image posts on specific blogs.
This user enjoys posting pictures of the sky, water, mountains, snow, ice, and stars. 

![alt tag](https://github.com/sjaoudi/tumblr-color/blob/master/build/Debug/examples/blue.png)

This user likes everything associated with food, clothes, makeup, and jewelry, as long as its pink.

![alt tag](https://github.com/sjaoudi/tumblr-color/blob/master/build/Debug/examples/pink.png)

Some blogs have themes that make for interesting visualizations. 

Here's one of an astrophotography blog:

![alt tag](https://github.com/sjaoudi/tumblr-color/blob/master/build/Debug/examples/astro.png)

Instead of a blog, here's the tag 'sunset' with a notes (reblogs and likes) threshold of 100 applied to the images gathered by the API call. The result is a collection of images that were decently well-received by other users and not spam.

![alt tag](https://github.com/sjaoudi/tumblr-color/blob/master/build/Debug/examples/sunset.png)

The colors for each row were chosen using [k-means clustering](https://en.wikipedia.org/wiki/K-means_clustering) with k = 12. (See [doc entry](http://docs.opencv.org/2.4/modules/core/doc/clustering.html#kmeans)) One optimization idea would be to implement would be dimensionality reduction to each image to speed up the k-means step.
