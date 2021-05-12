import pygame

from menu import *
from network import *
import random


class Piece:
    radius = 10

    def __init__(self, game, color, pos, no):
        self.game = game
        self.color = color
        self.pos = pos
        self.no = no

    def update(self, position, path):
        pygame.draw.circle(self.game.display, self.game.BLACK, path[position], self.radius + 5)
        pygame.draw.circle(self.game.display, self.color, path[position], self.radius)
        for circle in range(self.no):
            pygame.draw.circle(self.game.display, self.game.BLACK, path[position], self.radius / 10 + circle * 2)
        if path[position - 1] in self.game.exception:
            for index in range(len(self.game.exception)):
                if path[position - 1] == self.game.exception[index]:
                    if index < 6:
                        pygame.draw.circle(self.game.display, self.game.RED, path[position - 1], self.radius + 5)
                    elif index < 12:
                        pygame.draw.circle(self.game.display, self.game.YELLOW, path[position - 1], self.radius + 5)
                    elif index < 18:
                        pygame.draw.circle(self.game.display, self.game.BLUE, path[position - 1], self.radius + 5)
                    else:
                        pygame.draw.circle(self.game.display, self.game.GREEN, path[position - 1], self.radius + 5)
                    break
        else:
            pygame.draw.circle(self.game.display, self.game.WHITE, path[position - 1], self.radius + 5)
        self.game.window.blit(self.game.display, (0, 0))
        pygame.display.update()

    def draw(self, position):
        pygame.draw.circle(self.game.display, self.game.BLACK, position, self.radius + 5)
        pygame.draw.circle(self.game.display, self.color, position, self.radius)
        for circle in range(self.no):
            pygame.draw.circle(self.game.display, self.game.BLACK, position, self.radius / 10 + circle * 2)

    def __str__(self):
        return str(self.color) + str(self.no)


class Game:
    def __init__(self):
        pygame.init()
        pygame.display.set_caption('Ludo Game')
        self.running, self.playing_offline, self.playing_online = True, False, False
        self.UP_KEY, self.DOWN_KEY, self.START_KEY, self.BACK_KEY = False, False, False, False
        self.DISPLAY_W, self.DISPLAY_H = 800, 600
        self.display = pygame.Surface((self.DISPLAY_W, self.DISPLAY_H))
        self.window = pygame.display.set_mode((self.DISPLAY_W, self.DISPLAY_H))
        self.font_name = '8-BIT WONDER.TTF'
        self.main_menu = MainMenu(self)
        self.credits = CreditsMenu(self)
        self.online_menu = OnlineModeMenu(self)
        self.curr_menu = self.main_menu

        # all main game loop attributes
        self.n = 4  # number of pieces for each player
        self.players = 4  # number of players in match
        self.turn = 0  # current turn
        self.my_turn = 0  # player's turn (online mode)
        self.quitters = []
        self.starting_one = []
        for i in range(4):
            self.starting_one.append([0 for _ in range(self.n)])
        self.game_fps = 30
        self.clock = pygame.time.Clock()
        self.game_percent = [0, 0, 0, 0]
        self.dice = 0  # dice value
        self.winner = 0  # who the winner
        self.chance = 1
        # for drawing
        self.BLACK, self.WHITE = (0, 0, 0), (255, 255, 255)
        self.GREEN, self.BLUE = (0, 255, 0), (0, 0, 128)
        self.RED, self.YELLOW = (255, 0, 0), (255, 255, 0)
        self.big_rect_size = 240
        self.hole_radius = 30
        self.dice_img_size = 80
        self.game_over = False
        # path of piece
        self.path_x = [60]
        self.start_path_x = 60
        self.path_y = [260]
        self.start_path_y = 260
        self.generate_path()
        self.path_red = [(x1, y1) for x1, y1 in zip(self.path_x, self.path_y)]
        self.path_yellow = self.path_red[13:] + self.path_red[:13] + [(300, 60), (300, 100), (300, 140), (300, 180),
                                                                      (300, 220), (300, 260), (1000, 1000)]
        self.path_blue = self.path_red[26:] + self.path_red[:26] + [(540, 300), (500, 300), (460, 300), (420, 300),
                                                                    (380, 300), (340, 300), (1000, 1000)]
        self.path_green = self.path_red[39:] + self.path_red[:39] + [(300, 540), (300, 500), (300, 460), (300, 420),
                                                                     (300, 380), (300, 340), (1000, 1000)]
        self.path_red = self.path_red + [(60, 300), (100, 300), (140, 300), (180, 300), (220, 300), (260, 300),
                                         (1000, 1000)]
        del self.path_red[51], self.path_green[51], self.path_blue[51], self.path_yellow[51]
        self.exception = [self.path_red[0]] + self.path_red[51:] + [self.path_red[13]] + self.path_yellow[51:] + [
            self.path_red[26]] + self.path_blue[51:] + [self.path_red[39]] + self.path_green[51:]
        self.turn_color = {0: 'Red', 1: 'Yellow', 2: 'Blue', 3: 'Green'}
        self.turn_path = {0: self.path_red, 1: self.path_yellow, 2: self.path_blue, 3: self.path_green}
        self.safe_cells = [(self.turn_path[ind])[0] for ind in range(4)]
        self.board = pygame.image.load('assets/board.jpeg')
        self.position = []
        for _ in range(4):
            self.position.append([0 for _ in range(self.n)])
        self.user = -1

        # pieces
        self.red_pieces = self.pieces_init(self.RED, [self.big_rect_size / 4, self.big_rect_size / 4])
        self.green_pieces = self.pieces_init(self.GREEN,
                                             [self.big_rect_size / 4, self.DISPLAY_H - 3 * self.big_rect_size / 4])
        self.yellow_pieces = self.pieces_init(self.YELLOW, [self.DISPLAY_W - 200 - 3 * self.big_rect_size / 4,
                                                            self.big_rect_size / 4])
        self.blue_pieces = self.pieces_init(self.BLUE, [self.DISPLAY_W - 200 - 3 * self.big_rect_size / 4,
                                                        self.DISPLAY_H - 3 * self.big_rect_size / 4])

    def game_loop_offline(self):
        if self.playing_offline:
            self.turn = 0
            self.starting_one = []
            for i in range(4):
                self.starting_one.append([0 for _ in range(self.n)])
            self.game_fps = 30
            self.clock = pygame.time.Clock()
            self.game_percent = [0, 0, 0, 0]
            self.dice = random.randrange(1, 7)
            self.winner = 0
            self.game_over = False
            self.display.fill(self.BLACK)
            self.draw_board()

        while self.playing_offline:
            # self.check_events()

            self.chance = 1
            # Drawing

            self.draw_board()
            # self.draw_text('Thanks for Playing Ludo Offline', 20, self.DISPLAY_W / 2, self.DISPLAY_H / 2)
            try:
                self.display.blit(pygame.image.load("assets/dice" + str(self.dice) + ".jpeg"),
                                  ((self.DISPLAY_W - 200) / 2 - 40, self.DISPLAY_H / 2 - 40))
            except:
                pass

            while self.game_over:
                self.display.fill(self.BLACK)
                self.draw_text("Game Over", 30, self.DISPLAY_W / 2, self.DISPLAY_H / 2 - 100)
                self.draw_text("Winner is " + self.turn_color[self.winner], 30, self.DISPLAY_W / 2,
                               self.DISPLAY_H / 2)
                self.draw_text("Hit Enter To Quit", 30, self.DISPLAY_W / 2, self.DISPLAY_H / 2 + 50)

                self.window.blit(self.display, (0, 0))
                pygame.display.update()
                for event in pygame.event.get():
                    if event.type == pygame.KEYDOWN:
                        if event.key == pygame.K_RETURN:
                            self.playing_offline = False
                            self.game_over = False
                            self.curr_menu = self.main_menu
                            self.reset_game()
                        else:
                            break

            # Main Event Handler
            self.offline_events_handling()
            if self.BACK_KEY or self.START_KEY:
                self.playing_offline = False
                self.game_over = False
                self.curr_menu = self.main_menu
                self.reset_game()

            # Update Frame
            self.window.blit(self.display, (0, 0))
            pygame.display.update()
            self.clock.tick(self.game_fps)
            self.reset_keys()

    def game_loop_online(self):
        if self.playing_online:
            close_connection()
            if not connect_to_server():
                print("Connected.")
                self.curr_menu = self.online_menu
                self.curr_menu.run_display = True
                self.playing_online = False
                self.reset_keys()

            room_players = 1
            room_ready = 0
            room_id = 0

            # based on online_menu.selection, do corresponding preparation before main loop
            if self.online_menu.selection == 'Create':
                # send message to create new room
                send_create_room()
            elif self.online_menu.selection == 'Quick':
                # send message to join any room
                send_quick_join()
            elif self.online_menu.selection == 'Id':
                # display form for user to enter id
                done_input = False
                id_to_join = 0
                input_box1 = InputBox(self, self.DISPLAY_W / 2 - 100, self.DISPLAY_H / 2 - 16, 200, 44)
                input_boxes = [input_box1]
                while not done_input:
                    for event in pygame.event.get():
                        for box in input_boxes:
                            box.handle_event(event)
                        if event.type == pygame.KEYDOWN:
                            if event.key == pygame.K_RETURN:
                                try:
                                    id_to_join = int(input_box1.text)
                                except:
                                    pass
                                done_input = True

                    for box in input_boxes:
                        box.update()

                    self.display.fill(self.BLACK)
                    self.draw_text('Enter Room ID To Join', 30, self.DISPLAY_W / 2, self.DISPLAY_H / 2 - 100)
                    self.draw_text('Hit Enter to Join', 30, self.DISPLAY_W / 2, self.DISPLAY_H / 2 + 100)
                    for box in input_boxes:
                        box.draw(self.display)

                    self.window.blit(self.display, (0, 0))
                    pygame.display.update()

                # send message to join room with inputted room id
                send_join_a_room(id_to_join)
                pass

            game_started = False
            is_ready = False
            # while not game started, looping to render Room Menu and handler ready and start message
            while not game_started:
                # if 1 < room_players == room_ready > 1:
                #     self.players = room_players
                #     game_started = True

                self.check_events()
                if self.START_KEY:
                    if not is_ready:
                        # send ready message
                        send_ready()
                        is_ready = True

                # network handling
                room_id, room_players, room_ready, game_started = get_room_status()

                # render Room Menu
                self.display.fill(self.BLACK)
                self.draw_text('ROOM ID ' + str(room_id), 30, self.DISPLAY_W / 2, self.DISPLAY_H / 2 - 100)
                self.draw_text('Players ' + str(room_players) + ' of 4', 30, self.DISPLAY_W / 2,
                               self.DISPLAY_H / 2 + 0)
                self.draw_text('Ready ' + str(room_ready) + ' of ' + str(room_players), 30, self.DISPLAY_W / 2,
                               self.DISPLAY_H / 2 + 50)
                if is_ready:
                    self.draw_text('YOU ARE READY', 30, self.DISPLAY_W / 2, self.DISPLAY_H / 2 + 100)
                else:
                    self.draw_text('HIT ENTER TO READY', 30, self.DISPLAY_W / 2, self.DISPLAY_H / 2 + 100)
                self.window.blit(self.display, (0, 0))
                pygame.display.update()

                if self.BACK_KEY:
                    self.curr_menu = self.online_menu
                    self.playing_online = False
                    self.reset_keys()
                    send_quit_game()
                    break
                self.reset_keys()

            # receive init information from server before really enter main loop
            if game_started:
                print("Before init: ", self.players, self.my_turn)
                self.players, self.my_turn = get_game_init()
                print("After init: ", self.players, self.my_turn)

        # main loop
        while self.playing_online:
            # self.check_events()
            # if self.START_KEY:
            #     self.playing_online = False
            # self.display.fill(self.BLACK)
            # self.draw_board()
            # self.draw_text('Thanks for Playing Ludo Online', 20, self.DISPLAY_W / 2, self.DISPLAY_H / 2)
            # self.window.blit(self.display, (0, 0))
            # pygame.display.update()
            # self.reset_keys()
            # self.check_events()

            self.chance = 1
            self.turn = self.turn % self.players
            while self.turn in self.quitters:
                self.turn += 1
                self.turn = self.turn % self.players

            # Drawing
            self.draw_board()
            # self.draw_text('Thanks for Playing Ludo Offline', 20, self.DISPLAY_W / 2, self.DISPLAY_H / 2)
            try:
                self.display.blit(pygame.image.load("assets/dice" + str(self.dice) + ".jpeg"),
                                  ((self.DISPLAY_W - 200) / 2 - 40, self.DISPLAY_H / 2 - 40))
            except:
                pass

            while self.game_over:
                # GAME OVER
                self.display.fill(self.BLACK)
                self.draw_text("Game Over", 30, self.DISPLAY_W / 2, self.DISPLAY_H / 2 - 100)
                if self.winner == self.my_turn:
                    self.draw_text("You WON", 30, self.DISPLAY_W / 2,
                                   self.DISPLAY_H / 2)
                else:
                    self.draw_text("Winner is " + self.turn_color[self.winner], 30, self.DISPLAY_W / 2,
                                   self.DISPLAY_H / 2)
                self.draw_text("Hit Enter To Quit", 30, self.DISPLAY_W / 2, self.DISPLAY_H / 2 + 50)

                self.window.blit(self.display, (0, 0))
                pygame.display.update()
                for event in pygame.event.get():
                    if event.type == pygame.KEYDOWN:
                        if event.key == pygame.K_RETURN:
                            self.playing_online = False
                            self.game_over = False
                            self.curr_menu = self.main_menu
                            self.quit_online_game()
                        else:
                            break

            # Main Event Handler
            self.online_events_handling()

            # Handle quit game:
            if self.BACK_KEY or self.START_KEY:
                self.playing_online = False
                self.curr_menu = self.main_menu
                self.quit_online_game()

            # Update Frame
            self.window.blit(self.display, (0, 0))
            pygame.display.update()
            self.clock.tick(self.game_fps)
            self.reset_keys()

    def offline_events_handling(self):
        rect = pygame.Rect(self.DISPLAY_W - 200, 0, 200, 600)
        pygame.draw.rect(self.display, self.BLACK, rect)
        self.message('TURN ' + self.turn_color[self.turn % self.players], self.WHITE, self.DISPLAY_W - 100,
                     self.DISPLAY_H / 2 - 15)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running, self.playing_offline = False, False
                self.curr_menu.run_display = False
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_RETURN:
                    self.START_KEY = True
                if event.key == pygame.K_BACKSPACE:
                    self.BACK_KEY = True
                if event.key == pygame.K_DOWN:
                    self.DOWN_KEY = True
                if event.key == pygame.K_UP:
                    self.UP_KEY = True
            if event.type == pygame.QUIT:
                self.running = False
            if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                mx, my = pygame.mouse.get_pos()
                if ((((self.DISPLAY_W - 200 - self.dice_img_size) / 2) - 3) < mx < (
                        (((self.DISPLAY_W - 200 - self.dice_img_size) / 2) - 3) + self.dice_img_size)) and (
                        (((self.DISPLAY_H - self.dice_img_size) / 2) - 3) < my < (
                        (((self.DISPLAY_H - self.dice_img_size) / 2) - 3) + self.dice_img_size)):
                    self.dice = random.randrange(1, 7)
                    self.dice_roll(self.dice)
                    self.user = -1
                    # Motion for piece if it is pressed
                    while True:
                        # Manual movement
                        if self.starting_one[self.turn % self.players].count(1) > 1 or self.dice == 1:
                            for e in pygame.event.get():
                                if e.type == pygame.MOUSEBUTTONDOWN and e.button == 1:
                                    mouse_pos = pygame.mouse.get_pos()
                                    xxx = self.check_if_piece_is_pressed(mouse_pos, self.turn % self.players)
                                    if xxx == 0:
                                        self.user = 0
                                    elif xxx == 1:
                                        self.user = 1
                                    elif xxx == 2:
                                        self.user = 2
                                    elif xxx == 3:
                                        self.user = 3
                        # Auto movement
                        else:
                            try:
                                self.user = self.starting_one[self.turn % self.players].index(1)
                            except:
                                self.user = 0
                            pygame.time.delay(400)
                        if self.user >= 0:
                            break
                    if self.dice == 1 or self.dice == 6:
                        if self.dice == 1:
                            self.starting_one[self.turn % self.players][self.user] = 1
                        self.chance += 1
                    if self.starting_one[self.turn % self.players][self.user] >= 1:
                        checked = self.check_move(self.turn % self.players, self.dice)
                        if self.position[self.turn % 4][self.user] + self.dice < 57:
                            if self.check_kill(self.turn % self.players, self.dice, self.starting_one):
                                self.chance += 1
                        if checked == 2:
                            self.game_percent[self.turn % 4] += 100 / self.n
                            for any_percent in self.game_percent:
                                if any_percent >= 99:
                                    self.game_over = True
                                    self.winner = self.turn % self.players
                        # Validity of a move
                        if checked == 0:
                            self.message("Invalid Move", self.RED, self.DISPLAY_W - 200 + 15, self.DISPLAY_H / 2 - 40)
                            if self.dice != 1 and self.dice != 6:
                                self.turn += 1
                            break
                        else:
                            for _ in range(1, self.dice + 1):
                                self.display.blit(self.board, (0, 0))
                                self.display.blit(pygame.image.load("assets/dice" + str(self.dice) + ".jpeg"),
                                                  ((self.DISPLAY_W - 200) / 2 - 40, self.DISPLAY_H / 2 - 40))
                                for u in range(self.n):
                                    self.red_pieces[u].draw(
                                        ([self.red_pieces[u].pos] + self.path_red)[self.position[0][u]])
                                    self.yellow_pieces[u].draw(
                                        ([self.yellow_pieces[u].pos] + self.path_yellow)[self.position[1][u]])
                                    if self.players > 2:
                                        self.blue_pieces[u].draw(
                                            ([self.blue_pieces[u].pos] + self.path_blue)[self.position[2][u]])
                                    if self.players > 3:
                                        self.green_pieces[u].draw(
                                            ([self.green_pieces[u].pos] + self.path_green)[self.position[3][u]])
                                self.position[self.turn % self.players][self.user] += 1
                                if self.turn % self.players == 0:
                                    self.red_pieces[self.user].update(
                                        self.position[self.turn % self.players][self.user],
                                        [self.red_pieces[self.user].pos] + self.path_red)
                                elif self.turn % self.players == 1:
                                    self.yellow_pieces[self.user].update(
                                        self.position[self.turn % self.players][self.user],
                                        [self.yellow_pieces[
                                             self.user].pos] + self.path_yellow)
                                elif self.turn % self.players == 2:
                                    self.blue_pieces[self.user].update(
                                        self.position[self.turn % self.players][self.user],
                                        [self.blue_pieces[self.user].pos] + self.path_blue)
                                else:
                                    self.green_pieces[self.user].update(
                                        self.position[self.turn % self.players][self.user],
                                        [self.green_pieces[
                                             self.user].pos] + self.path_green)
                                pygame.time.delay(100)
                                self.window.blit(self.display, (0, 0))
                                pygame.display.update()
                    if self.chance == 1:
                        self.turn += 1

    def online_events_handling(self):
        rect = pygame.Rect(self.DISPLAY_W - 200, 0, 200, 600)
        pygame.draw.rect(self.display, self.BLACK, rect)
        if self.my_turn == self.turn:
            self.message('TURN ' + self.turn_color[self.turn % self.players], self.WHITE, self.DISPLAY_W - 100,
                         self.DISPLAY_H / 2 - 15)
            self.message('YOUR TURN', self.GREEN, self.DISPLAY_W - 100,
                         self.DISPLAY_H / 2 + 15)
        else:
            self.message('TURN ' + self.turn_color[self.turn % self.players], self.WHITE, self.DISPLAY_W - 100,
                         self.DISPLAY_H / 2 - 15)
            self.message('WAIT OTHER', self.RED, self.DISPLAY_W - 100,
                         self.DISPLAY_H / 2 + 15)

        for event in pygame.event.get():
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_RETURN:
                    self.START_KEY = True
                if event.key == pygame.K_BACKSPACE:
                    self.BACK_KEY = True
                if event.key == pygame.K_DOWN:
                    self.DOWN_KEY = True
                if event.key == pygame.K_UP:
                    self.UP_KEY = True
            if event.type == pygame.QUIT:
                self.running, self.playing_online = False, False
                self.curr_menu.run_display = False
                self.quit_online_game()
            if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1 and self.my_turn == self.turn:
                mx, my = pygame.mouse.get_pos()
                if ((((self.DISPLAY_W - 200 - self.dice_img_size) / 2) - 3) < mx < (
                        (((self.DISPLAY_W - 200 - self.dice_img_size) / 2) - 3) + self.dice_img_size)) and (
                        (((self.DISPLAY_H - self.dice_img_size) / 2) - 3) < my < (
                        (((self.DISPLAY_H - self.dice_img_size) / 2) - 3) + self.dice_img_size)):
                    self.dice = random.randrange(1, 7)
                    self.dice_roll(self.dice)
                    self.user = -1
                    # Motion for piece if it is pressed
                    while True:
                        # Manual movement
                        if self.starting_one[self.turn % self.players].count(1) > 1 or self.dice == 1:
                            for e in pygame.event.get():
                                if e.type == pygame.MOUSEBUTTONDOWN and e.button == 1:
                                    mouse_pos = pygame.mouse.get_pos()
                                    xxx = self.check_if_piece_is_pressed(mouse_pos, self.turn % self.players)
                                    if xxx == 0:
                                        self.user = 0
                                    elif xxx == 1:
                                        self.user = 1
                                    elif xxx == 2:
                                        self.user = 2
                                    elif xxx == 3:
                                        self.user = 3
                        # Auto movement
                        else:
                            try:
                                self.user = self.starting_one[self.turn % self.players].index(1)
                            except:
                                self.user = 0
                            pygame.time.delay(400)
                        if self.user >= 0:
                            break
                    # apply a move:
                    # send move to others
                    print("Send move: ", self.turn, self.user, self.dice)
                    send_move(self.turn, self.user, self.dice)
                    # self-apply
                    if self.dice == 1 or self.dice == 6:
                        if self.dice == 1:
                            self.starting_one[self.turn % self.players][self.user] = 1
                        self.chance += 1
                    if self.starting_one[self.turn % self.players][self.user] >= 1:
                        checked = self.check_move(self.turn % self.players, self.dice)
                        if self.position[self.turn % 4][self.user] + self.dice < 57:
                            if self.check_kill(self.turn % self.players, self.dice, self.starting_one):
                                self.chance += 1
                        if checked == 2:
                            self.game_percent[self.turn % 4] += 100 / self.n
                            for any_percent in self.game_percent:
                                if any_percent >= 99:
                                    self.game_over = True
                                    self.winner = self.turn % self.players
                        # Validity of a move
                        if checked == 0:
                            self.message("Invalid Move", self.RED, self.DISPLAY_W - 200 + 15, self.DISPLAY_H / 2 - 40)
                            if self.dice != 1 and self.dice != 6:
                                self.turn += 1
                            break
                        else:
                            for _ in range(1, self.dice + 1):
                                self.display.blit(self.board, (0, 0))
                                self.display.blit(pygame.image.load("assets/dice" + str(self.dice) + ".jpeg"),
                                                  ((self.DISPLAY_W - 200) / 2 - 40, self.DISPLAY_H / 2 - 40))
                                for u in range(self.n):
                                    self.red_pieces[u].draw(
                                        ([self.red_pieces[u].pos] + self.path_red)[self.position[0][u]])
                                    self.yellow_pieces[u].draw(
                                        ([self.yellow_pieces[u].pos] + self.path_yellow)[self.position[1][u]])
                                    if self.players > 2:
                                        self.blue_pieces[u].draw(
                                            ([self.blue_pieces[u].pos] + self.path_blue)[self.position[2][u]])
                                    if self.players > 3:
                                        self.green_pieces[u].draw(
                                            ([self.green_pieces[u].pos] + self.path_green)[self.position[3][u]])
                                self.position[self.turn % self.players][self.user] += 1
                                if self.turn % self.players == 0:
                                    self.red_pieces[self.user].update(
                                        self.position[self.turn % self.players][self.user],
                                        [self.red_pieces[self.user].pos] + self.path_red)
                                elif self.turn % self.players == 1:
                                    self.yellow_pieces[self.user].update(
                                        self.position[self.turn % self.players][self.user],
                                        [self.yellow_pieces[
                                             self.user].pos] + self.path_yellow)
                                elif self.turn % self.players == 2:
                                    self.blue_pieces[self.user].update(
                                        self.position[self.turn % self.players][self.user],
                                        [self.blue_pieces[self.user].pos] + self.path_blue)
                                else:
                                    self.green_pieces[self.user].update(
                                        self.position[self.turn % self.players][self.user],
                                        [self.green_pieces[
                                             self.user].pos] + self.path_green)
                                pygame.time.delay(100)
                                self.window.blit(self.display, (0, 0))
                                pygame.display.update()
                    if self.chance == 1:
                        self.turn += 1
                        self.turn = self.turn % self.players

        if check_quit_ready():
            quitter = get_quit_info()
            if quitter not in self.quitters:
                self.quitters.append(quitter)
                if len(self.quitters) >= self.players - 1:
                    self.winner = self.my_turn
                    self.game_over = True

        if self.my_turn != self.turn:  # other player's turn
            if check_move_ready():
                # take info of the move (dice, user)
                self.dice, self.user, self.turn = get_move_info()
                print("Received move: ", self.turn, self.user, self.dice)
                self.chance = 1

                # apply move
                if self.dice == 1 or self.dice == 6:
                    if self.dice == 1:
                        self.starting_one[self.turn % self.players][self.user] = 1
                    self.chance += 1
                if self.starting_one[self.turn % self.players][self.user] >= 1:
                    checked = self.check_move(self.turn % self.players, self.dice)
                    if self.position[self.turn % 4][self.user] + self.dice < 57:
                        if self.check_kill(self.turn % self.players, self.dice, self.starting_one):
                            self.chance += 1
                    if checked == 2:
                        self.game_percent[self.turn % 4] += 100 / self.n
                        for any_percent in self.game_percent:
                            if any_percent >= 99:
                                self.game_over = True
                                self.winner = self.turn % self.players
                    # Validity of a move
                    if checked == 0:
                        self.message("Invalid Move", self.RED, self.DISPLAY_W - 200 + 15, self.DISPLAY_H / 2 - 40)
                        if self.dice != 1 and self.dice != 6:
                            self.turn += 1
                        return
                    else:
                        for _ in range(1, self.dice + 1):
                            self.display.blit(self.board, (0, 0))
                            self.display.blit(pygame.image.load("assets/dice" + str(self.dice) + ".jpeg"),
                                              ((self.DISPLAY_W - 200) / 2 - 40, self.DISPLAY_H / 2 - 40))
                            for u in range(self.n):
                                self.red_pieces[u].draw(
                                    ([self.red_pieces[u].pos] + self.path_red)[self.position[0][u]])
                                self.yellow_pieces[u].draw(
                                    ([self.yellow_pieces[u].pos] + self.path_yellow)[self.position[1][u]])
                                if self.players > 2:
                                    self.blue_pieces[u].draw(
                                        ([self.blue_pieces[u].pos] + self.path_blue)[self.position[2][u]])
                                if self.players > 3:
                                    self.green_pieces[u].draw(
                                        ([self.green_pieces[u].pos] + self.path_green)[self.position[3][u]])
                            self.position[self.turn % self.players][self.user] += 1
                            if self.turn % self.players == 0:
                                self.red_pieces[self.user].update(
                                    self.position[self.turn % self.players][self.user],
                                    [self.red_pieces[self.user].pos] + self.path_red)
                            elif self.turn % self.players == 1:
                                self.yellow_pieces[self.user].update(
                                    self.position[self.turn % self.players][self.user],
                                    [self.yellow_pieces[
                                         self.user].pos] + self.path_yellow)
                            elif self.turn % self.players == 2:
                                self.blue_pieces[self.user].update(
                                    self.position[self.turn % self.players][self.user],
                                    [self.blue_pieces[self.user].pos] + self.path_blue)
                            else:
                                self.green_pieces[self.user].update(
                                    self.position[self.turn % self.players][self.user],
                                    [self.green_pieces[
                                         self.user].pos] + self.path_green)
                            pygame.time.delay(100)
                            self.window.blit(self.display, (0, 0))
                            pygame.display.update()
                if self.chance == 1:
                    self.turn += 1

    def check_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running, self.playing_offline = False, False
                self.curr_menu.run_display = False
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_RETURN:
                    self.START_KEY = True
                if event.key == pygame.K_BACKSPACE:
                    self.BACK_KEY = True
                if event.key == pygame.K_DOWN:
                    self.DOWN_KEY = True
                if event.key == pygame.K_UP:
                    self.UP_KEY = True

    def reset_keys(self):
        self.UP_KEY, self.DOWN_KEY, self.START_KEY, self.BACK_KEY = False, False, False, False

    def draw_text(self, text, size, x, y):
        font = pygame.font.Font(self.font_name, size)
        text_surface = font.render(text, True, self.WHITE)
        text_rect = text_surface.get_rect()
        text_rect.center = (x, y)
        self.display.blit(text_surface, text_rect)

    def generate_path(self):
        for i in range(2, 53):
            if (1 < i <= 5) or (19 < i <= 24) or (11 < i <= 13):
                self.start_path_x += self.big_rect_size / 6
            if (6 < i <= 11) or (39 < i <= 44) or (50 < i <= 53):
                self.start_path_y -= self.big_rect_size / 6
            if i == 6:
                self.start_path_x += self.big_rect_size / 6
                self.start_path_y -= self.big_rect_size / 6
            if (13 < i <= 18) or (32 < i <= 37) or (24 < i <= 26):
                self.start_path_y += self.big_rect_size / 6
            if i == 19:
                self.start_path_x += self.big_rect_size / 6
                self.start_path_y += self.big_rect_size / 6
            if (26 < i <= 31) or (37 < i <= 39) or (45 < i <= 50):
                self.start_path_x -= self.big_rect_size / 6
            if i == 32:
                self.start_path_x -= self.big_rect_size / 6
                self.start_path_y += self.big_rect_size / 6
            if i == 45:
                self.start_path_x -= self.big_rect_size / 6
                self.start_path_y -= self.big_rect_size / 6
            self.path_x.append(self.start_path_x)
            self.path_y.append(self.start_path_y)

    def pieces_init(self, color, coord):
        pieces_list = list()
        for i in range(self.n):
            clone = tuple(coord[:])
            piece = Piece(self, color, clone, i)
            if i == 0:
                coord[0] += self.big_rect_size / 2
            elif i == 1:
                coord[1] += self.big_rect_size / 2
            elif i == 2:
                coord[0] -= self.big_rect_size / 2
            pieces_list.append(piece)
        return pieces_list

    def message(self, msg, color, x_pos, y_pos):
        # font = pygame.font.Font(self.font_name, 15)
        # pygame.draw.rect(self.display, self.BLACK, [self.DISPLAY_W - 200 + 5, 0, 190, self.DISPLAY_H])
        # screen_text = font.render(msg, True, color)
        # self.display.blit(screen_text, [x_pos, y_pos])

        font = pygame.font.Font(self.font_name, 15)
        text_surface = font.render(msg, True, color)
        text_rect = text_surface.get_rect()
        text_rect.center = (x_pos, y_pos)
        self.display.blit(text_surface, text_rect)

    def dice_roll(self, num):
        for i in range(1, 7):
            self.draw_board()
            self.display.blit(pygame.image.load("assets/dice" + str(random.randrange(1, 7)) + ".jpeg"),
                              ((self.DISPLAY_W - 200) / 2, self.DISPLAY_H / 2))
            self.window.blit(self.display, (0, 0))
            pygame.display.update()
            pygame.time.delay(10)
            self.draw_board()
            self.display.blit(pygame.image.load("assets/dice_mid.jpeg"), (
                (self.DISPLAY_W - 200) / 2 - i * random.randrange(- 50, 50),
                self.DISPLAY_H / 2 - random.randrange(- 50, 50)))
            self.window.blit(self.display, (0, 0))
            pygame.display.update()
            pygame.time.delay(20)
        self.draw_board()
        self.display.blit(pygame.image.load("assets/dice" + str(num) + ".jpeg"),
                          ((self.DISPLAY_W - 200) / 2 - 40, self.DISPLAY_H / 2 - 40))
        self.window.blit(self.display, (0, 0))
        pygame.display.update()

    def draw_board(self):
        self.display.blit(self.board, (0, 0))
        for p in range(self.n):
            self.red_pieces[p].draw(([self.red_pieces[p].pos] + self.path_red)[self.position[0][p]])
            self.yellow_pieces[p].draw(([self.yellow_pieces[p].pos] + self.path_yellow)[self.position[1][p]])
            if self.players > 2:
                self.blue_pieces[p].draw(([self.blue_pieces[p].pos] + self.path_blue)[self.position[2][p]])
            if self.players > 3:
                self.green_pieces[p].draw(([self.green_pieces[p].pos] + self.path_green)[self.position[3][p]])

    def check_move(self, index, dice):
        if self.position[index][self.user] + dice < 57:
            return 1
        if self.position[index][self.user] + dice == 57:
            return 2
        else:
            return 0

    def check_kill(self, pos_index, dice, starting_one):
        for key in self.turn_path:
            if self.turn_path[key] != self.turn_path[pos_index]:
                for var in range(self.n):
                    if (self.turn_path[pos_index])[self.position[pos_index][self.user] + dice] == (self.turn_path[key])[
                        self.position[key][var]] \
                            and (self.turn_path[key])[self.position[key][var] - 1] not in self.safe_cells:
                        self.position[key][var] = 0
                        starting_one[key][var] = 0
                        return True
        return False

    def check_if_piece_is_pressed(self, mouse_pos, turn):
        turn_piece = {0: self.red_pieces, 1: self.yellow_pieces, 2: self.blue_pieces, 3: self.green_pieces}
        m1 = [0, 0, 0, 0]
        n1 = [0, 0, 0, 0]
        for var in range(self.n):
            if self.position[turn][var] == 0:
                m1[var], n1[var] = turn_piece[turn][var].pos
            else:
                m1[var], n1[var] = self.turn_path[turn][self.position[turn][var] - 1]
        mouse1, mouse2 = mouse_pos
        for var2 in range(self.n):
            if (m1[var2] - 20 < mouse1 < m1[var2] + 20) and (
                    n1[var2] - 20 < mouse2 < n1[var2] + 20):
                return var2

    def reset_game(self):
        # Menu
        self.curr_menu.run_display = False
        self.curr_menu = self.main_menu
        self.curr_menu.run_display = True

        # Game
        self.players = 4  # number of players in match
        self.turn = 0  # current turn
        self.my_turn = 0  # player's turn (online mode)
        self.starting_one = []
        for i in range(4):
            self.starting_one.append([0 for _ in range(self.n)])

        self.position = []
        for _ in range(4):
            self.position.append([0 for _ in range(self.n)])

        self.game_percent = [0, 0, 0, 0]
        self.winner = 0  # who the winner
        self.game_over = False

        self.quitters = []

    def quit_online_game(self):
        self.reset_game()
        send_quit_game()
