//
// Created by Simon on 22/01/2024.
//

#include <opencv2/imgproc/types_c.h>
#include <android/asset_manager.h>
#include "CV_Manager.h"
#include "Encoder.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

using namespace std;
using namespace cv;

void convertYUV_I420toNV12(unsigned char* i420bytes, unsigned char* nv12bytes, int width, int height)
{
    int nLenY = width * height;
    int nLenU = nLenY / 4;
    memcpy(nv12bytes, i420bytes, width * height);
    for (int i = 0; i < nLenU; i++) {
        nv12bytes[nLenY + 2 * i] = i420bytes[nLenY + i];             // U
        nv12bytes[nLenY + 2 * i + 1] = i420bytes[nLenY + nLenU + i]; // V
    }
}

void BGR2YUV_nv12(cv::Mat &src, cv::Mat &dst)
{
    int w_img = src.cols;
    int h_img = src.rows;
    dst = cv::Mat(h_img*1.5, w_img, CV_8UC1, cv::Scalar(0));
    cv::Mat YUV_I420(h_img*1.5, w_img, CV_8UC1, cv::Scalar(0));  //YUV_I420
    cv::cvtColor(src, YUV_I420, cv::COLOR_BGR2YUV_I420);
    convertYUV_I420toNV12(YUV_I420.data, dst.data, w_img, h_img);
}

CV_Manager::CV_Manager()
        : m_camera_ready(false), m_image(nullptr), m_image_reader(nullptr),
          m_native_camera(nullptr) {
};

CV_Manager::~CV_Manager() {
    // ACameraCaptureSession_stopRepeating(m_capture_session);
    if (m_native_camera != nullptr) {
        delete m_native_camera;
        m_native_camera = nullptr;
    }

    // make sure we don't leak native windows
    if (m_native_window != nullptr) {
        ANativeWindow_release(m_native_window);
        m_native_window = nullptr;
    }

    if (m_image_reader != nullptr) {
        delete (m_image_reader);
        m_image_reader = nullptr;
    }
}

void CV_Manager::SetNativeWindow(ANativeWindow *native_window) {
    // Save native window
    m_native_window = native_window;
}

void CV_Manager::SetUpCamera() {

    m_native_camera = new Native_Camera(m_selected_camera_type);

    m_native_camera->MatchCaptureSizeRequest(&m_view,
                                             ANativeWindow_getWidth(m_native_window),
                                             ANativeWindow_getHeight(m_native_window));

    ASSERT(m_view.width && m_view.height, "Could not find supportable resolution");

    // Here we set the buffer to use RGBX_8888 as default might be; RGB_565
    ANativeWindow_setBuffersGeometry(m_native_window, m_view.width, m_view.height,
                                     WINDOW_FORMAT_RGBX_8888);

    m_image_reader = new Image_Reader(&m_view, AIMAGE_FORMAT_YUV_420_888);
    m_image_reader->SetPresentRotation(m_native_camera->GetOrientation());

    ANativeWindow *image_reader_window = m_image_reader->GetNativeWindow();

    m_camera_ready = m_native_camera->CreateCaptureSession(image_reader_window);
}

void CV_Manager::CameraLoop() {

    bool buffer_printout = false;
    bool isImageSizeSent = false;

    SetUpFaceDetectionAI();

    while (1) {

        if (m_camera_thread_stopped) { break; }
        if (!m_camera_ready || !m_image_reader) { continue; }
        m_image = m_image_reader->GetLatestImage();
        if (m_image == nullptr) { continue; }
        ANativeWindow_acquire(m_native_window);
        ANativeWindow_Buffer buffer;
        if (ANativeWindow_lock(m_native_window, &buffer, nullptr) < 0) {
            m_image_reader->DeleteImage(m_image);
            m_image = nullptr;
            continue;
        }

        if (false == buffer_printout) {
            buffer_printout = true;
            LOGI("/// H-W-S-F: %d, %d, %d, %d", buffer.height, buffer.width, buffer.stride, buffer.format);
        }
        m_image_reader->DisplayImage(&buffer, m_image);
        display_mat = cv::Mat(buffer.height, buffer.stride, CV_8UC4, buffer.bits);

        //BarcodeDetect(display_mat);
        FaceDetection(display_mat);

        ANativeWindow_unlockAndPost(m_native_window);
        ANativeWindow_release(m_native_window);

        if(!isImageSizeSent){
            isImageSizeSent=true;
            int cols;
            int* ptrCols = &cols;

            int rows;
            int* ptrRows = &rows;

            AImage_getWidth(m_image,ptrCols);
            AImage_getHeight(m_image,ptrRows);

            do {
                m_Encode->InitCodec(display_mat.rows, display_mat.cols, 15,
                                    20000); //480, 640, 15, 100000

                m_retrievedImageWidth = display_mat.cols;
                m_retrievedImageHeight = display_mat.rows;

            }while(m_Encode->getStatus() != AMEDIA_OK);
        }
        BGR2YUV_nv12(display_mat, outputMat);
        int yPlaneSize = outputMat.total()*outputMat.elemSize();
        m_Encode->Encode(outputMat.data, yPlaneSize);

        ReleaseMats();

    }
    FlipCamera();

}

void CV_Manager::BarcodeDetect(Mat &frame) {
    int ddepth = CV_16S;

    // Convert to grayscale
    cvtColor(frame, frame_gray, CV_RGBA2GRAY);

    // Gradient X
    Sobel(frame_gray, grad_x, ddepth, 1, 0);
    convertScaleAbs(grad_x, abs_grad_x);
    // Gradient Y
    Sobel(frame_gray, grad_y, ddepth, 0, 1);
    convertScaleAbs(grad_y, abs_grad_y);

    // Total Gradient (approximate)
    addWeighted(abs_grad_x, 0.5, abs_grad_x, 0.5, 0, detected_edges);

    // Reduce noise with a 3x3 kernel
    GaussianBlur(detected_edges, detected_edges, Size(3,3), 0, 0, BORDER_DEFAULT);

    // Reducing noise further by using threshold
    threshold(detected_edges, thresh, 120, 255, THRESH_BINARY);

    // Otsu's threshold
    threshold(thresh, thresh, 0, 255, THRESH_BINARY+THRESH_OTSU);

    // Close gaps using a closing kernel
    kernel = getStructuringElement(MORPH_RECT, Size(21,7));
    morphologyEx(thresh, cleaned, MORPH_CLOSE, kernel);

    // Perform erosions and dilations
    erode(cleaned, cleaned, anchor, Point(-1,-1), 4);
    dilate(cleaned, cleaned, anchor, Point(-1,-1), 4);

    // Extract all contours
    findContours(cleaned, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // Sort contours in ascending order
    std::sort(contours.begin(), contours.end(), [](const vector<Point>& c1, const vector<Point>& c2) {
        return contourArea(c1, false) < contourArea(c2, false);
    });

    // Draw the largest contour
    drawContours(frame, contours, int(contours.size()-1), CV_GREEN, 2, LINE_8, hierarchy, 0, Point());
}

void CV_Manager::initFilePath(String FilePath) {
    AbsoluteFilePath = FilePath;
}

void CV_Manager::initAssetManager(AAssetManager* mgr){
    m_AssetManager = mgr;
};

void CV_Manager::FaceDetection(Mat &frame ) {
    vector<Rect> faces, faces2;
    Mat gray, smallImg;

    double scale=1;



    cv::CascadeClassifier cascade;
    AAsset* config_asset = AAssetManager_open(m_AssetManager, "haarcascade_frontalface_default.xml", AASSET_MODE_BUFFER);
    const char* data = (const char*) AAsset_getBuffer(config_asset);
    cv::FileStorage fs(data, cv::FileStorage::READ | cv::FileStorage::MEMORY);


    bool test4 = cascade.read(fs.getFirstTopLevelNode());
    bool test3 = cascade.empty();


    //if(!cascade.read(fs.root())){
    //    LOGI("---------- nique! ---------");
    //}

    //AAsset_close(assetFile);

    cvtColor( frame, gray, COLOR_BGR2GRAY ); // Convert to Gray Scale
    double fx = 1 / scale;

    // Resize the Grayscale Image
    resize( gray, smallImg, Size(), fx, fx, INTER_LINEAR );
    equalizeHist( smallImg, smallImg );
    //cascade.load

    // Detect faces of different sizes using cascade classifier
    cascade.detectMultiScale( smallImg, faces, 1.1,
                              2, 0|CASCADE_SCALE_IMAGE, Size(30, 30) );

    // Draw circles around the faces
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        Rect r = faces[i];
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = Scalar(255, 0, 0); // Color for Drawing tool
        int radius;

        double aspect_ratio = (double)r.width/r.height;
        if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
        {
            center.x = cvRound((r.x + r.width*0.5)*scale);
            center.y = cvRound((r.y + r.height*0.5)*scale);
            radius = cvRound((r.width + r.height)*0.25*scale);
            circle( frame, center, radius, color, 3, 8, 0 );
        }
        else
            rectangle( frame, cvPoint(cvRound(r.x*scale), cvRound(r.y*scale)),
                       cvPoint(cvRound((r.x + r.width-1)*scale),
                               cvRound((r.y + r.height-1)*scale)), color, 3, 8, 0);
        //if( nestedCascade.empty() )
        //    continue;
        smallImgROI = smallImg( r );

        // Detection of eyes in the input image
        /*nestedCascade.detectMultiScale( smallImgROI, nestedObjects, 1.1, 2,
                                        0|CASCADE_SCALE_IMAGE, Size(30, 30) );*/

        // Draw circles around eyes
        for ( size_t j = 0; j < nestedObjects.size(); j++ )
        {
            Rect nr = nestedObjects[j];
            center.x = cvRound((r.x + nr.x + nr.width*0.5)*scale);
            center.y = cvRound((r.y + nr.y + nr.height*0.5)*scale);
            radius = cvRound((nr.width + nr.height)*0.25*scale);
            circle( frame, center, radius, color, 3, 8, 0 );
        }
    }
}

/*cv::dnn::Net readNetFromCaffeWithAssetManager(const std::string& configuration_path, const std::string& weights_path) {

    AAsset* config_asset = AAssetManager_open(m_AssetManager, configuration_path.c_str(), AASSET_MODE_BUFFER);
    AAsset* weights_asset = AAssetManager_open(m_AssetManager, weights_path.c_str(), AASSET_MODE_BUFFER);

    if (config_asset != nullptr && weights_asset != nullptr) {
        const void* config_data = AAsset_getBuffer(config_asset);
        const void* weights_data = AAsset_getBuffer(weights_asset);
        size_t config_length = AAsset_getLength(config_asset);
        size_t weights_length = AAsset_getLength(weights_asset);

        cv::Mat config_mat(1, config_length, CV_8UC1, const_cast<void*>(config_data));
        cv::Mat weights_mat(1, weights_length, CV_8UC1, const_cast<void*>(weights_data));

        cv::dnn::Net network = cv::dnn::readNetFromCaffe(config_mat, weights_mat);

        AAsset_close(config_asset);
        AAsset_close(weights_asset);

        return network;
    } else {
        // Gérer les erreurs d'ouverture de fichier.
        throw std::runtime_error("Failed to open asset files");
    }
}*/

void CV_Manager::SetUpFaceDetectionAI(){
    AAsset* config_asset = AAssetManager_open(m_AssetManager, "deploy.prototxt", AASSET_MODE_BUFFER);
    AAsset* weights_asset = AAssetManager_open(m_AssetManager, "res10_300x300_ssd_iter_140000_fp16.caffemodel", AASSET_MODE_BUFFER);
    cv::dnn::Net network_;

    if (config_asset != nullptr && weights_asset != nullptr) {
        const void* config_data = AAsset_getBuffer(config_asset);
        const void* weights_data = AAsset_getBuffer(weights_asset);
        size_t config_length = AAsset_getLength(config_asset);
        size_t weights_length = AAsset_getLength(weights_asset);

        cv::Mat config_mat(1, config_length, CV_8UC1, const_cast<void*>(config_data));
        cv::Mat weights_mat(1, weights_length, CV_8UC1, const_cast<void*>(weights_data));

        cv::dnn::Net network = cv::dnn::readNetFromCaffe(config_mat, weights_mat);

        AAsset_close(config_asset);
        AAsset_close(weights_asset);

        m_network = network;
    } else {
        // Gérer les erreurs d'ouverture de fichier.
        throw std::runtime_error("Failed to open asset files");
    }


    if (m_network.empty()) {
        std::ostringstream ss;
        ss << "Failed to load network with the following settings:\n"
           << "Configuration: assets/deploy.prototxt\n"
           << "Binary: assets/res10_300x300_ssd_iter_140000_fp16.caffemodel\n";
        throw std::invalid_argument(ss.str());
    }
}

void CV_Manager::FaceDetection2(Mat &frame ) {
    const float confidence_threshold_ = 0.85;
    const int input_image_height_ = frame.rows;
    const int input_image_width_ = frame.cols;
    const double scale_factor_= 100;
    const cv::Scalar mean_values_=0.25;

    // Convertir l'image à 3 canaux une seule fois
    cv::Mat image_3_channels;
    cv::cvtColor(frame, image_3_channels, cv::COLOR_RGBA2RGB);

    cv::Mat input_blob = cv::dnn::blobFromImage(image_3_channels,
                                                scale_factor_,
                                                cv::Size(input_image_width_, input_image_height_),
                                                mean_values_,
                                                false,
                                                false);

    m_network.setInput(input_blob, "data");
    cv::Mat detection = m_network.forward("detection_out");
    cv::Mat detection_matrix(detection.size[2],
                             detection.size[3],
                             CV_32F,
                             detection.ptr<float>());

    std::vector<cv::Rect> faces;

    for (int i = 0; i < detection_matrix.rows; i++) {
        float confidence = detection_matrix.at<float>(i, 2);

        if (confidence < confidence_threshold_) {
            continue;
        }
        int x_left_bottom = static_cast<int>(
                detection_matrix.at<float>(i, 3) * frame.cols);

        int y_left_bottom = static_cast<int>(
                detection_matrix.at<float>(i, 4) * frame.rows);

        int x_right_top = static_cast<int>(
                detection_matrix.at<float>(i, 5) * frame.cols);

        int y_right_top = static_cast<int>(
                detection_matrix.at<float>(i, 6) * frame.rows);

        faces.emplace_back(x_left_bottom,
                           y_left_bottom,
                           (x_right_top - x_left_bottom),
                           (y_right_top - y_left_bottom));
    }

    // Dessiner les rectangles de détection sur l'image originale
    cv::Scalar color(0, 105, 205);
    int frame_thickness = 4;
    for(const auto & r : faces){
        cv::rectangle(frame, r, color, frame_thickness);
    }
}


void CV_Manager::ReleaseMats() {
    display_mat.release();
    frame_gray.release();
    grad_x.release();
    abs_grad_x.release();
    grad_y.release();
    abs_grad_y.release();
    detected_edges.release();
    thresh.release();
    kernel.release();
    anchor.release();
    cleaned.release();
    hierarchy.release();
}

void CV_Manager::FlipCamera() {
    m_camera_thread_stopped = false;

    // reset info
    if (m_image_reader != nullptr) {
        delete (m_image_reader);
        m_image_reader = nullptr;
    }
    delete m_native_camera;

    if (m_selected_camera_type == FRONT_CAMERA) {
        m_selected_camera_type = BACK_CAMERA;
    } else {
        m_selected_camera_type = FRONT_CAMERA;
    }
    SetUpCamera();
    std::thread loopThread(&CV_Manager::CameraLoop, this);
    loopThread.detach();
}

void CV_Manager::PauseCamera() {
    if (m_native_camera == nullptr) {
        LOGE("Can't flip camera without camera instance");
        return;
    } else if (m_native_camera->GetCameraCount() < 2) {
        LOGE("Only one camera is available");
        return;
    }

    m_camera_thread_stopped = true;
}

void CV_Manager::SetUpSocket(){
    //int port = 5555;
    int port = 5005;
    const char hostname[] = "192.168.1.19";
    //const char hostname[] = "192.168.154.40";
    m_socket = new SocketClient(hostname, port);
    m_socket->ConnectToServer();
    LOGI("Socket conected");
}

/*void CV_Manager::SetUpSocketWebRTC(){
    //int port = 5555;
    int port = 5005;
    const char hostname[] = "172.16.227.9";
    //const char hostname[] = "192.168.154.40";
    m_socketWebRTC = new SocketClientWebRTC(hostname, port);
    m_socketWebRTC->ConnectToServer();
    LOGI("Socket WebRTC conected");
}*/

void CV_Manager::SetUpEncoder()
{

    //setEncoder(encoder)
    media_status_t test;
    do {
        m_Encode = new Encoder();
        m_Encode->setSocketClientH264(m_socket);

        //Calcul bitrate = weight*height*fps (optimal)
        m_Encode->InitCodec(400, 608, 15, 20000); //480, 640, 15, 100000
        test = m_Encode->getStatus();
    }while(test != AMEDIA_OK);
    if (m_Encode->getStatus() != AMEDIA_OK){
        __android_log_print(ANDROID_LOG_ERROR, "CameraNDK", "Failed to create ancoder");
        //std::cout <<" CameraNDK", Failed to create ancoder" << std::endl
        return;
    }

}

