/*
This file is part of LabelmakerDrawBluetooth.

LabelmakerDrawBluetooth is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation version 3 or later.

LabelmakerDrawBluetooth is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with LabelmakerDrawBluetooth. If not, see <https://www.gnu.org/licenses/>.
*/
#include "WebServer.h"
#include <ArduinoJson.h>
#include <sstream>

WebServer::WebServer(std::string ssid, std::string password)
  : m_Ssid{ ssid }
  , m_Password{ password }
  , m_Server{PORT}
  , m_ClientConnected{ false }
{

}

void WebServer::Init()
{
  // Connect to WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(m_Ssid.c_str(), m_Password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start the server
  m_Server.begin();
  Serial.println("WebServer started.");
}

IPAddress WebServer::GetWifiAddress()
{
  return WiFi.localIP();
}

bool WebServer::ClientConnected()
{
  return m_ClientConnected;
}

void WebServer::Loop()
{
  WiFiClient client = m_Server.available();

  if (client)
  {
    std::stringstream requestBuff;
    std::string header;
    std::string headerRequest;
    std::string body;
    bool readingHeader = true;
    bool blankLine = true;
    
    while (client.connected() && !client.available())
    {
      delay(1);
    }
    m_ClientConnected = true;

    // Read HTTP Request
    while (client.available())
    {
      char c = client.read();
      requestBuff << c;
      if (c != '\r' && c != '\n')
        blankLine = false;
      if ((readingHeader) && (c == '\n'))
      {
        if (blankLine)
        {
          header = requestBuff.str();
          requestBuff = std::stringstream();
          readingHeader = false;
        }
        else
        {
          blankLine = true;
          if (headerRequest.empty())
            headerRequest = requestBuff.str();
        }
      }
    }
    body = requestBuff.str();

    auto parts = Split(headerRequest, ' '); 
    if (parts.size() >= 2)
    {
      auto verb = parts[0];
      auto target = ToLower(parts[1]);

      if (verb == "GET")
      {
        if ((target == "/") || (target == "/draw.html"))
        {
          SendReturn(client, m_Draw_html);
        }
        else if (target == "/draw.js")
        {
          SendReturn(client, m_Draw_js);
        }
        else
        {
          SendBadReturn(client);
        }
      }
      else if (verb == "POST")
      {
        if (target == "/image")
        {
          Serial.println("[WebServer] Receiving image");
          if (ParseImageData(body.c_str()))
            SendAjaxReturn(client, true, "Printing...");
          else
            SendAjaxReturn(client, false, "Error receiving data");
        }
        else if (target == "/status")
        {
          if (m_Points.size() == 0)
            SendAjaxReturn(client, true, "Waiting for image");
          else
            SendAjaxReturn(client, true, "Printing...");
        }
      }
      else
      {
        SendBadReturn(client);
      }
    }
  }
}

void WebServer::SendReturn(WiFiClient &client, const char *body)
{
  SendHeader(client);
  client.println(body);
}

void WebServer::SendHeader(WiFiClient &client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: keep-alive");
  client.println();  
}

void WebServer::SendBadReturn(WiFiClient &client)
{
  client.println("HTTP/1.1 404 Not Found");
  client.println("Content-type:text/html");
  client.println("Connection: keep-alive");
  client.println();  
}

void WebServer::SendAjaxReturn(WiFiClient &client, bool good, const char *body)
{
  if (good)
    client.println("HTTP/1.1 200 OK");
  else
    client.println("HTTP/1.1 500 Internal Server Error");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  if (body[0] != 0)
    client.println(body);
}

bool WebServer::ParseImageData(const char *data)
{
  JsonDocument doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, data);

  // Test if parsing succeeds
  if (error)
  {
    Serial.print(F("[ParseImageData] deserializeJson() failed: "));
    Serial.println(error.f_str());
    return false;
  }
  Serial.println("[ParseImageData] Load data");

  m_Points.clear();
  auto size = doc.size();

  if (size == 0)
  {
    Serial.println(F("[ParseImageData] Received data was empty"));
    return false;
  }
  uint32_t xMin = 1000;
  uint32_t yMin = 1000;
  uint32_t xMax = 0;
  uint32_t yMax = 0;
  for (size_t i = 0; i < size; i++)
  {
    auto entry = doc[i];
    uint32_t x = entry["x"];
    uint32_t y = entry["y"];
    bool draw = entry["draw"];

    if (x < xMin)
      xMin = x;
    if (y < yMin)
      yMin = y;
    if (x > xMax)
      xMax = x;
    if (y > yMax)
      yMax = y;

    m_Points.push_back({ x, y, draw });
  }
 
  return true;
}

bool WebServer::HasReceivedImage()
{
  return m_Points.size() > 0;
}

size_t WebServer::GetImagePointCount()
{
  return m_Points.size();
}

DrawPoint WebServer::GetPoint(size_t index)
{
  if (index < m_Points.size())
    return m_Points[index];
  else
    return DrawPoint();
}

void WebServer::ClearImage()
{
  m_Points.clear();
}

std::string WebServer::ToLower(std::string s)
{
  std::transform(s.begin(), s.end(), s.begin(),
                  [](unsigned char c){ return std::tolower(c); }
                );
  return s;
}

std::vector<std::string> WebServer::Split(const std::string &s, char delim) 
{
  std::vector<std::string> result;
  std::stringstream ss(s);
  std::string item;

  while (getline(ss, item, delim))
    result.push_back(item);

  return result;
}