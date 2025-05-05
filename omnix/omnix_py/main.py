from udp_comm import start_udp_listener
from visualizer import draw_pygame
from ui import start_tk
import threading

if __name__ == "__main__":
    threading.Thread(target=start_tk, daemon=True).start()
    threading.Thread(target=start_udp_listener, daemon=True).start()
    draw_pygame()
