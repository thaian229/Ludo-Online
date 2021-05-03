from ctypes import *

if __name__ == '__main__':
    so_file = "./client.so"
    my_clib = CDLL(so_file)
    s = c_char_p(bytes("hello", encoding='utf-8'))
    conn = my_clib.connecting()
    print(conn)
    reply = my_clib.sendMsg(s)
    print(reply)
    reply = my_clib.sendMsg(s)
    reply = c_char_p(reply).value
    _reply = reply.decode('utf-8')
    print(_reply)

