"""Writes the hands-on view's layout as text (docs/UI.md, "The hands-on view"; UI/LAYOUTS.md):
UI/layouts/ReforgerZ/RZ_HandsOnMenu.layout. The world fills the screen (the camera is on the thing);
the places to look are buttons the script moves over it each frame; the panel is on the right.
1920 x 1080 reference. Run it after changing the shape: py tools/ui_gen/hands_on_layout.py.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from site_builder_layouts import (G, image, text, frame, button, hlayout, write, BOLD, COND_BOLD,  # noqa: E402
                                  TEXT, DIM, AMBER, PANEL)

SPOTS = 6
LOGS = 10
ACTS = 3
W = 600
H = 900


def hands_on():
    g = G("RZ_HandsOnMenu")
    kids = []
    # The screen's flash: tinted by what was found, faded out by the script
    kids.append(image(g, "Flash", 0, 0, 1920, 1080, (1, 1, 1, 0.35), False))
    # A soft dark edge on the left so the labels read on any ground
    kids.append(image(g, "Shade", 0, 0, 1920, 1080, (0, 0, 0, 0.12)))
    # The places to look: a ring (its centre on the place) and a label beside it
    for i in range(SPOTS):
        kids.append(button(g, "Spot%d" % i, 0, 0, 340, 56, [
            image(g, "SpotRing%d" % i, 0, 8, 40, 40, AMBER),
            text(g, "SpotKey%d" % i, 0, 14, 40, 28, (0.13, 0.11, 0.08, 1), "1", 18, BOLD),
            image(g, "SpotLabelBg%d" % i, 48, 6, 292, 44, (0.055, 0.059, 0.067, 0.88)),
            text(g, "SpotLabel%d" % i, 58, 12, 276, 32, TEXT, "Look at the arms", 19),
            image(g, "SpotFill%d" % i, 48, 46, 0, 4, AMBER, False)]))
    # What a look found, at the place
    kids.append(frame(g, "Seen", 0, 0, 560, 76, [
        image(g, "SeenBg", 0, 0, 560, 76, (0.055, 0.059, 0.067, 0.94)),
        image(g, "SeenBar", 0, 0, 6, 76, AMBER),
        text(g, "SeenText", 20, 10, 528, 58, TEXT, "A bite under the sleeve", 24, BOLD, wrap=True)], visible=False))
    # The panel
    p = [image(g, "PanelBg", 0, 0, W, H, PANEL), image(g, "PanelAccent", 0, 0, W, 4, AMBER)]
    p.append(text(g, "Title", 32, 24, W - 120, 44, TEXT, "A woman, forties", 32, COND_BOLD))
    p.append(button(g, "CloseButton", W - 68, 20, 44, 44, [
        image(g, "CloseBg", 0, 0, 44, 44, (0.106, 0.122, 0.133, 1)),
        text(g, "CloseText", 14, 4, 30, 36, TEXT, "X", 24, BOLD)]))
    p.append(text(g, "Status", 32, 74, W - 64, 28, AMBER, "2 looks left", 20, BOLD))
    p.append(text(g, "Text", 32, 110, W - 64, 170, TEXT, "", 20, wrap=True))
    p.append(image(g, "FoundRule", 32, 290, W - 64, 1, (0.3, 0.3, 0.3, 1)))
    p.append(text(g, "FoundHead", 32, 300, W - 64, 26, DIM, "WHAT YOU FOUND", 18, BOLD))
    for i in range(LOGS):
        y = 334 + i * 40
        p.append(image(g, "LogDot%d" % i, 32, y + 10, 12, 12, DIM, False))
        p.append(text(g, "Log%d" % i, 56, y, W - 88, 38, TEXT, "", 18, visible=False, wrap=True))
    aw = (W - 64 - 2 * 10) // ACTS
    for i in range(ACTS):
        p.append(button(g, "Act%d" % i, 32 + i * (aw + 10), H - 100, aw, 72, [
            image(g, "ActBg%d" % i, 0, 0, aw, 72, (0.141, 0.157, 0.173, 1)),
            text(g, "ActKey%d" % i, 10, 6, 50, 22, DIM, "X", 16, BOLD),
            text(g, "ActText%d" % i, 10, 26, aw - 18, 44, TEXT, "Take them up", 19, BOLD, wrap=True)]))
    kids.append(frame(g, "Panel", -W - 32, 70, W, H, p, anchor="1 0 1 0"))
    kids.append(hlayout(g, "Prompts", 40, 1080 - 70, 1200, 44))
    kids.append(text(g, "PromptsText", 40, 1080 - 64, 1200, 32, DIM, "A LOOK    X TAKE    B CLOSE", 20))
    root = frame(g, "HandsOn", 0, 0, 0, 0, kids, anchor="0 0 1 1")
    write("RZ_HandsOnMenu.layout", root)


if __name__ == "__main__":
    hands_on()
