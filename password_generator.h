#ifndef PASSWORD_GENERATOR_H
#define PASSWORD_GENERATOR_H


#include <random>
#include <openssl/rand.h>
#include <QString>
#include <cmath>

class PasswordGenerator{
private:

    QString characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+-=[]{}|;:,.<>?";
    int length;
    double calculateEntropy(const QString password) {
        const int noOfCharacters = characters.length();
        double length = password.length();
        if (length == 0) {
            return 0.0;
        }
        return length * log2(noOfCharacters);
    }

public:

    QString generatePassword(const int len) {
        length = len;
        QString password;
        std::random_device secureRandom;
        std::mt19937 generator(secureRandom());
        std::uniform_int_distribution<> dis(0, characters.length() - 1);
        RAND_poll();
        for (int i = 0; i < length; ++i) {
            if (i % 2 == 0) {
                int index = dis(generator);
                password += characters[index];
            } else {
                unsigned char byte;
                RAND_bytes(&byte, 1);
                int index = byte % characters.length();
                password += characters[index];

            }
        }
        return password;
    }


    QString passwordStrength(const QString password) {
        double entropy = calculateEntropy(password);
        int length = password.length();

        if (length < 6) {
            return "Weak: Too short (less than 6 characters)";
        } else if (entropy < 50) {
            return "Weak: (easily cracked)";
        } else if (entropy < 80) {
            return "Medium: (low security)";
        } else if (entropy < 120) {
            return "Strong: (secure for most uses)";
        } else {
            return "Very Strong: (high security)";
        }
    }

};

#endif // PASSWORD_GENERATOR_H
