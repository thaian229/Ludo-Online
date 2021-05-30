from ctypes import *

so_file = "./network/Client/client.so"
client = CDLL(so_file)


def connect_to_server():
    rs = client.connect_to_server()
    if rs > 0:
        print("Connected: ", rs)
        return True
    else:
        print("Connect failed.")
        return False


def send_create_room():
    print("Send Create Room.")
    client.send_create_room()


def send_quick_join():
    print("Send Quick Join A Room.")
    client.send_quick_join()


def send_join_a_room(room_id):
    print("Send join room id: ", room_id)
    client.send_join_a_room(room_id)


def send_ready():
    client.send_ready()


def send_move(turn, user, dice):
    client.send_move(turn, user, dice)


def send_quit_game():
    client.send_quit_game()
    client.reset_game_info()
    client.close_connection()


def close_connection():
    client.close_connection()


def get_room_status():
    room_id = client.get_room_id()
    players = client.get_rs_players()
    ready = client.get_rs_ready()
    is_started = client.get_game_started()
    if is_started:
        print(get_game_init())
    return room_id, players, ready, is_started


def get_game_init():
    n = client.get_gif_player_count()
    c = client.get_gif_color()
    return n, c


def check_move_ready():
    rs = client.get_move_ready()
    return rs == 1


def check_quit_ready():
    rs = client.get_quit_event_ready()
    return rs == 1


def check_res_failed():
    rs = client.get_flag_res_failed()
    return rs == 1


def handle_res_failed():
    client.set_flag_res_failed(0)


def get_quit_info():
    quitter = client.get_quiter()
    client.set_quit_event_ready(0)
    return quitter


def get_move_info():
    dice = client.get_dice_value()
    piece_no = client.get_piece_no()
    turn = client.get_turn()
    client.set_move_ready(0)
    return dice, piece_no, turn
