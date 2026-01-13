/*
  Global Theme Definition: "Inverse Wheat"
  A comprehensive color system for the entire application.
*/
( function(){
  const RT = window.StyleRT = window.StyleRT || {};
  
  RT.theme = function(){
    RT.config = RT.config || {};
    
    // THE PALETTE DEFINITION
    RT.config.theme = {
       is_dark: true
       
      // --- BACKGROUNDS ---
      ,bg_app:      "hsl(0, 0%, 8%)"      // Main Window Background (Deep Charcoal)
      ,bg_panel:    "hsl(0, 0%, 12%)"     // Sidebars / Cards (Slightly lighter)
      ,bg_code:     "hsl(0, 0%, 14%)"     // Code blocks / Terminals
      ,bg_input:    "hsl(0, 0%, 16%)"     // Form inputs
      ,bg_select:   "hsl(45, 100%, 15%)"  // Text Selection Background (Dim Amber)

      // --- TYPOGRAPHY ---
      ,text_main:   "hsl(36, 30%, 85%)"   // Primary Content (Pale Wheat)
      ,text_muted:  "hsl(36, 15%, 60%)"   // Metadata / Footer / Subtitles
      ,text_faint:  "hsl(36, 10%, 40%)"   // Placeholders / Disabled text
      ,text_inv:    "hsl(0, 0%, 10%)"     // Inverted text (for buttons/highlights)

      // --- ACCENTS (The "Wheat/Amber" Spectrum) ---
      ,primary:     "hsl(45, 100%, 50%)"  // P3 Amber (Active states, H1, Key focus)
      ,secondary:   "hsl(38, 90%, 65%)"   // Warm Gold (H2, distinct UI elements)
      ,tertiary:    "hsl(30, 60%, 70%)"   // Bronze (H3, subtle highlights)
      ,link:        "hsl(48, 100%, 50%)"  // High-Vis Yellow (Links)
      ,link_hover:  "hsl(48, 100%, 70%)"  // Bright Yellow

      // --- UI & BORDERS ---
      ,border_main: "hsl(36, 20%, 25%)"   // Standard dividers
      ,border_light:"hsl(36, 20%, 35%)"   // Hover borders
      ,shadow:      "0 4px 6px rgba(0,0,0, 0.5)" 

      // --- STATUS / SYNTAX (Shifted to Earth Tones) ---
      // These are tuned to look good on the dark background without looking neon.
      ,success:     "hsl(100, 50%, 55%)"  // Muted Olive Green
      ,warning:     "hsl(35, 90%, 60%)"   // Burnt Orange
      ,error:       "hsl(0, 60%, 65%)"    // Brick Red
      ,info:        "hsl(200, 40%, 60%)"  // Slate Blue (for contrast)
      ,string:      "hsl(70, 40%, 65%)"   // Sage (for code strings)
      ,keyword:     "hsl(35, 100%, 65%)"  // Orange (for code keywords)
    };

    const palette = RT.config.theme;
    const body = document.body;
    const html = document.documentElement;

    // 1. Basic Reset
    html.style.margin = "0"; html.style.padding = "0";
    body.style.margin = "0"; body.style.padding = "0";
    body.style.minHeight = "100vh"; 

    // 2. Apply Base Colors
    html.style.backgroundColor = palette.bg_app;
    body.style.backgroundColor = palette.bg_app;
    body.style.color = palette.text_main;

    // 3. Export CSS Variables
    // This loop automatically makes every key above available as var(--rt-key)
    const s = body.style;
    for (const [key, value] of Object.entries(palette)) {
      // e.g. converts "bg_app" to "--rt-bg-app"
      s.setProperty(`--rt-${key.replace('_', '-')}`, value);
    }
    
    // 4. Global Selection Style
    // This ensures even standard text highlighting matches the theme
    const style_id = 'rt-global-overrides';
    if (!document.getElementById(style_id)) {
      const style = document.createElement('style');
      style.id = style_id;
      style.textContent = `
        ::selection { background: var(--rt-bg-select); color: var(--rt-primary); }
        ::-moz-selection { background: var(--rt-bg-select); color: var(--rt-primary); }
        
        /* Scrollbar Styling (Webkit) */
        ::-webkit-scrollbar { width: 10px; }
        ::-webkit-scrollbar-track { background: var(--rt-bg-app); }
        ::-webkit-scrollbar-thumb { background: var(--rt-border-main); border-radius: 5px; }
        ::-webkit-scrollbar-thumb:hover { background: var(--rt-secondary); }
      `;
      document.head.appendChild(style);
    }
  };
} )();
