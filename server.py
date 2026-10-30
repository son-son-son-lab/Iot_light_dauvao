
from http.server import BaseHTTPRequestHandler, HTTPServer  #Nhap lop xu ly yeu cau HTTP/may chu HTTP
from datetime import datetime, timezone     #moc thoi gian  
import json     
import math
from zoneinfo import ZoneInfo

# CAU HINH HTTP SERVER

HOST = "192.168.1.7"    
PORT = 8000

MAX_BODY_SIZE = 10000
MIN_LUX = 0
MAX_LUX = 100000


class LightIoTHandler(BaseHTTPRequestHandler):      #tao lop nay dua tren baseHTTPRequestHandler

    # Ham tra ve JSON
    def send_json(self, status_code, payload):       

        response_body = json.dumps(payload).encode("utf-8")     #luu payload cua du lieu tra ve vao ham(dict->json->bytes)

        self.send_response(status_code)                         #gui di status code
        self.send_header(                                       #gui di content-type header
            "content-type",
            "application/json; charset=utf-8"
        )
        self.send_header(                                       #gui di content length head
            "content-length",
            str(len(response_body))
        )
        self.end_headers()                                      #ket thuc header
        self.wfile.write(response_body)                         #in ra man hinh

  
    # Endpoint GET 
    # Dung de kiem tra server

    def do_GET(self):       #tao ham xu li GET tham chieu den handler dang xu ly request (self)
       
        if self.path == "/":        #kiem tra duong dan truy cap cua client
            self.send_json(200, {   #tra ve thanh cong
                "status": "ok",
                "service": "Light IoT HTTP Server",
            })

        else:                           #khong phai 2 duong dan tren
            self.send_json(404, {       #tra ve loi
                "error": "Endpoint not found"
            })

    # Endpoint POST /api/readings
    # Dung de nhan du lieu cam bien tu ESP32

    def do_POST(self):      #tao ham xu li POST nhan du lieu cam bien
        
        if self.path != "/api/readings":    #kiem tra duong dan, neu sai tra loi 404 ngay
            self.send_json(404, {
                "error": "Endpoint not found"
            })
            return

        # Kiem tra Content-Type
        #kiem tra dinh dang du lieu client gui di
        media_type = self.headers.get("content-type", "").split(";", 1)[0].strip().lower()     
        
        if media_type != "application/json":    #khong phai json tra loi 415
            self.send_json(415, {
                "error": "Content-Type must be application/json"
            })
            return

        # Kiem tra kich thuoc du lieu
        length = int(self.headers.get("content-length", "0"))

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

        #chuyen dau vao gui tu client tu dang bytes -> json -> python dict 
        #de python co the goi cac truong trong no
        #data se la mot bien dict co cac truong gom device_id va lux                                                            
        data = json.loads(self.rfile.read(length).decode("utf-8"))                                   


        if not isinstance(data, dict):                          #neu data khong co dang dict thi bao loi
            self.send_json(400, {
                "error": "JSON must be an object"
            })
            return

        # Kiem tra device_id                                    #goi truong device_id trong data
        device_id = data.get("device_id")

        if not isinstance(device_id, str):                      #bao loi neu truong nay khong co dang xau ki tu
            self.send_json(400, {
                "error": "device_id must be a string"
            })
            return

        # Kiem tra lux
        lux = data.get("lux")                                   #goi truong lux trong data

        if isinstance(lux, bool) or not isinstance(lux, (int, float)):      #tra loi neu lux khong phai so
            self.send_json(400, {
                "error": "lux must be a number"
            })
            return

        lux = float(lux)                                              #chuyen lux ve dang float

        if not math.isfinite(lux):                                  #tra loi neu lux vo cung
            self.send_json(400, {
                "error": "lux must be finite"   
            })
            return

        if not MIN_LUX <= lux <= MAX_LUX:                           #tra loi neu lux vuot gia tri gioi han
            self.send_json(400, {
                "error": "lux is out of allowed range"
            })
            return

        # Tao ban ghi voi thoi gian server nhan du lieu
        record = {
            "device_id": device_id,
            "lux": lux,
            "received_at": datetime.now(ZoneInfo("Asia/Ho_Chi_Minh")).isoformat()  
        }

        # Hien thi ban ghi tren Terminal
        print("\n NEW LIGHT READING ")
        print(json.dumps(record))
        print("\n")
        print(data)

        # Tra ve HTTP 201
        self.send_json(201, {
            "status": "created",
            "data": record
        })

    

if __name__ == "__main__":
    server = HTTPServer((HOST, PORT), LightIoTHandler)

    print("\n")
    print("Light IoT HTTP Server")
    print(f"Address: http://{HOST}:{PORT}")
    print(f"API:     http://{HOST}:{PORT}/api/readings")
    print("Server running...")


    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping server...")
    finally:
        server.server_close()
        print("Server stopped.")

        
        