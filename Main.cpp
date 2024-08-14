#include "Header.h"

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cout << "Begin:" << " <file.exe> <folderPath> <csvPath>" << std::endl;
        return -1;
    }
    
    std::cerr << "Initialization starts!" << std::endl;

    // Đọc từ câu lệnh của người dùng
    std::string folderPath = argv[1];
    std::string csvPath = argv[2];

    // Khởi tạo
    Dataset dataset(folderPath);
    System system(folderPath);
    Index index;
    index.ReadCSV(csvPath);
    system.ensureHistogramsComputed("histograms.yaml");
    system.ensureSIFTComputed("sift_features.yaml");
    system.ensureORBComputed("orb_features.yaml");

    std::cout << "Initialization completed!" << std::endl;


    while (true)
    {
        std::string input;
        std::cout << "Command 1: <type_of_searching> <query_image_path> <num_of_results> - for querying 1 image in dataset" << std::endl;
        std::cout << "Command 2: <type_of_searching> - for querying all images in dataset and collecting mAPs" << std::endl;
        std::cout << "Enter command: ";
        std::getline(std::cin, input);

        if (input == "ESC")
        {
            std::cerr << "Have a good day!" << std::endl;
            break;
        }

        std::istringstream iss(input);
        std::vector<std::string> tokens{ std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} };

        if (tokens.size() == 3) // Thực hiện truy vấn cho 1 ảnh cụ thể
        {
            const char* type_of_searching = tokens[0].c_str();
            std::string queryImagePath = tokens[1];
            int numResults = std::stoi(tokens[2]);

            // Tiến hành đọc ảnh được truy vấn và resize
            cv::Mat queryImage = cv::imread(queryImagePath);
            cv::resize(queryImage, queryImage, cv::Size(72, 128));

            if (queryImage.empty())
            {
                std::cerr << "Error loading query image: " << queryImagePath << std::endl;
                continue;
            }

            // Xử lý tên ảnh truy vấn từ đường dẫn
            std::string queryImageName = queryImagePath.substr(queryImagePath.find_last_of("/\\") + 1);
            queryImageName = queryImageName.substr(0, queryImageName.find_last_of('.'));
            int queryImageId = std::stoi(queryImageName);

            // Tìm nhãn của ảnh truy vấn trong CSV
            std::string queryLabel;
            bool queryFound = false;
            for (const auto& entry : index.getDataCSV())
            {
                if (entry.id == queryImageId)
                {
                    queryLabel = entry.label;
                    queryFound = true;
                    break;
                }
            }

            if (!queryFound)
            {
                std::cerr << "Error finding query image label in CSV" << std::endl;
                continue;
            }

            std::cout << "Query Label: " << queryLabel << std::endl;

            // Bắt đầu tính thời gian truy vấn chương trình
            auto start = std::chrono::high_resolution_clock::now();
            std::vector<std::pair<cv::Mat, int>> results = system.searchImage(queryImage, type_of_searching, numResults);

            // Kết thúc tính thời gian truy vấn
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end - start;
            std::cout << "Time taken for the " << type_of_searching << " query: " << duration.count() << " seconds" << std::endl;

            std::cout << "Top matched images using " << type_of_searching << ":" << std::endl;
            for (const auto& result : results)
            {
                std::cout << "Image: " << result.second << std::endl;
            }

            double mapScore = system.calculateMAP(results, queryLabel, index, numResults);
            std::cout << "mAP score: " << mapScore << std::endl;

            std::vector<cv::Mat> matchedImages;
            for (const auto& result : results)
            {
                matchedImages.push_back(result.first);
            }
            dataset.ShowImages(matchedImages, "Matched Images");
        }
        else if (tokens.size() == 1) // Thực hiện truy vấn cho từng ảnh trong dataset
        {
            // Bắt đầu tính thời gian thực thi chương trình
            auto start = std::chrono::high_resolution_clock::now();

            const char* type_of_searching = tokens[0].c_str();

            if (type_of_searching == "ESC" || type_of_searching == "esc")
            {
                std::cerr << "Have a good day!" << std::endl;
                break;
            }

            std::vector<int> numResultsArray = { 3, 5, 11, 21 };

            // Chạy truy vấn cho 1120 hình ảnh
            for (int i = 0; i < 1120; ++i)
            {
                // Tiến hành đọc ảnh truy vấn từ dataset đã load trước đó
                cv::Mat queryImage = dataset.getImages()[i];
                int queryImageId = dataset.getImageNames()[i];

                // Tìm nhãn của ảnh truy vấn trong CSV
                std::string queryLabel;
                DataCSV* queryDataCSV = nullptr;
                bool queryFound = false;
                for (auto& entry : index.getDataCSV())
                {
                    if (entry.id == queryImageId)
                    {
                        queryLabel = entry.label;
                        queryFound = true;
                        queryDataCSV = &entry;
                        break;
                    }
                }

                if (!queryFound)
                {
                    std::cerr << "Error finding query image label in CSV for image ID: " << queryImageId << std::endl;
                    continue;
                }

                std::cout << "Query Label for Image ID " << queryImageId << ": " << queryLabel << std::endl;

                // Tính toán mAP cho các số lượng kết quả khác nhau
                for (int numResults : numResultsArray)
                {

                    std::vector<std::pair<cv::Mat, int>> results = system.searchImage(queryImage, type_of_searching, numResults);

                    double mapScore = system.calculateMAP(results, queryLabel, index, numResults);
                    std::cout << "mAP score for " << numResults << " results: " << mapScore << std::endl;

                    // Cập nhật giá trị mAP vào cấu trúc dữ liệu
                    if (numResults == 3)
                        queryDataCSV->map3 = mapScore;
                    else if (numResults == 5)
                        queryDataCSV->map5 = mapScore;
                    else if (numResults == 11)
                        queryDataCSV->map11 = mapScore;
                    else if (numResults == 21)
                        queryDataCSV->map21 = mapScore;
                }
            }

            // Đọc file CSV hiện tại
            std::ifstream file(csvPath);
            std::string line;
            std::vector<std::string> csvLines;

            if (!file.is_open())
            {
                std::cerr << "Can't open the CSV file!" << std::endl;
                return -1;
            }

            // Đọc tiêu đề
            std::getline(file, line);
            csvLines.push_back(line + ",AP_3,AP_5,AP_11,AP_21");

            // Đọc từng dòng trong file CSV
            int indexDataCSV = 0;
            while (std::getline(file, line))
            {
                if (indexDataCSV < index.getDataCSV().size())
                {
                    const auto& entry = index.getDataCSV()[indexDataCSV];
                    line += "," + std::to_string(entry.map3) + "," +
                        std::to_string(entry.map5) + "," +
                        std::to_string(entry.map11) + "," +
                        std::to_string(entry.map21);
                    csvLines.push_back(line);
                    ++indexDataCSV;
                }
            }
            file.close();

            // Ghi lại vào file CSV hiện tại
            std::ofstream outFile(csvPath);
            if (!outFile.is_open())
            {
                std::cerr << "Error opening CSV file for writing." << std::endl;
                return -1;
            }

            for (const auto& csvLine : csvLines)
            {
                outFile << csvLine << std::endl;
            }
            outFile.close();

            // Kết thúc tính thời gian truy vấn
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end - start;
            std::cout << "Total time taken for the " << type_of_searching << " query: " << duration.count() << " seconds" << std::endl;
        }
        else
        {
            std::cout << "Invalid command. Please try again." << std::endl;
        }
    }

}
