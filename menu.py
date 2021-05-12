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
                self.game.curr_menu = self.game.online_menu
            elif self.state == 'Credits':
                self.game.curr_menu = self.game.credits
            self.run_display = False


class OnlineModeMenu(Menu):
    def __init__(self, game):
        Menu.__init__(self, game)
        self.state = 'Create'
        self.create_x, self.create_y = self.mid_w, self.mid_h + 0
        self.quick_join_x, self.quick_join_y = self.mid_w, self.mid_h + 50
        self.join_id_x, self.join_id_y = self.mid_w, self.mid_h + 100
        self.cursor_rect.midtop = (self.create_x + self.offset, self.create_y)
        self.selection = ''

    def display_menu(self):
        self.run_display = True
        while self.run_display:
            self.game.check_events()
            self.check_input()
            self.game.display.fill(self.game.BLACK)
            self.game.draw_text('LUDO ONLINE', 30, self.game.DISPLAY_W / 2, self.game.DISPLAY_H / 2 - 100)
            self.game.draw_text("Create A Room", 30, self.create_x, self.create_y)
            self.game.draw_text("Quick Join", 30, self.quick_join_x, self.quick_join_y)
            self.game.draw_text("Join By Id", 30, self.join_id_x, self.join_id_y)
            self.draw_cursor()
            self.blit_screen()

    def move_cursor(self):
        if self.game.DOWN_KEY:
            if self.state == 'Create':
                self.cursor_rect.midtop = (self.quick_join_x + self.offset, self.quick_join_y)
                self.state = 'Quick'
            elif self.state == 'Quick':
                self.cursor_rect.midtop = (self.join_id_x + self.offset, self.join_id_y)
                self.state = 'Join Id'
            elif self.state == 'Join Id':
                self.cursor_rect.midtop = (self.create_x + self.offset, self.create_y)
                self.state = 'Create'
        elif self.game.UP_KEY:
            if self.state == 'Create':
                self.cursor_rect.midtop = (self.join_id_x + self.offset, self.join_id_y)
                self.state = 'Join Id'
            elif self.state == 'Quick':
                self.cursor_rect.midtop = (self.create_x + self.offset, self.create_y)
                self.state = 'Create'
            elif self.state == 'Join Id':
                self.cursor_rect.midtop = (self.quick_join_x + self.offset, self.quick_join_y)
                self.state = 'Quick'

    def check_input(self):
        self.move_cursor()
        if self.game.START_KEY:
            if self.state == 'Create':
                self.selection = 'Create'
                self.game.playing_online = True
            elif self.state == 'Quick':
                self.selection = 'Quick'
                self.game.playing_online = True
            elif self.state == 'Join Id':
                self.selection = 'Id'
                self.game.playing_online = True
            self.run_display = False
        if self.game.BACK_KEY:
            self.game.curr_menu = self.game.main_menu
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
            self.game.draw_text('Credits', 30, self.game.DISPLAY_W / 2, self.game.DISPLAY_H / 2 - 100)
            self.game.draw_text('Nguyen Thai An', 30, self.game.DISPLAY_W / 2, self.game.DISPLAY_H / 2 + 0)
            self.game.draw_text('Vu Minh Hoang', 30, self.game.DISPLAY_W / 2, self.game.DISPLAY_H / 2 + 50)
            self.blit_screen()


class InputBox:

    def __init__(self, game, x, y, w, h, text='0'):
        self.game = game
        self.rect = pygame.Rect(x, y, w, h)
        self.color = self.game.RED
        self.text = text
        self.font = pygame.font.Font(self.game.font_name, 30)
        self.txt_surface = self.font.render(text, True, self.color)
        self.active = False

    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN:
            # If the user clicked on the input_box rect.
            if self.rect.collidepoint(event.pos):
                # Toggle the active variable.
                self.active = not self.active
            else:
                self.active = False
            # Change the current color of the input box.
            self.color = self.game.WHITE if self.active else self.game.RED
        if event.type == pygame.KEYDOWN:
            if self.active:
                if event.key == pygame.K_RETURN:
                    print(self.text)
                    # self.text = ''
                elif event.key == pygame.K_BACKSPACE:
                    self.text = self.text[:-1]
                else:
                    if event.unicode.isdigit():
                        self.text += event.unicode
                # Re-render the text.
                self.txt_surface = self.font.render(self.text, True, self.color)

    def update(self):
        # Resize the box if the text is too long.
        width = max(200, self.txt_surface.get_width() + 10)
        self.rect.w = width

    def draw(self, screen):
        # Blit the text.
        screen.blit(self.txt_surface, (self.rect.x + 5, self.rect.y + 5))
        # Blit the rect.
        pygame.draw.rect(screen, self.color, self.rect, 2)
