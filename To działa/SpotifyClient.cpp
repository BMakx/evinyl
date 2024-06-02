

#define CORE_DEBUG_LEVEL ARDUHAL_LOG_LEVEL_VERBOSE

#include <HTTPClient.h>
#include "SpotifyClient.h"
#include <base64.h>
#include <ArduinoJson.h>

SpotifyClient::SpotifyClient(String clientId, String clientSecret, String deviceName, String refreshToken) {
  this->clientId = clientId;
  this->clientSecret = clientSecret;
  this->refreshToken = refreshToken;
  this->deviceName = deviceName;

  client.setCACert(digicert_root_ca);
}

int SpotifyClient::FetchToken() {
    HTTPClient http;
    http.begin("https://accounts.spotify.com/api/token");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "grant_type=refresh_token&refresh_token=" + String(refreshToken) + "&client_id=" + String(clientId) + "&client_secret=" + String(clientSecret);
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
        String payload = http.getString();
        Serial.println("FetchToken response: " + payload);
        
        // Parse JSON response
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        accessToken = doc["access_token"].as<String>();
        Serial.println("Access Token: " + accessToken);
    } else {
        Serial.print("Error on HTTP request: ");
        Serial.println(http.errorToString(httpResponseCode));
    }

    http.end();
    return httpResponseCode;
}

int SpotifyClient::Shuffle() {
    HTTPClient http;
    String url = "https://api.spotify.com/v1/me/player/shuffle?state=true&device_id=" + deviceId;
    
    Serial.println("Starting HTTP request to URL: " + url);
    
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + accessToken);
    http.addHeader("Content-Length", "0");  // Explicitly set Content-Length to 0

    int httpResponseCode = http.PUT("");  // Empty body

    Serial.println("HTTP response code: " + String(httpResponseCode));
    
   

    http.end();
    return httpResponseCode;
}


int SpotifyClient::Next()
{
    Serial.println("Next()");
    HttpResult result = CallAPI( "POST", "https://api.spotify.com/v1/me/player/next?device_id=" + deviceId, "" );
    return result.httpCode;
}


int SpotifyClient::Play(const String& contextUri) {
    HTTPClient http;
    String url = "https://api.spotify.com/v1/me/player/play?device_id=" + deviceId;
    
    Serial.println("Starting HTTP request to URL: " + url);
    
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + accessToken);
    http.addHeader("Content-Type", "application/json");

    String postData = "{\"context_uri\":\"" + contextUri + "\"}";
    
    Serial.println("Sending POST data: " + postData);
    
    int httpResponseCode = http.PUT(postData);
    
    Serial.println("HTTP response code: " + String(httpResponseCode));
    http.end();
    return httpResponseCode;
}


HttpResult SpotifyClient::CallAPI( String method, String url, String body )
{
    HttpResult result;
    result.httpCode = 0;
    Serial.print(url);
    Serial.print( " returned: " );

    HTTPClient http;
    http.begin(client, url); 
    
    String authorization = "Bearer " + accessToken;

    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("Authorization"), authorization);

    // address bug where Content-Length not added by HTTPClient is content length is zero
    if ( body.length() == 0 )
    {
         http.addHeader(F("Content-Length"), String(0));
    }

    if ( method == "PUT" )
    {
        result.httpCode = http.PUT(body); 
    }
    else if ( method == "POST" )
    {
        result.httpCode = http.POST(body); 
    }
    else if ( method == "GET" )
    {
        result.httpCode = http.GET(); 
    }    

    if (result.httpCode > 0) 
    { 
        Serial.println(result.httpCode);
        if ( http.getSize() > 0 )
        {
            result.payload = http.getString();
        }
    }
    else 
    {
        Serial.print("Failed to connect to ");
        Serial.println(url);
    }
    http.end(); 

    return result;
}

/*
int SpotifyClient::CallAPI( String method, String url, String body )
{
    HTTPClient http;
    http.begin(client, url); 
    http.addHeader("Content-Type", "application/json");
    String authorization = "Bearer " + accessToken;
    http.addHeader("Authorization", authorization);

    int httpCode = 0;
    if ( method == "PUT" )
    {
        httpCode = http.PUT(body); 
    }
    else if ( method == "POST" )
    {
        httpCode = http.POST(body); 
    }
    else if ( method == "GET" )
    {
        httpCode = http.GET(); 
    }
    if (httpCode > 0) { //Check for the returning code
        String returnedPayload = http.getString();
        if ( httpCode == 200 )
        {
            accessToken = ParseJson("access_token", returnedPayload );
            Serial.println("Got new access token");
        }
        else
        {
            Serial.print("API call returned error: ");
            Serial.println(httpCode);
            Serial.println(returnedPayload);
        }
    }
    else {
        Serial.print("Failed to connect to ");
        Serial.println(url);
    }
    http.end(); //Free the resources

    return httpCode;
}
*/

int SpotifyClient::GetDevices() {
    HTTPClient http;
    http.begin("https://api.spotify.com/v1/me/player/devices");
    http.addHeader("Authorization", "Bearer " + accessToken);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        String payload = http.getString();
        Serial.println("GetDevices response: " + payload);
        // Parse and handle device data here
        parseDeviceId(payload);
    } else {
        Serial.print("Error on HTTP request: ");
        Serial.println(http.errorToString(httpResponseCode));
    }

    http.end();
    return httpResponseCode;
}


void SpotifyClient::parseDeviceId(const String& payload) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
        return;
    }

    JsonArray devices = doc["devices"].as<JsonArray>();
    for (JsonObject device : devices) {
        String name = device["name"].as<String>();
        if (name == deviceName) {
            deviceId = device["id"].as<String>();
            Serial.println("Device ID: " + deviceId);
            return;
        }
    }

    Serial.println("Device not found");
}
