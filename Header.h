#pragma once
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>

namespace fs = std::filesystem;

//std::vector<cv::Mat> LoadImages(const std::string& folderPath);
//void ShowImages(const std::vector<cv::Mat>& images, const std::string& nameWindow);

//class Dataset
//{
//public:
//	std::vector<cv::Mat> LoadImages(const std::string& folderPath, int numImages);
//	void ShowImages(const std::string& nameWindow, int numImagesPerRow);
//};

struct DataCSV
{
	int id;
	std::string label;
	double map3;
	double map5; 
	double map11;
	double map21;
};

class Index
{
private:
	std::vector<DataCSV> data;
public:
	void ReadCSV(const std::string& filename);
	std::vector<DataCSV>& getDataCSV()
	{
		return this->data;
	}
};


class Dataset
{
private:
	std::vector<cv::Mat> dataset;
	std::vector<int> imageNames;
public:
	Dataset(const std::string& folderPath) {
		LoadImages(folderPath);
	}
	void LoadImages(const std::string& folderPath);
	void ShowImages(const std::vector<cv::Mat>& images, const std::string& nameWindow); //Cho kết quả
	//void ShowDatasetImages(const std::string& nameWindow, int maxImages);

	// Getter
	const std::vector<cv::Mat>& getImages() const
	{
		return this->dataset;
	}

	const std::vector<int>& getImageNames() const 
	{ 
		return this->imageNames; 
	}
};

class System : public Dataset
{
private:
	std::vector<cv::Mat> histograms;
	std::vector<int> clusters;
	std::vector<std::vector<cv::KeyPoint>> sift_keypoints, orb_keypoints;
	std::vector<cv::Mat> sift_descriptors, orb_descriptors;

	cv::Mat SIFT_centers; // Chưa dùng đến

public:
	System(const std::string& folderPath) : Dataset(folderPath) {} //Constructor

	// Những xử lý liên quan đến Histogram
	std::vector<cv::Mat> calculateHistogram(const cv::Mat& image);
	cv::Mat normalizeHistogram(const std::vector<cv::Mat>& histograms);
	void computeHistograms();

	// Những xử lý liên quan đến SIFT
	void computeSIFTFeatures();

	// Những xử lý liên quan đến ORB
	void computeORBFeatures();
	
	// Chưa dùng đến
	//std::vector<int> applyKMeans(int k);
	std::vector<int> applyKMeans(int k, bool useSIFT);
	void initializeKMeans(int clusterCount);

	// Những xử lý liên quan đến lưu, xuất dữ liệu Histogram
	bool fileExists(const std::string& fileName);
	void saveData(const std::string& histogramFile);
	void loadData(const std::string& histogramFile);
	void ensureHistogramsComputed(const std::string& histogramFile);
	
	// Những xử lý liên quan đến lưu, xuất dữ liệu SIFT: 
	void saveSIFTData(const std::string& siftFile);
	void loadSIFTData(const std::string& siftFile);
	void ensureSIFTComputed(const std::string& SIFTFile);

	// Những xử lý liên quan đến lưu, xuất dữ liệu ORB: 
	void saveORBData(const std::string& ORBFile);
	void loadORBData(const std::string& ORBFile);
	void ensureORBComputed(const std::string& ORBFile);
	

	// Những xử lý liên quan đến tìm kiếm:
	std::vector<std::pair<int, double>> compareHistograms(const cv::Mat& image);
	std::vector<std::pair<int, double>> compareSIFT(const cv::Mat& queryImage);
	std::vector<std::pair<int, double>> compareORB(const cv::Mat& queryImage);
	std::vector<std::pair<cv::Mat, int>> searchImage(const cv::Mat& queryImage, const char* type_of_searching, int numResults);

	// Tính độ đo MAP:
	double calculateMAP(const std::vector<std::pair<cv::Mat, int>>& results, const std::string& queryLabel, Index index, int numResults);
};


