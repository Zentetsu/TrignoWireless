#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP


#include <QStandardPaths>
#include <QCloseEvent>
#include <QListWidget>
#include <QMainWindow>
#include <QStringList>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QComboBox>
#include <QSettings>
#include <QDebug>
#include <QTimer>

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include <lsl_cpp.h>

#include "../window/ui_mainwindow.h"
#include "../include/recording.hpp"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent, const char *config_file);
        ~MainWindow() noexcept override;
    
    private slots:
        void connectLabrecorder();
        void connectEMG();

        void loadConfigDialog();
        void saveConfigDialog();

        void loadChanlocsDialog();
        
        void acquisitionEMG();

        void closeEvent(QCloseEvent *ev) override;

    private:
        void loadConfig(const std::string filename);
        void saveConfig(const std::string filename);

        bool stop_labrecorder;
        bool stop_EMG;

        boost::shared_ptr<boost::thread> labrecorder_thread;
        boost::shared_ptr<boost::thread> EMG_thread;

        std::unique_ptr<QTimer> timer;
        std::unique_ptr<Ui::MainWindow> ui;

        Recording *record;
};

#endif