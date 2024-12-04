/*
This file is part of LabelmakerDrawBluetooth.

LabelmakerDrawBluetooth is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation version 3 or later.

LabelmakerDrawBluetooth is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with LabelmakerDrawBluetooth. If not, see <https://www.gnu.org/licenses/>.
*/
#include "WebServer.h"

const char *WebServer::m_Draw_html =
"<!DOCTYPE html> \
<html> \
<head> \
  <title>HackPack Draw Label</title> \
  <style> \
    .content { \
      display: flex; \
      justify-content: center; \
      align-items: center; \
      flex-direction: column; \
    } \
 \
    button { \
      width: 100px; \
    } \
    .buttonCell { \
      padding-right: 10px; \
    } \
 \
    canvas { \
      border: 3px solid blue; \
      width: 800px; \
      height: 100px; \
    } \
 \
    .statusTable { \
      margin-top: 20px \
    } \
  </style> \
</head> \
<body> \
  <div class=\"content\"> \
    <h1> \
      HackPack Draw Label \
    </h1> \
    <table width=\"800\"> \
      <tr> \
        <td colspan=\"8\"><canvas id=\"canvas\" width=\"800\" height=\"100\"></canvas></td> \
      </tr> \
      <tr> \
        <td width=\"50\" class=\"buttonCell\"><button id=\"draw\">Free Draw</button></td> \
        <td width=\"50\" class=\"buttonCell\"><button id=\"line\">Line</button></td> \
        <td width=\"50\" class=\"buttonCell\"><button id=\"circle\">Circle</button></td> \
        <td></td> \
        <td width=\"50\" class=\"buttonCell\"><button id=\"undo\">Undo</button></td> \
        <td width=\"50\"><button id=\"clear\">Clear</button></td> \
        <td></td> \
        <td width=\"50\"><button id=\"print\">Print</button></td> \
      </tr> \
    </table> \
    <table width=\"800\" class=\"statusTable\"> \
      <tr> \
        <td align=\"left\" width=\"30\">Status:</td> \
        <td id=\"status\" align=\"left\">Waiting for image</td> \
      </tr> \
    </table> \
  </div> \
  <script src=\"draw.js\"></script> \
</body> \
</html>";
