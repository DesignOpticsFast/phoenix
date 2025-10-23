#!/usr/bin/env python3
"""
Simple HTTP server to serve Phoenix UI screenshot
"""

import http.server
import socketserver
import os
import webbrowser
from pathlib import Path

def serve_screenshot():
    """Start HTTP server to serve the screenshot"""
    project_root = Path(__file__).parent.parent
    screenshots_dir = project_root / "screenshots"
    
    # Change to screenshots directory
    os.chdir(screenshots_dir)
    
    PORT = 8080
    
    class MyHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
        def end_headers(self):
            # Add CORS headers
            self.send_header('Access-Control-Allow-Origin', '*')
            self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
            self.send_header('Access-Control-Allow-Headers', 'Content-Type')
            super().end_headers()
    
    with socketserver.TCPServer(("", PORT), MyHTTPRequestHandler) as httpd:
        print(f"🌐 Screenshot server starting...")
        print(f"📸 Serving Phoenix UI screenshot at: http://localhost:{PORT}")
        print(f"📁 Screenshot directory: {screenshots_dir}")
        print(f"🖼️  View screenshot: http://localhost:{PORT}/view_screenshot.html")
        print(f"📄 Direct image: http://localhost:{PORT}/phoenix_ui_qt.png")
        print(f"\n⏹️  Press Ctrl+C to stop the server")
        
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print(f"\n🛑 Server stopped")

if __name__ == "__main__":
    serve_screenshot()
