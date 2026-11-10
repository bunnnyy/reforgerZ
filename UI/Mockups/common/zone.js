// ReforgerZ mockups: icon inlining and procedural backgrounds (no screenshots, no external assets).
// Pages put <canvas class="scene" data-scene="dusk|map"> first and <i class="ic" data-i="name"> for icons.
// window.RZ_READY turns true when icons, fonts and the scene are done (tools/render_ui.js waits for it).
(function () {
  'use strict';

  // ---------------------------------------------------------------- seeded random and value noise
  function rng(seed) {
    let s = seed >>> 0;
    return function () {
      s = (s + 0x6D2B79F5) >>> 0;
      let t = s;
      t = Math.imul(t ^ (t >>> 15), t | 1);
      t ^= t + Math.imul(t ^ (t >>> 7), t | 61);
      return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
    };
  }
  function noise2(seed) {
    const r = rng(seed), N = 256, g = new Float32Array(N * N);
    for (let i = 0; i < g.length; i++) g[i] = r();
    const sm = (t) => t * t * (3 - 2 * t);
    const at = (x, y) => g[((y & 255) * N) + (x & 255)];
    return function (x, y) {
      const xi = Math.floor(x), yi = Math.floor(y), xf = sm(x - xi), yf = sm(y - yi);
      const a = at(xi, yi), b = at(xi + 1, yi), c = at(xi, yi + 1), d = at(xi + 1, yi + 1);
      return a + (b - a) * xf + (c - a) * yf + (a - b - c + d) * xf * yf;
    };
  }
  function fbm(n, x, y, oct) {
    let v = 0, a = 0.5, f = 1;
    for (let i = 0; i < oct; i++) { v += a * n(x * f, y * f); f *= 2; a *= 0.5; }
    return v;
  }

  function setup(canvas) {
    const dpr = window.devicePixelRatio || 1;
    canvas.width = 1920 * dpr; canvas.height = 1080 * dpr;
    const ctx = canvas.getContext('2d');
    ctx.scale(dpr, dpr);
    return ctx;
  }

  // ---------------------------------------------------------------- dusk over the Zone
  function dusk(canvas) {
    const ctx = setup(canvas), W = 1920, H = 1080, r = rng(1411), n = noise2(77);
    const hz = 610; // horizon
    let g = ctx.createLinearGradient(0, 0, 0, hz);
    g.addColorStop(0, '#0a0f13'); g.addColorStop(0.55, '#16201f'); g.addColorStop(0.86, '#33372e'); g.addColorStop(1, '#4a4436');
    ctx.fillStyle = g; ctx.fillRect(0, 0, W, hz + 40);
    // low cloud streaks
    for (let i = 0; i < 9; i++) {
      const y = 120 + r() * 380, w = 500 + r() * 900, x = r() * W - 200;
      const a = 0.10 + r() * 0.12;
      ctx.save(); ctx.translate(x + w / 2, y); ctx.scale(w / 40, 1);
      const cg = ctx.createRadialGradient(0, 0, 0, 0, 0, 20);
      cg.addColorStop(0, 'rgba(72,76,68,' + a + ')'); cg.addColorStop(1, 'rgba(72,76,68,0)');
      ctx.fillStyle = cg; ctx.beginPath(); ctx.arc(0, 0, 20, 0, 7); ctx.fill(); ctx.restore();
    }
    // ridgelines, far to near
    const ridges = [
      { base: hz - 70, amp: 70, col: '#22292a', sc: 0.0018 },
      { base: hz - 30, amp: 55, col: '#1a2021', sc: 0.0026 },
      { base: hz + 10, amp: 40, col: '#131819', sc: 0.0035 },
    ];
    ridges.forEach((rd, k) => {
      ctx.fillStyle = rd.col; ctx.beginPath(); ctx.moveTo(0, H);
      for (let x = 0; x <= W; x += 6) ctx.lineTo(x, rd.base - fbm(n, x * rd.sc + k * 40, k * 9, 4) * rd.amp);
      ctx.lineTo(W, H); ctx.fill();
      // fog over each ridge
      const fg = ctx.createLinearGradient(0, rd.base - rd.amp, 0, rd.base + 40);
      fg.addColorStop(0, 'rgba(80,86,78,0)'); fg.addColorStop(1, 'rgba(80,86,78,' + (0.18 - k * 0.04) + ')');
      ctx.fillStyle = fg; ctx.fillRect(0, rd.base - rd.amp, W, rd.amp + 40);
    });
    // treeline on the near ridge
    const near = ridges[2];
    ctx.fillStyle = '#0c1011';
    for (let x = -10; x < W + 10; x += 7 + r() * 9) {
      if (x > 1180 && x < 1420) continue; // a gap for the village
      const y = near.base - fbm(n, x * near.sc + 80, 18, 4) * near.amp + 4;
      const h = 22 + r() * 46, w = 9 + r() * 9;
      ctx.beginPath(); ctx.moveTo(x, y - h); ctx.lineTo(x + w / 2, y + 2); ctx.lineTo(x - w / 2, y + 2); ctx.fill();
    }
    // the village: church spire, water tower, houses, two lit windows
    const vy = near.base - fbm(n, 1300 * near.sc + 80, 18, 4) * near.amp + 6;
    ctx.fillStyle = '#0b0e0f';
    [[1190, 34, 26], [1226, 44, 30], [1262, 30, 22], [1340, 38, 28], [1380, 26, 24]].forEach(([x, w, h]) => {
      ctx.fillRect(x, vy - h, w, h + 6);
      ctx.beginPath(); ctx.moveTo(x - 3, vy - h); ctx.lineTo(x + w / 2, vy - h - 14); ctx.lineTo(x + w + 3, vy - h); ctx.fill();
    });
    ctx.fillRect(1296, vy - 64, 22, 70); // church tower
    ctx.beginPath(); ctx.moveTo(1292, vy - 64); ctx.lineTo(1307, vy - 120); ctx.lineTo(1322, vy - 64); ctx.fill();
    ctx.fillRect(1306, vy - 132, 2, 14); ctx.fillRect(1301, vy - 127, 12, 2);
    ctx.fillRect(1430, vy - 96, 3, 100); ctx.fillRect(1462, vy - 96, 3, 100); // water tower legs
    ctx.beginPath(); ctx.ellipse(1447, vy - 104, 30, 16, 0, 0, Math.PI * 2); ctx.fill();
    ctx.fillStyle = 'rgba(227,163,59,.85)'; ctx.fillRect(1236, vy - 18, 5, 6); ctx.fillRect(1352, vy - 22, 4, 5);
    ctx.fillStyle = 'rgba(227,163,59,.15)'; ctx.beginPath(); ctx.arc(1238, vy - 15, 14, 0, 7); ctx.fill();
    // fields and the road
    g = ctx.createLinearGradient(0, hz, 0, H);
    g.addColorStop(0, '#11150f'); g.addColorStop(1, '#070908');
    ctx.fillStyle = g; ctx.beginPath(); ctx.moveTo(0, H);
    for (let x = 0; x <= W; x += 8) ctx.lineTo(x, hz + 24 + fbm(n, x * 0.004, 50, 3) * 16);
    ctx.lineTo(W, H); ctx.fill();
    ctx.fillStyle = '#1d1f1a'; ctx.beginPath();
    ctx.moveTo(760, H); ctx.bezierCurveTo(900, 860, 1150, 720, 1236, hz + 32); ctx.lineTo(1246, hz + 32);
    ctx.bezierCurveTo(1210, 740, 1120, 880, 1240, H); ctx.fill();
    // furrows
    ctx.strokeStyle = 'rgba(40,44,34,.55)'; ctx.lineWidth = 1;
    for (let i = 0; i < 40; i++) {
      const y = hz + 40 + Math.pow(i / 40, 1.8) * (H - hz - 40);
      ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(720 - i * 3, y + 6); ctx.stroke();
    }
    // telegraph poles and wires
    const poles = [[1620, 470, 9], [1420, 560, 5], [1300, 598, 3]];
    ctx.strokeStyle = '#06080a'; ctx.fillStyle = '#06080a';
    poles.forEach(([x, top, w]) => {
      ctx.fillRect(x, top, w, H - top); ctx.fillRect(x - w * 3, top + w * 2, w * 7, w * 0.8);
    });
    ctx.lineWidth = 1.5;
    for (let k = -1; k <= 1; k += 2) {
      ctx.beginPath();
      for (let i = 0; i < poles.length - 1; i++) {
        const [x0, t0, w0] = poles[i], [x1, t1, w1] = poles[i + 1];
        const ax = x0 + w0 / 2 + k * w0 * 3, ay = t0 + w0 * 2, bx = x1 + w1 / 2 + k * w1 * 3, by = t1 + w1 * 2;
        ctx.moveTo(ax, ay); ctx.quadraticCurveTo((ax + bx) / 2, Math.max(ay, by) + 22, bx, by);
      }
      ctx.lineTo(1920, 420); ctx.stroke();
    }
    // ground fog
    for (let i = 0; i < 6; i++) {
      const y = hz + 10 + i * 34;
      const fg = ctx.createLinearGradient(0, y - 30, 0, y + 30);
      fg.addColorStop(0, 'rgba(90,96,88,0)'); fg.addColorStop(0.5, 'rgba(90,96,88,' + (0.09 - i * 0.012) + ')'); fg.addColorStop(1, 'rgba(90,96,88,0)');
      ctx.fillStyle = fg; ctx.fillRect(0, y - 30, W, 60);
    }
    finish(ctx, r);
  }

  // ---------------------------------------------------------------- a paper-like map of an island
  function map(canvas) {
    const ctx = setup(canvas), W = 1920, H = 1080, r = rng(2024), n = noise2(311);
    const lw = 960, lh = 540, off = document.createElement('canvas');
    off.width = lw; off.height = lh;
    const o = off.getContext('2d'), img = o.createImageData(lw, lh), hs = new Float32Array(lw * lh);
    const land = (x, y) => {
      const dx = (x - 0.46) / 0.5, dy = (y - 0.52) / 0.56;
      return fbm(n, x * 5, y * 5, 5) * 1.25 - Math.sqrt(dx * dx + dy * dy) * 0.72;
    };
    for (let j = 0; j < lh; j++) for (let i = 0; i < lw; i++) {
      const x = i / lw, y = j / lh, h = land(x, y), k = (j * lw + i) * 4;
      hs[j * lw + i] = h;
      let c;
      if (h < 0.06) { const d = Math.max(0, Math.min(1, (0.06 - h) * 4)); c = [52 - d * 18, 70 - d * 20, 74 - d * 14]; }
      else if (h < 0.09) c = [150, 146, 118];
      else { const t = Math.min(1, (h - 0.09) * 2.4), f = fbm(n, x * 30 + 9, y * 30, 2); c = [96 - t * 30 + f * 12, 108 - t * 22 + f * 10, 78 - t * 18]; }
      img.data[k] = c[0]; img.data[k + 1] = c[1]; img.data[k + 2] = c[2]; img.data[k + 3] = 255;
    }
    o.putImageData(img, 0, 0);
    ctx.imageSmoothingEnabled = true; ctx.drawImage(off, 0, 0, W, H);
    // forests
    ctx.fillStyle = 'rgba(48,64,40,.55)';
    for (let i = 0; i < 2600; i++) {
      const x = r(), y = r();
      if (land(x, y) > 0.2 && fbm(n, x * 9 + 50, y * 9, 3) > 0.52) { ctx.beginPath(); ctx.arc(x * W, y * H, 2 + r() * 3, 0, 7); ctx.fill(); }
    }
    // contours
    ctx.fillStyle = 'rgba(40,36,24,.22)';
    for (let lev = 0.18; lev < 0.6; lev += 0.07) {
      for (let j = 0; j < lh - 1; j += 1) for (let i = 0; i < lw - 1; i += 1) {
        const a = hs[j * lw + i] > lev, b = hs[j * lw + i + 1] > lev, c2 = hs[(j + 1) * lw + i] > lev;
        if (a !== b || a !== c2) ctx.fillRect(i * 2, j * 2, 1.4, 1.4);
      }
    }
    // roads
    ctx.lineCap = 'round';
    const roads = [
      [[300, 820], [520, 700], [760, 640], [980, 560]], [[980, 560], [1160, 470], [1330, 430], [1500, 300]],
      [[980, 560], [1010, 700], [1080, 820], [1230, 880]], [[760, 640], [700, 480], [640, 360], [560, 250]],
      [[1330, 430], [1420, 560], [1520, 640], [1600, 760]], [[560, 250], [760, 230], [960, 280], [1160, 470]],
    ];
    [['rgba(30,26,20,.7)', 7], ['#D8CBA4', 4]].forEach(([col, w]) => {
      ctx.strokeStyle = col; ctx.lineWidth = w;
      roads.forEach((p) => { ctx.beginPath(); ctx.moveTo(p[0][0], p[0][1]); ctx.bezierCurveTo(p[1][0], p[1][1], p[2][0], p[2][1], p[3][0], p[3][1]); ctx.stroke(); });
    });
    // towns
    const towns = window.RZ_TOWNS || [];
    towns.forEach((t) => {
      ctx.fillStyle = 'rgba(70,64,56,.85)';
      for (let i = 0; i < t.size * 9; i++) {
        const a = r() * Math.PI * 2, d = r() * t.size * 13;
        ctx.save(); ctx.translate(t.x + Math.cos(a) * d, t.y + Math.sin(a) * d); ctx.rotate(r() * 0.6);
        ctx.fillRect(-3, -2.5, 6 + r() * 4, 5 + r() * 3); ctx.restore();
      }
      ctx.font = '700 ' + (window.RZ_TOWN_FONT || 15) + 'px Inter'; ctx.lineWidth = 4; ctx.strokeStyle = 'rgba(214,206,182,.85)'; ctx.fillStyle = 'rgba(24,22,18,.95)';
      ctx.strokeText(t.name.toUpperCase(), t.x + (t.lx || 18), t.y + (t.ly || -16));
      ctx.fillText(t.name.toUpperCase(), t.x + (t.lx || 18), t.y + (t.ly || -16));
    });
    // grid
    ctx.strokeStyle = 'rgba(30,30,24,.22)'; ctx.lineWidth = 1; ctx.font = '500 12px JetBrains Mono'; ctx.fillStyle = 'rgba(30,30,24,.55)';
    for (let x = 0, k = 0; x <= W; x += 160, k++) { ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, H); ctx.stroke(); ctx.fillText(String(k).padStart(2, '0'), x + 4, 14); }
    for (let y = 0, k = 0; y <= H; y += 160, k++) { ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(W, y); ctx.stroke(); ctx.fillText(String(k).padStart(2, '0'), 4, y + 14); }
    // the Perimeter: hatched band where land meets the map edge
    ctx.strokeStyle = 'rgba(242,194,48,.55)'; ctx.setLineDash([14, 10]); ctx.lineWidth = 3;
    ctx.strokeRect(70, 60, W - 140, H - 120); ctx.setLineDash([]);
    finish(ctx, r, 0.03);
  }

  function finish(ctx, r, grain) {
    const W = 1920, H = 1080;
    const v = ctx.createRadialGradient(W / 2, H * 0.45, H * 0.35, W / 2, H * 0.5, H * 1.05);
    v.addColorStop(0, 'rgba(0,0,0,0)'); v.addColorStop(1, 'rgba(0,0,0,.55)');
    ctx.fillStyle = v; ctx.fillRect(0, 0, W, H);
    const gr = grain === undefined ? 0.05 : grain;
    for (let i = 0; i < 26000; i++) {
      ctx.fillStyle = r() < 0.5 ? 'rgba(255,255,255,' + gr + ')' : 'rgba(0,0,0,' + gr * 1.6 + ')';
      ctx.fillRect(r() * W, r() * H, 1.2, 1.2);
    }
  }

  // ---------------------------------------------------------------- icons
  async function icons() {
    const els = Array.from(document.querySelectorAll('[data-i]'));
    const names = Array.from(new Set(els.map((e) => e.dataset.i)));
    const svgs = {};
    await Promise.all(names.map(async (nm) => {
      const res = await fetch('../Icons/src/' + nm + '.svg');
      if (!res.ok) throw new Error('missing icon ' + nm);
      svgs[nm] = (await res.text()).replace(/<title>.*?<\/title>/, '');
    }));
    els.forEach((e) => { e.innerHTML = svgs[e.dataset.i]; e.classList.add('ic'); });
  }

  window.addEventListener('DOMContentLoaded', async () => {
    document.querySelectorAll('canvas.scene').forEach((c) => (c.dataset.scene === 'map' ? map(c) : dusk(c)));
    await icons();
    await document.fonts.ready;
    window.RZ_READY = true;
  });
})();
