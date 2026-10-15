
from http.server import BaseHTTPRequestHandler, HTTPServer  #Nhap lop xu ly yeu cau HTTP/may chu HTTP
from datetime import datetime, timezone     #moc thoi gian  
import json     
import math

# CAU HINH HTTP SERVER

HOST = "0.0.0.0"    #dia chi dung tam de kiem tra
PORT = 5000

MAX_BODY_SIZE = 10000
MIN_LUX = 0
MAX_LUX = 100000


class LightIoTHandler(BaseHTTPRequestHandler):      #tao lop nay dua tren baseHTTPRequestHandler

  
    # Endpoint GET 
    # Dung de kiem tra server

    def do_GET(self):       #tao ham xu li GET tham chieu den handlẻr dang xu ly request
        if self.path == "/":        #kiem tra duong dan truy cap cua client
            self.send_json(200, {   #tra ve thanh cong
                "status": "ok",
                "service": "Light IoT HTTP Server",
                "version": "1.0"
            })

        elif self.path == "/health":    #kiem tra suc khoe server   
            self.send_json(200, {       #co hoat dong
                "status": "healthy"
            })

        else:                           #khong phai 2 duong dan tren
            self.send_json(404, {       #tra ve loi
                "error": "Endpoint not found"
            })

    # Endpoint POST /api/readings
    # Dung de nhan du lieu cam bien

    def do_POST(self):      #tao ham xu li POST nhan du lieu cam bien
        if self.path != "/api/readings":    #kiem tra duong dan, neu sai tra loi 404 ngay
            self.send_json(404, {
                "error": "Endpoint not found"
            })
            return

        # Kiem tra Content-Type
        content_type = self.headers.get("Content-Type", "") #kiem tra dinh dang du lieu client gui di
        media_type = content_type.split(";", 1)[0].strip().lower() 
        if media_type != "application/json":    #khong phai json tra loi 415
            self.send_json(415, {
                "error": "Content-Type must be application/json"
            })
            return

        # Kiem tra kich thuoc du lieu
        try:
            length = int(self.headers.get("Content-Length", "0"))
        except ValueError:                                         #tra loi 400 neu khong phai so nguyen
            self.send_json(400, {
                "error": "Invalid Content-Length"
            })
            return

        if length <= 0:                                             #tra loi neu body rong
            self.send_json(400, {
                "error": "Request body is empty"
            })
            return

        if length > MAX_BODY_SIZE:                                  #tra loi neu body qua lon
            self.send_json(413, {
                "error": "Request body too large"
            })
            return
        