"""Writes the site builder's two layouts as text (docs/UI.md, "The site builder"; UI/LAYOUTS.md):
UI/layouts/ReforgerZ/RZ_SiteBuilderMenu.layout (the list and detail screen) and RZ_SiteHud.layout (the
panel shown while a site is built). Absolute positions on a 1920 x 1080 reference. Run it after changing
the screen's shape: py tools/ui_gen/site_builder_layouts.py. Ids are seeded by widget name, so a rerun of
an unchanged screen is unchanged.
"""
import hashlib
import os

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
OUT = os.path.join(ROOT, "UI", "layouts", "ReforgerZ")

BOLD = '{CD2634D279AB011A}UI/Fonts/Roboto/Roboto_Bold.fnt'
REG = '{E2CBA6F76AAA42AF}UI/Fonts/Roboto/Roboto_Regular.fnt'
COND = '{3E7733BAC8C831F6}UI/Fonts/RobotoCondensed/RobotoCondensed_Regular.fnt'
COND_BOLD = '{EABA4FE9D014CCEF}UI/Fonts/RobotoCondensed/RobotoCondensed_Bold.fnt'

TEXT = (0.925, 0.914, 0.882, 1)
DIM = (0.604, 0.592, 0.561, 1)
AMBER = (0.91, 0.698, 0.227, 1)
PANEL = (0.031, 0.035, 0.039, 0.98)
PANE = (0.051, 0.057, 0.063, 1)
ROW = (0.106, 0.118, 0.129, 1)


class G:
    def __init__(self, layout):
        self.layout = layout

    def gid(self, key):
        return hashlib.sha1((self.layout + key).encode()).hexdigest()[:16].upper()


def lin(v):
    """A display (sRGB) value as the linear one layouts hold: the screen converts them back."""
    if v <= 0.04045:
        return v / 12.92
    return ((v + 0.055) / 1.055) ** 2.4


def col(c):
    """Colours here are written as they should look (sRGB, like a mockup's hex); layouts take linear."""
    return " ".join(("%g" % round(lin(v) if i < 3 else v, 4)) for i, v in enumerate(c))


def slot(g, name, x, y, w, h, anchor="0 0 0 0"):
    return ['Slot FrameWidgetSlot "{%s}" {' % g.gid(name + "slot"),
            " Anchor %s" % anchor,
            " PositionX %g" % x, " OffsetLeft %g" % x,
            " PositionY %g" % y, " OffsetTop %g" % y,
            " SizeX %g" % w, " OffsetRight %g" % -(x + w),
            " SizeY %g" % h, " OffsetBottom %g" % -(y + h),
            " Alignment 0 0", "}"]


def ind(lines, n=1):
    return [(" " * n) + ln for ln in lines]


def image(g, name, x, y, w, h, c, visible=True):
    out = ['ImageWidgetClass "{%s}" {' % g.gid(name), ' Name "%s"' % name]
    out += ind(slot(g, name, x, y, w, h))
    if not visible:
        out.append(' "Is Visible" 0')
    out += [" Color %s" % col(c), " Size 1 1", "}"]
    return out


def text(g, name, x, y, w, h, c, txt, size, font=REG, visible=True, wrap=False):
    out = ['TextWidgetClass "{%s}" {' % g.gid(name), ' Name "%s"' % name]
    out += ind(slot(g, name, x, y, w, h))
    if not visible:
        out.append(' "Is Visible" 0')
    out += [" Color %s" % col(c), ' Text "%s"' % txt, ' "Font Size" %d' % size, ' "Min Font Size" %d' % size]
    if wrap:
        out.append(" Wrap 1")
    out += [' FontProperties FontProperties "{%s}" {' % g.gid(name + "font"), '  Font "%s"' % font, " }", "}"]
    return out


def frame(g, name, x, y, w, h, children, anchor="0 0 0 0", visible=True):
    out = ['FrameWidgetClass "{%s}" {' % g.gid(name), ' Name "%s"' % name]
    out += ind(slot(g, name, x, y, w, h, anchor))
    if not visible:
        out.append(' "Is Visible" 0')
    out.append(" {")
    for c in children:
        out += ind(c, 2)
    out += [" }", "}"]
    return out


def button(g, name, x, y, w, h, children):
    """A clickable box: the button fills a frame whose children are placed absolutely inside it."""
    inner = frame(g, name + "Box", 0, 0, w, h, children)
    inner[2] = ' Slot ButtonWidgetSlot "{%s}" {' % g.gid(name + "bslot")
    # Replace the frame slot with a stretched button slot
    start = 2
    end = start
    while not inner[end].strip() == "}":
        end += 1
    inner[start:end + 1] = [' Slot ButtonWidgetSlot "{%s}" {' % g.gid(name + "bslot"), "  HorizontalAlign 3", "  VerticalAlign 3", " }"]
    out = ['ButtonWidgetClass "{%s}" {' % g.gid(name), ' Name "%s"' % name]
    out += ind(slot(g, name, x, y, w, h))
    out += [" style blank", " {"]
    out += ind(inner, 2)
    out += [" }", "}"]
    return out


def hlayout(g, name, x, y, w, h):
    out = ['HorizontalLayoutWidgetClass "{%s}" {' % g.gid(name), ' Name "%s"' % name]
    out += ind(slot(g, name, x, y, w, h))
    out.append("}")
    return out


def write(name, lines):
    path = os.path.join(OUT, name)
    open(path, "w", encoding="utf-8", newline="\n").write("\n".join(lines) + "\n")
    print("wrote", os.path.relpath(path, ROOT))


# ------------------------------------------------------------------------------------ the menu
ROWS = 8
SPOTS = 16


def menu():
    """The site builder's screen as cards: the faction switcher on top, the picked site's big card on
    the left with everything about it, the faction's other sites as small cards on the right."""
    g = G("RZ_SiteBuilderMenu")
    W, H = 1640, 940
    px = 48
    kids = [image(g, "PanelBg", 0, 0, W, H, PANEL), image(g, "PanelAccent", 0, 0, W, 4, AMBER)]
    kids.append(text(g, "Title", px, 26, 500, 44, TEXT, "SITE BUILDER", 34, COND_BOLD))
    kids.append(text(g, "Sub", px, 72, 600, 28, DIM, "Everon - 17 sites - 3 hand-built", 19))
    kids.append(button(g, "CloseButton", W - px - 44, 26, 44, 44, [
        image(g, "CloseBg", 0, 0, 44, 44, (0.141, 0.157, 0.173, 1)),
        text(g, "CloseText", 13, 6, 30, 32, AMBER, "X", 24, BOLD)]))
    cx = W // 2
    kids.append(text(g, "FactionLB", cx - 330, 40, 40, 30, DIM, "LB", 18, BOLD))
    kids.append(button(g, "FactionPrev", cx - 290, 30, 50, 50, [image(g, "FactionPrevBg", 0, 0, 50, 50, (0.141, 0.157, 0.173, 1)),
        text(g, "FactionPrevText", 17, 4, 24, 42, AMBER, "<", 30, BOLD)]))
    kids.append(text(g, "FactionName", cx - 230, 30, 460, 44, TEXT, "Bastion (4)", 34, COND_BOLD))
    kids.append(button(g, "FactionNext", cx + 240, 30, 50, 50, [image(g, "FactionNextBg", 0, 0, 50, 50, (0.141, 0.157, 0.173, 1)),
        text(g, "FactionNextText", 18, 4, 24, 42, AMBER, ">", 30, BOLD)]))
    kids.append(text(g, "FactionRB", cx + 300, 40, 40, 30, DIM, "RB", 18, BOLD))
    for i in range(5):
        kids.append(image(g, "FactionDot%d" % i, cx - 60 + i * 26, 86, 12, 12, (0.35, 0.35, 0.35, 1)))
    fx = W - px - 4 * 158
    for i, n in enumerate(["All", "Not built", "Hand-built", "Pinned"]):
        kids.append(button(g, "Filter%d" % i, fx + i * 158, 84, 150, 36, [
            image(g, "FilterBg%d" % i, 0, 0, 150, 36, (0.165, 0.176, 0.192, 1)),
            text(g, "FilterText%d" % i, 12, 5, 126, 28, (0.812, 0.8, 0.769, 1), n, 19)]))
    kids.append(text(g, "SortText", fx, 124, 4 * 158, 24, DIM, "LT RT filter", 16))
    dx, dy, dw, dh = px, 156, 660, 640
    d = [image(g, "DetailBg", 0, 0, dw, dh, (0.137, 0.153, 0.165, 1)), image(g, "DetailEdge", 0, 0, dw, 4, AMBER)]
    d.append(text(g, "DetailHead", 28, 22, dw - 56, 26, DIM, "TASK FORCE BASTION - STRONGHOLD", 19, BOLD))
    d.append(text(g, "DetailName", 28, 50, dw - 56, 92, TEXT, "Lamentin", 40, COND_BOLD, wrap=True))
    d.append(image(g, "DetailStateBg", 28, 146, 300, 32, (0.153, 0.314, 0.039, 1)))
    d.append(text(g, "DetailState", 38, 149, 290, 28, TEXT, "HAND-BUILT, IN USE", 20, BOLD))
    d.append(image(g, "DetailPinBg", 340, 146, 110, 32, (0.267, 0.267, 0.255, 1), False))
    d.append(text(g, "DetailPin", 352, 149, 96, 28, TEXT, "PINNED", 20, BOLD, False))
    ky = 192
    for i in range(4):
        d.append(text(g, "Key%d" % i, 28, ky, 160, 28, DIM, "Where", 20))
        d.append(text(g, "Val%d" % i, 190, ky, dw - 218, 28, TEXT, "12-59, 3.4 km W of you", 20))
        ky += 30
    d.append(text(g, "SpotsHead", 28, ky + 10, dw - 56, 26, DIM, "ROLE SPOTS - 7 OF 11", 19, BOLD))
    sy = ky + 40
    half = (dw - 56) // 2
    for i in range(SPOTS):
        x = 28 + (i % 2) * half
        y = sy + (i // 2) * 27
        d.append(image(g, "SpotIcon%d" % i, x, y + 3, 20, 20, (0.592, 0.769, 0.349, 1)))
        d.append(text(g, "Spot%d" % i, x + 28, y, half - 34, 26, (0.592, 0.769, 0.349, 1), "Quartermaster", 19))
    d.append(frame(g, "SpotPicker", 28, dh - 54, dw - 56, 40, [
        image(g, "SpotPickerBg", 0, 0, dw - 56, 40, (0.165, 0.176, 0.192, 1)),
        text(g, "SpotPickerLabel", 12, 7, 150, 28, DIM, "Next spot", 20),
        button(g, "SpotPrev", 170, 0, 44, 40, [text(g, "SpotPrevText", 12, 0, 30, 38, AMBER, "<", 30, BOLD)]),
        text(g, "SpotPickerType", 222, 6, dw - 56 - 290, 30, TEXT, "Wash spot", 22, BOLD),
        button(g, "SpotNext", dw - 56 - 60, 0, 44, 40, [text(g, "SpotNextText", 14, 0, 30, 38, AMBER, ">", 30, BOLD)])], visible=False))
    d.append(text(g, "DetailNote", 28, dh - 50, dw - 56, 44, AMBER, "", 18, wrap=True))
    kids.append(frame(g, "DetailPane", dx, dy, dw, dh, d))
    gx = px + dw + 24
    cw = (W - px - gx - 16) // 2
    ch = 148
    for i in range(ROWS):
        x = gx + (i % 2) * (cw + 16)
        y = dy + (i // 2) * (ch + 16)
        kids.append(button(g, "Row%d" % i, x, y, cw, ch, [
            image(g, "RowBg%d" % i, 0, 0, cw, ch, (0.106, 0.118, 0.129, 1)),
            image(g, "RowMark%d" % i, 0, 0, cw, 4, AMBER, False),
            text(g, "RowHead%d" % i, 18, 14, cw - 36, 22, DIM, "BASTION - OUTPOST", 16, BOLD),
            text(g, "RowName%d" % i, 18, 38, cw - 50, 64, TEXT, "Gravette", 26, COND, wrap=True),
            image(g, "RowDot%d" % i, cw - 30, 46, 12, 12, AMBER, False),
            image(g, "RowStateBg%d" % i, 18, ch - 40, 190, 28, (0.267, 0.267, 0.255, 1)),
            text(g, "RowState%d" % i, 26, ch - 38, 178, 26, TEXT, "AUTOMATED", 17, BOLD),
            text(g, "RowSpots%d" % i, cw - 110, ch - 38, 92, 26, DIM, "7/11", 20)]))
    kids.append(text(g, "ListMore", gx, dy + 4 * (ch + 16), 600, 26, DIM, "", 18))
    kids.append(text(g, "ListEmpty", gx, dy + 20, 700, 40, DIM, "No site here.", 22, visible=False))
    q = [image(g, "AskBg", 0, 0, 900, 260, (0.11, 0.122, 0.133, 1)), image(g, "AskAccent", 0, 0, 900, 4, AMBER),
         text(g, "AskTitle", 32, 28, 836, 40, TEXT, "Unsaved changes", 30, COND_BOLD),
         text(g, "AskText", 32, 80, 836, 150, TEXT, "", 22, wrap=True)]
    kids.append(frame(g, "Ask", (W - 900) // 2, 260, 900, 260, q, visible=False))
    kids.append(text(g, "Message", px, H - 136, W - 2 * px, 28, (0.592, 0.769, 0.349, 1), "", 20))
    kids.append(image(g, "BarRule", px, H - 104, W - 2 * px, 2, (0.2, 0.216, 0.231, 1)))
    kids.append(hlayout(g, "Prompts", px, H - 88, W - 2 * px, 44))
    kids.append(text(g, "PromptsText", px, H - 82, W - 2 * px, 32, DIM, "A GO THERE    X BUILD    Y USE    B CLOSE", 20))
    root = frame(g, "SiteBuilderMenu", 0, 0, 0, 0, [
        image(g, "Dim", 0, 0, 0, 0, (0.031, 0.035, 0.039, 0.65)),
        frame(g, "SitePanel", -W / 2, -H / 2, W, H, kids, anchor="0.5 0.5 0.5 0.5")], anchor="0 0 1 1")
    for i, ln in enumerate(root):
        if ln.strip() == 'Name "Dim"':
            root[i + 2] = root[i + 2].replace("Anchor 0 0 0 0", "Anchor 0 0 1 1")
            break
    write("RZ_SiteBuilderMenu.layout", root)


# ------------------------------------------------------------------------------------ the HUD
def hud():
    """The build bar (UI/RZ_SiteHud.c): a compact panel top right while a site is built, clicked with
    Game Master's cursor. Three pages of eight buttons (people, places, patrols), Undo, Save, Stop."""
    g = G("RZ_SiteHud")
    W = 460
    bw = (W - 40) // 2
    H = 330
    k = [image(g, "BarBg", 0, 0, W, H, (0.055, 0.059, 0.067, 0.94)), image(g, "BarAccent", 0, 0, 3, H, AMBER)]
    k.append(text(g, "BarName", 16, 10, W - 170, 28, TEXT, "Lamentin", 22, COND_BOLD))
    k.append(text(g, "BarProgress", W - 160, 14, 150, 24, DIM, "7/11 - unsaved", 17))
    k.append(button(g, "PagePrev", 16, 46, 40, 32, [image(g, "PagePrevBg", 0, 0, 40, 32, (0.141, 0.157, 0.173, 1)),
        text(g, "PagePrevText", 13, 1, 20, 30, TEXT, "<", 22, BOLD)]))
    k.append(text(g, "PageTitle", 70, 50, W - 140, 26, DIM, "People - 1/3", 18))
    k.append(button(g, "PageNext", W - 56, 46, 40, 32, [image(g, "PageNextBg", 0, 0, 40, 32, (0.141, 0.157, 0.173, 1)),
        text(g, "PageNextText", 14, 1, 20, 30, TEXT, ">", 22, BOLD)]))
    for i in range(8):
        x = 16 + (i % 2) * (bw + 8)
        y = 88 + (i // 2) * 40
        k.append(button(g, "Btn%d" % i, x, y, bw, 34, [
            image(g, "BtnBg%d" % i, 0, 0, bw, 34, (0.141, 0.157, 0.173, 1)),
            text(g, "BtnName%d" % i, 10, 5, bw - 70, 26, TEXT, "Gate", 19),
            text(g, "BtnState%d" % i, bw - 60, 5, 54, 26, DIM, "0/1", 18)]))
    k.append(text(g, "BarHint", 16, 252, W - 32, 22, AMBER, "", 16))
    fw = (W - 32 - 16) // 3
    for i, n in enumerate(["Undo", "Save", "Stop"]):
        k.append(button(g, "Bar" + n, 16 + i * (fw + 8), 282, fw, 34, [
            image(g, "Bar%sBg" % n, 0, 0, fw, 34, (0.141, 0.157, 0.173, 1)),
            text(g, "Bar%sText" % n, 12, 5, fw - 20, 26, TEXT, n, 19)]))
    panel = frame(g, "SiteHudPanel", -W - 24, 96, W, H, k, anchor="1 0 1 0")
    root = frame(g, "SiteHud", 0, 0, 0, 0, [panel], anchor="0 0 1 1")
    write("RZ_SiteHud.layout", root)


MARKS = 48


def marks():
    """The role spots' labels over their cones (UI/RZ_SiteBuilderMarks): a letter far away, the name
    up close (two lines when it's long). Each label's anchor point (where its cone is) is 160, 84."""
    g = G("RZ_SiteMarks")
    kids = []
    for i in range(MARKS):
        kids.append(frame(g, "Mark%d" % i, 0, 0, 320, 90, [
            image(g, "MarkNameBg%d" % i, 0, 0, 320, 50, (0.055, 0.059, 0.067, 0.85), False),
            text(g, "MarkName%d" % i, 8, 3, 304, 44, TEXT, "Gate", 17, visible=False, wrap=True),
            image(g, "MarkBadge%d" % i, 146, 56, 28, 28, AMBER),
            text(g, "MarkLetter%d" % i, 150, 58, 24, 24, (0.13, 0.11, 0.08, 1), "G", 17, BOLD)], visible=False))
    root = frame(g, "SiteMarks", 0, 0, 0, 0, kids, anchor="0 0 1 1")
    write("RZ_SiteMarks.layout", root)


if __name__ == "__main__":
    menu()
    hud()
    marks()
