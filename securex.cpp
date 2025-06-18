#include "securex.h"
#include "ui_securex.h"
#include "detect_usb.h"
#include "password_generator.h"
#include "image_stegnography.h"
#include "encrypt_bound_with_usb.h"
#include "file_encryption_algo.h"
#include "timelock.h"
#include <qmessagebox.h>
#include <QString>
#include <QDebug>
#include <QFileDialog>
#include <fstream>
#include <QInputDialog>
#include <QFileInfo>
#include <QTimer>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <cstring>

secureX::secureX(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::secureX)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    QIcon icon = QIcon("logo.png");
    QApplication::setWindowIcon(icon);
}

image_stegnography hideDataInImage;
EncryptionApp EncryptDecrypt;
QString fileName;
secureX::~secureX()
{
    delete ui;
}
//self-defined functions
void secureX::loadFile(){
    fileName = QFileDialog::getOpenFileName(this, "Open File", "", "All Files (*.*);;Text Files (*.txt)");
    if(!(fileName.isEmpty())){
        QMessageBox::information(nullptr, "", "✅ " + fileName + " has been loaded successfully!");
    }
    else{
        QMessageBox::critical(nullptr, "", "No File Was Selected!");
    }
}
void secureX::emptyFileName(){
    fileName = "";
}
void secureX::goBackToMain(){
    ui->stackedWidget->setCurrentIndex(0);
    emptyFileName();
}
string secureX::saveFile(){
    QString outputPath = QFileDialog::getExistingDirectory(this, "Select Folder To Save File", QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    return outputPath.toStdString();
}
std::string secureX::getKeyInput(){
    bool ok;
    QString qKey;
    do{
        qKey = QInputDialog::getText(nullptr, "", "Enter A Key:", QLineEdit::Normal, "", &ok);
        if(!(ok && !qKey.isEmpty())){
            QMessageBox::information(nullptr, "", "Please Enter Key Again!");
        }
    }while(!(ok && !qKey.isEmpty()));
    QMessageBox::information(nullptr, "", "Key has been entered!");

    return qKey.toStdString();
}
QString secureX::showENCoptions(bool &a){
    QStringList items;
    items << "ReflectoSub Encryption" << "Shift Encryption";

    QString selectedMethod = QInputDialog::getItem(this, "Select Encryption Method",
                                                   "Choose encryption algorithm:",
                                                   items, 0, false, &a);
    return selectedMethod;
}
QString secureX::showShiftValues(bool &a){
    QStringList items;
    items << "1" << "2" << "3" << "4" << "5" << "6" << "7";

    QString selectedMethod = QInputDialog::getItem(this, "Select Shift Value",
                                                   "Value:",
                                                   items, 0, false, &a);
    return selectedMethod;
}
void secureX::successMessage(){
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setWindowTitle("Success");
    msgBox->setText("✅ Process completed successfully!");
    msgBox->setStandardButtons(QMessageBox::Ok);
    msgBox->show();
    QTimer::singleShot(3000, msgBox, &QMessageBox::close);
}

//usb-based
void secureX::on_detect_usb_clicked()
{
    DetectUSB detector;
    detector.detectUSBDevices();
    if(detector.getUSBDevices().empty()){
        QMessageBox::information(nullptr, "", "No USB Found!");
    }
    else{
        QString IdsInMessage;
        for (QString id : detector.getUSBDevices()) {
            IdsInMessage += id + "\n";
        }
        QMessageBox::information(nullptr, "", IdsInMessage);
    }

}




void secureX::on_back_clicked()
{
    goBackToMain();
}


void secureX::on_load_file_clicked()
{
    loadFile();
}


void secureX::on_encryptFileUSB_clicked()
{

    if (!fileName.isEmpty()) {
        std::string strFile = fileName.toStdString();

        EncryptBindWithUSB binder("MySecretKey123");

        std::string outputPath = saveFile();
        qDebug() << "Path: " <<outputPath;
        binder.bindUSBWithEncryptedFileSimple(strFile, outputPath);
    } else {
        QMessageBox::information(nullptr, "", "No file has been loaded!");
    }
    emptyFileName();
    qDebug() << "Trusted USB ID:" << trustedUSBID;
}

void secureX::on_dencryptFileUSB_clicked()
{
    qDebug() << "Trusted USB ID:" << trustedUSBID;
    qDebug() << "[*] Entered decryptUSBEncryptedFile function";

    if (!fileName.isEmpty()) {
        std::string strFile = fileName.toStdString();

        EncryptBindWithUSB binder("MySecretKey123");

        std::string outputPath = saveFile();
        qDebug() << "Path: " <<outputPath;
        binder.decryptUSBEncryptedFileSimple(strFile, outputPath);
    } else {
        QMessageBox::information(nullptr, "", "No file has been loaded!");
    }
    emptyFileName();
}

//file encryption
void secureX::on_load_file_2_clicked()
{
    loadFile();
}


void secureX::on_back_2_clicked()
{
    goBackToMain();
}


void secureX::on_encrypt_file_clicked()
{
    if(!fileName.isEmpty()){
        bool ok;
        QString selectedENC = showENCoptions(ok);

        if (ok && !selectedENC.isEmpty()) {
            std::string key = getKeyInput();

            if(selectedENC == "ReflectoSub Encryption"){

                auto algorithm = EncryptDecrypt.createReflectoSubAlgorithm(key);
                std::string outputPath = saveFile();
                FileProcessor processor(std::move(algorithm), fileName.toStdString(), outputPath);
                processor.encryptFile();

            } else if(selectedENC == "Shift Encryption"){

                bool ok;
                QString shiftValue = showShiftValues(ok);
                if(ok && !shiftValue.isEmpty()){

                    auto algorithm = EncryptDecrypt.createSecureRotationalAlgorithm(key, shiftValue.toInt());
                    std::string outputPath = saveFile();
                    FileProcessor processor(std::move(algorithm), fileName.toStdString(), outputPath);
                    processor.encryptFile();
                }

            }

        }
    }
    else{
        QMessageBox::warning(nullptr, "Failure", "No file has been selected.");
    }

    emptyFileName();
}
void secureX::on_decrypt_file_clicked()
{
    if(!fileName.isEmpty()){
        bool ok;
        QString selectedENC = showENCoptions(ok);

        if (ok && !selectedENC.isEmpty()) {
            std::string key = getKeyInput();

            if(selectedENC == "ReflectoSub Encryption"){

                auto algorithm = EncryptDecrypt.createReflectoSubAlgorithm(key);
                std::string outputPath = saveFile();
                FileProcessor processor(std::move(algorithm), fileName.toStdString(), outputPath);
                processor.decryptFile();

            } else if(selectedENC == "Shift Encryption"){

                bool ok;
                QString shiftValue = showShiftValues(ok);
                if(ok && !shiftValue.isEmpty()){

                    auto algorithm = EncryptDecrypt.createSecureRotationalAlgorithm(key, shiftValue.toInt());
                    std::string outputPath = saveFile();
                    FileProcessor processor(std::move(algorithm), fileName.toStdString(), outputPath);
                    processor.decryptFile();
                }

            }

        }
    }
    else{
        QMessageBox::warning(nullptr, "Failure", "No file has been selected.");
    }

    emptyFileName();
}



//image steganography
void secureX::on_back_3_clicked()
{
    goBackToMain();
}


void secureX::on_load_file_3_clicked()
{
    fileName = QFileDialog::getOpenFileName(this, "Select Image", "", "Images (*.png *.jpg *.jpeg *.bmp *.gif)");
    if(!(fileName.isEmpty())){
        QMessageBox::information(nullptr, "", fileName + " has been loaded successfully!");
    }
    else{
        QMessageBox::information(nullptr, "", "No File Was Selected!");
    }

    if(!(fileName.isEmpty())){
        cout << "Current working directory: " << QDir::currentPath().toStdString() << endl;
        qDebug() << "Attempting to load image from: " << fileName;
        hideDataInImage.load_image(fileName.toStdString());
    }
}
void secureX::on_hide_data_img_clicked()
{
    if(fileName.isEmpty()){
        QMessageBox::information(nullptr, "", "No File has been loaded to hide data.");
    }
    else{
        QMessageBox::information(nullptr, "", "Now Select Text File Containing Message You Want To Hide.");
        QString secretMessagefileName = QFileDialog::getOpenFileName(this, "Select File", "", "Text Files (*.txt)");
        if(!(secretMessagefileName.isEmpty())){
            QMessageBox::information(nullptr, "", secretMessagefileName + " has been selected successfully!");
        }
        else{
            QMessageBox::information(nullptr, "", "No File Was Selected!");
        }
        if(!(secretMessagefileName.isEmpty())){
            hideDataInImage.read_msg(secretMessagefileName.toStdString());
            hideDataInImage.encrypt();

            string outputPath = saveFile();
            qDebug() << "Value: " << hideDataInImage.save_image(outputPath);
            QMessageBox::information(nullptr, "", "Output image has been saved to " + QString::fromStdString(outputPath));
        }

    }
    emptyFileName();
}



void secureX::on_extract_data_img_clicked()
{
    if(fileName.isEmpty()){
        QMessageBox::information(nullptr, "", "No File has been loaded to extract data.");
    }
    else{
        string recoveredMsg = hideDataInImage.decrypt();
        QMessageBox::information(nullptr, "", "Now Select Folder In Which You Want to Write Message.");
        string secretMessagePath = saveFile();

        if (!secretMessagePath.empty()) {
            ofstream outFile(secretMessagePath + "/recoveredMessage.txt");
            if (outFile.is_open()) {
                outFile << recoveredMsg;
                outFile.close();
                QMessageBox::information(nullptr, "", "Message has been written to the file successfully.");
            } else {
                QMessageBox::warning(nullptr, "", "Failed to open the file for writing.");
            }
        } else {
            QMessageBox::warning(nullptr, "", "No file path selected to save the message.");
        }
    }
    emptyFileName();
}




//strong password
void secureX::on_back_4_clicked()
{
    ui->generated_pass->setText("click the button!");
    goBackToMain();
}


void secureX::on_generatePass_button_clicked()
{
    int length;
    QString input;
    input = QInputDialog::getText(nullptr, "", "Enter Length of Password:", QLineEdit::Normal, "");
    bool flag = 1;
    for(int i = 0; i < input.size(); i++){
        if(input[i] != '0' && input[i] != '1' && input[i] != '2' && input[i] != '3' && input[i] != '4' && input[i] != '5' && input[i] != '6' && input[i] != '7' && input[i] != '8' && input[i] != '9'){
            flag = 0;
        }
    }
    if(flag == 0){
        QMessageBox::information(nullptr, "", "Invalid Input!");
    }
    else{
        length = std::stoi(input.toStdString());
        if(length > 24){
            QMessageBox::warning(nullptr, "Failure", "Please enter length less than or equal to 24.");
        }
        else{
            PasswordGenerator genPass;
            ui->generated_pass->setText(genPass.generatePassword(length));
        }

    }



}


void secureX::on_check_strength_clicked()
{
    PasswordGenerator genPass;
    QString password = QInputDialog::getText(nullptr, "", "Enter Password:", QLineEdit::Normal, "");
    QMessageBox::information(NULL,"", genPass.passwordStrength(password));
}







//time lock
void secureX::on_load_file_8_clicked()
{
    loadFile();
}


void secureX::on_encrypt_timelock_clicked()
{
    if(!fileName.isEmpty()){
        TimeLockFile timelock;

        if (!timelock.SetUnlockTime()) {
            std::cout << "User canceled the date/time selection." << std::endl;
            qDebug() << "Cancelled";
        }

        std::cout << "Unlock time set to: " << timelock.getunlocktime();
        std::string key = getKeyInput();
        Encryptor enc(key);
        SaveAndLoad saver;
        std::string inputFile = fileName.toStdString();
        std::string encryptedFile = saveFile() + "//timelock.enc";
        qDebug() << encryptedFile;
        if (saver.encryptFile(inputFile, encryptedFile, enc, timelock.GetUnlockTime())) {
            QMessageBox::information(nullptr, "Success", "File encrypted with time lock:\n" +
                                                             QString::fromStdString(timelock.getunlocktime()));
        } else {
            QMessageBox::critical(nullptr, "Failure", "Failed to encrypt the file.");
            qDebug() << 1;
        }
    }
    else{
        QMessageBox::warning(nullptr, "Failure", "No file has been selected.");
    }
    emptyFileName();
}


void secureX::on_decrypt_timelock_clicked()
{
    if(!fileName.isEmpty()){
        TimeLockFile timelock;

        std::string key = getKeyInput();
        Decryptor dnc(key);
        SaveAndLoad saver;
        std::string outputFile = saveFile() + "/time_lock";
        std::string decryptedFile = fileName.toStdString();
        qDebug() << outputFile;
        if (saver.decryptFile(decryptedFile, outputFile, dnc, timelock)) {
            QMessageBox::information(nullptr, "Success", "Decryption successful!");
        } else {
            QMessageBox::critical(nullptr, "Failure", "Failed to decrypt the file.");
            qDebug() << 1;
        }
    }
    else{
        QMessageBox::warning(nullptr, "Failure", "No file has been selected.");
    }
    emptyFileName();
}


void secureX::on_back_10_clicked()
{
    goBackToMain();
}



//main page
void secureX::on_fileEncrypt_3_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void secureX::on_usbEncrypt_3_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void secureX::on_imgSteg_3_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
}


void secureX::on_password_3_clicked()
{
    ui->stackedWidget->setCurrentIndex(4);
}

void secureX::on_time_lock_clicked()
{
    ui->stackedWidget->setCurrentIndex(5);
}

void secureX::on_exit_clicked()
{
    QApplication::quit();
}
