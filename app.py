from game import Game

g = Game()

while g.running:
    g.curr_menu.display_menu()
    g.game_loop_offline()
    g.game_loop_online()
