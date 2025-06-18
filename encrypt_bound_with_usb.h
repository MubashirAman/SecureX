#ifndef ENCRYPT_BOUND_WITH_USB_H
#define ENCRYPT_BOUND_WITH_USB_H

#include <iostream>
#include <fstream>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QMessageBox>
#include <QString>
#include <QVector>
#include <QProgressDialog>
#include <QApplication>
#include <atomic>
#include <thread>
#include <mutex>
#include "detect_usb.h"
#include <QFileInfo>
#include <QDebug>

QString trustedUSBID;

class EncryptBindWithUSB {
private:
    std::string key;
    std::thread usbMonitorThread;
    std::atomic<bool> usbConnected;
    std::mutex usbMutex;
    bool stopMonitor = false;

public:
    EncryptBindWithUSB(std::string k) : key(std::move(k)), usbConnected(true) {
    }

    ~EncryptBindWithUSB() {
        stopMonitor = true;
        if (usbMonitorThread.joinable()) {
            usbMonitorThread.join();
        }
    }

    void monitorUSB() {
        DetectUSB detector;
        while (!stopMonitor) {
            detector.detectUSBDevices();
            QVector<QString> currentIDs = detector.getUSBDevices();

            std::lock_guard<std::mutex> lock(usbMutex);
            if (!currentIDs.contains(trustedUSBID)) {
                usbConnected.store(false);
                qDebug() << "[!] USB removed: " << trustedUSBID;
                break;
            }

            QThread::msleep(300);
        }
    }

    void bindUSBWithEncryptedFileSimple(const std::string& plainFile, const std::string& outputFileName) {
        DetectUSB detector;
        detector.detectUSBDevices();
        QVector<QString> usbIDs = detector.getUSBDevices();

        if (usbIDs.isEmpty()) {
            QMessageBox::warning(nullptr, "Encryption Failed", "No USB device found.");
            return;
        }

        {
            std::lock_guard<std::mutex> lock(usbMutex);
            trustedUSBID = usbIDs.first();
        }

        std::string usbID = trustedUSBID.toStdString();

        std::string simpleKey;
        for (size_t i = 0; i < std::max(key.size(), usbID.size()) * 2; ++i) {
            simpleKey += key[i % key.size()] ^ usbID[i % usbID.size()];
        }

        std::ifstream inputFile(plainFile, std::ios::binary);
        if (!inputFile.is_open()) {
            QMessageBox::critical(nullptr, "Error", "Cannot open input file!");
            return;
        }

        std::string fullOutputPath = QString::fromStdString(outputFileName).append("/outputUSB_simple.usbenc").toStdString();
        std::ofstream outputFile(fullOutputPath, std::ios::binary);
        if (!outputFile.is_open()) {
            QMessageBox::critical(nullptr, "Error", "Cannot create output file!");
            return;
        }

        QString qFilePath = QString::fromStdString(plainFile);
        QString extension = QFileInfo(qFilePath).suffix();
        uint8_t extLen = static_cast<uint8_t>(extension.size());
        outputFile.put(extLen);
        outputFile.write(extension.toStdString().c_str(), extLen);

        uint8_t usbIDLen = static_cast<uint8_t>(usbID.size());
        outputFile.put(usbIDLen);
        outputFile.write(usbID.c_str(), usbIDLen);

        inputFile.seekg(0, std::ios::end);
        std::streamoff originalSize = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);

        outputFile.write(reinterpret_cast<const char*>(&originalSize), sizeof(originalSize));

        QProgressDialog progress("Encrypting file...", "Cancel", 0, static_cast<int>(originalSize));
        progress.setWindowModality(Qt::ApplicationModal);
        progress.show();

        usbConnected.store(true);
        stopMonitor = false;
        usbMonitorThread = std::thread(&EncryptBindWithUSB::monitorUSB, this);

        const size_t bufferSize = 8192;
        std::vector<char> buffer(bufferSize);
        size_t index = 0;
        std::streamsize totalBytes = 0;

        while (!inputFile.eof()) {
            if (!usbConnected.load() || progress.wasCanceled()) {
                QMessageBox::warning(nullptr, "Interrupted", "USB removed or operation canceled during encryption.");
                break;
            }

            inputFile.read(buffer.data(), bufferSize);
            std::streamsize bytesRead = inputFile.gcount();

            for (std::streamsize i = 0; i < bytesRead; ++i) {
                buffer[i] ^= simpleKey[index % simpleKey.length()];
                ++index;
            }

            outputFile.write(buffer.data(), bytesRead);
            totalBytes += bytesRead;

            progress.setValue(static_cast<int>(totalBytes));
            QCoreApplication::processEvents();
        }


        stopMonitor = true;
        if (usbMonitorThread.joinable()) {
            usbMonitorThread.join();
        }

        inputFile.close();
        outputFile.close();

        if (usbConnected.load() && !progress.wasCanceled()) {
            QMessageBox::information(nullptr, "Success", "File encrypted with simple XOR method.");
        }
    }

    void decryptUSBEncryptedFileSimple(const std::string& encryptedFile, const std::string& outputFolder) {
        DetectUSB detector;
        detector.detectUSBDevices();
        QVector<QString> usbIDs = detector.getUSBDevices();

        std::ifstream inputFile(encryptedFile, std::ios::binary);
        if (!inputFile.is_open()) {
            QMessageBox::critical(nullptr, "Error", "Cannot open encrypted file!");
            return;
        }

        uint8_t extLen;
        inputFile.read(reinterpret_cast<char*>(&extLen), 1);
        std::string extension(extLen, '\0');
        inputFile.read(&extension[0], extLen);

        uint8_t usbIDLen;
        inputFile.read(reinterpret_cast<char*>(&usbIDLen), 1);
        std::string storedUSBID(usbIDLen, '\0');
        inputFile.read(&storedUSBID[0], usbIDLen);

        bool usbFound = false;
        for (const QString& id : usbIDs) {
            if (id.toStdString() == storedUSBID) {
                usbFound = true;
                std::lock_guard<std::mutex> lock(usbMutex);
                trustedUSBID = id;
                break;
            }
        }

        if (!usbFound) {
            QMessageBox::warning(nullptr, "Decryption Failed", "Trusted USB not connected!");
            inputFile.close();
            return;
        }

        std::streamoff originalSize;
        inputFile.read(reinterpret_cast<char*>(&originalSize), sizeof(originalSize));

        std::string usbID = trustedUSBID.toStdString();

        std::string simpleKey;
        for (size_t i = 0; i < std::max(key.size(), usbID.size()) * 2; ++i) {
            simpleKey += key[i % key.size()] ^ usbID[i % usbID.size()];
        }

        std::string fullOutputPath = outputFolder + "/decryptusboutput_simple." + extension;
        std::ofstream output(fullOutputPath, std::ios::binary);
        if (!output.is_open()) {
            QMessageBox::critical(nullptr, "Error", "Cannot create decrypted output file!");
            inputFile.close();
            return;
        }

        QProgressDialog progress("Decrypting file...", "Cancel", 0, static_cast<int>(originalSize));
        progress.setWindowModality(Qt::ApplicationModal);
        progress.show();

        usbConnected.store(true);
        stopMonitor = false;
        usbMonitorThread = std::thread(&EncryptBindWithUSB::monitorUSB, this);

        const size_t bufferSize = 8192;
        std::vector<char> buffer(bufferSize);
        size_t index = 0;
        std::streamsize totalBytes = 0;

        while (totalBytes < originalSize && !inputFile.eof()) {
            if (!usbConnected.load() || progress.wasCanceled()) {
                QMessageBox::warning(nullptr, "Interrupted", "USB removed or operation canceled during decryption.");
                break;
            }

            std::streamsize bytesToRead = std::min(static_cast<std::streamsize>(bufferSize), originalSize - totalBytes);
            inputFile.read(buffer.data(), bytesToRead);
            std::streamsize bytesRead = inputFile.gcount();

            for (std::streamsize i = 0; i < bytesRead; ++i) {
                buffer[i] ^= simpleKey[index % simpleKey.length()];
                ++index;
            }

            output.write(buffer.data(), bytesRead);
            totalBytes += bytesRead;

            progress.setValue(static_cast<int>(totalBytes));
            QCoreApplication::processEvents();
        }


        stopMonitor = true;
        if (usbMonitorThread.joinable()) {
            usbMonitorThread.join();
        }

        inputFile.close();
        output.close();

        if (usbConnected.load() && !progress.wasCanceled()) {
            QMessageBox::information(nullptr, "Success", "File decrypted successfully with simple XOR method!");
        }
    }
};

#endif // ENCRYPT_BOUND_WITH_USB_H
