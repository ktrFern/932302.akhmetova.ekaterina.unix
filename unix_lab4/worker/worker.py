import redis
import json
import time

redis_client = redis.Redis(host='redis', port=6379, db=0)

print("Email processor started. Waiting for email requests...")

while True:
    queue_item = redis_client.blpop('email_queue', timeout=30)
    
    if queue_item:
        request_data = json.loads(queue_item[1])
        request_id = request_data['request_id']
        email = request_data['email']
        subject = request_data['subject']
        body = request_data['body']
        
        print(f"Processing email request {request_id} for {email}...")
        print(f"Subject: {subject}")
        print(f"Body preview: {body[:50]}...")
        
        time.sleep(2)
        
        redis_client.setex(
            f'email_result:{request_id}',
            300,
            json.dumps({
                'email': email,
                'status': 'sent',
                'subject': subject,
                'processed_at': time.time(),
                'message': 'Email was successfully sent'
            })
        )
        
        print(f"Request {request_id}: email processing completed.")
    time.sleep(0.1)
