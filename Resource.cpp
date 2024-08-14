#include "Header.h"

// Với cấu trúc dữ liệu DataCSV hỗ trợ cho việc đọc + ghi kết quả lên csv, tạo một class để thực hiện việc đọc nhằm
// lưu lại thông tin về id + label dựa trên .csv
void Index::ReadCSV(const std::string& filename)
{
    std::vector<DataCSV> csv;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open())
    {
        std::cout << "Can't open the file!" << std::endl;
        return;
    }

    std::getline(file, line);

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string token;
        DataCSV row;

        // Đọc tên ảnh:
        std::getline(ss, token, ',');
        row.id = std::stoi(token);

        // Đọc nhãn: 
        std::getline(ss, token, ',');
        row.label = token;

        // Khởi tạo giá trị mAP mặc định
        row.map3 = -1.0;
        row.map5 = -1.0;
        row.map11 = -1.0;
        row.map21 = -1.0;

        csv.push_back(row);
    }

    file.close();
    this->data = csv;
}

// Hàm load ảnh
void Dataset::LoadImages(const std::string& folderPath)
{
    // Lưu hình ảnh dưới dạng mảng của các hình ảnh
    std::vector<cv::Mat> images;
    std::vector<int> names;

    //Truy cập vào folder ảnh và tiến hành duyệt qua từng ảnh, lấy đường dẫn ảnh và đọc
    //Nếu nó tồn tại thì đẩy vào mảng vector đã khởi tạo
    for (const auto& entry : fs::directory_iterator(folderPath))
    {
        if (entry.is_regular_file())
        {
            std::string imagePath = entry.path().string();
            cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);

            if (!image.empty())
            {
                cv::resize(image, image, cv::Size(72, 128));
                images.push_back(image);
                std::string filename = entry.path().filename().string();
                size_t lastindex = filename.find_last_of(".");
                std::string rawname = filename.substr(0, lastindex);
                int id = std::stoi(rawname);
                names.push_back(id);
            }
            else
            {
                std::cout << "Không thể đọc ảnh" << std::endl;
            }
        }
    }

    if (images.empty())
        std::cout << "Không có hình ảnh nào để hiển thị" << std::endl;
    else
    {
        this->dataset = images;
        this->imageNames = names;
    }
}

// Hàm xuất ảnh
void Dataset::ShowImages(const std::vector<cv::Mat>& images, const std::string& nameWindow)
{
    //Tiến hành hiển thị hình ảnh của folder ảnh đó nhưng trong cùng 1 cửa sổ chứ không tách biệt ra các cửa sổ con
    int imagesPerRow = 5; // Số ảnh trên một hàng
    int rows = (images.size() + imagesPerRow - 1) / imagesPerRow;

    // Xác định kích thước của ảnh đầu tiên để tạo ra chiều dài nhỏ nhất và chiều rộng nhỏ nhất
    int minHeight = 150;
    int minWidth = 100;
    for (const auto& img : images)
    {
        if (img.rows < minHeight) minHeight = img.rows;
        if (img.cols < minWidth) minWidth = img.cols;
    }

    // Tạo cửa sổ đích để chứa tất cả các ảnh dựa vào kích thước nhỏ nhất đã có được
    int dstWidth = imagesPerRow * minWidth;
    int dstHeight = rows * minHeight;
    cv::Mat dst = cv::Mat(dstHeight, dstWidth, CV_8UC3, cv::Scalar(0, 0, 0));

    // Copy các ảnh vào cửa sổ đích
    for (size_t i = 0; i < images.size(); ++i)
    {
        int x = (i % imagesPerRow) * minWidth;
        int y = (i / imagesPerRow) * minHeight;
        cv::Rect roi(x, y, minWidth, minHeight);
        cv::Mat resizedImg;
        cv::resize(images[i], resizedImg, cv::Size(minWidth, minHeight));
        resizedImg.copyTo(dst(roi));
    }

    // Hiển thị cửa sổ kết quả
    cv::namedWindow(nameWindow, cv::WINDOW_FULLSCREEN);
    cv::imshow(nameWindow, dst);
    cv::waitKey(5000);
    cv::destroyWindow(nameWindow);
}

//void Dataset::ShowDatasetImages(const std::string& nameWindow, int maxImages = 100)
//{
//    if (this->dataset.empty())
//    {
//        std::cout << "Load data thất bại, không có ảnh nào trong tập dataset";
//        return;
//    }
//
//    // Kiểm tra nếu số lượng ảnh hiển thị có vượt quá số lượng ảnh trong dataset hay không
//    maxImages = std::min(maxImages, static_cast<int>(this->dataset.size()));
//    maxImages = std::min(maxImages, 100);
//
//    
//    int imagesPerRow = 15;
//    int rows = (maxImages + imagesPerRow - 1) / imagesPerRow;
//
//    
//    int minHeight = 100;
//    int minWidth = 100;
//    for (const auto& img : this->dataset) {
//        if (img.rows < minHeight) minHeight = img.rows;
//        if (img.cols < minWidth) minWidth = img.cols;
//    }
//
//    
//    int dstWidth = imagesPerRow * minWidth;
//    int dstHeight = rows * minHeight;
//    cv::Mat dst = cv::Mat(dstHeight, dstWidth, CV_8UC3, cv::Scalar(0, 0, 0));
//
//    
//    for (size_t i = 0; i < maxImages; ++i) {
//        int x = (i % imagesPerRow) * minWidth;
//        int y = (i / imagesPerRow) * minHeight;
//        cv::Rect roi(x, y, minWidth, minHeight);
//        cv::resize(this->dataset[i], this->dataset[i], cv::Size(minWidth, minHeight));
//        this->dataset[i].copyTo(dst(roi));
//    }
//
//    cv::namedWindow(nameWindow, cv::WINDOW_FULLSCREEN);
//    cv::imshow(nameWindow, dst);
//    cv::waitKey(0);
//}

// Hàm tính Histogram của ảnh
std::vector<cv::Mat> System::calculateHistogram(const cv::Mat& image)
{
    // Tách các kênh màu
    std::vector<cv::Mat> bgr_planes;
    cv::split(image, bgr_planes);

    // Thiết lập tham số Histogram
    int histSize = 256;
    float range[] = { 0, 256 };
    const float* histRange = { range };
    bool uniform = true, accumulate = false;

    cv::Mat b_hist, g_hist, r_hist;

    // Tính histogram cho mỗi kênh màu
    cv::calcHist(&bgr_planes[0], 1, 0, cv::Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
    cv::calcHist(&bgr_planes[1], 1, 0, cv::Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
    cv::calcHist(&bgr_planes[2], 1, 0, cv::Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);

    // Lưu histogram vào vector
    std::vector<cv::Mat> histogram = { b_hist, g_hist, r_hist };

    return histogram;
}

// Hàm chuẩn hóa histogram
cv::Mat System::normalizeHistogram(const std::vector<cv::Mat>& histograms)
{
    cv::Mat hist_flat;
    for (const auto& hist : histograms)
    {
        cv::Mat hist_norm;
        cv::normalize(hist, hist_norm, 0, 1, cv::NORM_MINMAX);
        hist_flat.push_back(hist_norm.reshape(1, 1));
    }
    return hist_flat.reshape(1, 1);
}


// Thay vì phải lưu dữ liệu Histogram của dataset dưới dạng vector của vector, ta tiến hành tính toán trực tiếp
// bằng cách tính histogram của 1 ảnh -> chuẩn hóa nó vào một ma trận Mat lớn có trong class để chứa
void System::computeHistograms()
{
    const std::vector<cv::Mat>& images = getImages();
    histograms.clear();
    for (const auto& image : images)
    {
        auto hist = calculateHistogram(image);
        histograms.push_back(normalizeHistogram(hist));
    }
}

// Lưu dữ liệu trong file .dat cho data thô. Ghi dưới dạng binary
void System::saveData(const std::string& histogramFile)
{
    //std::ofstream histFile(histogramFile, std::ios::binary);
    //if (histFile.is_open()) 
    //{
    //    for (const auto& hist : histograms)
    //    {
    //        int rows = hist.rows;
    //        int cols = hist.cols;
    //        int type = hist.type();
    //        histFile.write(reinterpret_cast<char*>(&rows), sizeof(rows));
    //        histFile.write(reinterpret_cast<char*>(&cols), sizeof(cols));
    //        histFile.write(reinterpret_cast<char*>(&type), sizeof(type));
    //        histFile.write(reinterpret_cast<const char*>(hist.data), hist.total() * hist.elemSize());
    //    }
    //    histFile.close();
    //}
    //else 
    //{
    //    std::cerr << "Cannot open file to save histograms." << std::endl;
    //    //std::cerr << "Error: " << strerror(errno) << std::endl; // Thêm chi tiết lỗi
    //}
    cv::FileStorage fs(histogramFile, cv::FileStorage::WRITE);
    if (!fs.isOpened())
    {
        std::cerr << "Cannot open file to save histograms." << std::endl;
        return;
    }

    fs << "histograms" << "[";
    for (const auto& hist : histograms)
    {
        fs << "{";
        fs << "rows" << hist.rows;
        fs << "cols" << hist.cols;
        fs << "type" << hist.type();
        fs << "data" << hist;
        fs << "}";
    }
    fs << "]";
    fs.release();

}

// Load dữ liệu từ file .dat
void System::loadData(const std::string& histogramFile)
{
    //std::ifstream histFile(histogramFile, std::ios::binary);
    //if (histFile.is_open()) 
    //{
    //    histograms.clear();
    //    while (histFile.peek() != EOF)
    //    {
    //        int rows, cols, type;
    //        histFile.read(reinterpret_cast<char*>(&rows), sizeof(rows));
    //        histFile.read(reinterpret_cast<char*>(&cols), sizeof(cols));
    //        histFile.read(reinterpret_cast<char*>(&type), sizeof(type));

    //        cv::Mat hist(rows, cols, type);
    //        histFile.read(reinterpret_cast<char*>(hist.data), hist.total() * hist.elemSize());

    //        histograms.push_back(hist);
    //    }
    //    histFile.close();
    //}
    //else 
    //{
    //    std::cerr << "Cannot open file to load histograms." << std::endl;
    //    //std::cerr << "Error: " << strerror(errno) << std::endl; // Thêm chi tiết lỗi
    //}

    cv::FileStorage fs(histogramFile, cv::FileStorage::READ);
    if (!fs.isOpened())
    {
        std::cerr << "Cannot open file to load histograms." << std::endl;
        return;
    }

    histograms.clear();
    cv::FileNode histogramsNode = fs["histograms"];
    for (const auto& node : histogramsNode)
    {
        int rows = (int)node["rows"];
        int cols = (int)node["cols"];
        int type = (int)node["type"];
        cv::Mat hist(rows, cols, type);
        node["data"] >> hist;
        histograms.push_back(hist);
    }
    fs.release();
}

// Hàm này để kiểm tra xem file ta đang truy xuất có tồn tại hay không
bool System::fileExists(const std::string& fileName)
{
    return std::filesystem::exists(fileName);
}


// Nếu histogram của dataset chưa được tính, tiến hành tính thông qua các hàm xử lý ở trên
// Nếu đã tồn tại thì chỉ việc load file lên
void System::ensureHistogramsComputed(const std::string& histogramFile)
{
    if (fileExists(histogramFile)) 
    {
        loadData(histogramFile);
        std::cerr << "Your histogram from dataset is already calculated!" << std::endl;
    }
    else 
    {
        std::cerr << "Your histogram from dataset has not been calculated!" << std::endl;
        computeHistograms();
        saveData(histogramFile);
    }
}

// Hàm tính các đặc trưng SIFT. Sau khi tính được thì tiến hành lưu nó trong 2 thuộc tính của class là mảng keypoints và mảng descriptors
void System::computeSIFTFeatures()
{
    sift_keypoints.clear();
    sift_descriptors.clear();
    cv::Ptr<cv::SIFT> sift = cv::SIFT::create();
    const std::vector<cv::Mat>& images = getImages();
    
    for (const auto& image : images)
    {
        std::vector<cv::KeyPoint> kp;
        cv::Mat desc;
        sift->detectAndCompute(image, cv::noArray(), kp, desc);

        if (desc.type() != CV_32F) {
            desc.convertTo(desc, CV_32F); // Chuyển đổi sang CV_32F
        }

        // Giới hạn số lượng keypoints và descriptors
        /*if (desc.rows > 500)
        {
            desc = desc.rowRange(0, 500);
        }*/
        sift_keypoints.push_back(kp);
        sift_descriptors.push_back(desc);
    }
}

// Hàm tính các đặc trưng ORB. Sau khi tính được thì tiến hành lưu nó trong 2 thuộc tính của class là mảng keypoints và mảng descriptors
void System::computeORBFeatures()
{
    orb_keypoints.clear();
    orb_descriptors.clear();
    cv::Ptr<cv::ORB> orb = cv::ORB::create();
    const std::vector<cv::Mat>& images = getImages();
    
    for (const auto& image : images)
    {
        std::vector<cv::KeyPoint> kp;
        cv::Mat desc;
        orb->detectAndCompute(image, cv::noArray(), kp, desc);

        if (desc.type() != CV_8U) {
            desc.convertTo(desc, CV_8U); // Chuyển đổi sang CV_8U
        }
        // Giới hạn số lượng keypoints và descriptors nếu cần thiết
        /*if (desc.rows > 500)
        {
            desc = desc.rowRange(0, 500);
        }*/
        orb_keypoints.push_back(kp);
        orb_descriptors.push_back(desc);
    }
}

// Hàm tính toán KMeans cho SIFT: (Chưa dùng đến)
std::vector<int> System::applyKMeans(int k, bool useSIFT)
{
    cv::Mat data;

    if (useSIFT)
    {
        if (sift_descriptors.empty())
        {
            computeSIFTFeatures();
        }


        for (const auto& desc : sift_descriptors)
        {
            if (!desc.empty())
            {
                data.push_back(desc);
            }
        }
    }
    else
    {
        if (histograms.empty())
        {
            computeHistograms();
        }


        for (const auto& hist : histograms)
        {
            data.push_back(hist);
        }
    }

    data.convertTo(data, CV_32F);

    cv::Mat labels, centers;
    cv::kmeans(data, k, labels, cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 100, 0.2), 3, cv::KMEANS_PP_CENTERS, centers);

    clusters.clear();
    const std::vector<cv::Mat>& images = getImages();
    for (int i = 0; i < images.size(); ++i)
    {
        clusters.push_back(labels.at<int>(i, 0));
    }

    return clusters;
}

// Hàm này nhằm đảm bảo rằng file SIFT đã được ghi. Nếu nó chưa tồn tại thì tiến hành tính lại và lưu nó vào đường dẫn truyền vào
void System::ensureSIFTComputed(const std::string& SIFTFile)
{
    if (fileExists(SIFTFile))
    {
        loadSIFTData(SIFTFile);
        std::cerr << "Your SIFT from dataset is already calculated!" << std::endl;
    }
    else
    {
        std::cerr << "Your SIFT from dataset has not been calculated!" << std::endl;
        computeSIFTFeatures();
        saveSIFTData(SIFTFile);
    }
}

void System::saveORBData(const std::string& ORBFile)
{
    /*std::ofstream file(ORBFile, std::ios::binary);
    if (file.is_open()) {
        for (const auto& desc : orb_descriptors)
        {
            int rows = desc.rows;
            int cols = desc.cols;
            int type = desc.type();
            file.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
            file.write(reinterpret_cast<const char*>(&cols), sizeof(cols));
            file.write(reinterpret_cast<const char*>(&type), sizeof(type));
            file.write(reinterpret_cast<const char*>(desc.data), desc.total() * desc.elemSize());
        }
        file.close();
    }
    else
    {
        std::cerr << "Cannot open file to save ORB data." << std::endl;
    }*/
    cv::FileStorage fs(ORBFile, cv::FileStorage::WRITE);
    if (!fs.isOpened()) {
        std::cerr << "Cannot open file to save ORB data." << std::endl;
        return;
    }

    fs << "descriptors" << "[";
    for (const auto& desc : orb_descriptors) {
        cv::Mat desc8U;
        if (desc.type() != CV_8U) {
            desc.convertTo(desc8U, CV_8U);
        }
        else {
            desc8U = desc;
        }
        fs << desc8U;
    }
    fs << "]";
    fs.release();
}

void System::loadORBData(const std::string& ORBFile)
{
    /*std::ifstream file(ORBFile, std::ios::binary);
    if (file.is_open()) {
        orb_descriptors.clear();
        while (file.peek() != EOF)
        {
            int rows, cols, type;
            file.read(reinterpret_cast<char*>(&rows), sizeof(rows));
            file.read(reinterpret_cast<char*>(&cols), sizeof(cols));
            file.read(reinterpret_cast<char*>(&type), sizeof(type));

            cv::Mat desc(rows, cols, type);
            file.read(reinterpret_cast<char*>(desc.data), desc.total() * desc.elemSize());
            sift_descriptors.push_back(desc);
        }
        file.close();
    }
    else
    {
        std::cerr << "Cannot open file to load SIFT data." << std::endl;
    }*/
    cv::FileStorage fs(ORBFile, cv::FileStorage::READ);
    if (!fs.isOpened()) {
        std::cerr << "Cannot open file to load ORB data." << std::endl;
        return;
    }

    orb_descriptors.clear();
    cv::FileNode descs = fs["descriptors"];
    for (cv::FileNodeIterator it = descs.begin(); it != descs.end(); ++it) {
        cv::Mat descriptor;
        *it >> descriptor;
        if (descriptor.type() != CV_8U) {
            descriptor.convertTo(descriptor, CV_8U);
        }
        orb_descriptors.push_back(descriptor);
    }
    fs.release();
}

// Hàm này nhằm đảm bảo rằng file ORB đã được ghi. Nếu nó chưa tồn tại thì tiến hành tính lại và lưu nó vào đường dẫn truyền vào
void System::ensureORBComputed(const std::string& ORBFile)
{
    if (fileExists(ORBFile))
    {
        loadORBData(ORBFile);
        std::cerr << "Your ORB from dataset is already calculated!" << std::endl;
    }
    else
    {
        std::cerr << "Your ORB from dataset has not been calculated!" << std::endl;
        computeORBFeatures();
        saveORBData(ORBFile);
    }
}

// Hàm này được dùng để lưu lại dữ liệu Descriptor đã trích xuất được từ các đặc trưng SIFT
void System::saveSIFTData(const std::string& siftFile)
{
    /*std::ofstream outFile(siftFile, std::ios::binary);
    if (outFile.is_open()) 
    {
        for (size_t i = 0; i < sift_descriptors.size(); ++i) 
        {
            int rows = sift_descriptors[i].rows;
            int cols = sift_descriptors[i].cols;
            int type = sift_descriptors[i].type();
            outFile.write(reinterpret_cast<char*>(&rows), sizeof(rows));
            outFile.write(reinterpret_cast<char*>(&cols), sizeof(cols));
            outFile.write(reinterpret_cast<char*>(&type), sizeof(type));
            outFile.write(reinterpret_cast<const char*>(sift_descriptors[i].data), sift_descriptors[i].total() * sift_descriptors[i].elemSize());
        }
        outFile.close();
    }
    else 
    {
        std::cerr << "Cannot open file to save SIFT data." << std::endl;
    }*/
    cv::FileStorage fs(siftFile, cv::FileStorage::WRITE);
    if (!fs.isOpened()) 
    {
        std::cerr << "Cannot open file to save SIFT data." << std::endl;
        return;
    }

    fs << "descriptors" << "[";
    for (const auto& desc : sift_descriptors) 
    {
        cv::Mat desc32F;
        if (desc.type() != CV_32F) 
        {
            desc.convertTo(desc32F, CV_32F);
        }
        else 
        {
            desc32F = desc;
        }
        fs << desc32F;
    }
    fs << "]";
    fs.release();
}

// Hàm này được dùng để tải lên dữ liệu Descriptor đã trích xuất được từ các đặc trưng SIFT
void System::loadSIFTData(const std::string& siftFile)
{
    /*std::ifstream inFile(siftFile, std::ios::binary);
    if (inFile.is_open()) 
    {
        sift_descriptors.clear();
        while (inFile.peek() != EOF) 
        {
            int rows, cols, type;
            inFile.read(reinterpret_cast<char*>(&rows), sizeof(rows));
            inFile.read(reinterpret_cast<char*>(&cols), sizeof(cols));
            inFile.read(reinterpret_cast<char*>(&type), sizeof(type));
            cv::Mat descriptor(rows, cols, type);
            inFile.read(reinterpret_cast<char*>(descriptor.data), descriptor.total() * descriptor.elemSize());
            sift_descriptors.push_back(descriptor);
        }
        inFile.close();
    }
    else 
    {
        std::cerr << "Cannot open file to load SIFT data." << std::endl;
    }*/
    cv::FileStorage fs(siftFile, cv::FileStorage::READ);
    if (!fs.isOpened()) 
    {
        std::cerr << "Cannot open file to load SIFT data." << std::endl;
        return;
    }

    sift_descriptors.clear();
    cv::FileNode descs = fs["descriptors"];
    for (cv::FileNodeIterator it = descs.begin(); it != descs.end(); ++it) 
    {
        cv::Mat descriptor;
        *it >> descriptor;
        if (descriptor.type() != CV_32F) 
        {
            descriptor.convertTo(descriptor, CV_32F);
        }
        sift_descriptors.push_back(descriptor);
    }
    fs.release();
}

// Hàm này tiến hành so sánh Histogram của ảnh truy vấn so với bộ histogram đã tính toán từ trước
// Kết quả trả về là index của ảnh trong mảng ảnh truyền vào ban đầu và độ tương đồng của ảnh tương ứng với index đó
// Sau đó tiến hành sort lại để có được những ảnh có độ tương đồng giảm dần so với ảnh đã truy vấn
std::vector<std::pair<int, double>> System::compareHistograms(const cv::Mat& queryImage)
{
    // Tính toán histogram của ảnh truy vấn
    std::vector<cv::Mat> queryHist = calculateHistogram(queryImage);
    cv::Mat queryHistNorm = normalizeHistogram(queryHist);

    // So sánh với các histogram trong dataset
    std::vector<std::pair<int, double>> results;
    for (size_t i = 0; i < histograms.size(); ++i)
    {
        double similarity = cv::compareHist(queryHistNorm, histograms[i], cv::HISTCMP_CORREL);
        results.push_back({ i, similarity });
    }

    // Sắp xếp kết quả theo độ tương đồng giảm dần
    std::sort(results.begin(), results.end(), [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
        return a.second > b.second;
        });

    return results;
}

// Chưa dùng đến
void System::initializeKMeans(int clusterCount)
{
    cv::Mat allDescriptors;
    for (const auto& desc : orb_descriptors)
    {
        allDescriptors.push_back(desc);
    }

    cv::Mat labels;
    cv::kmeans(allDescriptors, clusterCount, labels,
        cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 100, 0.1),
        3, cv::KMEANS_PP_CENTERS, this->SIFT_centers);
}

// Hàm này tiến hành so sánh khoảng cách của các đặc trưng SIFT của ảnh truy vấn so với bộ đặc trưng đã tính toán từ trước
// Kết quả trả về là index của ảnh trong mảng ảnh truyền vào ban đầu và độ tương đồng của ảnh tương ứng với index đó
// Sau đó tiến hành sort lại để có được những ảnh có độ tương đồng giảm dần so với ảnh đã truy vấn
std::vector<std::pair<int, double>> System::compareSIFT(const cv::Mat& queryImage)
{
    cv::Ptr<cv::SIFT> sift = cv::SIFT::create();

    // Tính toán SIFT cho ảnh truy vấn
    std::vector<cv::KeyPoint> queryKeypoints;
    cv::Mat queryDescriptors;
    sift->detectAndCompute(queryImage, cv::noArray(), queryKeypoints, queryDescriptors);


    if (queryDescriptors.type() != CV_32F) {
        queryDescriptors.convertTo(queryDescriptors, CV_32F); // Chuyển đổi sang CV_32F
    }
    
    std::vector<std::pair<int, double>> results;
    

    for (size_t i = 0; i < sift_descriptors.size(); ++i)
    {
        cv::BFMatcher matcher(cv::NORM_L2, true); // Sử dụng matcher với chuẩn L2
        std::vector<cv::DMatch> matches;
        matcher.match(queryDescriptors, sift_descriptors[i], matches);

        double totalDist = 0.0;
        for (const auto& match : matches)
        {
            totalDist += match.distance;
        }
        double avgDist = totalDist / matches.size();
        results.push_back({ i, avgDist });
    }

    // Sắp xếp kết quả theo khoảng cách tăng dần => độ tương đồng giảm dần
    std::sort(results.begin(), results.end(), [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
        return a.second < b.second;
        });

    return results;

    //std::vector<std::pair<int, double>> matchesResult;

    //for (size_t i = 0; i < sift_descriptors.size(); ++i) {
    //    cv::BFMatcher matcher(cv::NORM_L2, true);
    //    std::vector<std::vector<cv::DMatch>> matches;
    //    matcher.knnMatch(queryDescriptors, sift_descriptors[i], matches, 2);

    //    double goodMatches = 0;
    //    for (size_t j = 0; j < matches.size(); ++j) {
    //        if (matches[j][0].distance < 0.75 * matches[j][1].distance) {
    //            goodMatches++;
    //        }
    //    }

    //    matchesResult.push_back(std::make_pair(i, goodMatches));
    //}

    //// Sắp xếp các kết quả theo số lượng matches tốt giảm dần
    //std::sort(matchesResult.begin(), matchesResult.end(), [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
    //    return a.second > b.second;
    //    });

    //return matchesResult;
}

// Hàm này tiến hành so sánh khoảng cách của các đặc trưng ORB của ảnh truy vấn so với bộ đặc trưng đã tính toán từ trước
// Kết quả trả về là index của ảnh trong mảng ảnh truyền vào ban đầu và độ tương đồng của ảnh tương ứng với index đó
// Sau đó tiến hành sort lại để có được những ảnh có độ tương đồng giảm dần so với ảnh đã truy vấn
std::vector<std::pair<int, double>> System::compareORB(const cv::Mat& queryImage)
{
    cv::Ptr<cv::ORB> orb = cv::ORB::create();

    // Tính toán ORB cho ảnh truy vấn
    std::vector<cv::KeyPoint> queryKeypoints;
    cv::Mat queryDescriptors;
    orb->detectAndCompute(queryImage, cv::noArray(), queryKeypoints, queryDescriptors);

    if (queryDescriptors.type() != CV_8U) {
        queryDescriptors.convertTo(queryDescriptors, CV_8U); // Chuyển đổi sang CV_8U
    }

    // So sánh với các ORB descriptors trong dataset
    std::vector<std::pair<int, double>> results;

    for (size_t i = 0; i < orb_descriptors.size(); ++i)
    {
        cv::BFMatcher matcher(cv::NORM_HAMMING, true); // Sử dụng matcher với chuẩn Hamming
        std::vector<cv::DMatch> matches;
        matcher.match(queryDescriptors, orb_descriptors[i], matches);

        double totalDist = 0.0;
        for (const auto& match : matches)
        {
            totalDist += match.distance;
        }
        double avgDist = totalDist / matches.size();
        results.push_back({ i, avgDist });
    }

    // Sắp xếp kết quả theo khoảng cách tăng dần => độ tương đồng giảm dần
    std::sort(results.begin(), results.end(), [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
        return a.second < b.second;
        });

    return results;
    // So sánh với các ORB descriptors trong dataset
    /*std::vector<std::pair<int, double>> results;

    for (size_t i = 0; i < orb_descriptors.size(); ++i)
    {
        cv::BFMatcher matcher(cv::NORM_HAMMING);
        std::vector<std::vector<cv::DMatch>> matches;
        matcher.knnMatch(queryDescriptors, orb_descriptors[i], matches, 2);

        double goodMatches = 0;
        for (size_t j = 0; j < matches.size(); ++j)
        {
            if (matches[j][0].distance < 0.75 * matches[j][1].distance)
            {
                goodMatches++;
            }
        }

        results.push_back(std::make_pair(i, goodMatches));*/
}


// Hàm này được sử dụng để tiến hành tìm kiếm các ảnh trong bộ dữ liệu so với ảnh truy vấn
// 3 tham số truyền vào bao gồm: Ảnh truy vấn, loại truy vấn (SIFT - histogram), số lượng kết quả trả về
// Kết quả trả về sẽ là ảnh + id ảnh (Đã được chuyển về int để dễ dàng so sánh khi thực hiện truy xuất trong file .csv để tìm label cho
// việc tính toán độ đo mAP sau này)
std::vector<std::pair<cv::Mat, int>> System::searchImage(const cv::Mat& queryImage, const char* type_of_searching, int numResults)
{
    std::vector<std::pair<int, double>> results;
    if (strcmp(type_of_searching, "SIFT") == 0)
    {
        results = compareSIFT(queryImage);
    }
    else if (strcmp(type_of_searching, "histogram") == 0)
    {
        results = compareHistograms(queryImage);
    }
    else if (strcmp(type_of_searching, "ORB") == 0)
    {
        results = compareORB(queryImage);
    }

    std::vector<std::pair<cv::Mat, int>> matchedImages;
    for (size_t i = 0; i < std::min(static_cast<size_t>(numResults), results.size()); ++i)
    {
        int imageIndex = results[i].first;
        const cv::Mat& matchedImage = getImages()[imageIndex];

        // Kiểm tra nếu ảnh truy vấn nằm trong tập dữ liệu
        if (cv::norm(queryImage, matchedImage, cv::NORM_L2) == 0)
        {
            continue; // Bỏ qua ảnh truy vấn
        }

        matchedImages.push_back({ getImages()[results[i].first], getImageNames()[results[i].first] });
    }

    return matchedImages;
}

// Hàm tính độ đo mAP, nó sẽ tiến hành lặp trong bộ dữ liệu DataCSV đã lưu từ trước để tìm id của các ảnh kết quả. Sau khi tìm thấy
// được id của các ảnh kết quả, nó truy ra label của các ảnh kết quả này và tiến hành so sánh chuỗi so với label của ảnh truy vấn
double System::calculateMAP(const std::vector<std::pair<cv::Mat, int>>& results, const std::string& queryLabel, Index index, int numResults)
{
    /*int predictTrue = 0;
    for (const auto& result : results) {
        int resultId = result.second;

        std::string resultLabel;
        bool resultFound = false;
        for (const auto& entry : index.getDataCSV()) {
            if (entry.id == resultId) {
                resultLabel = entry.label;
                resultFound = true;
                break;
            }
        }

        if (resultFound && strcmp(resultLabel.c_str(), queryLabel.c_str()) == 0) {
            predictTrue++;
            std::cout << "Corrected Image ID: " << resultId << " with label: " << resultLabel << std::endl;
        }
    }

    return static_cast<double>(predictTrue) / numResults;*/
    int truePositives = 0;
    double sumPrecision = 0.0;

    // Đếm số lượng kết quả đúng trong kết quả trả về
    for (int i = 0; i < results.size(); ++i)
    {
        int resultId = results[i].second;
        std::string resultLabel;
        bool resultFound = false;

        for (const auto& entry : index.getDataCSV())
        {
            if (entry.id == resultId)
            {
                resultLabel = entry.label;
                resultFound = true;
                break;
            }
        }

        if (resultFound && resultLabel == queryLabel)
        {
            truePositives++;
            if (i == 0)
                sumPrecision += 1;
            else
            {
                //std::cerr << "Index of right results: " << i + 1 << std::endl;
                sumPrecision += (numResults - (i + 1)) * 1.0 / numResults;
            }
        }
    }

    // Nếu không có kết quả đúng nào, trả về AP là 0 để tránh chia cho 0
    if (truePositives == 0) {
        return 0.0;
    }

    // Tính average precision
    //std::cerr << "Number of right results: " << truePositives << std::endl;
    double averagePrecision = sumPrecision / truePositives;

    return averagePrecision;
}

