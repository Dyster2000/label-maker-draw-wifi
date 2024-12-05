/*
This file is part of LabelmakerDrawBluetooth.

LabelmakerDrawBluetooth is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation version 3 or later.

LabelmakerDrawBluetooth is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with LabelmakerDrawBluetooth. If not, see <https://www.gnu.org/licenses/>.
*/
#include "WebServer.h"

const char *WebServer::m_Draw_js = 
"var canvas = document.getElementById('canvas'); \r\n\
var lineButton = document.getElementById('line'); \r\n\
var circleButton = document.getElementById('circle'); \r\n\
var undoButton = document.getElementById('undo'); \r\n\
var drawButton = document.getElementById('draw'); \r\n\
var clearButton = document.getElementById('clear'); \r\n\
var printButton = document.getElementById('print'); \r\n\
var statusText = document.getElementById('status'); \r\n\
var ctx = canvas.getContext('2d'); \r\n\
var canasRect = canvas.getBoundingClientRect(); \r\n\
var storedLines = []; \r\n\
var storedGroups = []; \r\n\
var startX = 0; \r\n\
var startY = 0; \r\n\
var lastX = 0; \r\n\
var lastY = 0; \r\n\
var isDown = false; \r\n\
var groupStart = false; \r\n\
var mode = 'draw'; \r\n\
 \r\n\
ctx.strokeStyle = 'black'; \r\n\
ctx.lineWidth = 7; \r\n\
 \r\n\
printButton.disabled = true; \r\n\
 \r\n\
canvas.addEventListener('mousedown', (e) => \r\n\
{ \r\n\
  handleMouseDown(e, e.offsetX, e.offsetY); \r\n\
}); \r\n\
canvas.addEventListener('mousemove', (e) => \r\n\
{ \r\n\
  handleMouseMove(e, e.offsetX, e.offsetY); \r\n\
}); \r\n\
canvas.addEventListener('mouseup', (e) => \r\n\
{ \r\n\
  handleMouseUp(e); \r\n\
}); \r\n\
canvas.addEventListener('mouseout', (e) => \r\n\
{ \r\n\
  handleMouseOut(e); \r\n\
}); \r\n\
document.body.addEventListener('touchstart', (e) => \r\n\
{ \r\n\
  if (e.target == canvas) \r\n\
  { \r\n\
    let x = e.touches[0].clientX - canasRect.left; \r\n\
    let y = e.touches[0].clientY - canasRect.top; \r\n\
    handleMouseDown(e, x, y); \r\n\
  } \r\n\
}, false); \r\n\
document.body.addEventListener('touchend', (e) => \r\n\
{ \r\n\
  if (e.target == canvas) \r\n\
  { \r\n\
    handleMouseUp(e); \r\n\
  } \r\n\
}, false); \r\n\
document.body.addEventListener('touchmove', (e) => \r\n\
{ \r\n\
  if (e.target == canvas) \r\n\
  { \r\n\
    let x = e.touches[0].clientX - canasRect.left; \r\n\
    let y = e.touches[0].clientY - canasRect.top; \r\n\
    handleMouseMove(e, x, y); \r\n\
  } \r\n\
}, false); \r\n\
document.body.addEventListener('touchcancel', (e) => \r\n\
{ \r\n\
  if (e.target == canvas) \r\n\
  { \r\n\
    handleMouseOut(e); \r\n\
  } \r\n\
}, false); \r\n\
drawButton.addEventListener('click', (e) => \r\n\
{ \r\n\
  mode = 'draw'; \r\n\
}); \r\n\
lineButton.addEventListener('click', (e) => \r\n\
{ \r\n\
  mode = 'line'; \r\n\
}); \r\n\
circleButton.addEventListener('click', (e) => \r\n\
{ \r\n\
  mode = 'circle'; \r\n\
}); \r\n\
undoButton.addEventListener('click', (e) => \r\n\
{ \r\n\
  if (storedGroups.length > 0) \r\n\
  { \r\n\
    let clearTo = storedGroups.pop(); \r\n\
 \r\n\
    storedLines.length = clearTo; \r\n\
    redrawStoredLines(); \r\n\
  } \r\n\
}); \r\n\
clearButton.addEventListener('click', (e) => \r\n\
{ \r\n\
  storedLines.length = 0; \r\n\
  storedGroups.length = 0; \r\n\
  redrawStoredLines(); \r\n\
  printButton.disabled = true; \r\n\
}); \r\n\
printButton.addEventListener('click', (e) => \r\n\
{ \r\n\
  printButton.disabled = true; \r\n\
  sendImage(); \r\n\
}); \r\n\
 \r\n\
function handleMouseDown(e, xPos, yPos) \r\n\
{ \r\n\
  e.preventDefault(); \r\n\
  e.stopPropagation(); \r\n\
 \r\n\
  isDown = true; \r\n\
  groupStart = true; \r\n\
  startX = xPos; \r\n\
  startY = yPos; \r\n\
  lastX = xPos; \r\n\
  lastY = yPos; \r\n\
} \r\n\
 \r\n\
function handleMouseMove(e, xPos, yPos) \r\n\
{ \r\n\
  e.preventDefault(); \r\n\
  e.stopPropagation(); \r\n\
 \r\n\
  if (!isDown) \r\n\
    return; \r\n\
 \r\n\
  if (mode == 'line') \r\n\
  { \r\n\
    redrawStoredLines(); \r\n\
 \r\n\
    ctx.beginPath(); \r\n\
    ctx.moveTo(startX, startY); \r\n\
    ctx.lineTo(xPos, yPos); \r\n\
    ctx.stroke(); \r\n\
  } \r\n\
  else if (mode == 'circle') \r\n\
  { \r\n\
    redrawStoredLines(); \r\n\
 \r\n\
    let distX = startX - xPos; \r\n\
    let distY = startY - yPos; \r\n\
    let radius = Math.sqrt(distX * distX + distY * distY) \r\n\
    ctx.beginPath(); \r\n\
    ctx.arc(startX, startY, radius, 0, 2 * Math.PI); \r\n\
    ctx.stroke(); \r\n\
  } \r\n\
  else \r\n\
  { \r\n\
    addLine(startX, startY, xPos, yPos); \r\n\
    startX = xPos; \r\n\
    startY = yPos; \r\n\
  } \r\n\
  lastX = xPos; \r\n\
  lastY = yPos; \r\n\
} \r\n\
 \r\n\
function handleMouseUp(e) \r\n\
{ \r\n\
  e.preventDefault(); \r\n\
  e.stopPropagation(); \r\n\
 \r\n\
  if (!isDown) \r\n\
    return; \r\n\
 \r\n\
  if (mode == 'line') \r\n\
  { \r\n\
    addLine(startX, startY, lastX, lastY); \r\n\
  } \r\n\
  else if (mode == 'circle') \r\n\
  { \r\n\
    let distX = startX - lastX; \r\n\
    let distY = startY - lastY; \r\n\
    let radius = Math.sqrt(distX * distX + distY * distY); \r\n\
    let numPoints = Math.max(parseInt(radius / 2), 10); \r\n\
    let angleStep = 2.0 * Math.PI / numPoints; \r\n\
    let angle = 0; \r\n\
    let firstX = startX + radius * Math.cos(angle); \r\n\
    let firstY = startY + radius * Math.sin(angle); \r\n\
    let prevX = firstX; \r\n\
    let prevY = firstY; \r\n\
    let x = 0; \r\n\
    let y = 0; \r\n\
    for (i = 1; i < numPoints; i++) \r\n\
    { \r\n\
      angle = angleStep * i; \r\n\
      x = startX + radius * Math.cos(angle); \r\n\
      y = startY + radius * Math.sin(angle); \r\n\
 \r\n\
      addLine(prevX, prevY, x, y, false); \r\n\
      prevX = x; \r\n\
      prevY = y; \r\n\
    } \r\n\
    addLine(x, y, firstX, firstY); \r\n\
  } \r\n\
  isDown = false; \r\n\
} \r\n\
 \r\n\
function handleMouseOut(e) \r\n\
{ \r\n\
  e.preventDefault(); \r\n\
  e.stopPropagation(); \r\n\
 \r\n\
  isDown = false; \r\n\
} \r\n\
 \r\n\
function addLine(x1, y1, x2, y2, redraw = true) \r\n\
{ \r\n\
  if (groupStart) \r\n\
  { \r\n\
    groupStart = false; \r\n\
    storedGroups.push(storedLines.length); \r\n\
  } \r\n\
 \r\n\
  storedLines.push({ \r\n\
    x1: x1, \r\n\
    y1: y1, \r\n\
    x2: x2, \r\n\
    y2: y2 \r\n\
  }); \r\n\
 \r\n\
  if (redraw) \r\n\
    redrawStoredLines(); \r\n\
  printButton.disabled = false; \r\n\
} \r\n\
 \r\n\
function redrawStoredLines() \r\n\
{ \r\n\
 \r\n\
  ctx.clearRect(0, 0, canvas.width, canvas.height); \r\n\
 \r\n\
  if (storedLines.length == 0) \r\n\
    return; \r\n\
 \r\n\
  for (var i = 0; i < storedLines.length; i++) \r\n\
  { \r\n\
    ctx.beginPath(); \r\n\
    ctx.moveTo(storedLines[i].x1, storedLines[i].y1); \r\n\
    ctx.lineTo(storedLines[i].x2, storedLines[i].y2); \r\n\
    ctx.stroke(); \r\n\
  } \r\n\
} \r\n\
 \r\n\
function GeneratePrintPoints() \r\n\
{ \r\n\
  let drawPoints = []; \r\n\
  let minX = 1000; \r\n\
  let groupIndex = 0; \r\n\
  let groupStart = storedGroups[groupIndex]; \r\n\
 \r\n\
  for (var i = 0; i < storedLines.length; i++) \r\n\
  { \r\n\
    let line = storedLines[i]; \r\n\
 \r\n\
    if (i == groupStart) \r\n\
    { \r\n\
      let adjustedY = 100 - line.y1; \r\n\
      drawPoints.push({ x: line.x1, y: adjustedY, draw: false }); \r\n\
      groupStart = storedGroups[++groupIndex]; \r\n\
    } \r\n\
    let adjustedY = 100 - line.y2; \r\n\
    drawPoints.push({ x: line.x2, y: adjustedY, draw: true }); \r\n\
 \r\n\
    if (line.x1 < minX) \r\n\
      minX = line.x1; \r\n\
    if (line.x2 < minX) \r\n\
      minX = line.x2; \r\n\
  } \r\n\
  for (var i = 0; i < drawPoints.length; i++) \r\n\
    drawPoints[i].x -= minX; \r\n\
 \r\n\
  return drawPoints; \r\n\
} \r\n\
 \r\n\
function sendImage() \r\n\
{ \r\n\
  let points = GeneratePrintPoints(); \r\n\
  let request = new XMLHttpRequest(); \r\n\
  request.open('POST', '/image', false); \r\n\
  request.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded'); \r\n\
  request.onreadystatechange = function () \r\n\
  { \r\n\
    if (request.readyState === XMLHttpRequest.DONE) \r\n\
    { \r\n\
      statusText.innerHTML = request.responseText; \r\n\
      getStatus(); \r\n\
    } \r\n\
  } \r\n\
  request.send(JSON.stringify(points)); \r\n\
} \r\n\
 \r\n\
function getStatus() \r\n\
{ \r\n\
  let points = GeneratePrintPoints(); \r\n\
  let request = new XMLHttpRequest(); \r\n\
  request.open('POST', '/status', false); \r\n\
  request.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded'); \r\n\
  request.onreadystatechange = function () \r\n\
  { \r\n\
    if (request.readyState === XMLHttpRequest.DONE) \r\n\
    { \r\n\
      statusText.innerHTML = request.responseText; \r\n\
      if (request.responseText == 'Waiting for image') \r\n\
        printButton.disabled = false; \r\n\
      else \r\n\
      { \r\n\
        setTimeout(() => \r\n\
        { \r\n\
          getStatus(); \r\n\
        }, 1000); \r\n\
      } \r\n\
    } \r\n\
  } \r\n\
  request.send(JSON.stringify(points)); \r\n\
}";
