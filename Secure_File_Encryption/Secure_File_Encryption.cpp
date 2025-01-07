// Secure_File_Encryption.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
/*How It Works:
Key and IV Generation: The generateKeyAndIV function creates a random 256-bit key and 128-bit IV using OpenSSL's RAND_bytes.
File Encryption: The encryptFile function reads the input file in chunks, encrypts it with AES-256-CBC, and writes the encrypted data to the output file.
Key and IV Storage: The generated key and IV are saved to a binary file (key.bin) for later decryption.
Setup Instructions:
Install OpenSSL:
On Ubuntu: sudo apt install libssl-dev
On macOS: brew install openssl
Compile the Program:
bash
Copy code
g++ -std=c++23 -o encrypt main.cpp -lssl -lcrypto
Run the Program: Place a file named input.txt in the same directory as the executable and run:
bash
Copy code
./encrypt
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <openssl/evp.h>
#include <openssl/rand.h>

// AES configuration
constexpr int AES_KEY_SIZE = 32;  // 256 bits
constexpr int AES_BLOCK_SIZE = 16;  // 128 bits

// Function to generate a random key and IV
bool generateKeyAndIV(std::vector<unsigned char>& key, std::vector<unsigned char>& iv) {
    key.resize(AES_KEY_SIZE);
    iv.resize(AES_BLOCK_SIZE);

    if (!RAND_bytes(key.data(), AES_KEY_SIZE) || !RAND_bytes(iv.data(), AES_BLOCK_SIZE)) {
        std::cerr << "Error generating random key/IV!" << std::endl;
        return false;
    }

    return true;
}

// Function to encrypt data
bool encryptFile(const std::string& inputFilePath, const std::string& outputFilePath,
    const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv) {
    // Open input file
    std::ifstream inputFile(inputFilePath, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open input file!" << std::endl;
        return false;
    }

    // Open output file
    std::ofstream outputFile(outputFilePath, std::ios::binary);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not open output file!" << std::endl;
        return false;
    }

    // Create and initialize the encryption context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cerr << "Error: Could not create encryption context!" << std::endl;
        return false;
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        std::cerr << "Error initializing encryption!" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    // Encrypt file data
    std::vector<unsigned char> buffer(1024);  // Read in chunks
    std::vector<unsigned char> encryptedBuffer(buffer.size() + AES_BLOCK_SIZE);
    int bytesRead, encryptedBytes;

    while ((bytesRead = inputFile.read(reinterpret_cast<char*>(buffer.data()), buffer.size()).gcount()) > 0) {
        if (EVP_EncryptUpdate(ctx, encryptedBuffer.data(), &encryptedBytes, buffer.data(), bytesRead) != 1) {
            std::cerr << "Error during encryption!" << std::endl;
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        outputFile.write(reinterpret_cast<char*>(encryptedBuffer.data()), encryptedBytes);
    }

    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx, encryptedBuffer.data(), &encryptedBytes) != 1) {
        std::cerr << "Error finalizing encryption!" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    outputFile.write(reinterpret_cast<char*>(encryptedBuffer.data()), encryptedBytes);

    // Clean up
    EVP_CIPHER_CTX_free(ctx);
    return true;
}

int main() {
    // File paths
    std::string inputFilePath = "input.txt";  // Replace with your file
    std::string outputFilePath = "output.enc";

    // Generate key and IV
    std::vector<unsigned char> key, iv;
    if (!generateKeyAndIV(key, iv)) {
        return 1;
    }

    // Encrypt the file
    if (encryptFile(inputFilePath, outputFilePath, key, iv)) {
        std::cout << "File encrypted successfully!" << std::endl;

        // Save the key and IV (in a real application, secure these properly!)
        std::ofstream keyFile("key.bin", std::ios::binary);
        keyFile.write(reinterpret_cast<const char*>(key.data()), key.size());
        keyFile.write(reinterpret_cast<const char*>(iv.data()), iv.size());
        keyFile.close();

        std::cout << "Key and IV saved to 'key.bin'." << std::endl;
    }
    else {
        std::cerr << "File encryption failed!" << std::endl;
        return 1;
    }

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
