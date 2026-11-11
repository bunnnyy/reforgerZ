#!/usr/bin/env node
// ReforgerZ UI renderer: PNG exports of the icons and previews of the mockups.
//
//   node tools/render_ui.js icons     # UI/Icons/png/{64,128,color/64,color/128} and UI/Previews/icons_sheet.png
//   node tools/render_ui.js screens   # every UI/Mockups/*.html at 1920x1080 (x2) into UI/Previews/<name>.png
//   node tools/render_ui.js all
//
// Needs Node and Playwright with its Chromium (require('playwright')). Nothing else.
// The icon sources are UI/Icons/src/*.svg (currentColor, 64x64). Mono exports are white,
// so the game can tint them; colour variants are for the map and the faction emblems.
'use strict';
const fs = require('fs');
const path = require('path');
const http = require('http');
const { chromium } = require('playwright');

const ROOT = path.resolve(__dirname, '..');
const UI = path.join(ROOT, 'UI');
const SRC = path.join(UI, 'Icons', 'src');
const PNG = path.join(UI, 'Icons', 'png');
const PREVIEWS = path.join(UI, 'Previews');

// One accent per faction, the Long Table's wood brown, plus hazard yellow for infection and the
// Perimeter (docs/UI.md).
const C = {
  haven: '#E3A33B', bastion: '#9AA35E', redhand: '#C8352B', halcyon: '#4FB8B0', longtable: '#B07A5A',
  hazard: '#F2C230', bone: '#E8E2D0', dark: '#16181B',
};
// Colour variants: [icon, variant suffix, colour]
const VARIANTS = [
  ['faction_haven', '', C.haven], ['faction_bastion', '', C.bastion], ['faction_redhand', '', C.redhand],
  ['faction_halcyon', '', C.halcyon], ['faction_infected', '', C.hazard], ['faction_longtable', '', C.longtable],
  ['marker_lab', '', C.halcyon], ['marker_safehouse', '', C.haven], ['marker_event', '', C.hazard],
  ['marker_power', '', C.hazard], ['marker_radio', '', C.bone], ['marker_checkpoint', '', C.hazard],
];
for (const m of ['marker_stronghold', 'marker_outpost', 'marker_quartermaster']) {
  for (const f of ['haven', 'bastion', 'redhand', 'halcyon']) VARIANTS.push([m, '_' + f, C[f]]);
}

function icons() {
  const index = JSON.parse(fs.readFileSync(path.join(UI, 'Icons', 'icons.json'), 'utf8'));
  return index.map((e) => Object.assign(e, { svg: fs.readFileSync(path.join(SRC, e.name + '.svg'), 'utf8') }));
}

function iconPage(svg, size, color, plated) {
  const under = plated ? `.under{fill:${C.dark};stroke:${C.dark};stroke-width:3}` : '';
  return `<!doctype html><html><head><style>html,body{margin:0;background:transparent}
    #i{width:${size}px;height:${size}px;color:${color}} #i svg{width:100%;height:100%;display:block} ${under}</style></head>
    <body><div id="i">${svg}</div></body></html>`;
}

async function renderIcons(browser) {
  const list = icons();
  const page = await browser.newPage({ deviceScaleFactor: 1 });
  for (const dir of ['64', '128', 'color/64', 'color/128']) fs.mkdirSync(path.join(PNG, dir), { recursive: true });
  for (const ic of list) {
    for (const size of [64, 128]) {
      await page.setContent(iconPage(ic.svg, size, '#FFFFFF', false));
      await page.locator('#i').screenshot({ path: path.join(PNG, String(size), ic.name + '.png'), omitBackground: true });
    }
  }
  for (const [name, suffix, color] of VARIANTS) {
    const ic = list.find((e) => e.name === name);
    for (const size of [64, 128]) {
      await page.setContent(iconPage(ic.svg, size, color, ic.plate));
      await page.locator('#i').screenshot({ path: path.join(PNG, 'color', String(size), name + suffix + '.png'), omitBackground: true });
    }
  }
  await page.close();
  console.log(`icons: ${list.length} x 2 sizes, ${VARIANTS.length} colour variants x 2 sizes`);
  await renderSheet(browser, list);
}

async function renderSheet(browser, list) {
  const cats = [];
  for (const ic of list) {
    let c = cats.find((x) => x.name === ic.category);
    if (!c) cats.push((c = { name: ic.category, items: [] }));
    c.items.push(ic);
  }
  const cell = (ic) => `<div class="cell"><div class="big">${ic.svg}</div><div class="row">
      <div class="s32">${ic.svg}</div><div class="s20">${ic.svg}</div><div class="lt s32">${ic.svg}</div></div>
      <div class="name">${ic.name}</div></div>`;
  const variantCells = VARIANTS.map(([name, suffix, color]) => {
    const ic = list.find((e) => e.name === name);
    return `<div class="vcell"><div class="v ${ic.plate ? 'plated' : ''}" style="color:${color}">${ic.svg}</div><div class="name">${name.replace('marker_', '').replace('faction_', '')}${suffix}</div></div>`;
  }).join('');
  const html = `<!doctype html><html><head><style>
    body{margin:0;background:#101214;color:#E8E2D0;font-family:Inter,sans-serif;width:1600px;padding:40px 48px;box-sizing:border-box}
    h1{font:800 34px 'Inter Display',Inter;letter-spacing:.12em;text-transform:uppercase;margin:0 0 4px}
    .sub{color:#9A968A;font:500 15px Inter;margin-bottom:26px}
    h2{font:700 15px 'JetBrains Mono';letter-spacing:.16em;text-transform:uppercase;color:#F2C230;margin:26px 0 12px;border-bottom:2px solid #2A2E31;padding-bottom:6px}
    .grid{display:flex;flex-wrap:wrap;gap:12px}
    .cell{width:140px;background:#181B1E;border:1px solid #262A2D;border-radius:4px;padding:12px 8px 8px;box-sizing:border-box;text-align:center}
    .big{width:64px;height:64px;margin:0 auto;color:#fff}
    .row{display:flex;justify-content:center;align-items:center;gap:8px;margin-top:10px}
    .s32{width:32px;height:32px;color:#E8E2D0}.s20{width:20px;height:20px;color:#E8E2D0}
    .lt{background:#D9D4C4;color:#1A1C1E;border-radius:3px;padding:3px}
    .lt.s32{width:26px;height:26px}
    svg{width:100%;height:100%;display:block}
    .name{font:500 11px 'JetBrains Mono';color:#B9B4A6;margin-top:8px;word-break:break-all}
    .vcell{width:104px;text-align:center}.v{width:64px;height:64px;margin:0 auto}
    .plated .under{fill:#16181B;stroke:#16181B;stroke-width:3}
    .vbg{background:linear-gradient(135deg,#3b4a3a,#4d5440 40%,#5d5a48 70%,#3c463f);padding:16px;border-radius:4px}
  </style></head><body>
    <h1>ReforgerZ icon set</h1>
    <div class="sub">${list.length} icons. 64 px white (as exported for tinting), then 32 px, 20 px and 32 px on light. Sources: UI/Icons/src (64x64, currentColor).</div>
    ${cats.map((c) => `<h2>${c.name} (${c.items.length})</h2><div class="grid">${c.items.map(cell).join('')}</div>`).join('')}
    <h2>Colour variants for the map and emblems (${VARIANTS.length})</h2>
    <div class="vbg"><div class="grid">${variantCells}</div></div>
  </body></html>`;
  const page = await browser.newPage({ deviceScaleFactor: 1, viewport: { width: 1600, height: 900 } });
  await page.setContent(html);
  fs.mkdirSync(PREVIEWS, { recursive: true });
  await page.screenshot({ path: path.join(PREVIEWS, 'icons_sheet.png'), fullPage: true });
  await page.close();
  console.log('sheet: UI/Previews/icons_sheet.png');
}

// A tiny static server for UI/, so the mockups can fetch the icon sources.
function serve() {
  const types = { '.html': 'text/html', '.css': 'text/css', '.js': 'text/javascript', '.svg': 'image/svg+xml', '.json': 'application/json', '.png': 'image/png' };
  const server = http.createServer((req, res) => {
    const p = path.normalize(path.join(UI, decodeURIComponent(req.url.split('?')[0])));
    if (!p.startsWith(UI) || !fs.existsSync(p) || fs.statSync(p).isDirectory()) { res.writeHead(404); res.end(); return; }
    res.writeHead(200, { 'Content-Type': types[path.extname(p)] || 'application/octet-stream' });
    fs.createReadStream(p).pipe(res);
  });
  return new Promise((resolve) => server.listen(0, '127.0.0.1', () => resolve(server)));
}

async function renderScreens(browser, only) {
  const server = await serve();
  const port = server.address().port;
  const files = fs.readdirSync(path.join(UI, 'Mockups')).filter((f) => f.endsWith('.html') && (!only || f.startsWith(only)));
  const page = await browser.newPage({ deviceScaleFactor: 2, viewport: { width: 1920, height: 1080 } });
  for (const f of files) {
    await page.goto(`http://127.0.0.1:${port}/Mockups/${f}`);
    await page.waitForFunction(() => window.RZ_READY === true, null, { timeout: 15000 });
    const out = path.join(PREVIEWS, f.replace('.html', '.png'));
    await page.screenshot({ path: out });
    console.log('screen:', path.relative(ROOT, out));
  }
  await page.close();
  server.close();
}

(async () => {
  const what = process.argv[2] || 'all';
  const browser = await chromium.launch();
  try {
    if (what === 'icons' || what === 'all') await renderIcons(browser);
    if (what === 'screens' || what === 'all') await renderScreens(browser, process.argv[3]);
  } finally {
    await browser.close();
  }
})().catch((e) => { console.error(e); process.exit(1); });
