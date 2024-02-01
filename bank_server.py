import socket
from _thread import *
import pymysql
from datetime import datetime

HOST = '0.0.0.0'  # 모든 인터페이스에서 접속 가능하도록 설정
PORT = 9876
detect_server_host = '192.168.0.3'
detect_server_port = 9999

# 소켓 생성
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# 소켓 바인딩
server_socket.bind((HOST, PORT))

# 소켓 리스닝
server_socket.listen()

def check_service(uid):
    """
    Retrieving service value from db with uid received as parameter

    Args:
        uid (byte arry): uid from card reader

    Returns:
        bool : 1 or 0
    """

    host = "localhost"
    user = "user"
    password = "user"
    database = "bankserver"
    db = pymysql.connect(host=host, user=user, password=password, database=database)
    cursor = db.cursor()
    
    sql = "SELECT service FROM user WHERE uid=%s"
    cursor.execute(sql, (uid,))

    result = cursor.fetchone()
    print(type(result))
    service = result[0]
    print(f"service = {service}")

    if(service == 1):
        print("service == 1")
        return 1
    else:
        print("service == 0")
        return 0

def get_user_num(uid):
    """
    Retrieving user_num value from db with uid received as parameter

    Args:
        uid (byte arry): uid from card reader

    Returns:
        int : user_num from db
    """
    host = "localhost"
    user = "user"
    password = "user"
    database = "bankserver"
    db = pymysql.connect(host=host, user=user, password=password, database=database)
    cursor = db.cursor()
    
    sql = "SELECT user_num FROM user WHERE uid=%s"
    cursor.execute(sql, (uid,))
    result = cursor.fetchone()
    user_num = result[0]
    print(f"user_num = {user_num}")
    db.close()

    return user_num


def payment_approval(money, user_num):
    """
    아직은 그냥 결제 승인한다고 가정하고 무조건 1 반환
    대충 나중에 추가 가능한 부분이라는 소리
    Returns:
        bool : 1 or 0
    """
    host = "localhost"
    user = "user"
    password = "user"
    database = "bankserver"
    db = pymysql.connect(host=host, user=user, password=password, database=database)
    cursor = db.cursor()
    
    sql = "SELECT money FROM user WHERE user_num=%s"
    cursor.execute(sql, (user_num,))
    result = cursor.fetchone()
    db_money = result[0]
    #print("user_num")
    #print(user_num)
    print("db_money")
    print(db_money)
    db_money = db_money -  money
    print(db_money)
    sql = "UPDATE user SET money = %s WHERE user_num = %s"
    cursor.execute(sql, (db_money, user_num,))
    db.commit()
    db.close()
    print("--------")

    filename = "C:/xampp/htdocs/Admin_Dashboard/moneyfile/" + str(user_num) + ".dat"
    current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    with open(filename, "a") as file:
        file.write("coke\n")
        file.write(str(money) + "\n")
        file.write(current_time + "\n")
    
    return 1



def main():
    while True:
        # 클라이언트 접속 대기
        client_socket, client_addr = server_socket.accept()
        print(f"Client connected from {client_addr[0]}:{client_addr[1]}")

        # 데이터 수신
        data = b""
        uid = b""
        hash = b""
        recv_data = b""
        
        '''
        잘 작동하면 나중에 지울것
        uid = client_socket.recv(4)
        hash = client_socket.recv(32) 
        '''
        
        data = client_socket.recv(36)
        print(f"data: {data}")
        print(data.hex())
        uid = data[:4]
        hash = data[4:]
        ok_data = b'ok'
        f_data = b'fa'
        # 수신된 데이터 출력
        print(f"uid: {uid}")
        print(uid.hex())
        print(f"Received data: {hash}")
        print(hash.hex())


        # recv money
        money_data = client_socket.recv(4)
        money_str = money_data.decode('ascii')
        money = int(money_str)
    


        if(check_service(uid) == 1):    # use detect service
            # send hash to detect server
            user_num = get_user_num(uid)
            my_client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            my_client_socket.connect((detect_server_host, detect_server_port))
            my_client_socket.sendall(uid)
            my_client_socket.sendall(hash)
            my_client_socket.sendall(user_num.to_bytes(4, byteorder='big'))
            recv_data = my_client_socket.recv(2)
            
            
            
            if(recv_data == f_data):    # detect replicated card
                print("detect replicated card")
                client_socket.sendall(f_data)
            else:   # nomal card
                recv_data = my_client_socket.recv(32)
                payment_approval(money, user_num)
                print(recv_data.hex())
                print("nomal card")
                # send new hash
                client_socket.sendall(recv_data)
            # 클라이언트 소켓 닫기    
            my_client_socket.close()
        else:   # Do not use detection service
            print("no de service")
            user_num = get_user_num(uid)
            payment_approval(money, user_num)
            client_socket.sendall(ok_data)
       
        # 클라이언트 소켓 닫기
        client_socket.close()
        
        

if __name__ == "__main__":
    main()