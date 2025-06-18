#ifndef TIMELOCK_H
#define TIMELOCK_H

#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <QDateTimeEdit>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QMessageBox>
#include <QProgressDialog>
#include "file_encryption_algo.h"

class DateTimeDialog : public QDialog {
    Q_OBJECT
public:
    DateTimeDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("Select Unlock Date and Time");
        QVBoxLayout *layout = new QVBoxLayout(this);

        dateTimeEdit = new QDateTimeEdit(this);
        dateTimeEdit->setCalendarPopup(true);
        dateTimeEdit->setDateTime(QDateTime::currentDateTime());

        QPushButton *okButton = new QPushButton("OK", this);
        QPushButton *cancelButton = new QPushButton("Cancel", this);

        layout->addWidget(dateTimeEdit);
        layout->addStretch();
        layout->addWidget(okButton);
        layout->addWidget(cancelButton);

        connect(okButton, &QPushButton::clicked, this, &DateTimeDialog::accept);
        connect(cancelButton, &QPushButton::clicked, this, &DateTimeDialog::reject);
    }

    QDateTime getDateTime() const {
        return dateTimeEdit->dateTime();
    }

private:
    QDateTimeEdit *dateTimeEdit;
};

class TimeLockFile {
private:
    time_t unlocktime;
public:
    TimeLockFile() : unlocktime(0) {}

    bool SetUnlockTime() {
        if (QApplication::instance() == nullptr) {
            std::cerr << "Error: QApplication must be initialized before calling SetUnlockTime().\n";
            return false;
        }

        DateTimeDialog dialog;
        if (dialog.exec() == QDialog::Accepted) {
            QDateTime selectedDateTime = dialog.getDateTime();
            tm unlock_tm = {};
            unlock_tm.tm_mday = selectedDateTime.date().day();
            unlock_tm.tm_mon = selectedDateTime.date().month() - 1;
            unlock_tm.tm_year = selectedDateTime.date().year() - 1900;
            unlock_tm.tm_hour = selectedDateTime.time().hour();
            unlock_tm.tm_min = selectedDateTime.time().minute();
            unlock_tm.tm_sec = 0;
            unlocktime = mktime(&unlock_tm);
            return true;
        }
        return false;
    }

    void SetUnlockTime(int day, int mon, int year, int hour, int mint) {
        tm unlock_tm = {};
        unlock_tm.tm_mday = day;
        unlock_tm.tm_mon = mon - 1;
        unlock_tm.tm_year = year - 1900;
        unlock_tm.tm_hour = hour;
        unlock_tm.tm_min = mint;
        unlock_tm.tm_sec = 0;
        unlocktime = mktime(&unlock_tm);
    }

    void SetUnlockTime(time_t time) {
        unlocktime = time;
    }

    time_t GetUnlockTime() const {
        return unlocktime;
    }

    bool isunlocked() const {
        return time(nullptr) >= unlocktime;
    }

    std::string getunlocktime() const {
        return ctime(&unlocktime);
    }

    std::string timeRemaining() const {
        time_t now = time(nullptr);
        int seconds = static_cast<int>(unlocktime - now);
        if (seconds <= 0) return "Unlocked";

        int h = seconds / 3600;
        int m = (seconds % 3600) / 60;
        int s = seconds % 60;

        return std::to_string(h) + "h " + std::to_string(m) + "m " + std::to_string(s) + "s";
    }
};

class Encryptor {
protected:
    std::string key;
public:
    Encryptor(const std::string& k) : key(k) {}
    std::string get_key() const { return key; }
};

class Decryptor : public Encryptor {
public:
    Decryptor(const std::string& k) : Encryptor(k) {}
};

class SaveAndLoad {
private:
    static const size_t BUFFER_SIZE = 4096;

public:
    bool encryptFile(const std::string& inFile, const std::string& outFile,
                     const Encryptor& enc, time_t unlocktime)
    {
        ReflectoSubAlgorithm encryptionAlgo(enc.get_key());

        std::ifstream input(inFile, std::ios::binary);
        std::ofstream output(outFile, std::ios::binary);

        if (!input.is_open() || !output.is_open())
            return false;

        input.seekg(0, std::ios::end);
        std::streamsize totalSize = input.tellg();
        input.seekg(0, std::ios::beg);

        QProgressDialog progress("Encrypting file...", "Abort", 0, static_cast<int>(totalSize));
        progress.setWindowModality(Qt::ApplicationModal);

        output.write(reinterpret_cast<const char*>(&unlocktime), sizeof(time_t));

        std::string extension = inFile.substr(inFile.find_last_of('.'));
        uint8_t extLen = static_cast<uint8_t>(extension.size());
        output.write(reinterpret_cast<const char*>(&extLen), sizeof(uint8_t));
        output.write(extension.c_str(), extLen);

        char buffer[BUFFER_SIZE];
        size_t pos = 0;

        encryptionAlgo.reset();

        while (input) {
            input.read(buffer, BUFFER_SIZE);
            std::streamsize bytes = input.gcount();

            for (int i = 0; i < bytes; ++i)
                buffer[i] = encryptionAlgo.encryptByte(buffer[i], pos++);

            output.write(buffer, bytes);
            progress.setValue(static_cast<int>(input.tellg()));

            if (progress.wasCanceled()) {
                input.close();
                output.close();
                return false;
            }
        }

        input.close();
        output.close();
        return true;
    }

    bool decryptFile(const std::string& inFile, const std::string& outFile,
                     const Decryptor& dec, TimeLockFile& timelock)
    {
        ReflectoSubAlgorithm encryptionAlgo(dec.get_key());

        std::ifstream input(inFile, std::ios::binary);
        if (!input.is_open())
            return false;

        time_t unlocktime = 0;
        input.read(reinterpret_cast<char*>(&unlocktime), sizeof(time_t));
        timelock.SetUnlockTime(unlocktime);

        uint8_t extLen = 0;
        input.read(reinterpret_cast<char*>(&extLen), sizeof(uint8_t));
        std::string extension(extLen, '\0');
        input.read(&extension[0], extLen);

        if (!timelock.isunlocked()) {
            QMessageBox::information(nullptr, "Access Denied",
                                     "This file is time-locked.\nYou must wait until:\n" +
                                         QString::fromStdString(timelock.getunlocktime()));
            input.close();
            return false;
        }

        input.seekg(0, std::ios::end);
        std::streamsize totalSize = input.tellg();
        input.seekg(sizeof(time_t) + sizeof(uint8_t) + extLen, std::ios::beg);

        QProgressDialog progress("Decrypting file...", "Abort", 0, static_cast<int>(totalSize));
        progress.setWindowModality(Qt::ApplicationModal);

        std::ofstream output(outFile + extension, std::ios::binary);
        if (!output.is_open()) {
            input.close();
            return false;
        }

        char buffer[BUFFER_SIZE];
        size_t pos = 0;

        encryptionAlgo.reset();

        while (input) {
            input.read(buffer, BUFFER_SIZE);
            std::streamsize bytes = input.gcount();

            for (int i = 0; i < bytes; ++i)
                buffer[i] = encryptionAlgo.decryptByte(buffer[i], pos++);

            output.write(buffer, bytes);
            progress.setValue(static_cast<int>(input.tellg()));

            if (progress.wasCanceled()) {
                input.close();
                output.close();
                return false;
            }
        }

        input.close();
        output.close();
        return true;
    }
};

#endif // TIMELOCK_H
