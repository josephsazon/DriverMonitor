#include "mainwindow.h"
#include "ui_mainwindow.h"

QCPItemLine *HeadTurnLeft_threshold_line;
QCPItemLine *HeadTurnRight_threshold_line;
QCPItemLine *Blink_threshold_line;
QCPItemLine *YawnMouth_threshold_line;

int HeadTurnLeft_threshold = 65;
int HeadTurnRight_threshold = 35;
int Blink_threshold = 25;
int YawnMouth_threshold = 30;
int YawnEyes_threshold = 35;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->HeadTurn_plot->addGraph();

    ui->HeadTurn_plot->yAxis->setRange(0, 100);
    ui->HeadTurn_plot->setInteractions(QCP::iSelectItems | QCP::iRangeDrag);
    ui->HeadTurn_plot->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->HeadTurn_plot->graph(0)->setLineStyle(QCPGraph::lsLine);

    HeadTurnLeft_threshold_line = new QCPItemLine(ui->HeadTurn_plot);
    HeadTurnLeft_threshold_line->setSelectable(true);
    HeadTurnLeft_threshold_line->setPen(QPen(Qt::DashDotLine));
    HeadTurnLeft_threshold_line->start->setCoords(QCPRange::minRange, 65);
    HeadTurnLeft_threshold_line->end->setCoords(QCPRange::maxRange, 65);

    HeadTurnRight_threshold_line = new QCPItemLine(ui->HeadTurn_plot);
    HeadTurnRight_threshold_line->setSelectable(true);
    HeadTurnRight_threshold_line->setPen(QPen(Qt::DashDotLine));
    HeadTurnRight_threshold_line->start->setCoords(QCPRange::minRange, 35);
    HeadTurnRight_threshold_line->end->setCoords(QCPRange::maxRange, 35);

    ui->Blink_plot->addGraph();
    ui->Blink_plot->addGraph();

    ui->Blink_plot->yAxis->setRange(0, 100);
    ui->Blink_plot->setInteractions(QCP::iSelectItems | QCP::iRangeDrag);
    ui->Blink_plot->axisRect()->setRangeDrag(Qt::Horizontal);

    ui->Blink_plot->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->Blink_plot->graph(0)->setPen(QPen(Qt::red));

    //ui->Blink_plot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
    ui->Blink_plot->graph(1)->setLineStyle(QCPGraph::lsLine);
    ui->Blink_plot->graph(1)->setPen(QPen(Qt::green));

    Blink_threshold_line = new QCPItemLine(ui->Blink_plot);
    Blink_threshold_line->setSelectable(true);
    Blink_threshold_line->setPen(QPen(Qt::DashDotLine));
    Blink_threshold_line->start->setCoords(QCPRange::minRange, 25);
    Blink_threshold_line->end->setCoords(QCPRange::maxRange, 25);

    ui->Yawn_plot->addGraph();

    ui->Yawn_plot->yAxis->setRange(0, 100);
    ui->Yawn_plot->setInteractions(QCP::iSelectItems | QCP::iRangeDrag);
    ui->Yawn_plot->axisRect()->setRangeDrag(Qt::Horizontal);

    ui->Yawn_plot->graph(0)->setLineStyle((QCPGraph::lsLine));
    ui->Yawn_plot->graph(0)->setPen(QPen(Qt::green));

    YawnMouth_threshold_line = new QCPItemLine(ui->Yawn_plot);
    YawnMouth_threshold_line->setSelectable(true);
    YawnMouth_threshold_line->setPen(QPen(Qt::DashDotLine));
    YawnMouth_threshold_line->start->setCoords(QCPRange::minRange, 30);
    YawnMouth_threshold_line->end->setCoords(QCPRange::maxRange, 30);

    timer = new QTimer(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_open_camera_clicked()
{
    cap.open(0);

    if(!cap.isOpened())
    {
        cout << "camera is not open" << endl;
    }
    else
    {
        cout << "camera is open" << endl;

        connect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
        timer->start(20);

        ui->pushButton_close_camera->setEnabled(true);

        detector = get_frontal_face_detector();
        deserialize("C://shape_predictor_68_face_landmarks.dat") >> shape_model;
    }

    HeadTurn_timer.start();
    Blink_timer.start();
    Yawn_timer.start();
}

void MainWindow::on_pushButton_close_camera_clicked()
{
    disconnect(timer, SIGNAL(timeout()), this, SLOT(update_window()));
    cap.release();

    Mat image = Mat::zeros(frame.size(), CV_8UC3);
    show_frame(image);

    cout << "camera is closed" << endl;

    FaceLeft_instanceTrigger = false;
    FaceRight_instanceTrigger = false;
    Blink_instanceTrigger = false;
    Yawn_instanceTrigger = false;
}

void MainWindow::update_window()
{
    cap >>  frame;

    array2d<bgr_pixel> dlib_image;
    assign_image(dlib_image, cv_image<bgr_pixel>(frame));

    std::vector<dlib::rectangle> faces = detector(dlib_image);

    //int number_of_detected_faces = faces.size();
    //cout << "Number of detected faces : " << number_of_detected_faces << endl;

    std::vector<full_object_detection> shapes;
    for(unsigned long i = 0; i < faces.size(); i++)
    {
        full_object_detection shape = shape_model(dlib_image, faces[i]);
        shapes.push_back(shape);

        face_layout(shape);

        driver_monitor FaceLeft(shape);
        FaceLeft.measure(30, 36, 'x', 35, 31, 'x', 10);

        driver_monitor FaceRight(shape);
        FaceRight.measure(45, 30, 'x', 35, 31, 'x', 10);

        if(ui->HeadTurnLeft_spinBox->value() != HeadTurnLeft_threshold)
        {
            HeadTurnLeft_threshold = ui->HeadTurnLeft_spinBox->value();
            ui->HeadTurn_plot->removeItem(HeadTurnLeft_threshold_line);
            HeadTurnLeft_threshold_line = new QCPItemLine(ui->HeadTurn_plot);
            HeadTurnLeft_threshold_line->setPen(QPen(Qt::DashDotLine));
            HeadTurnLeft_threshold_line->start->setCoords(QCPRange::minRange, HeadTurnLeft_threshold);
            HeadTurnLeft_threshold_line->end->setCoords(QCPRange::maxRange, HeadTurnLeft_threshold);
        }

        FaceLeft.head_parameters(FaceLeft.facial_feature, FaceRight.facial_feature);
        FaceLeft.instance('h', FaceLeft_instanceTrigger, HeadTurn_timer, FaceLeft.percent, HeadTurnLeft_threshold);
        FaceLeft.classify('l', FaceLeft_instanceTrigger, HeadTurn_timer, HeadTurn_thresholdTime, FaceLeft_displayTrigger);
        FaceLeft.instance_rate(HeadTurn_rate, HeadTurn_refreshRate, HeadTurn_thresholdTime, FaceLeft_displayTrigger);        
        FaceLeft.guiDisplay_text(main_string, HeadTurn_string, HeadTurn_count, FaceLeft_displayTrigger);

        FaceLeft.DriverStatus_Distracted(ui->HeadTurn_timeLimit_spinBox->value(), HeadTurn_timer, HeadTurn_count, main_string);

        if(ui->HeadTurnRight_spinBox->value() != HeadTurnRight_threshold)
        {
            HeadTurnRight_threshold = ui->HeadTurnRight_spinBox->value();
            ui->HeadTurn_plot->removeItem(HeadTurnRight_threshold_line);
            HeadTurnRight_threshold_line = new QCPItemLine(ui->HeadTurn_plot);
            HeadTurnRight_threshold_line->setPen(QPen(Qt::DashDotLine));
            HeadTurnRight_threshold_line->start->setCoords(QCPRange::minRange, HeadTurnRight_threshold);
            HeadTurnRight_threshold_line->end->setCoords(QCPRange::maxRange, HeadTurnRight_threshold);
        }

        FaceRight.head_parameters(FaceRight.facial_feature, FaceLeft.facial_feature);
        FaceRight.instance('h', FaceRight_instanceTrigger, HeadTurn_timer, FaceRight.percent, 65);
        FaceRight.classify('r', FaceRight_instanceTrigger, HeadTurn_timer, HeadTurn_thresholdTime, FaceRight_displayTrigger);
        FaceRight.instance_rate(HeadTurn_rate, HeadTurn_refreshRate, HeadTurn_thresholdTime, FaceRight_displayTrigger);        
        FaceRight.guiDisplay_text(main_string, HeadTurn_string, HeadTurn_count, FaceRight_displayTrigger);

        FaceRight.DriverStatus_Distracted(ui->HeadTurn_timeLimit_spinBox->value(), HeadTurn_timer, HeadTurn_count, main_string);

        HeadTurn_plot_data.append(double(FaceLeft.percent));
        xAxis_plot_data.append(double(HeadTurn_plot_data.size()));

        ui->HeadTurn_plot->graph(0)->setData(xAxis_plot_data, HeadTurn_plot_data);
        ui->HeadTurn_plot->xAxis->setRange((HeadTurn_plot_data.size()), 100, Qt::AlignRight);
        ui->HeadTurn_plot->replot();
        ui->HeadTurn_plot->update();

        if(FaceLeft_instanceTrigger == true || FaceRight_instanceTrigger == true)
            ui->HeadTurn_trigger_indicator->setStyleSheet("QLabel { background-color : lime; }");
        else
            ui->HeadTurn_trigger_indicator->setStyleSheet("QLabel { background-color : darkgreen; }");

        driver_monitor LeftEye(shape);
        LeftEye.measure(41, 37, 'y', 39, 36,'x', 100);
        LeftEye.eye_parameters(LeftEye_max, LeftEye_min);

        driver_monitor RightEye(shape);
        RightEye.measure(46, 44, 'y', 45, 42, 'x', 100);
        RightEye.eye_parameters(RightEye_max, RightEye_min);

        LeftEye_plot_data.append(LeftEye.percent);
        ui->Blink_plot->graph(0)->setData(xAxis_plot_data, LeftEye_plot_data);
        RightEye_plot_data.append(RightEye.percent);
        ui->Blink_plot->graph(1)->setData(xAxis_plot_data, RightEye_plot_data);

        ui->Blink_plot->xAxis->setRange((xAxis_plot_data.size()), 100, Qt::AlignRight);
        ui->Blink_plot->replot();
        ui->Blink_plot->update();

        if(ui->Blink_spinBox->value() != Blink_threshold)
        {
            Blink_threshold = ui->Blink_spinBox->value();
            ui->Blink_plot->removeItem(Blink_threshold_line);
            Blink_threshold_line = new QCPItemLine(ui->Blink_plot);
            Blink_threshold_line->setPen(QPen(Qt::DashDotLine));
            Blink_threshold_line->start->setCoords(QCPRange::minRange, Blink_threshold);
            Blink_threshold_line->end->setCoords(QCPRange::maxRange, Blink_threshold);
        }

        driver_monitor Blink(shape);
        Blink.instance('b', Blink_instanceTrigger, Blink_timer, LeftEye.percent, Blink_threshold, RightEye.percent, Blink_threshold);
        Blink.classify('b', Blink_instanceTrigger, Blink_timer, Blink_thresholdTime, Blink_displayTrigger);
        Blink.instance_rate(Blink_rate, Blink_refreshRate, Blink_thresholdTime, Blink_displayTrigger);
        Blink.instance_rate(SlowBlink_rate, SlowBlink_refreshRate, SlowBlink_thresholdTime, Blink_displayTrigger);
        Blink.guiDisplay_text(main_string, Blink_string, Blink_count, Blink_displayTrigger);

        if(Blink_instanceTrigger == true)
            ui->Blink_trigger_indicator->setStyleSheet("QLabel { background-color : lime; }");
        else
            ui->Blink_trigger_indicator->setStyleSheet("QLabel { background-color : darkgreen; }");

        if(ui->YawnEyes_spinBox->value() != YawnEyes_threshold)
        {
            YawnEyes_threshold = ui->YawnEyes_spinBox->value();
        }

        Blink.DriverStatus_Drowsy(ui->SlowBlink_rate_spinBox->value(), SlowBlink_rate, main_string);
        Blink.DriverStatus_Asleep(ui->ClosedEyes_timeLimit_spinBox->value(), Blink_timer, Blink_count, main_string);

        if(ui->YawnMouth_spinBox->value() != YawnMouth_threshold)
        {
            YawnMouth_threshold = ui->YawnMouth_spinBox->value();
            ui->Yawn_plot->removeItem(YawnMouth_threshold_line);
            YawnMouth_threshold_line = new QCPItemLine(ui->Yawn_plot);
            YawnMouth_threshold_line->setPen(QPen(Qt::DashDotLine));
            YawnMouth_threshold_line->start->setCoords(QCPRange::minRange, YawnMouth_threshold);
            YawnMouth_threshold_line->end->setCoords(QCPRange::maxRange, YawnMouth_threshold);
        }

        driver_monitor Mouth(shape);
        Mouth.measure(66, 62, 'y', 64, 60, 'x', 100);
        Mouth.instance('y', Yawn_instanceTrigger, Yawn_timer, Mouth.facial_feature, YawnMouth_threshold, LeftEye.percent, YawnEyes_threshold, RightEye.percent, YawnEyes_threshold);
        Mouth.classify('y', Yawn_instanceTrigger, Yawn_timer, Yawn_thresholdTime, Yawn_displayTrigger);
        Mouth.instance_rate(Yawn_rate, Yawn_refreshRate, Yawn_thresholdTime, Yawn_displayTrigger);
        Mouth.guiDisplay_text(main_string, Yawn_string, Yawn_count, Yawn_displayTrigger);

        Mouth_plot_data.append(Mouth.facial_feature);
        ui->Yawn_plot->graph(0)->setData(xAxis_plot_data, Mouth_plot_data);
        ui->Yawn_plot->xAxis->setRange((xAxis_plot_data.size()), 100, Qt::AlignRight);
        ui->Yawn_plot->replot();
        ui->Yawn_plot->update();

        if(Yawn_instanceTrigger == true)
            ui->Yawn_trigger_indicator->setStyleSheet("QLabel { background-color : lime; }");
        else
            ui->Yawn_trigger_indicator->setStyleSheet("QLabel { background-color : darkgreen; }");

        Mouth.DriverStatus_Drowsy(ui->Yawn_rate_threshold->value(), Yawn_rate, main_string);

        HeadTurn_refreshRate = ui->RefreshRate_spinBox->value();
        Blink_refreshRate = ui->RefreshRate_spinBox->value();
        SlowBlink_refreshRate = ui->RefreshRate_spinBox->value();
        Yawn_refreshRate = ui->RefreshRate_spinBox->value();

        ui->textBrowser->setText(main_string);
        QScrollBar *instance_string_sb = ui->textBrowser->verticalScrollBar();
        instance_string_sb->setValue(instance_string_sb->maximum());

        ui->head_turn_text->setText(HeadTurn_string);
        QScrollBar *HeadTurn_string_sb = ui->head_turn_text->verticalScrollBar();
        HeadTurn_string_sb->setValue(HeadTurn_string_sb->maximum());

        QString HeadTurn_rate_string;
        HeadTurn_rate_string += "Head turn rate(";
        HeadTurn_rate_string += QString::number(HeadTurn_refreshRate);
        HeadTurn_rate_string += "s): ";
        HeadTurn_rate_string += QString::number(HeadTurn_rate.size());
        ui->HeadTurn_rate_text->setText(HeadTurn_rate_string);

        ui->blink_text->setText(Blink_string);
        QScrollBar *Blink_string_sb = ui->blink_text->verticalScrollBar();
        Blink_string_sb->setValue(Blink_string_sb->maximum());

        QString Blink_rate_string;
        Blink_rate_string.append("Blink rate (");
        Blink_rate_string.append(QString::number(Blink_refreshRate));
        Blink_rate_string.append("s): ");
        Blink_rate_string.append(QString::number(Blink_rate.size()));
        ui->Blink_rate_text->setText(Blink_rate_string);

        QString SlowBlink_rate_string;
        SlowBlink_rate_string.append("Slow Blink rate (");
        SlowBlink_rate_string.append(QString::number(SlowBlink_refreshRate));
        SlowBlink_rate_string.append("s): ");
        SlowBlink_rate_string.append(QString::number(SlowBlink_rate.size()));
        ui->SlowBlink_rate_text->setText(SlowBlink_rate_string);

        ui->yawn_text->setText(Yawn_string);
        QScrollBar *Yawn_string_sb = ui->yawn_text->verticalScrollBar();
        Yawn_string_sb->setValue(Yawn_string_sb->maximum());

        QString Yawn_rate_string;
        Yawn_rate_string.append("Yawn rate (");
        Yawn_rate_string.append(QString::number(Yawn_refreshRate));
        Yawn_rate_string.append("s): ");
        Yawn_rate_string.append(QString::number(Yawn_rate.size()));
        ui->Yawn_rate_text->setText(Yawn_rate_string);

        ui->blue_horizontalSlider->setRange(0, 255);
        blue = ui->blue_horizontalSlider->value();

        ui->green_horizontalSlider->setRange(0, 255);
        green = ui->green_horizontalSlider->value();

        ui->red_horizontalSlider->setRange(0, 255);
        red = ui->red_horizontalSlider->value();

        QTime time = QTime::currentTime();
        QString time_string = time.toString("hh:mm:ss ap");
        ui->CurrentTime_label->setText(time_string);
    }
    show_frame(frame);
}


void MainWindow::show_frame(Mat &image)
{
    Mat resized_image = image.clone();

    int width_of_label = ui->label_camera->width();
    int height_of_label = ui->label_camera->height();

    Size size(width_of_label, height_of_label);

    cv::resize(image, resized_image, size);
    cvtColor(image, image, CV_BGR2RGB);
    QImage qt_image((const unsigned char*) (image.data), image.cols, image.rows, QImage::Format_RGB888);
    ui->label_camera->setPixmap(QPixmap::fromImage(qt_image));
}

void MainWindow::face_layout(const dlib::full_object_detection& shape)
{
    std::vector<cv::Point> points;
    int start_point[] = {0, 17, 22, 27, 30, 36, 42, 48, 60};
    int end_point[] = {16, 21, 26, 30, 35, 41, 47, 59, 67};
    bool isClosed;

    for(int j = 0; j <= 8 ; j++)
    {
        for(int i = start_point[j]; i <= end_point[j]; ++i)
        {
            points.push_back(cv::Point(shape.part(i).x(), shape.part(i).y()));
        }
        if(j < 4)
            isClosed = false;
        else
            isClosed = true;
        cv::polylines(frame, points, isClosed, cv::Scalar(blue, green, red), 1, 16);
        points.clear();
    }
}



void MainWindow::on_pushButton_reset_clicked()
{
    LeftEye_max = 0;
    RightEye_max = 0;
    LeftEye_min = 100;
    RightEye_min = 100;

    FaceLeft_instanceTrigger = false;
    FaceRight_instanceTrigger = false;
    Blink_instanceTrigger = false;
    Yawn_instanceTrigger = false;

    HeadTurn_count = 0;
    Blink_count = 0;
    Yawn_count = 0;

    main_string.clear();
    HeadTurn_string.clear();
    Blink_string.clear();
    Yawn_string.clear();

    HeadTurn_rate.clear();
    Blink_rate.clear();
    SlowBlink_rate.clear();
    Yawn_rate.clear();

    HeadTurn_plot_data.clear();
    LeftEye_plot_data.clear();
    RightEye_plot_data.clear();
    Mouth_plot_data.clear();
    xAxis_plot_data.clear();

    ui->HeadTurnLeft_spinBox->setValue(65);
    ui->HeadTurnRight_spinBox->setValue(35);
    ui->Blink_spinBox->setValue(25);
    ui->YawnEyes_spinBox->setValue(35);
    ui->YawnMouth_spinBox->setValue(30);
}
