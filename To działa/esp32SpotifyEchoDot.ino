#include <Arduino.h>
#include <WiFi.h>
#include "SpotifyClient.h"
#include <SPI.h>
#include "MFRC522.h"
#include "settings.h"

#define RST_PIN 4 // Configurable, see typical pin layout above
#define SS_PIN 21 // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
byte const BUFFERSIZE = 176;

SpotifyClient spotify(clientId, clientSecret, deviceName, refreshToken);

void setup() {
    Serial.begin(115200);
    Serial.println("Setup started");

    connectWifi();

    // Initialize SPI bus and MFRC522 NFC reader
    SPI.begin();
    mfrc522.PCD_Init();

    // Refresh Spotify Auth token and Device ID
    int fetchTokenResult = spotify.FetchToken();
    if (fetchTokenResult == 200) {
        int getDevicesResult = spotify.GetDevices();
        if (getDevicesResult != 200) {
            Serial.println("Failed to get Spotify devices");
        }
    } else {
        Serial.println("Failed to fetch Spotify token");
    }

    Serial.println("Setup complete");
}

void loop() {
    if (mfrc522.PICC_IsNewCardPresent()) {
        Serial.println("NFC tag present");
        readNFCTag();
        delay(1000); // Add delay to avoid immediate re-triggering
        
    }
}

void readNFCTag() {
    if (mfrc522.PICC_ReadCardSerial()) {
        byte dataBuffer[BUFFERSIZE];
        bool success = readNFCTagData(dataBuffer);

        if (success) {
            Serial.print("Read NFC tag: ");
            String context_uri = parseNFCTagData(dataBuffer);
            Serial.println(context_uri);
            playSpotifyUri(context_uri);
        } else {
            Serial.println("Failed to read NFC tag data");
        }

  
    } else {
        Serial.println("Failed to read card serial");
    }
}

void playSpotifyUri(String context_uri) {
    int code = spotify.Play(context_uri);
    switch (code) {
        case 404:
            // device id changed, get new one
            spotify.GetDevices();
            spotify.Play(context_uri);
            spotify.Shuffle();
            break;
        case 401:
            // auth token expired, get new one
            spotify.FetchToken();
            spotify.Play(context_uri);
            spotify.Shuffle();
            break;
        default:
            spotify.Shuffle();
            break;
    }
    spotify.Shuffle();
}

bool readNFCTagData(byte* dataBuffer) {
    MFRC522::StatusCode status;
    byte byteCount;
    byte buffer[18];
    byte x = 0;

    int totalBytesRead = 0;

    // Reset the dataBuffer
    memset(dataBuffer, 0, BUFFERSIZE);

    for (byte page = 0; page < BUFFERSIZE / 4; page += 4) {
        // Read pages
        byteCount = sizeof(buffer);
        status = mfrc522.MIFARE_Read(page, buffer, &byteCount);
        if (status == mfrc522.STATUS_OK) {
            totalBytesRead += byteCount - 2;

            for (byte i = 0; i < byteCount - 2; i++) {
                dataBuffer[x++] = buffer[i]; // Add data to output buffer
            }
        } else {
            Serial.print("MIFARE_Read() failed: ");
            Serial.println(mfrc522.GetStatusCodeName(status));
            break;
        }
    }
    return totalBytesRead > 0;
}

String parseNFCTagData(byte* dataBuffer) {
    // First 28 bytes is header info
    // Data ends with 0xFE
    String retVal = "spotify:";
    for (int i = 28+12; i < BUFFERSIZE; i++) {
        if (dataBuffer[i] == 0xFE || dataBuffer[i] == 0x00) {
            break;
        }
        if (dataBuffer[i] == '/') {
            retVal += ':';
        } else {
            retVal += (char)dataBuffer[i];
        }
    }
    return retVal;
}

void connectWifi() {
    WiFi.begin(ssid, pass);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}
