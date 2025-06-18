#ifndef IMAGE_STEGNOGRAPHY_H
#define IMAGE_STEGNOGRAPHY_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <QDebug>
#include <QString>

using namespace std;

class stegno_handler
{
public:
    int width, height, channel;
    unsigned char* image;
    string fileExtension;

    unsigned char* load_image(const string& inputimg)
    {
        fileExtension = inputimg.substr(inputimg.find_last_of(".") + 1);
        image = stbi_load(inputimg.c_str(), &width, &height, &channel, 0);
        if (image == NULL)
        {
            cout << " Image Not loaded: " << endl;
            return 0;
        }
        cout << "Loaded image: " << width << "x" << height << " channels: " << channel << endl;
        return image;
    }

    bool save_image(string& saveimg)
    {
        qDebug() << fileExtension;
        saveimg = saveimg.append("/output." + fileExtension);
        if (fileExtension == "png") {
            return stbi_write_png(saveimg.c_str(), width, height, channel, image, width * channel);
        }
        else if (fileExtension == "jpg" || fileExtension == "jpeg") {
            return stbi_write_jpg(saveimg.c_str(), width, height, channel, image, width * channel);
        }
        else if (fileExtension == "bmp") {
            return stbi_write_bmp(saveimg.c_str(), width, height, channel, image);
        }
        else if (fileExtension == "tga") {
            return stbi_write_tga(saveimg.c_str(), width, height, channel, image);
        }
        else {
            qDebug() << "Error: Unsupported file format '%s'\n" << fileExtension;
            return false;
        }
    }

    int getimagesize()
    {
        return width * height * channel;
    }

    ~stegno_handler()
    {
        if (image)
            stbi_image_free(image);
    }
};

class message_handler
{
public:
    string message;

    string read_msg(const string& msg)
    {
        ifstream file(msg.c_str());
        if (file.is_open())
        {
            message.assign((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            cout << "Message to hide :" << message << endl;
            file.close();
            return message;
        }
        cout << "Message Can Not readable: " << endl;
        return "";
    }

    bool write_msg(const string& msg, const string& path)
    {
        ofstream file(path.c_str());
        if (file.is_open())
        {
            file << msg;
            file.close();
            cout << "Message Recovered : " << msg << endl;
            return true;
        }
        cout << "Failed to write message file." << endl;
        return false;
    }
};

class image_stegnography : public stegno_handler, public message_handler
{
public:
    int imgsize;

    void printProgress(float progress) {
        int barWidth = 50;
        cout << "[";
        int pos = barWidth * progress;
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) cout << "=";
            else if (i == pos) cout << ">";
            else cout << " ";
        }
        cout << "] " << int(progress * 100.0) << " %\r";
        cout.flush();
    }

    void encrypt()
    {
        message += '\0';
        imgsize = getimagesize();
        int bit_index = 0;
        int msg_index = 0;
        int total_bits = message.length() * 8;

        for (int i = 0; i < imgsize && msg_index < message.length(); i++)
        {
            bitset<8> bits(message[msg_index]);
            image[i] &= 0xFE;
            image[i] |= bits[7 - bit_index];
            bit_index++;
            if (bit_index == 8)
            {
                bit_index = 0;
                msg_index++;
            }

            if ((msg_index * 8 + bit_index) % 8000 == 0) {
                printProgress(float(msg_index * 8 + bit_index) / total_bits);
            }
        }

        printProgress(1.0f);
        cout << "\nEncryption done.\n";
    }

    string decrypt()
    {
        imgsize = getimagesize();
        bitset<8> bits;
        message.clear();

        int bit_index = 0;
        int char_count = 0;

        for (int i = 0; i < imgsize; i++)
        {
            bits[7 - bit_index] = image[i] & 1;
            bit_index++;
            if (bit_index == 8)
            {
                char ch = static_cast<char>(bits.to_ulong());
                if (ch == '\0') break;
                message += ch;
                bit_index = 0;
                char_count++;

                if (char_count % 100 == 0) {
                    printProgress(float(i) / imgsize);
                }

                if (message.length() > 1000000) break;
            }
        }

        printProgress(1.0f);
        cout << "\nDecryption done.\n";
        return message;
    }
};

#endif // IMAGE_STEGNOGRAPHY_H
