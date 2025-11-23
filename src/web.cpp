#include <web.h>

SimpleWebServer::SimpleWebServer() : server(80), ssid(WIFI_SSID), password(WIFI_PASS) {}

bool SimpleWebServer::begin() 
{
    if (initialized)
        return true;

    Serial.println("Starting web server...");
    
    // Connect to WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    Serial.print("Connecting to WiFi");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) 
    {
        delay(1000);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("\nWiFi connection failed!");
        return false;
    }
    
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Setup mDNS
    if (MDNS.begin("thermostat")) 
    {
        Serial.println("mDNS responder started");
        Serial.println("You can access via: http://thermostat.local");
    }
    
    // Initialize LittleFS
    if (!LittleFS.begin(true))
    {
        Serial.println("LittleFS Mount Failed");
        return false;
    }

    // Define web server routes --------------------------------------------------
    
    server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request)
    {
        String response = apiHandler.handleStatus();
        request->send(200, "application/json", response);
    });
    
    server.on("/api/temperature", HTTP_GET, [this](AsyncWebServerRequest *request) 
    {
       String response = apiHandler.handleGetCurrentTemperature();
       request->send(200, "application/json", response); 
    });
    
    server.on("/api/humidity", HTTP_GET, [this](AsyncWebServerRequest *request) 
    {
       String response = apiHandler.handleGetCurrentHumidity();
       request->send(200, "application/json", response); 
    });
    
    server.on("/api/target", HTTP_GET, [this](AsyncWebServerRequest *request)
    {
        String response = apiHandler.handleGetTargetTemperature();
        request->send(200, "application/json", response);
    });
    
    server.on("/api/target/set", HTTP_POST, [this](AsyncWebServerRequest *request) {}, NULL, [this](AsyncWebServerRequest *request, uint8_t *body, size_t len, size_t index, size_t total)
    {
        String requestBody = String((char*)body, len);
        String response = apiHandler.handleSetTargetTemperature(requestBody);
        request->send(200, "application/json", response);
    });
    
    server.on("/api/eco-temp", HTTP_GET, [this](AsyncWebServerRequest *request)
    {
        String response = apiHandler.handleGetEcoTemperature();
        request->send(200, "application/json", response);
    });

    server.on("/api/eco-temp/set", HTTP_POST, [this](AsyncWebServerRequest *request) {}, NULL, [this](AsyncWebServerRequest *request, uint8_t *body, size_t len, size_t index, size_t total)
    {
        String requestBody = String((char*)body, len);
        String response = apiHandler.handleSetEcoTemperature(requestBody);
        request->send(200, "application/json", response);
    });
    
    server.on("/api/mode", HTTP_GET, [this](AsyncWebServerRequest *request) 
    {
        String response = apiHandler.handleGetMode();
        request->send(200, "application/json", response);
    });
    
    server.on("/api/mode/set", HTTP_POST, [this](AsyncWebServerRequest *request) {}, NULL, [this](AsyncWebServerRequest *request, uint8_t *body, size_t len, size_t index, size_t total) 
    {
       String requestBody = String((char*)body, len);
       String response = apiHandler.handleSetMode(requestBody);
       request->send(200, "application/json", response); 
    });
    
    server.on("/api/temperature/max", HTTP_GET, [this](AsyncWebServerRequest *request) 
    {
        String response = apiHandler.handleGetMaxTemperature();
        request->send(200, "application/json", response);
    });
    
    server.on("/api/temperature/max/set", HTTP_POST, [this](AsyncWebServerRequest *request) {}, NULL, [this](AsyncWebServerRequest *request, uint8_t *body, size_t len, size_t index, size_t total) 
    {
        String requestBody = String((char*)body, len);
        String response = apiHandler.handleSetMaxTemperature(requestBody);
        request->send(200, "application/json", response);
    });
    
    server.on("/api/temperature/min", HTTP_GET, [this](AsyncWebServerRequest *request) 
    {
        String response = apiHandler.handleGetMinTemperature();
        request->send(200, "application/json", response);
    });
    
    server.on("/api/temperature/min/set", HTTP_POST, [this](AsyncWebServerRequest *request) {}, NULL, [this](AsyncWebServerRequest *request, uint8_t *body, size_t len, size_t index, size_t total) 
    {
        String requestBody = String((char*)body, len);
        String response = apiHandler.handleSetMinTemperature(requestBody);
        request->send(200, "application/json", response);
    });

    // ---------------------------------------------------------------------------


    // Handle app.js with correct MIME type
    server.on("/app.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/app.js", "application/javascript");
    });

    // Handle style.css with correct MIME type
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/style.css", "text/css");
    });
    
    // Handle icons with correct MIME types
    server.on("/icon-192.png", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/icon-192.png", "image/png");
    });

    server.on("/icon-512.png", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/icon-512.png", "image/png");
    });
    
    // Server static files from LittleFS
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    // Not found catch
    server.onNotFound([this](AsyncWebServerRequest *request) { handleNotFound(request); });
    
    // Start web server
    server.begin();
    Serial.println("HTTP server started on port 80");
    Serial.println();

    initialized = true;

    return true;
}

bool SimpleWebServer::isInitialized() 
{
    return initialized;
}

bool SimpleWebServer::isConnected() 
{
    return WiFi.status() == WL_CONNECTED;
}

String SimpleWebServer::getIP() 
{
    return WiFi.localIP().toString();
}

// =============================================================================
// Route Handlers
// =============================================================================

void SimpleWebServer::handleStatus(AsyncWebServerRequest *request)
{
    String json = "{";
    json += "\"status\":\"ok\",";
    json += "\"ip\":\"";
    json += WiFi.localIP().toString();
    json += "\",\"rssi\":";
    json += String(WiFi.RSSI());
    json += ",\"heap\":";
    json += String(ESP.getFreeHeap());
    json += ",\"uptime\":";
    json += String(millis()/1000);
    json += "}";
    
    request->send(200, "application/json", json);
}

void SimpleWebServer::handleNotFound(AsyncWebServerRequest *request)
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += request->url();
    message += "\nMethod: ";
    message += (request->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += String(request->args());
    message += "\n";
    
    for (int i = 0; i < request->args(); i++) {
        message += " ";
        message += request->argName(i);
        message += ": ";
        message += request->arg(i);
        message += "\n";
    }
    
    request->send(404, "text/plain", message);
}