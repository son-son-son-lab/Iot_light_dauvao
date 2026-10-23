
from http.server import BaseHTTPRequestHandler, HTTPServer  #Nhap lop xu ly yeu cau HTTP/may chu HTTP
from datetime import datetime, timezone     #moc thoi gian  
import json     
import math

# CAU HINH HTTP SERVER

HOST = "192.168.1.7"    #dia chi dung tam de kiem tra
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


        body = self.rfile.read(length)
        data = json.loads(body.decode("utf-8"))
       
        if not isinstance(data, dict):
            self.send_json(400, {
                "error": "JSON must be an object"
            })
            return

        # Kiem tra device_id
        device_id = data.get("device_id")

        if not isinstance(device_id, str):
            self.send_json(400, {
                "error": "device_id must be a string"
            })
            return

        # Kiem tra lux
        lux = data.get("lux")

        if isinstance(lux, bool) or not isinstance(lux, (int, float)):      #tra loi neu lux khong phai so
            self.send_json(400, {
                "error": "lux must be a number"
            })
            return

        lux = float(lux)

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
            "received_at": datetime.now(timezone.utc).isoformat()
        }

        # Hien thi ban ghi tren Terminal
        print("\n NEW LIGHT READING ")
        print(json.dumps(record, indent=2, ensure_ascii=False))
        print("\n", flush=True)

        # Tra ve HTTP 201
        self.send_json(201, {
            "status": "created",
            "data": record
        })

    
    # Ham tra ve JSON
    def send_json(self, status_code, payload):
        response_body = json.dumps(
            payload,
            ensure_ascii=False
        ).encode("utf-8")

        self.send_response(status_code)
        self.send_header(
            "Content-Type",
            "application/json; charset=utf-8"
        )
        self.send_header(
            "Content-Length",
            str(len(response_body))
        )
        self.send_header("Cache-Control", "no-store")
        self.end_headers()

        self.wfile.write(response_body)


    # Log yeu cau HTTP
    def log_message(self, format_string, *args):
        print(
            "[HTTP] %s - %s" % (
                self.address_string(),
                format_string % args
            ),
            flush=True
        )




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

        
        