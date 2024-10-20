import socket

s = socket.socket()

try:
    s.connect(("127.0.0.1", 9190))
    print("连接成功！", end="\n")

    while True:
        message = input("输入:\n")
        if message.lower() == "exit":
            print("退出客户端。")
            break

        if message == "":
            print("无效输入！\n")
        else:
            try:
                byte_len = len(message)
                total_data = byte_len.to_bytes(4, byteorder="big") + message.encode("utf-8")
                s.sendall(total_data)
                print("发送数据包总长度：" + str(len(total_data)) + " bytes")
                print(f"数据长度: {byte_len}")
                print("内容: " + "".join(chr(byte) for byte in total_data[4:]) + "\n")

                # 接收响应
                data = s.recv(1024)
                if data:
                    print("接收数据包总长度：" + str(len(data)) + " bytes")
                    byte_len = int.from_bytes(data[:4], byteorder="big")
                    print(f"数据长度: {byte_len}")
                    print("内容: " + "".join(chr(byte) for byte in data[4:]) + "\n")
                else:
                    print("服务器关闭连接。")
                    break
            except Exception as e:
                print(f"发送或接收数据时发生错误: {e}")
                break

finally:
    s.close()
