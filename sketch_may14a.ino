#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "SpotifyEsp32.h"

const char* SSID = "sqrt(5,76)GHz";
const char* PASSWORD = "AhaMelina23!";
const char* CLIENT_ID = "df744a36e1d14361a7c7998e24df2b47";
const char* CLIENT_SECRET = "e6bb529bb2ee48858f021713de8c9020";
const char* refresh = "AQBVLgsZq7FmsXP8t_afidx73TTNPAJ6tfpIGZw_SO3Usc7GENdTghyqNBnuvkuXuLGyxmoU3_l5Qv713z7FJgHHHN_-u3InhAed0zizczNElQ2VhV8TEXZAHFUzUKs15ck";

//Create an instance of the Spotify class Optional: you can set the Port for the webserver the debug mode(This prints out data to the serial monitor) and number of retries
Spotify sp(CLIENT_ID, CLIENT_SECRET,refresh);

void setup() {
    Serial.begin(115200);
    connect_to_wifi();//Connect to your wifi
    
    sp.begin();//Start the webserver
    while(!sp.is_auth()){//Wait for the user to authenticate
        sp.handle_client();//Handle the client, this is necessary otherwise the webserver won't work
    }
    Serial.println("Authenticated");
    response r = sp.current_playback_state();
    print_response(r);
    Serial.println(sp.current_artist_names());
}

void loop() {
    //Add your code here
}
void connect_to_wifi(){
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.printf("\nConnected to WiFi\n");
}
