from flask import Flask, request, jsonify
import redis
import uuid
import json

app = Flask(__name__)
redis_client = redis.Redis(host='redis', port=6379, db=0)

@app.route('/send', methods=['POST'])
def send_email():
    data = request.get_json()
    email = data.get('email')
    subject = data.get('subject')
    body = data.get('body')
    
    if not email or not subject or not body:
        return jsonify({'error': 'Missing email, subject or body'}), 400
    
    request_id = str(uuid.uuid4())
    
    email_data = {
        'request_id': request_id,
        'email': email,
        'subject': subject,
        'body': body
    }
    
    redis_client.rpush('email_queue', json.dumps(email_data))
    
    return jsonify({
        'request_id': request_id,
        'status': 'queued',
        'message': 'Email request accepted for processing'
    }), 202

@app.route('/status/<request_id>', methods=['GET'])
def get_status(request_id):
    result = redis_client.get(f'email_result:{request_id}')
    
    if result:
        status_data = json.loads(result)
        return jsonify({
            'request_id': request_id,
            'status': 'completed',
            'details': status_data
        })

    return jsonify({
        'request_id': request_id,
        'status': 'processing',
        'message': 'Email is being processed'
    }), 202
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
