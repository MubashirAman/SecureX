#ifndef FILE_ENCRYPTION_ALGO_H
#define FILE_ENCRYPTION_ALGO_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <iomanip>

#include <QDebug>
#include <QProgressDialog>
#include <QApplication>

using namespace std;

class EncryptionAlgorithm {
protected:
    string key;

public:
    EncryptionAlgorithm(const string& encryptionKey) : key(encryptionKey) {
        if (key.empty()) {
            throw invalid_argument("Encryption key cannot be empty");
        }
    }

    virtual ~EncryptionAlgorithm() = default;
    virtual unsigned char encryptByte(unsigned char byte, size_t position) = 0;
    virtual unsigned char decryptByte(unsigned char byte, size_t position) = 0;
    virtual string getAlgorithmName() const = 0;
    virtual void reset() {}
};

class ReflectoSubAlgorithm : public EncryptionAlgorithm {
private:
    vector<unsigned char> sbox;
    vector<unsigned char> rsbox;
    vector<unsigned char> rbox;
    size_t keyIndex;

    void initializeRBox() {
        rbox.resize(256);
        for (int i = 0; i < 256; i += 2) {
            rbox[i] = static_cast<unsigned char>(i + 1);
            rbox[i + 1] = static_cast<unsigned char>(i);
        }
    }

    void generateSBoxFromKey() {
        sbox.resize(256);
        for (int i = 0; i < 256; ++i)
            sbox[i] = static_cast<unsigned char>(i);

        int j = 0;
        for (int i = 0; i < 256; ++i) {
            j = (j + sbox[i] + static_cast<unsigned char>(key[i % key.length()])) % 256;
            swap(sbox[i], sbox[j]);
        }
    }

    void generateInverseSBox() {
        rsbox.resize(256);
        for (int i = 0; i < 256; ++i) {
            rsbox[sbox[i]] = static_cast<unsigned char>(i);
        }
    }

public:
    ReflectoSubAlgorithm(const string& encryptionKey)
        : EncryptionAlgorithm(encryptionKey), keyIndex(0) {
        generateSBoxFromKey();
        generateInverseSBox();
        initializeRBox();
    }

    unsigned char encryptByte(unsigned char byte, size_t position) override {
        byte = sbox[byte];
        byte = rbox[byte];
        byte = sbox[byte];
        byte ^= static_cast<unsigned char>(key[keyIndex % key.length()]);
        keyIndex++;
        return byte;
    }

    unsigned char decryptByte(unsigned char byte, size_t position) override {
        byte ^= static_cast<unsigned char>(key[keyIndex % key.length()]);
        keyIndex++;
        byte = rsbox[byte];
        byte = rbox[byte];
        byte = rsbox[byte];
        return byte;
    }

    string getAlgorithmName() const override {
        return "ReflectoSub";
    }

    void reset() override {
        keyIndex = 0;
    }
};

class SecureRotationalAlgorithm : public EncryptionAlgorithm {
private:
    int shift;
    unsigned char derivedKey;

    unsigned char deriveKey(const string& password) {
        unsigned char result = 0;
        for (char c : password) {
            result ^= static_cast<unsigned char>(c);
        }
        return result;
    }

    unsigned char leftRotate(unsigned char b) {
        return ((b << shift) | (b >> (8 - shift))) & 0xFF;
    }

    unsigned char rightRotate(unsigned char b) {
        return ((b >> shift) | (b << (8 - shift))) & 0xFF;
    }

public:
    SecureRotationalAlgorithm(const string& password, int shiftValue)
        : EncryptionAlgorithm(password), shift(shiftValue) {
        if (shift < 1 || shift > 7) {
            throw invalid_argument("Shift value must be between 1 and 7");
        }
        derivedKey = deriveKey(password);
    }

    unsigned char encryptByte(unsigned char byte, size_t position) override {
        return leftRotate(byte ^ derivedKey);
    }

    unsigned char decryptByte(unsigned char byte, size_t position) override {
        return rightRotate(byte) ^ derivedKey;
    }

    string getAlgorithmName() const override {
        return "SecureRotational (Shift: " + to_string(shift) + ")";
    }
};

class FileProcessor {
private:
    unique_ptr<EncryptionAlgorithm> algorithm;
    string inputPath;
    string outputPath;

    string getFileExtension(const string& filename) {
        size_t dot = filename.find_last_of('.');
        return (dot == string::npos) ? "" : filename.substr(dot);
    }

    string getBaseName(const string& filepath) {
        size_t slash = filepath.find_last_of("/\\");
        return (slash == string::npos) ? filepath : filepath.substr(slash + 1);
    }

    string getFileNameWithoutExtension(const string& filename) {
        size_t dot = filename.find_last_of('.');
        return (dot == string::npos) ? filename : filename.substr(0, dot);
    }

public:
    FileProcessor(unique_ptr<EncryptionAlgorithm> encAlgorithm,
                  const string& inPath, const string& outPath = "")
        : algorithm(move(encAlgorithm)), inputPath(inPath), outputPath(outPath) {}

    void encryptFile() {
        string baseName = getBaseName(inputPath);
        string nameWithoutExt = getFileNameWithoutExtension(baseName);
        outputPath = outputPath + "/" + nameWithoutExt + ".enc";

        ifstream inputFile(inputPath, ios::binary);
        if (!inputFile) throw runtime_error("Cannot open input file: " + inputPath);

        inputFile.seekg(0, ios::end);
        size_t totalBytes = static_cast<size_t>(inputFile.tellg());
        inputFile.seekg(0, ios::beg);

        ofstream outputFile(outputPath, ios::binary);
        if (!outputFile) throw runtime_error("Cannot open output file: " + outputPath);

        algorithm->reset();

        string originalExt = getFileExtension(baseName);

        QProgressDialog progress("Encrypting...", "Cancel", 0, static_cast<int>(totalBytes));
        progress.setWindowModality(Qt::ApplicationModal);
        progress.setMinimumDuration(0);

        unsigned char byte;
        size_t position = 0;
        while (inputFile.read(reinterpret_cast<char*>(&byte), 1)) {
            unsigned char encrypted = algorithm->encryptByte(byte, position++);
            outputFile.write(reinterpret_cast<char*>(&encrypted), 1);

            if (position % 1024 == 0 || position == totalBytes) {
                progress.setValue(static_cast<int>(position));
                qApp->processEvents();
                if (progress.wasCanceled()) break;
            }
        }

        string metadata = "||EXT:" + originalExt + "||";
        for (char c : metadata) {
            unsigned char encrypted = algorithm->encryptByte(static_cast<unsigned char>(c), position++);
            outputFile.write(reinterpret_cast<char*>(&encrypted), 1);
        }

        progress.setValue(static_cast<int>(totalBytes));
        inputFile.close();
        outputFile.close();

        cout << "\nEncryption completed using " << algorithm->getAlgorithmName() << " algorithm.\n";
        cout << "Output file: " << outputPath << "\n";
    }

    void decryptFile() {
        ifstream inputFile(inputPath, ios::binary);
        if (!inputFile) throw runtime_error("Cannot open input file: " + inputPath);

        inputFile.seekg(0, ios::end);
        size_t fileSize = static_cast<size_t>(inputFile.tellg());
        inputFile.seekg(0, ios::beg);

        vector<unsigned char> encryptedData(fileSize);
        inputFile.read(reinterpret_cast<char*>(encryptedData.data()), fileSize);
        inputFile.close();

        algorithm->reset();
        vector<unsigned char> decryptedData(fileSize);

        QProgressDialog progress("Decrypting...", "Cancel", 0, static_cast<int>(fileSize));
        progress.setWindowModality(Qt::ApplicationModal);
        progress.setMinimumDuration(0);

        for (size_t i = 0; i < fileSize; ++i) {
            decryptedData[i] = algorithm->decryptByte(encryptedData[i], i);
            if (i % 1024 == 0 || i == fileSize - 1) {
                progress.setValue(static_cast<int>(i));
                qApp->processEvents();
                if (progress.wasCanceled()) break;
            }
        }

        string decryptedStr(decryptedData.begin(), decryptedData.end());
        string pattern = "||EXT:";
        size_t metadataStart = decryptedStr.rfind(pattern);

        string originalExt = "";
        size_t dataSize = fileSize;

        if (metadataStart != string::npos) {
            size_t extStart = metadataStart + pattern.length();
            size_t extEnd = decryptedStr.find("||", extStart);
            if (extEnd != string::npos) {
                originalExt = decryptedStr.substr(extStart, extEnd - extStart);
                dataSize = metadataStart;
            }
        }

        string baseName = getBaseName(inputPath);
        string nameWithoutExt = getFileNameWithoutExtension(baseName);
        if (nameWithoutExt.size() >= 4 && nameWithoutExt.substr(nameWithoutExt.size() - 4) == ".enc") {
            nameWithoutExt = nameWithoutExt.substr(0, nameWithoutExt.size() - 4);
        }

        outputPath = outputPath + "/" + nameWithoutExt + "_decrypted" + (originalExt.empty() ? ".txt" : originalExt);
        qDebug() << outputPath;

        ofstream outputFile(outputPath, ios::binary);
        if (!outputFile) throw runtime_error("Cannot open output file: " + outputPath);

        outputFile.write(reinterpret_cast<char*>(decryptedData.data()), dataSize);
        outputFile.close();

        progress.setValue(static_cast<int>(fileSize));

        cout << "\nDecryption completed using " << algorithm->getAlgorithmName() << " algorithm.\n";
        cout << "Original extension restored: " << (originalExt.empty() ? "none" : originalExt) << "\n";
        cout << "Output file: " << outputPath << "\n";
        cout << "--- File restored successfully ---\n";
    }

    string getInputPath() const { return inputPath; }
    string getOutputPath() const { return outputPath; }
    string getAlgorithmName() const { return algorithm->getAlgorithmName(); }
};

class EncryptionApp {
public:
    unique_ptr<EncryptionAlgorithm> createReflectoSubAlgorithm(string key) {
        return make_unique<ReflectoSubAlgorithm>(key);
    }

    unique_ptr<EncryptionAlgorithm> createSecureRotationalAlgorithm(string password, int shift) {
        return make_unique<SecureRotationalAlgorithm>(password, shift);
    }
};

#endif // FILE_ENCRYPTION_ALGO_H
