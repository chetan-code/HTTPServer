import socket
import threading
import time

# Configuration
TARGET_IP = "127.0.0.1"
TARGET_PORT = 8080
TOTAL_REQUESTS = 500  # Number of "users"

def send_request(user_id):
    try:
        # Create a socket
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.connect((TARGET_IP, TARGET_PORT))
        
        # Send a standard HTTP GET
        request = "GET /api/data HTTP/1.1\r\nHost: localhost\r\n\r\n"
        client.send(request.encode())
        
        # Wait for response
        response = client.recv(4096)
        
        # Check if we got the right data
        if "C++ Server" in response.decode():
            print(f"User {user_id}: Success")
        else:
            print(f"User {user_id}: Failed (Wrong Data)")
            
        client.close()
    except Exception as e:
        print(f"User {user_id}: Connection Failed({e})")

# Spawn threads
threads = []
start_time = time.time()

print(f"Starting stress test with {TOTAL_REQUESTS} users...")

for i in range(TOTAL_REQUESTS):
    t = threading.Thread(target=send_request, args=(i,))
    threads.append(t)
    t.start()

# Wait for all to finish
for t in threads:
    t.join()

end_time = time.time()
print(f"Test Completed in {end_time - start_time:.2f} seconds!")