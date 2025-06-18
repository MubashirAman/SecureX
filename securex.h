#ifndef SECUREX_H
#define SECUREX_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class secureX;
}
QT_END_NAMESPACE

class secureX : public QMainWindow
{
    Q_OBJECT

public:
    secureX(QWidget *parent = nullptr);
    ~secureX();

private slots:

    void on_detect_usb_clicked();

    void on_usbEncrypt_3_clicked();

    void on_back_clicked();

    void on_load_file_clicked();

    void on_encryptFileUSB_clicked();

    void on_load_file_2_clicked();

    void on_back_2_clicked();

    void loadFile();

    void emptyFileName();

    QString showENCoptions(bool &a);

    QString showShiftValues(bool &a);

    void goBackToMain();

    std::string getKeyInput();

    std::string saveFile();

    void successMessage();

    void on_fileEncrypt_3_clicked();

    void on_encrypt_file_clicked();

    void on_exit_clicked();

    void on_back_3_clicked();

    void on_load_file_3_clicked();

    void on_imgSteg_3_clicked();

    void on_password_3_clicked();

    void on_back_4_clicked();

    void on_generatePass_button_clicked();

    void on_check_strength_clicked();

    void on_hide_data_img_clicked();

    void on_dencryptFileUSB_clicked();

    void on_extract_data_img_clicked();

    void on_decrypt_file_clicked();

    void on_load_file_8_clicked();

    void on_encrypt_timelock_clicked();

    void on_decrypt_timelock_clicked();

    void on_back_10_clicked();

    void on_time_lock_clicked();

private:
    Ui::secureX *ui;
};
#endif // SECUREX_H
