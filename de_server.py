import socket
import hashlib
import random
import time
import pymysql
from datetime import datetime

HOST = '0.0.0.0'
PORT = 9999
# 소켓 생성
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((HOST, PORT))
server_socket.listen()

def compare_hash(user_num, uid, hash):
    """
    Load hash for user_num and uid values from db and compare it to hash received as a parameter

    Args:
        user_num (char arry): user_num in db
        uid (byte arry): uid in db
        hash (byte arry): hash in db

    Returns:
        bool : 1(same hash, nomal card) or 0(different hash, detect card)
    """
    
    # db 연결
    host = "localhost"
    user = "user"
    password = "user"
    database = "detectserver"
    db = pymysql.connect(host=host, user=user, password=password, database=database)
    cursor = db.cursor()

    # uid 와 user_num에 맞는 hash
    sql = "SELECT hash FROM detect WHERE uid=%s AND user_num=%s"
    cursor.execute(sql, (uid, user_num))

    result = cursor.fetchone()
    db.close()
    if(result[0] == hash):
        return 1
    else:
        return 0

def create_new_hash(uid):
    """
    Create hash with 16-bit random value, timestamp, and uid
    
    Args:
        uid (byte arry): Uid received by socket

    Returns:
        byte arry: new hash
    """
    random_bytes = bytearray(random.randrange(256) for _ in range(120))

    message = bytearray(str(time.time()).encode())
    message += random_bytes
    message += uid

    hash_object = hashlib.sha256(message).digest()
    print("new_hahs len")
    print(len(hash_object))


    return hash_object

def update_new_hash(uid, user_num, hash):
    """
    update new hash in db

    Args:
        uid (byte arry): uid in db    
        user_num (char arry): user_num in db
        hash (byte arry): hash in db
    """
    # db 연결
    host = "localhost"
    user = "user"
    password = "user"
    database = "detectserver"
    db = pymysql.connect(host=host, user=user, password=password, database=database)
    cursor = db.cursor()
    
    sql = "UPDATE detect SET hash = %s WHERE uid = %s AND user_num = %s"
    cursor.execute(sql, (hash, uid, user_num))
    db.commit()
    db.close()

    return

def update_log(uid, user_num, hash, check_status):
    """
    logging detect hash in db.
    check_status == 1 : detect

    Args:
        uid (byte arry): uid in db    
        user_num (char arry): user_num in db
        hash (byte arry): hash in db
        check_status (bool): (detect result) 1 or 0
    """
    print(type(check_status))
    # db 연결
    host = "localhost"
    user = "user"
    password = "user"
    database = "detectserver"
    db = pymysql.connect(host=host, user=user, password=password, database=database)
    #cursor = db.cursor()
    try:
        with db.cursor() as cursor:
            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            sql = "INSERT INTO log (uid, user_num, hash, check_status, timestamp) VALUES (%s, %s, %s, %s, %s)"
            cursor.execute(sql, (uid, user_num, hash, check_status, timestamp))
            db.commit()
            result = cursor.fetchone()
            print(result)
    except pymysql.Error as e:
        print(f"Error: {e}")
    
    db.close()

    return

def main():
    while True:
        print("start")
        # 클라이언트 접속 대기
        client_socket, client_addr = server_socket.accept()
        print(f"Client connected from {client_addr[0]}:{client_addr[1]}")

        # 데이터 수신
        uid = b""
        hash = b""
        uid = client_socket.recv(4)
        hash = client_socket.recv(32)
        recv_user_num = client_socket.recv(4)
        user_num = int.from_bytes(recv_user_num, byteorder='big')

        # 수신된 데이터 출력
        print(f"uid: {uid}")
        print(uid.hex())
        print(f"Received data: {hash}")
        print(f"user_num: {user_num}")
        print(hash.hex())

        ok_data = b'ok'
        f_data = b'fa'
        if(compare_hash(user_num,uid,hash)):     # normal card 
            new_hash = create_new_hash(uid)
            print("new hash")
            print(new_hash.hex())
            print(type(new_hash))
            update_new_hash(uid, user_num, new_hash)
            update_log(uid, user_num, hash, True)
            print("normal card")
            client_socket.sendall(ok_data)
            client_socket.sendall(new_hash)
        else:   # detect replicated card
            print("detect card")
            update_log(uid, user_num, hash, False)
            client_socket.sendall(f_data)

        # 클라이언트 소켓 닫기
        client_socket.close()

    

if __name__ == "__main__":
    main()
