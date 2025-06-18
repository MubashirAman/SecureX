
 🛡️ secureX – Basic Level File & Data Protection Tool (C++ / Qt)

secureX is a beginner-to-intermediate level C++/Qt-based desktop application designed to provide multiple data protection tools in one place. It uses simple logic and UI-based control to perform file encryption, image steganography, USB-based file binding, password generation, and time-locked encryption.

🔧 Features & Logic Used

1.  📁 **File Encryption / Decryption**

   * Two basic custom encryption algorithms are supported:

     * **ReflectoSub Encryption** (character substitution using key)
     * **Shift Encryption** (ROT-like logic with key-based shifting)
   * Key is entered by user through a dialog.
   * Output is saved in a chosen folder using standard file I/O.

2.  💽 **USB-Based File Encryption**

   * The selected file is encrypted and bound with the **connected USB device**.
   * Decryption is only allowed if the **same USB is plugged in**.
   * The USB ID is detected using basic device ID detection.

3.  🖼️ **Image Steganography**

   * Hides a text message inside a selected image (PNG, JPG, etc.).
   * Uses simple LSB (Least Significant Bit)-like logic (assumed).
   * Extracted message is written to a new text file.

4.  🔐 **Password Generator**

   * Generates a random strong password (up to 24 characters).
   * Includes a strength checker that evaluates password quality.
   * Logic is based on checking length, digits, and symbol presence.

5.  ⏳ **Time-Lock Encryption**

   * Lets users encrypt a file and set a **future unlock time**.
   * File can't be decrypted until the defined time is reached.
   * Uses basic datetime comparison logic for access control.

🛠️ Technologies Used

* **C++ (OOP Concepts)**
* **Qt Widgets (UI Framework)**
* File I/O (`fstream`)
* Basic string processing
* Simple encryption logic (no external cryptography library)

📚 Skill Level

This project is **beginner-to-intermediate** level:

* Good for students learning C++, Qt, and basic encryption concepts.
* No complex algorithms or external libraries are used.
* Focus is on understanding the **core logic** behind file protection methods in a simple GUI.
