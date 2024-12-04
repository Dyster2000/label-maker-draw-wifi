/*
This file is part of LabelMakerDrawBluetooth.

LabelMakerDrawBluetooth is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation version 3 or later.

LabelMakerDrawBluetooth is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with LabelMakerDrawBluetooth. If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include <WiFi.h>
#include <string>
#include <vector>

struct DrawPoint
{
  DrawPoint(uint32_t x = 0, uint32_t y = 0, bool draw = false)
    : X{x}
    , Y{y}
    , Draw{draw}
  {}

  uint32_t X{0};
  uint32_t Y{0};
  bool Draw{0};
};

class WebServer
{
public:
  WebServer(std::string ssid, std::string password);

  void Init();
  void Loop();

  IPAddress GetWifiAddress();
  bool ClientConnected();

  bool HasReceivedImage();
  size_t GetImagePointCount();
  DrawPoint GetPoint(size_t index);
  void ClearImage();

private:
  void SendReturn(WiFiClient &client, const char *body);
  void SendHeader(WiFiClient &client);
  void SendBadReturn(WiFiClient &client);
  void SendAjaxReturn(WiFiClient &client, bool good, const char *body);

  bool ParseImageData(const char *data);

  std::string ToLower(std::string s);
  std::vector<std::string> Split(const std::string &s, char delim);

private:
  static constexpr uint16_t PORT = 80;
  static const char *m_Draw_html;
  static const char *m_Draw_js;

  std::string m_Ssid;
  std::string m_Password;
  WiFiServer m_Server;
  bool m_ClientConnected;

  std::vector<DrawPoint> m_Points;
};
