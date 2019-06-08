#include "../include/mainwindow.hpp"


MainWindow::MainWindow(QWidget *parent, const char *config_file) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    stop_EMG = true;
    stop_labrecorder = true;

    QObject::connect(ui->action_quit, SIGNAL(triggered()), this, SLOT(close()));
    QObject::connect(ui->connect_EMG, SIGNAL(clicked()), this, SLOT(connectEMG()));
    QObject::connect(ui->connect_labrecorder, SIGNAL(clicked()), this, SLOT(connectLabrecorder()));
    QObject::connect(ui->action_load_configuration, SIGNAL(triggered()), this, SLOT(loadConfigDialog()));
    QObject::connect(ui->action_save_configuration, SIGNAL(triggered()), this, SLOT(saveConfigDialog()));
}

MainWindow::~MainWindow() noexcept = default;

void MainWindow::loadConfigDialog() {
    QString sel = QFileDialog::getOpenFileName(this,"Load Configuration File","","Configuration Files (*.cfg)");

    if (!sel.isEmpty())
        loadConfig(sel.toStdString());
}

void MainWindow::saveConfigDialog() {
    QString sel = QFileDialog::getSaveFileName(this,"Save Configuration File","","Configuration Files (*.cfg)");
    
    if (!sel.isEmpty())
        saveConfig(sel.toStdString());
}

void MainWindow::loadChanlocsDialog() {

}

void MainWindow::connectLabrecorder() {
    if (!stop_labrecorder) {
        try {
            record->setLSLSharing(false);
            stop_labrecorder = true;
        } catch(std::exception &e) {
            QMessageBox::critical(this,"Error",(std::string("Could not stop the background processing: ")+=e.what()).c_str(),QMessageBox::Ok);
        
            return;
        }

        ui->connect_labrecorder->setText("Link");
    } else {
        stop_labrecorder = false;
        lslSharing();

        ui->connect_labrecorder->setText("Unlink");
    }
}

void MainWindow::connectEMG() { 
    if (!stop_EMG) {
        try {
            record->setRecord(false);
            stop_EMG = true;
            EMG_thread->interrupt();
            EMG_thread->join();
            EMG_thread.reset();
            
            delete record;
        } catch(std::exception &e) {
            QMessageBox::critical(this,"Error",(std::string("Could not stop the background processing: ")+=e.what()).c_str(),QMessageBox::Ok);
            
            return;
        }

        ui->connect_EMG->setText("Connect");
    } else {
        try {
            record = new Recording(ui->ip_address->text().toStdString(), 50040);
        } catch(std::exception &e) {
            QMessageBox::critical(this,"Error", "Unable to start connection with the EMG", QMessageBox::Ok);
        
            return;
        }
        
        stop_EMG = false;

        EMG_thread.reset(new boost::thread(&MainWindow::acquisitionEMG, this));
        ui->connect_EMG->setText("Disconnect");
    }
}

void MainWindow::closeEvent(QCloseEvent *ev) {
    if (EMG_thread || labrecorder_thread)
        ev->ignore();	
}

void MainWindow::loadConfig(const std::string filename) {
    QMessageBox::information(this, "Warning", "Not yet implemented", QMessageBox::Ok);
    //TODO loadConfig
}

void MainWindow::saveConfig(const std::string filename) {
    QMessageBox::information(this, "Warning", "Not yet implemented", QMessageBox::Ok);
    //TODO saveConfig
}

void MainWindow::lslSharing() {
    lsl::stream_info info("Trigno Wireless", "EMG", 64, 2000, lsl::cf_float32, "localhost");
    lsl::stream_outlet outlet(info);

    record->setupLSL(&info);
    //TODO correctly the information to the stream_info
    /* record->set_lsl_sharing(true); */
    /* 	lsl::stream_info info("Trigno Wireless", "EMG", 64, 2000, lsl::cf_float32, boost::asio::ip::host_name());
    info.desc().append_child("EMG").append_child_value("number", "1");

    lsl::stream_outlet outlet(info); */
}

void MainWindow::acquisitionEMG() {
    record->checkConnection();
    record->startRecording();
}