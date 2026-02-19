#include <dlib/dnn.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <opencv2/opencv.hpp>
#include <sqlite3.h>
#include <gpiod.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <unistd.h>

#define RELAY_PIN 17
#define REED_PIN 27
#define CHIPNAME "gpiochip0"

struct User {
    int id;
    std::string name;
    std::vector<double> embedding;
};

double computeDistance(const std::vector<double>& a,
                       const std::vector<double>& b) {
    double sum = 0;
    for(int i = 0; i < 128; i++)
        sum += (a[i] - b[i]) * (a[i] - b[i]);
    return sqrt(sum);
}

std::vector<User> loadDatabase() {
    sqlite3* db;
    sqlite3_open("faces.db", &db);

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "SELECT id,name,embedding FROM users", -1, &stmt, NULL);

    std::vector<User> users;

    while(sqlite3_step(stmt) == SQLITE_ROW) {
        User user;
        user.id = sqlite3_column_int(stmt, 0);
        user.name = (const char*)sqlite3_column_text(stmt, 1);

        const void* blob = sqlite3_column_blob(stmt, 2);
        int size = sqlite3_column_bytes(stmt, 2);

        double* data = (double*)blob;
        user.embedding.assign(data, data + 128);

        users.push_back(user);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return users;
}

void unlockDoor() {
    struct gpiod_chip *chip = gpiod_chip_open_by_name(CHIPNAME);
    struct gpiod_line *relay = gpiod_chip_get_line(chip, RELAY_PIN);
    gpiod_line_request_output(relay, "unlock", 1);

    sleep(5);

    gpiod_line_set_value(relay, 0);
    gpiod_chip_close(chip);
}

int main() {
    dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
    dlib::shape_predictor sp;
    dlib::deserialize("shape_predictor_68_face_landmarks.dat") >> sp;

    dlib::anet_type net;
    dlib::deserialize("dlib_face_recognition_resnet_model_v1.dat") >> net;

    std::vector<User> users = loadDatabase();

    cv::VideoCapture cap(0);
    if(!cap.isOpened()) {
        std::cout << "Camera error\n";
        return -1;
    }

    while(true) {
        cv::Mat frame;
        cap >> frame;
        cv::Mat rgb;
        cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);

        dlib::cv_image<dlib::rgb_pixel> img(rgb);
        auto faces = detector(img);

        if(faces.size() > 0) {
            auto shape = sp(img, faces[0]);
            dlib::matrix<dlib::rgb_pixel> face_chip;
            dlib::extract_image_chip(img,
                dlib::get_face_chip_details(shape,150,0.25),
                face_chip);

            std::vector<dlib::matrix<float,0,1>> embeddings = net({face_chip});
            std::vector<double> live(embeddings[0].begin(), embeddings[0].end());

            double bestDist = 999;
            User* bestUser = nullptr;

            for(auto& user : users) {
                double dist = computeDistance(live, user.embedding);
                if(dist < bestDist) {
                    bestDist = dist;
                    bestUser = &user;
                }
            }

            if(bestDist < 0.5 && bestUser != nullptr) {
                std::cout << "Welcome " << bestUser->name << std::endl;
                unlockDoor();
                sleep(2);
            }
        }

        cv::imshow("Face Unlock", frame);
        if(cv::waitKey(1) == 27) break;
    }

    return 0;
}
