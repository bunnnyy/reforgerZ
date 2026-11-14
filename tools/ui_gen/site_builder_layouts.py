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
ROWS = 12
SPOTS = 16


def menu():
    g = G("RZ_SiteBuilderMenu")
    W, H = 1640, 940
    px = 48
    kids = [image(g, "PanelBg", 0, 0, W, H, PANEL), image(g, "PanelAccent", 0, 0, W, 4, AMBER)]
    kids.append(text(g, "Title", px, 26, 700, 44, TEXT, "SITE BUILDER", 34, COND_BOLD))
    kids.append(text(g, "Sub", W - px - 760, 36, 760, 30, DIM, "Everon - 17 sites - 3 hand-built", 20))
    # The faction tabs, each a button: its name and an underline
    tx = px
    names = ["All", "Haven", "Bastion", "Red Hand", "Halcyon"]
    kids.append(text(g, "TabLeftHint", tx, 92, 40, 30, DIM, "LB", 18, BOLD))
    tx += 44
    for i, n in enumerate(names):
        w = 200
        kids.append(button(g, "Tab%d" % i, tx, 84, w, 48, [
            text(g, "TabText%d" % i, 10, 8, w - 20, 32, DIM, n, 24, COND),
            image(g, "TabLine%d" % i, 0, 44, w, 4, AMBER, False)]))
        tx += w + 8
    kids.append(text(g, "TabRightHint", tx + 4, 92, 40, 30, DIM, "RB", 18, BOLD))
    kids.append(image(g, "TabRule", px, 132, W - 2 * px, 2, (0.2, 0.216, 0.231, 1)))
    # The list pane
    lx, ly, lw, lh = px, 150, 820, 660
    list_kids = [image(g, "ListBg", 0, 0, lw, lh, PANE)]
    fx = 16
    for i, n in enumerate(["All", "Not built", "Hand-built", "Pinned"]):
        w = 150
        list_kids.append(button(g, "Filter%d" % i, fx, 12, w, 38, [
            image(g, "FilterBg%d" % i, 0, 0, w, 38, (0.165, 0.176, 0.192, 1)),
            text(g, "FilterText%d" % i, 12, 6, w - 24, 28, (0.812, 0.8, 0.769, 1), n, 20)]))
        fx += w + 8
    list_kids.append(text(g, "SortText", lw - 336, lh - 34, 320, 28, DIM, "LT RT filter", 18))
    ry = 62
    for i in range(ROWS):
        list_kids.append(button(g, "Row%d" % i, 12, ry, lw - 24, 46, [
            image(g, "RowBg%d" % i, 0, 0, lw - 24, 46, ROW, False),
            image(g, "RowMark%d" % i, 0, 0, 5, 46, AMBER, False),
            text(g, "RowName%d" % i, 18, 8, 470, 32, TEXT, "Le Moule stronghold", 24, COND),
            text(g, "RowKind%d" % i, 18 + 470, 12, 110, 26, DIM, "Haven", 18),
            image(g, "RowStateBg%d" % i, lw - 24 - 200, 9, 190, 28, (0.267, 0.267, 0.255, 1)),
            text(g, "RowState%d" % i, lw - 24 - 192, 11, 178, 26, TEXT, "AUTOMATED", 18, BOLD)]))
        ry += 48
    list_kids.append(text(g, "ListMore", 16, lh - 34, 300, 28, DIM, "", 18))
    list_kids.append(text(g, "ListEmpty", 24, 80, lw - 48, 60, DIM, "No site here.", 22, visible=False))
    kids.append(frame(g, "ListPane", lx, ly, lw, lh, list_kids))
    # The detail pane
    dx, dy, dw, dh = px + lw + 24, 150, W - 2 * px - lw - 24, 660
    d = [image(g, "DetailBg", 0, 0, dw, dh, (0.063, 0.07, 0.078, 1))]
    d.append(text(g, "DetailHead", 24, 20, dw - 48, 26, DIM, "THE HAVEN - STRONGHOLD", 18, BOLD))
    d.append(text(g, "DetailName", 24, 48, dw - 48, 50, TEXT, "Le Moule", 40, COND_BOLD))
    d.append(image(g, "DetailStateBg", 24, 104, 300, 32, (0.153, 0.314, 0.039, 1)))
    d.append(text(g, "DetailState", 34, 107, 290, 28, TEXT, "HAND-BUILT, IN USE", 20, BOLD))
    d.append(image(g, "DetailPinBg", 336, 104, 110, 32, (0.267, 0.267, 0.255, 1), False))
    d.append(text(g, "DetailPin", 348, 107, 96, 28, TEXT, "PINNED", 20, BOLD, False))
    ky = 152
    for i in range(3):
        d.append(text(g, "Key%d" % i, 24, ky, 180, 30, DIM, "Where", 22))
        d.append(text(g, "Val%d" % i, 210, ky, dw - 234, 30, TEXT, "26-54, 1.2 km NE of you", 22))
        ky += 34
    d.append(text(g, "SpotsHead", 24, ky + 14, dw - 48, 28, DIM, "ROLE SPOTS - 9 OF 11", 20, BOLD))
    sy = ky + 48
    half = (dw - 48) // 2
    for i in range(SPOTS):
        cx = 24 + (i % 2) * half
        cy = sy + (i // 2) * 30
        d.append(image(g, "SpotIcon%d" % i, cx, cy + 4, 22, 22, (0.592, 0.769, 0.349, 1)))
        d.append(text(g, "Spot%d" % i, cx + 30, cy, half - 38, 30, (0.592, 0.769, 0.349, 1), "Quartermaster", 22))
    d.append(frame(g, "SpotPicker", 24, sy + 250, dw - 48, 40, [
        image(g, "SpotPickerBg", 0, 0, dw - 48, 40, (0.165, 0.176, 0.192, 1)),
        text(g, "SpotPickerLabel", 12, 7, 150, 28, DIM, "Next spot", 20),
        button(g, "SpotPrev", 170, 0, 44, 40, [text(g, "SpotPrevText", 12, 0, 30, 38, AMBER, "<", 30, BOLD)]),
        text(g, "SpotPickerType", 222, 6, dw - 48 - 290, 30, TEXT, "Wash spot", 22, BOLD),
        button(g, "SpotNext", dw - 48 - 60, 0, 44, 40, [text(g, "SpotNextText", 14, 0, 30, 38, AMBER, ">", 30, BOLD)])], visible=False))
    d.append(text(g, "DetailNote", 24, dh - 62, dw - 48, 54, AMBER, "", 20, wrap=True))
    kids.append(frame(g, "DetailPane", dx, dy, dw, dh, d))
    # The unsaved-changes question, over the panes
    q = [image(g, "AskBg", 0, 0, 900, 260, (0.11, 0.122, 0.133, 1)), image(g, "AskAccent", 0, 0, 900, 4, AMBER),
         text(g, "AskTitle", 32, 28, 836, 40, TEXT, "Unsaved changes", 30, COND_BOLD),
         text(g, "AskText", 32, 80, 836, 150, TEXT, "", 22, wrap=True)]
    kids.append(frame(g, "Ask", (W - 900) // 2, 260, 900, 260, q, visible=False))
    # The button bar
    kids.append(image(g, "BarRule", px, H - 112, W - 2 * px, 2, (0.2, 0.216, 0.231, 1)))
    kids.append(hlayout(g, "Prompts", px, H - 92, W - 2 * px, 44))
    kids.append(text(g, "PromptsText", px, H - 86, W - 2 * px, 32, DIM, "A GO THERE    X BUILD    Y USE    B CLOSE", 20))
    kids.append(text(g, "Message", px, H - 40, W - 2 * px, 30, AMBER, "", 20))
    root = frame(g, "SiteBuilderMenu", 0, 0, 0, 0, [
        image(g, "Dim", 0, 0, 0, 0, (0.031, 0.035, 0.039, 0.65)),
        frame(g, "SitePanel", -W / 2, -H / 2, W, H, kids, anchor="0.5 0.5 0.5 0.5")], anchor="0 0 1 1")
    # The dim fills the screen
    for i, ln in enumerate(root):
        if ln.strip() == 'Name "Dim"':
            j = i + 1
            root[j + 1] = root[j + 1].replace("Anchor 0 0 0 0", "Anchor 0 0 1 1")
            break
    write("RZ_SiteBuilderMenu.layout", root)


# ------------------------------------------------------------------------------------ the HUD
def hud():
    g = G("RZ_SiteHud")
    W, H = 360, 330
    k = [image(g, "HudBg", 0, 0, W, H, (0.063, 0.071, 0.078, 0.9)), image(g, "HudAccent", 0, 0, 3, H, AMBER)]
    k.append(text(g, "HudHead", 16, 10, W - 28, 20, DIM, "BUILDING - BASTION STRONGHOLD", 14, BOLD))
    k.append(text(g, "HudName", 16, 30, W - 28, 28, TEXT, "Lamentin", 22, COND_BOLD))
    k.append(text(g, "HudInfo", 16, 58, W - 28, 20, DIM, "41 objects - unsaved changes", 15))
    k.append(text(g, "HudSpotsHead", 16, 84, W - 28, 20, DIM, "ROLE SPOTS - 7 OF 11", 14, BOLD))
    k.append(image(g, "HudBarBg", 16, 106, W - 32, 4, (0.2, 0.216, 0.231, 1)))
    k.append(image(g, "HudBarFill", 16, 106, 160, 4, (0.592, 0.769, 0.349, 1)))
    half = (W - 32) // 2
    for i in range(SPOTS):
        x = 16 + (i % 2) * half
        y = 116 + (i // 2) * 21
        k.append(image(g, "HudSpotIcon%d" % i, x, y + 3, 15, 15, (0.592, 0.769, 0.349, 1)))
        k.append(text(g, "HudSpot%d" % i, x + 20, y, half - 24, 20, (0.592, 0.769, 0.349, 1), "Quartermaster", 15))
    k.append(image(g, "HudTypeBg", 16, 290, W - 32, 28, (0.165, 0.176, 0.192, 1)))
    k.append(text(g, "HudType", 26, 294, W - 52, 22, TEXT, "< Wash spot >", 17, BOLD))
    panel = frame(g, "SiteHudPanel", -W - 24, 96, W, H, k, anchor="1 0 1 0")
    root = frame(g, "SiteHud", 0, 0, 0, 0, [panel], anchor="0 0 1 1")
    write("RZ_SiteHud.layout", root)


if __name__ == "__main__":
    menu()
    hud()
