#!/usr/bin/env python3
"""
Email notification script for Bitrecover
Supports startup and match notifications
"""

import smtplib
import json
import sys
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
from datetime import datetime
import os

def load_config():
    """Load configuration from config.json"""
    config_path = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'config', 'config.json')
    try:
        with open(config_path, 'r') as f:
            return json.load(f)
    except Exception as e:
        print(f"[-] Failed to load config: {e}")
        return None

def send_email(subject, body, config):
    """Send email using SMTP"""
    if not config or not config.get('email', {}).get('enabled', False):
        return
    
    email_config = config['email']
    
    try:
        msg = MIMEMultipart()
        msg['From'] = email_config['username']
        msg['To'] = ', '.join(email_config['recipients'])
        msg['Subject'] = subject
        
        msg.attach(MIMEText(body, 'plain'))
        
        server = smtplib.SMTP(email_config['smtp_server'], email_config['smtp_port'])
        server.starttls()
        server.login(email_config['username'], email_config['password'])
        
        text = msg.as_string()
        server.sendmail(email_config['username'], email_config['recipients'], text)
        server.quit()
        
        print(f"[+] Email sent successfully")
    except Exception as e:
        print(f"[-] Email failed: {e}")

def send_startup_notification(sys_info):
    """Send startup notification"""
    config = load_config()
    
    subject = "🚀 Bitrecover Started - Multi-GPU Pre-2012 Search"
    
    body = f"""Bitrecover Multi-GPU Bitcoin Key Finder Started!

⏰ Time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
🖥️  Host: {sys_info.get('hostname', 'unknown')}
💻 OS: {sys_info.get('os', 'unknown')}
🎯 Target: Pre-2012 Bitcoin wallets (Random 256-bit keys)
⚡ GPUs: {sys_info.get('gpu_count', 0)} device(s) detected

GPU Details:
"""
    
    for i, gpu in enumerate(sys_info.get('gpus', [])):
        body += f"  GPU {i}: {gpu.get('name', 'Unknown')} ({gpu.get('memory_mb', 0)} MB)\n"
    
    body += f"""
📁 Targets: {sys_info.get('targets_file', 'address.txt')}
💾 Output: {sys_info.get('output_file', 'Success.txt')}

Starting parallel search across all GPUs...
"""
    
    send_email(subject, body, config)

def send_match_notification(match_info):
    """Send match notification"""
    config = load_config()
    
    subject = "💰 Bitcoin Key Found! - Bitrecover Match"
    
    body = f"""🎉 MATCH FOUND! 🎉

⏰ Time: {match_info.get('timestamp', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))}
🎯 Address: {match_info.get('address', 'unknown')}
🔑 Private Key: {match_info.get('private_key', 'unknown')}
📝 WIF: {match_info.get('wif', 'unknown')}
⚡ GPU: {match_info.get('gpu_id', 'unknown')}


💰 Pre-2012 wallet found!
"""
    
    send_email(subject, body, config)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        # Startup notification
        sys_info = {
            'hostname': sys.argv[1] if len(sys.argv) > 1 else 'unknown',
            'os': sys.argv[2] if len(sys.argv) > 2 else 'unknown',
            'gpu_count': int(sys.argv[3]) if len(sys.argv) > 3 else 0,
            'gpus': []
        }
        
        # Parse GPU info
        idx = 4
        for i in range(sys_info['gpu_count']):
            if idx + 1 < len(sys.argv):
                gpu = {
                    'name': sys.argv[idx],
                    'memory_mb': int(sys.argv[idx + 1]) if idx + 1 < len(sys.argv) else 0
                }
                sys_info['gpus'].append(gpu)
                idx += 2
        
        send_startup_notification(sys_info)
    else:
        # Match notification
        if len(sys.argv) >= 4:
            match_info = {
                'address': sys.argv[1],
                'private_key': sys.argv[2],
                'wif': sys.argv[3],
                'gpu_id': sys.argv[4] if len(sys.argv) > 4 else 'unknown',
                'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            }
            send_match_notification(match_info)
        else:
            print("Usage: python send_email.py [address] [private_key] [wif] [gpu_id]")
            print("   or: python send_email.py [startup args...]")

