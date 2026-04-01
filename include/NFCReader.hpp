#ifndef NFCREADER_HPP
#define NFCREADER_HPP

#include <nfc/nfc.h>
#include <string>
#include <vector>

class NFCReader {
public:
    NFCReader();
    ~NFCReader();

    bool connect();          // Initializes the I2C connection
    std::string scanCard();  // Returns the ID of a card if present

private:
    nfc_context *context;
    nfc_device *device;
};

#endif