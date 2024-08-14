/* FIRST APPROACH TO LEARN AND BUILD A RETRIEVAL SYSTEM */
Thanks for taking a visit on my GitHub page. This is my first project about building a retrieval system. Here is the dataset (I only use the images dataset, and I'm trying to combine the texture dataset
and shape dataset later. 

This first approach is likely same when you're trying to compare 2 numbers. In general, I only calculte the dataset features, then I store it for reusing. When the retrieval system works, it calculates the
feature of query image (maybe histogram or ORB/SIFT/color correlogram features) and compare with corresponding features. For example, if you want to use Histogram features for retrieval, the system calculates
your Histogram feature from your query image, then compare with Histogram.yaml (the histogram of entire dataset). 

For months, I learnt a lot of techniques that can be used in these kind of systems like K-Means clustering or Bag-Of-Words,... but I don't have much time to implement it. Hopefully in future, I can improve my
system.
