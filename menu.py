import pygame


class Menu:
    def __init__(self, game):
        self.game = game
        self.mid_w, self.mid_h = self.game.DISPLAY_W / 2, self.game.DISPLAY_H / 2
        self.run_display = True
        self.cursor_rect = pygame.Rect(0, 0, 20, 20)
        self.offset = - 200

    def draw_cursor(self):
        self.game.draw_text('*', 15, self.cursor_rect.x, self.cursor_rect.y)

    def blit_screen(self):
        self.game.window.blit(self.game.display, (0, 0))
        pygame.display.update()
        self.game.reset_keys()


class MainMenu(Menu):
    def __init__(self, game):
        Menu.__init__(self, game)
        self.state = "Offline"
        self.offline_x, self.offline_y = self.mid_w, self.mid_h + 0
        self.online_x, self.online_y = self.mid_w, self.mid_h + 50
        self.credits_x, self.credits_y = self.mid_w, self.mid_h + 100
        self.cursor_rect.midtop = (self.offline_x + self.offset, self.offline_y)

    def display_menu(self):
        self.run_display = True
        while self.run_display:
            self.game.check_events()
            self.check_input()
            self.game.display.fill(self.game.BLACK)
            self.game.draw_text('LUDO GAME', 30, self.game.DISPLAY_W / 2, self.game.DISPLAY_H / 2 - 100)
            self.game.draw_text("Offline Mode", 30, self.offline_x, self.offline_y)
            self.game.draw_text("Online Mode", 30, self.online_x, self.online_y)
            self.game.draw_text("Credits", 30, self.credits_x, self.credits_y)
            self.draw_cursor()
            self.blit_screen()

    def move_cursor(self):
        if self.game.DOWN_KEY:
            if self.state == 'Offline':
                self.cursor_rect.midtop = (self.online_x + self.offset, self.online_y)
                self.state = 'Online'
            elif self.state == 'Online':
                self.cursor_rect.midtop = (self.credits_x + self.offset, self.credits_y)
                self.state = 'Credits'
            elif self.state == 'Credits':
                self.cursor_rect.midtop = (self.offline_x + self.offset, self.offline_y)
                self.state = 'Offline'
        elif self.game.UP_KEY:
            if self.state == 'Offline':
                self.cursor_rect.midtop = (self.credits_x + self.offset, self.credits_y)
                self.state = 'Credits'
            elif self.state == 'Online':
                self.cursor_rect.midtop = (self.offline_x + self.offset, self.offline_y)
                self.state = 'Offline'
            elif self.state == 'Credits':
                self.cursor_rect.midtop = (self.online_x + self.offset, self.online_y)
                self.state = 'Online'

    def check_input(self):
        self.move_cursor()
        if self.game.START_KEY:
            if self.state == 'Offline':
                self.game.playing_offline = True
            elif self.state == 'Online':
                self.game.playing_online = True
            elif self.state == 'Credits':
                self.game.curr_menu = self.game.credits
            self.run_display = False


class CreditsMenu(Menu):
    def __init__(self, game):
        Menu.__init__(self, game)

    def display_menu(self):
        self.run_display = True
        while self.run_display:
            self.game.check_events()
            if self.game.START_KEY or self.game.BACK_KEY:
                self.game.curr_menu = self.game.main_menu
                self.run_display = False
            self.game.display.fill(self.game.BLACK)
            self.game.draw_text('Credits', 20, self.game.DISPLAY_W / 2, self.game.DISPLAY_H / 2 - 20)
            self.game.draw_text('Made by me', 15, self.game.DISPLAY_W / 2, self.game.DISPLAY_H / 2 + 10)
            self.blit_screen()
